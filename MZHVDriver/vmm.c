/**
 * @file vmm.c
 * @brief VMM setup and teardown
 * 
 * This file implements setup and teardown initialization procedures. It is responsible for
 * checking prerequisites and interfacing with driver.
 */


#include "context.h"
#include "ept.h"
#include "ia32.h"
#include <intrin.h>
#include "memory.h"
#include "asmproc.h"
#include "vmcs.h"
#include "vmexit.h"
#include "vmm.h"
#include "vmx.h"
#include "vmxon.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static KIPI_BROADCAST_WORKER virtualizeLogicalCore;


static NTSTATUS checkPrerequisites(VOID);


static BOOLEAN isVmxCpuidSupported(VOID);


static NTSTATUS setControlRegisterBits(VOID);


static KIPI_BROADCAST_WORKER restoreLogicalCore;


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Begins virtualization process
 * 
 * Encloses all steps required to virtualize system. It delegates creating
 * EPT structures and stating virtualization process on each logical core.
 * 
 * @return STATUS_SUCCESS if system was virtualized successfully, an error code otherwise
 */
NTSTATUS VMM_enable(VOID)
{
  Context_Context* const context = Context_getContext();
  context->systemCR3 = (IA32_Cr3){ .bits = __readcr3() };

  NTSTATUS status;
  for (UINT64 logicalCoreIndex = 0;
    logicalCoreIndex < context->noOfLogicalCores;
    logicalCoreIndex++)
  {
    Context_LogicalCore* const logicalCore = &context->logicalCores[logicalCoreIndex];

    status = EPT_setupDefaltStructures(&logicalCore->eptp);
    if (!NT_SUCCESS(status))
    {
      VMM_disable();
      return status;
    }
  }

  status = (NTSTATUS)KeIpiGenericCall(virtualizeLogicalCore, 0);

  for (UINT64 logicalCoreIndex = 0; logicalCoreIndex < context->noOfLogicalCores; logicalCoreIndex++)
  {
    const Context_LogicalCore* const logicalCore = &context->logicalCores[logicalCoreIndex];
    if (!logicalCore->isVirtualized)
    {
      VMM_disable();
      return status;
    }
  }

  return STATUS_SUCCESS;
}


/**
 * @brief Devirtualizes system
 * 
 * Encloses all steps required to devirtualize system. It delegates starting
 * devirtualization process on each logical core and destroying EPT structures.
 * 
 * @return VOID
 */
VOID VMM_disable(VOID)
{
  KeIpiGenericCall(restoreLogicalCore, 0);

  Context_Context* const context = Context_getContext();
  for (UINT64 logicalCoreIndex = 0;
    logicalCoreIndex < context->noOfLogicalCores;
    logicalCoreIndex++)
  {
    Context_LogicalCore* const logicalCore = &context->logicalCores[logicalCoreIndex];
    if (logicalCore->eptp != 0)
    {
      EPT_destroyEPTStructure(logicalCore->eptp);
    }
  }
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Virtualizes a single logical core
 * 
 * Is called on each logical core. It checks prerequisites, enables VMX,
 * sets up VMCS region, and enters VMCS.
 *
 * @param argument Unused
 * 
 * @return STATUS_SUCCESS if virtualization was successful, an error code otherwise
 */
_Use_decl_annotations_ 
static ULONG_PTR virtualizeLogicalCore(ULONG_PTR argument)
{
  UNREFERENCED_PARAMETER(argument);
  NTSTATUS ntStatus = checkPrerequisites();
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }

  ntStatus = setControlRegisterBits();
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }

  ntStatus = VMXON_setup();
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }

  ntStatus = VMCS_setup();
  if (!NT_SUCCESS(ntStatus))
  {
    __vmx_off();
    return ntStatus;
  }

  ntStatus = ASMPROC_enterVmcs();
  if (!NT_SUCCESS(ntStatus))
  {
    __vmx_off();
    return ntStatus;
  }

  Context_getLogicalCore()->isVirtualized = TRUE;
  return STATUS_SUCCESS;
}


/**
 * @brief Check if system supports virtualization
 * 
 * Checks whether system supports all the features required by this hypervisor.
 * 
 * @return STATUS_SUCCESS if system supports all the features, an error code otherwise
 */
static NTSTATUS checkPrerequisites(VOID)
{
  if (!isVmxCpuidSupported())
  {
    return STATUS_UNSUCCESSFUL;
  }

  const IA32_VmxBasic vmxBasic = { .bits = __readmsr(IA32_VMX_BASIC) };
  if (!vmxBasic.trueControls)
  {
    return STATUS_UNSUCCESSFUL;
  }

  return STATUS_SUCCESS;
}


/**
 * @brief Check if VMX is supported in CPUID
 * 
 * Checks whether this CPU supports VMX by checking CPUID function 0x1.
 * 
 * @return TRUE if VMX is supported, FALSE otherwise
 */
static BOOLEAN isVmxCpuidSupported(VOID)
{
  IA32_CpuidBasicInformation0 basicInfo0 = { 0 };
  __cpuid(basicInfo0.registers, IA32_CPUID_BASIC_INFORMATION_0);

  CHAR vendorString[3 * sizeof(UINT32) + 1] = { 0 };
  Memory_copy(vendorString, &basicInfo0.vendor1, sizeof(INT32));
  Memory_copy(vendorString + sizeof(INT32), &basicInfo0.vendor2, sizeof(INT32));
  Memory_copy(vendorString + 2 * sizeof(INT32), &basicInfo0.vendor3, sizeof(INT32));

  if (!Memory_compare(vendorString, "GenuineIntel", sizeof(vendorString)))
  {
    return FALSE;
  }

  IA32_CpuidBasicInformation1 basicInfo1 = { 0 };
  __cpuid(basicInfo1.registers, IA32_CPUID_BASIC_INFORMATION_1);
  return basicInfo1.vmx != 0;
}


/**
 * @brief Locks bits in control registers and feature control MSR
  * 
  * Checks if bits in mentioned registers are locked to correct values.
  * If bits are locked and values are not correct, this function returns an error code.
  * If bits are not locked, the function locks them to the correct values.
  * 
  * @return STATUS_SUCCESS if bits have correct values, an error code otherwise
  */
static NTSTATUS setControlRegisterBits(VOID)
{
  IA32_FeatureControl featureControl = { .bits = __readmsr(IA32_FEATURE_CONTROL) };
  if (featureControl.lockBit && !featureControl.enableVmxOutsideSmx)
  {
    return STATUS_UNSUCCESSFUL;
  }

  if (!featureControl.lockBit)
  {
    featureControl.enableVmxOutsideSmx = TRUE;
    featureControl.lockBit = TRUE;
    __writemsr(IA32_FEATURE_CONTROL, featureControl.bits);
  }

  const UINT64 cr4Fixed0 = __readmsr(IA32_VMX_CR4_FIXED0);
  const UINT64 cr4Fixed1 = __readmsr(IA32_VMX_CR4_FIXED1);
  IA32_Cr4 cr4 = { .bits = __readcr4() };
  cr4.vmxEnableBit = TRUE;
  cr4.bits |= cr4Fixed0;
  cr4.bits &= cr4Fixed1;
  __writecr4(cr4.bits);

  const UINT64 cr0Fixed0 = __readmsr(IA32_VMX_CR0_FIXED0);
  const UINT64 cr0Fixed1 = __readmsr(IA32_VMX_CR0_FIXED1);
  IA32_Cr0 cr0 = { .bits = __readcr0() };
  cr0.bits |= cr0Fixed0;
  cr0.bits &= cr0Fixed1;
  __writecr0(cr0.bits);

  return STATUS_SUCCESS;
}


/**
 * @brief Devirtualizes a single logical core
 * 
 * Is called on each logical core. It devirtualizes logical core by calling
 * vmcall with VMEXIT_VMCALL_INITIATE_SHUTDOWN as first argument.
 * 
 * @param argument Unused
 * 
 * @return STATUS_SUCCESS
 */
_Use_decl_annotations_
static ULONG_PTR restoreLogicalCore(ULONG_PTR argument)
{
  UNREFERENCED_PARAMETER(argument);

  Context_LogicalCore* const thisLogicalCore = Context_getLogicalCore();
  if (thisLogicalCore->isVirtualized)
  {
    VMX_vmcall(VMEXIT_VMCALL_INITIATE_SHUTDOWN, 0, 0, 0);
    thisLogicalCore->isVirtualized = FALSE;
  }

  return (NTSTATUS)STATUS_SUCCESS;
}
