/**
 * @file vmexit.c
 * @brief VM exit handlers
 *
 * This file contains VM exit handlers for CPUID, VMCALL, and EPT violations.
 */


#include "bsod.h"
#include "context.h"
#include "ept.h"
#include "vmx.h"
#include "vmcs.h"
#include "vmexit.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static VOID cpuidHandler(VMEXIT_Registers* const registers);


static VOID vmCallHandler(VMEXIT_Registers* const registers, BOOLEAN* const initiateShutdown);


static VOID eptViolationHandler(VOID);


static VOID vmCallMapPage(VMEXIT_Registers* const registers);


static VOID vmCallUnmapPage(VMEXIT_Registers* const registers);


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Handles a single VM exit
 *
 * Delegates VM exits to appropriate handlers. It also handles incrementing
 * guest RIP and loading guest RSP into registers structure. It is called from assembly.
 * This function will bugcheck if exit reason is not CPUID, VMCALL, or EPT violation.
 *
 * @param registers Guest general purpose registers, provided by assembly
 *
 * @return TRUE if VM should be shutdown, FALSE otherwise
 */
BOOLEAN VMEXIT_handler(VMEXIT_Registers* const registers)
{
  __vmx_vmread(VMCS_GUEST_RSP, &registers->RSP);

  BOOLEAN initiateShutdown = FALSE;
  BOOLEAN incrementRIP = TRUE;

  VMEXIT_ExitReason exitReason = { 0 };
  __vmx_vmread(VMCS_EXIT_REASON, &exitReason.bits);

  switch (exitReason.basicExitReason)
  {
  case VMEXIT_CPUID:
  {
    cpuidHandler(registers);
    break;
  }
  case VMEXIT_VMCALL:
  {
    vmCallHandler(registers, &initiateShutdown);
    break;
  }
  case VMEXIT_EPT_VIOLATION:
  {
    eptViolationHandler();
    incrementRIP = FALSE;
    break;
  }
  default:
  {
    KeBugCheck(BSOD_VMEXIT_UNKNOWN);
  }
  }

  if (incrementRIP)
  {
    const UINT64 instructionLength = { 0 };
    const UINT64 rip = { 0 };
    __vmx_vmread(VMCS_VM_EXIT_INSTRUCTION_LENGTH, &instructionLength);
    __vmx_vmread(VMCS_GUEST_RIP, &rip);
    __vmx_vmwrite(VMCS_GUEST_RIP, rip + instructionLength);
  }

  __vmx_vmwrite(VMCS_GUEST_RSP, registers->RSP);

  return initiateShutdown;
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Handles CPUID VM exit
 *
 * Handles CPUID VM exits by executing CPUID instruction and returning
 * results. There are two CPUID functions that are modified:
 *  - IA32_CPUID_BASIC_INFORMATION_0: Vendor string is modified to "AvocadoIntel"
 *  - IA32_CPUID_BASIC_INFORMATION_1: Hypervisor present bit is set to TRUE
 *
 * @param registers Guest registers
 *
 * @return VOID
 */
static VOID cpuidHandler(VMEXIT_Registers* const registers)
{
  const INT32 cpuidFunction = (INT32)registers->RAX;
  const INT32 cpuidSubleaf = (INT32)registers->RCX;

  INT32 cpuid[4] = { 0 };
  __cpuidex(cpuid, cpuidFunction, cpuidSubleaf);

  if (cpuidFunction == IA32_CPUID_BASIC_INFORMATION_0)
  {
    IA32_CpuidBasicInformation0* const basicInformation0 = (IA32_CpuidBasicInformation0*)cpuid;
    basicInformation0->vendor1 = 'covA';
    basicInformation0->vendor2 = 'Ioda';
    basicInformation0->vendor3 = 'letn';
  }

  if (cpuidFunction == IA32_CPUID_BASIC_INFORMATION_1)
  {
    IA32_CpuidBasicInformation1* const basicInformation1 = (IA32_CpuidBasicInformation1*)cpuid;
    basicInformation1->hypervisorPresentBit = TRUE;
  }

  registers->RAX = cpuid[0];
  registers->RBX = cpuid[1];
  registers->RCX = cpuid[2];
  registers->RDX = cpuid[3];
}


/**
 * @brief Handles VMCALL VM exit
 *
 * Handles VMCALLs. Currently, there are three VMCALLs:
 *  - VMEXIT_VMCALL_INITIATE_SHUTDOWN: Initiates a shutdown of the VM
 *  - VMEXIT_VMCALL_MAP_PAGE: Changes EPT mapping
 *  - VMEXIT_VMCALL_UNMAP_PAGE: Removes EPT mapping change
 *
 * @param registers Guest registers
 * @param initiateShutdown Pointer to a BOOLEAN that is set to TRUE on shutdown
 *
 * @return VOID
 */
static VOID vmCallHandler(VMEXIT_Registers* const registers, BOOLEAN* const initiateShutdown)
{
  const UINT64 vmCallCode = registers->RCX;
  switch (vmCallCode)
  {
  case VMEXIT_VMCALL_INITIATE_SHUTDOWN:
  {
    registers->RAX = STATUS_SUCCESS;
    *initiateShutdown = TRUE;
    break;
  }
  case VMEXIT_VMCALL_MAP_PAGE:
  {
    vmCallMapPage(registers);
    break;
  }
  case VMEXIT_VMCALL_UNMAP_PAGE:
  {
    vmCallUnmapPage(registers);
    break;
  }
  default:
  {
    break;
  }
  }
}


/**
 * @brief Handles EPT violations
 *
 * Handles EPT violations by swapping mapping to the one previously stored
 * int the Context_EptChangedMapping structure. Function will bugcheck if mapping is not
 * found or if the violation is not a data read, data write, or instruction fetch.
 *
 * @return VOID
 */
static VOID eptViolationHandler(VOID)
{
  VMEXIT_EptViolation eptViolation = { 0 };
  __vmx_vmread(VMCS_EXIT_QUALIFICATION, &eptViolation.bits);

  EPT_Address exitAddr = { 0 };
  __vmx_vmread(VMCS_GUEST_PHYSICAL_ADDRESS_FULL, &exitAddr.address);
  exitAddr.offset = 0;

  const Context_LogicalCore* const thisCore = Context_getLogicalCore();

  const Context_EptChangedMapping* foundMapping = NULL;
  for (UINT64 mappingIndex = 0; mappingIndex < CONTEXT_EPT_MAX_MAPPINGS; mappingIndex++)
  {
    const Context_EptChangedMapping* const currentMapping =
      &thisCore->eptMappingData.changedMappings[mappingIndex];
    if (currentMapping->valid && currentMapping->guestAddress == exitAddr.address)
    {
      foundMapping = currentMapping;
      break;
    }
  }

  if (foundMapping == NULL)
  {
    KeBugCheck(BSOD_VMEXIT_EPT_NO_MAPPING);
  }

  if (eptViolation.dataRead || eptViolation.dataWrite)
  {
    EPT_changeMapping(foundMapping->guestAddress, foundMapping->hostRwAddress, TRUE, FALSE);
    VMX_inveptAll();
    return;
  }

  if (eptViolation.instructionFetch)
  {
    EPT_changeMapping(foundMapping->guestAddress, foundMapping->hostFetchAddress, FALSE, TRUE);
    VMX_inveptAll();
    return;
  }

  KeBugCheck(BSOD_VMEXIT_EPT_UNKNOWN);
}


/**
 * @brief Handles VMEXIT_VMCALL_MAP_PAGE
 *
 * Handles VMEXIT_VMCALL_MAP_PAGE VMCALL. It receives mapping in RDX, R8, and R9.
 * Mapping is stored in Context_EptChangedMapping structure, and guest access to
 * page is disabled. Function will fail if:
 * - any of the addresses are not page aligned
 * - any of the proviced addresses have their mappings changed
 * - guestAddress is used as a target in any other mappings
 * - all mapping slots are used
 * - the internal buffer used for page splitting is full
 *
 * The result is returned in RAX. It returns STATUS_SUCCESS on success,
 * and error code on failure.
 *
 * @param registers Guest registers
 *
 * @return VOID
 */
static VOID vmCallMapPage(VMEXIT_Registers* const registers)
{
  const EPT_Address guestAddress = { .address = registers->RDX };
  const EPT_Address hostRwAddress = { .address = registers->R8 };
  const EPT_Address hostFetchAddress = { .address = registers->R9 };

  if (guestAddress.offset != 0 || hostRwAddress.offset != 0 || hostFetchAddress.offset != 0)
  {
    registers->RAX = (UINT64)STATUS_UNSUCCESSFUL;
    return;
  }

  Context_LogicalCore* const thisCore = Context_getLogicalCore();
  Context_EptChangedMapping* const changedMappings =
    thisCore->eptMappingData.changedMappings;
  for (UINT64 mappingIndex = 0; mappingIndex < CONTEXT_EPT_MAX_MAPPINGS; mappingIndex++)
  {
    const Context_EptChangedMapping* const currentMapping = &changedMappings[mappingIndex];
    if (currentMapping->valid &&
      (currentMapping->guestAddress == guestAddress.address ||
        currentMapping->hostFetchAddress == guestAddress.address ||
        currentMapping->hostRwAddress == guestAddress.address ||
        currentMapping->guestAddress == hostRwAddress.address ||
        currentMapping->guestAddress == hostFetchAddress.address))
    {
      registers->RAX = (UINT64)STATUS_UNSUCCESSFUL;
      return;
    }
  }

  UINT64 selectedIndex = 0;
  while (selectedIndex < CONTEXT_EPT_MAX_MAPPINGS && changedMappings[selectedIndex].valid)
  {
    selectedIndex++;
  }

  if (selectedIndex >= CONTEXT_EPT_MAX_MAPPINGS)
  {
    registers->RAX = (UINT64)STATUS_UNSUCCESSFUL;
    return;
  }

  NTSTATUS ntStatus = EPT_changeMapping(guestAddress.address, guestAddress.address, FALSE, FALSE);
  if (!NT_SUCCESS(ntStatus))
  {
    registers->RAX = ntStatus;
    return;
  }

  changedMappings[selectedIndex] = (Context_EptChangedMapping)
  {
    .guestAddress = registers->RDX,
    .hostRwAddress = registers->R8,
    .hostFetchAddress = registers->R9,
    .valid = TRUE
  };

  VMX_inveptAll();
  registers->RAX = (UINT64)STATUS_SUCCESS;
}


/**
 * @brief Handles VMEXIT_VMCALL_UNMAP_PAGE
 *
 * Handles VMEXIT_VMCALL_UNMAP_PAGE VMCALL. It receives guest address in RDX.
 * Mapping is removed from Context_EptChangedMapping structure, and EPT mapping
 * is restored to original. This function will fail if mapping is not found. Result is
 * returned in RAX. It returns STATUS_SUCCESS on success, and error code on failure.
 *
 * @param registers Guest registers
 *
 * @return VOID
 */
static VOID vmCallUnmapPage(VMEXIT_Registers* const registers)
{
  const UINT64 guestAddress = registers->RDX;

  Context_LogicalCore* const thisCore = Context_getLogicalCore();
  Context_EptChangedMapping* foundMapping = NULL;
  for (UINT64 mappingIndex = 0; mappingIndex < CONTEXT_EPT_MAX_MAPPINGS; mappingIndex++)
  {
    Context_EptChangedMapping* const currentMapping =
      &thisCore->eptMappingData.changedMappings[mappingIndex];
    if (currentMapping->valid && currentMapping->guestAddress == guestAddress)
    {
      foundMapping = currentMapping;
      break;
    }
  }

  if (foundMapping == NULL)
  {
    registers->RAX = (UINT64)STATUS_UNSUCCESSFUL;
    return;
  }

  NTSTATUS ntStatus = EPT_changeMapping(guestAddress, guestAddress, TRUE, TRUE);
  if (!NT_SUCCESS(ntStatus))
  {
    // Should never happen
    registers->RAX = ntStatus;
    return;
  }

  *foundMapping = (Context_EptChangedMapping){ 0 };

  VMX_inveptAll();
  registers->RAX = STATUS_SUCCESS;
}
