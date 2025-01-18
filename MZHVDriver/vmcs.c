/**
 * @file vmcs.c
 * @brief VMCS setup and restore.
 *
 * Handles initialization of the VMCS and its fields, and restoring VMCS state.
 */


#include <intrin.h>
#include "context.h"
#include "ia32.h"
#include "memory.h"
#include "vmcs.h"
#include "asmproc.h"
#include "segmentation.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static NTSTATUS setupMemoryRegion(Context_LogicalCore* const thisCore);


static VOID setupGuestStateArea(VOID);


static VOID setupGuestSegmentFields(
  const UINT64 selectorField,
  const UINT64 baseField,
  const UINT64 limitField,
  const UINT64 accessRightsField,
  const SEGMENTATION_SegmentSelector
  segmentSelector,
  const VMCS_GDTR gdtr);


static SEGMENTATION_SegmentBase getSegmentBase(
  const SEGMENTATION_SegmentSelector segmentSelector,
  const VMCS_GDTR gdtr);


static VOID setupHostStateArea(Context_LogicalCore* const thisCore);


static VOID setupHostSegmentSelector(
  const UINT64 selectorField,
  const SEGMENTATION_SegmentSelector segmentSelector);


static VOID setupVMExecutionControlFields(Context_LogicalCore* const thisCore);


static VOID setupVMExitControlFields(VOID);


static VOID setupVMEntryControlFields(VOID);


static VOID adjustAndApplyControls(
  const UINT64 vmcsControlsField,
  const UINT32 msrAddress,
  UINT32* const controlsBits);


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Prepares VMCS for VM entry.
 *
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
NTSTATUS VMCS_setup(VOID)
{
  Context_LogicalCore* const thisCore = Context_getLogicalCore();

  NTSTATUS ntStatus = setupMemoryRegion(thisCore);
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }

  setupGuestStateArea();
  setupHostStateArea(thisCore);
  setupVMExecutionControlFields(thisCore);
  setupVMExitControlFields();
  setupVMEntryControlFields();

  return STATUS_SUCCESS;
}


/**
 * @brief Restores host state from VMCS.
 *
 * Restores host state from VMCS. It is called before returning to the host
 * in devirtualized state.
 *
 * @return VOID
 */
VOID VMCS_restore(VOID)
{
  IA32_Cr3 cr3 = { 0 };
  __vmx_vmread(VMCS_GUEST_CR3, &cr3.bits);
  __writecr3(cr3.bits);

  UINT64 base = { 0 };
  UINT64 limit = { 0 };
  __vmx_vmread(VMCS_GUEST_GDTR_BASE, &base);
  __vmx_vmread(VMCS_GUEST_GDTR_LIMIT, &limit);
  _lgdt(&(VMCS_GDTR) { .base = base, .limit = (UINT16)limit });

  __vmx_vmread(VMCS_GUEST_IDTR_BASE, &base);
  __vmx_vmread(VMCS_GUEST_IDTR_LIMIT, &limit);
  __lidt(&(SEGMENTATION_IDTR) { .base = base, .limit = (UINT16)limit });
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Sets up the VMCS memory region and loads it.
 *
 * Sets up the VMCS memory region and loads it. It is called before any writes to
 * VMCS can be made.
 *
 * @param thisCore Pointer to logical core's context.
 *
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
static NTSTATUS setupMemoryRegion(Context_LogicalCore* const thisCore)
{
  const IA32_VmxBasic vmxBasic = { .bits = __readmsr(IA32_VMX_BASIC) };
  const UINT64 revisionIdentifier = vmxBasic.revisionIdentifier;
  Memory_copy(thisCore->vmcsRegion, &revisionIdentifier, sizeof(UINT64));

  UINT64 vmcsPhysicalAddress = Memory_getPhysicalAddress(thisCore->vmcsRegion);
  if (__vmx_vmclear(&vmcsPhysicalAddress) != 0)
  {
    return STATUS_UNSUCCESSFUL;
  }
  if (__vmx_vmptrld(&vmcsPhysicalAddress) != 0)
  {
    return STATUS_UNSUCCESSFUL;
  }

  return STATUS_SUCCESS;
}


/**
 * @brief Sets up guest state area.
 *
 * @return VOID
 */
static VOID setupGuestStateArea(VOID)
{
  __vmx_vmwrite(VMCS_GUEST_CR0, __readcr0());
  __vmx_vmwrite(VMCS_GUEST_CR3, __readcr3());
  __vmx_vmwrite(VMCS_GUEST_CR4, __readcr4());

  __vmx_vmwrite(VMCS_GUEST_DR7, __readdr(7));

  // VMCS_GUEST_RSP is set in ASM handler
  __vmx_vmwrite(VMCS_GUEST_RIP, (ULONG_PTR)ASMPROC_vmcsEntryPoint);
  __vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());

  VMCS_GDTR gdtr;
  _sgdt(&gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_CS_SELECTOR,
    VMCS_GUEST_CS_BASE,
    VMCS_GUEST_CS_LIMIT,
    VMCS_GUEST_CS_ACCESS_RIGHTS,
    SEGMENTATION_readCs(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_SS_SELECTOR,
    VMCS_GUEST_SS_BASE,
    VMCS_GUEST_SS_LIMIT,
    VMCS_GUEST_SS_ACCESS_RIGHTS,
    SEGMENTATION_readSs(),
    gdtr);

  setupGuestSegmentFields(VMCS_GUEST_DS_SELECTOR,
    VMCS_GUEST_DS_BASE,
    VMCS_GUEST_DS_LIMIT,
    VMCS_GUEST_DS_ACCESS_RIGHTS,
    SEGMENTATION_readDs(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_ES_SELECTOR,
    VMCS_GUEST_ES_BASE,
    VMCS_GUEST_ES_LIMIT,
    VMCS_GUEST_ES_ACCESS_RIGHTS,
    SEGMENTATION_readEs(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_FS_SELECTOR,
    VMCS_GUEST_FS_BASE,
    VMCS_GUEST_FS_LIMIT,
    VMCS_GUEST_FS_ACCESS_RIGHTS,
    SEGMENTATION_readFs(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_GS_SELECTOR,
    VMCS_GUEST_GS_BASE,
    VMCS_GUEST_GS_LIMIT,
    VMCS_GUEST_GS_ACCESS_RIGHTS,
    SEGMENTATION_readGs(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_LDTR_SELECTOR,
    VMCS_GUEST_LDTR_BASE,
    VMCS_GUEST_LDTR_LIMIT,
    VMCS_GUEST_LDTR_ACCESS_RIGHTS,
    SEGMENTATION_readLdtr(),
    gdtr);

  setupGuestSegmentFields(
    VMCS_GUEST_TR_SELECTOR,
    VMCS_GUEST_TR_BASE,
    VMCS_GUEST_TR_LIMIT,
    VMCS_GUEST_TR_ACCESS_RIGHTS,
    SEGMENTATION_readTr(),
    gdtr);

  __vmx_vmwrite(VMCS_GUEST_GDTR_BASE, gdtr.base);
  __vmx_vmwrite(VMCS_GUEST_GDTR_LIMIT, gdtr.limit);

  SEGMENTATION_IDTR idtr;
  __sidt(&idtr);

  __vmx_vmwrite(VMCS_GUEST_IDTR_BASE, idtr.base);
  __vmx_vmwrite(VMCS_GUEST_IDTR_LIMIT, idtr.limit);

  __vmx_vmwrite(VMCS_GUEST_IA32_DEBUGCTL_FULL, __readmsr(IA32_DEBUGCTL));
  __vmx_vmwrite(VMCS_GUEST_IA32_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
  __vmx_vmwrite(VMCS_GUEST_A32_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));
  __vmx_vmwrite(VMCS_GUEST_A32_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));

  __vmx_vmwrite(VMCS_GUEST_VMCS_LINK_POINTER_FULL, 0xFFFFFFFFFFFFFFFFLLU);
}


/**
 * @brief Fills guest segment fields in VMCS.
 *
 * Fills segment fields in VMCS for guest, based on segment selector and GDT.
 *
 * @param selectorField Field for segment selector.
 * @param baseField Field for segment base.
 * @param limitField Field for segment limit.
 * @param accessRightsField Field for segment access rights.
 * @param segmentSelector Segment selector.
 * @param gdtr GDT register.
 *
 * @return VOID
 */
static VOID setupGuestSegmentFields(
  const UINT64 selectorField,
  const UINT64 baseField,
  const UINT64 limitField,
  const UINT64 accessRightsField,
  const SEGMENTATION_SegmentSelector
  segmentSelector,
  const VMCS_GDTR gdtr)
{
  __vmx_vmwrite(selectorField, segmentSelector.bits);
  __vmx_vmwrite(limitField, __segmentlimit(segmentSelector.bits));


  if (segmentSelector.tableIndicator != 0 || segmentSelector.index == 0)
  {
    __vmx_vmwrite(baseField, 0);
    __vmx_vmwrite(
      accessRightsField,
      (SEGMENTATION_VMCSSegmentAccessRights) {
      .segmentUnusable = TRUE
    }.bits);
    return;
  }


  const SEGMENTATION_SegmentDescriptor* const gdtEntry =
    (const SEGMENTATION_SegmentDescriptor*)gdtr.base + segmentSelector.index;
  const SEGMENTATION_VMCSSegmentAccessRights vmcsAccessRights =
    (SEGMENTATION_VMCSSegmentAccessRights)
  {
    .segmentAccessRightsByte = gdtEntry->segmentAccessRightsByte,
    .segmentFlags = gdtEntry->segmentFlags
  };
  __vmx_vmwrite(accessRightsField, vmcsAccessRights.bits);


  SEGMENTATION_SegmentBase segmentBase = { 0 };
  switch (baseField)
  {
  case VMCS_GUEST_FS_BASE:
  {
    segmentBase.bits = __readmsr(IA32_FS_BASE);
    break;
  }
  case VMCS_GUEST_GS_BASE:
  {
    segmentBase.bits = __readmsr(IA32_GS_BASE);
    break;
  }
  default:
  {
    segmentBase = getSegmentBase(segmentSelector, gdtr);
    break;
  }
  }
  __vmx_vmwrite(baseField, segmentBase.bits);
}


/**
 * @brief Returns segment base for a segment selector.
 *
 * Reads segment base for a segment selector from the GDT.
 *
 * @param segmentSelector Segment selector.
 * @param gdtr GDT register.
 *
 * @return Segment base.
 */
static SEGMENTATION_SegmentBase getSegmentBase(
  const SEGMENTATION_SegmentSelector segmentSelector,
  const VMCS_GDTR gdtr)
{
  if (segmentSelector.tableIndicator != 0 || segmentSelector.index == 0)
  {
    return (SEGMENTATION_SegmentBase) { 0 };
  }

  const SEGMENTATION_SegmentDescriptor* const gdtEntry =
    (const SEGMENTATION_SegmentDescriptor*)gdtr.base + segmentSelector.index;

  return (SEGMENTATION_SegmentBase)
  {
    .address1 = gdtEntry->segmentBaseAddress1,
      .address2 = gdtEntry->segmentBaseAddress2,
      .address3 = gdtEntry->segmentBaseAddress3,
      .address4 =
      (gdtEntry->segmentAccessRightsByte.descriptorType ?
        0 :
        ((const SEGMENTATION_SystemSegmentDescriptor*)gdtEntry)->segmentBaseAddress4
        )
  };
}


/**
 * @brief Sets up host state area.
 *
 * @param thisCore Pointer to logical core's context.
 *
 * @return VOID
 */
static VOID setupHostStateArea(Context_LogicalCore* const thisCore)
{
  __vmx_vmwrite(VMCS_HOST_CR0, __readcr0());
  __vmx_vmwrite(VMCS_HOST_CR3, Context_getContext()->systemCR3.bits);
  __vmx_vmwrite(VMCS_HOST_CR4, __readcr4());

  // Stack must be 16-byte aligned for Windows
  __vmx_vmwrite(
    VMCS_HOST_RSP,
    ((ULONG_PTR)thisCore->rootModeStack + CONTEXT_ROOT_MODE_STACK_SIZE - 16));
  __vmx_vmwrite(VMCS_HOST_RIP, (ULONG_PTR)ASMPROC_vmExitHandler);

  setupHostSegmentSelector(VMCS_HOST_CS_SELECTOR, SEGMENTATION_readCs());
  setupHostSegmentSelector(VMCS_HOST_SS_SELECTOR, SEGMENTATION_readSs());
  setupHostSegmentSelector(VMCS_HOST_DS_SELECTOR, SEGMENTATION_readDs());
  setupHostSegmentSelector(VMCS_HOST_ES_SELECTOR, SEGMENTATION_readEs());
  setupHostSegmentSelector(VMCS_HOST_FS_SELECTOR, SEGMENTATION_readFs());
  setupHostSegmentSelector(VMCS_HOST_GS_SELECTOR, SEGMENTATION_readGs());
  setupHostSegmentSelector(VMCS_HOST_TR_SELECTOR, SEGMENTATION_readTr());

  VMCS_GDTR gdtr;
  _sgdt(&gdtr);

  __vmx_vmwrite(VMCS_HOST_TR_BASE, getSegmentBase(SEGMENTATION_readTr(), gdtr).bits);
  __vmx_vmwrite(VMCS_HOST_FS_BASE, __readmsr(IA32_FS_BASE));
  __vmx_vmwrite(VMCS_HOST_GS_BASE, __readmsr(IA32_GS_BASE));

  __vmx_vmwrite(VMCS_HOST_GDTR_BASE, gdtr.base);

  SEGMENTATION_IDTR idtr;
  __sidt(&idtr);

  __vmx_vmwrite(VMCS_HOST_IDTR_BASE, idtr.base);

  __vmx_vmwrite(VMCS_HOST_IA32_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
  __vmx_vmwrite(VMCS_HOST_IA32_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));
  __vmx_vmwrite(VMCS_HOST_IA32_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));
}


/**
 * @brief Sets up a host segment selector.
 *
 * Sets up a host segment selector by clearing table indicator and rpl bits.
 *
 * @param selectorField Field for segment selector.
 * @param segmentSelector Segment selector.
 *
 * @return VOID
 */
static VOID setupHostSegmentSelector(
  const UINT64 selectorField,
  const SEGMENTATION_SegmentSelector segmentSelector)
{
  const SEGMENTATION_SegmentSelector selector = { .index = segmentSelector.index };
  __vmx_vmwrite(selectorField, selector.bits);
}


/**
 * @brief Sets up VM execution control fields.
 *
 * @param thisCore Pointer to logical core's context.
 *
 * @return VOID
 */
static VOID setupVMExecutionControlFields(Context_LogicalCore* const thisCore)
{
  VMCS_PinBasedVmExecutionControls pinControls = { 0 };
  adjustAndApplyControls(
    VMCS_PIN_BASED_VM_EXECUTION_CONTROLS,
    IA32_VMX_TRUE_PINBASED_CTLS,
    &pinControls.bits);

  VMCS_PrimaryProcessorBasedVmExecutionControls primaryControls =
  {
    .useMsrBitmaps = TRUE,
    .activateSecondaryControls = TRUE
  };
  adjustAndApplyControls(
    VMCS_PRIMARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS,
    IA32_VMX_TRUE_PROCBASED_CTLS,
    &primaryControls.bits);

  VMCS_SecondaryProcessorBasedVmExecutionControls secondaryControls =
  {
    .enableRdtscp = TRUE,
    .enableInvpcid = TRUE,
    .enableXsavesXrstors = TRUE,
    .enableEpt = TRUE
  };
  adjustAndApplyControls(
    VMCS_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS,
    IA32_VMX_PROCBASED_CTLS2,
    &secondaryControls.bits);

  __vmx_vmwrite(VMCS_ADDRESS_OF_MSR_BITMAPS_FULL, Memory_getPhysicalAddress(thisCore->msrBitmap));
  __vmx_vmwrite(VMCS_EPT_POINTER_FULL, thisCore->eptp);
}


/**
 * @brief Sets up VM exit control fields.
 *
 * @return VOID
 */
static VOID setupVMExitControlFields(VOID)
{
  VMCS_PrimaryVmExitControls exitControls = {
    .hostAddressSpaceSize = TRUE,
  };
  adjustAndApplyControls(
    VMCS_PRIMARY_VM_EXIT_CONTROLS,
    IA32_VMX_TRUE_EXIT_CTLS,
    &exitControls.bits);
}

/**
 * @brief Sets up VM entry control fields.
 *
 * @return VOID
 */
static VOID setupVMEntryControlFields(VOID)
{
  VMCS_VmEntryControls entryControls = {
    .ia32eModeGuest = TRUE
  };
  adjustAndApplyControls(
    VMCS_VM_ENTRY_CONTROLS,
    IA32_VMX_TRUE_ENTRY_CTLS,
    &entryControls.bits);
}


/**
 * @brief Adjust controls by MSR and applies them to VMCS.
 *
 * @param vmcsControlsField Field for controls.
 * @param msrAddress Address of MSR.
 * @param controlsBits Pointer to controls bits.
 *
 * @return VOID
 */
static VOID adjustAndApplyControls(
  const UINT64 vmcsControlsField,
  const UINT32 msrAddress,
  UINT32* const controlsBits)
{
  const UINT64 msrMask = __readmsr(msrAddress);
  *controlsBits &= (msrMask >> 32);
  *controlsBits |= (msrMask & 0xFFFFFFFFULL);
  __vmx_vmwrite(vmcsControlsField, *controlsBits);
}
