/**
 * @file vmxon.c
 * @brief VMXON region setup.
 *
 * This file implements VMXON region setup functionality.
 */


#include "context.h"
#include "ia32.h"
#include "memory.h"
#include "vmxon.h"


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Sets up the VMXON region for the current logical core.
 *
 * This function writes the revision identifier to current logical core's VMXON region.
 * It then switches this core into VMX root operation.
 *
 * @return STATUS_SUCCESS if logical core was successfully switched, STATUS_UNSUCCESSFUL otherwise.
 */
NTSTATUS VMXON_setup(void)
{
  Context_LogicalCore* thisCore = Context_getLogicalCore();

  IA32_VmxBasic vmxBasic = { .bits = __readmsr(IA32_VMX_BASIC) };
  UINT64 revisionIdentifier = vmxBasic.revisionIdentifier;
  Memory_copy(thisCore->vmxonRegion, &revisionIdentifier, sizeof(UINT64));

  UINT64 vmxonPhysicalAddress = Memory_getPhysicalAddress(thisCore->vmxonRegion);
  if (__vmx_on(&vmxonPhysicalAddress) != 0)
  {
    return STATUS_UNSUCCESSFUL;
  }

  return STATUS_SUCCESS;
}
