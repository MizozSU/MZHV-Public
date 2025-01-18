/**
 * @file vmcs.h
 * @brief Virtual Machine Control Structure fields and definitions
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name 16-Bit Guest-State Fields
 * @anchor VMCS16BitGuestStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.1.2](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_GUEST_ES_SELECTOR                                0x00000800
#define VMCS_GUEST_CS_SELECTOR                                0x00000802
#define VMCS_GUEST_SS_SELECTOR                                0x00000804
#define VMCS_GUEST_DS_SELECTOR                                0x00000806
#define VMCS_GUEST_FS_SELECTOR                                0x00000808
#define VMCS_GUEST_GS_SELECTOR                                0x0000080A
#define VMCS_GUEST_LDTR_SELECTOR                              0x0000080C
#define VMCS_GUEST_TR_SELECTOR                                0x0000080E
///@}

/**
 * @name 16-Bit Host-State Fields
 * @anchor VMCS16BitHostStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.1.3](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_HOST_ES_SELECTOR                                 0x00000C00
#define VMCS_HOST_CS_SELECTOR                                 0x00000C02
#define VMCS_HOST_SS_SELECTOR                                 0x00000C04
#define VMCS_HOST_DS_SELECTOR                                 0x00000C06
#define VMCS_HOST_FS_SELECTOR                                 0x00000C08
#define VMCS_HOST_GS_SELECTOR                                 0x00000C0A
#define VMCS_HOST_TR_SELECTOR                                 0x00000C0C
///@}

/**
 * @name 64-Bit Control Fields
 * @anchor VMCS64BitControlFields
 * @see [Intel SDM, Vol. 3C, Appendix B.2.1](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_ADDRESS_OF_MSR_BITMAPS_FULL                      0x00002004
#define VMCS_EPT_POINTER_FULL                                 0x0000201A
///@}

/**
 * @name 64-Bit Read-Only Data Fields
 * @anchor VMCS64BitReadOnlyDataFields
 * @see [Intel SDM, Vol. 3C, Appendix B.2.2](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_GUEST_PHYSICAL_ADDRESS_FULL                      0x00002400
///@}

/**
 * @name 64-Bit Guest-State Fields
 * @anchor VMCS64BitGuestStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.2.3](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_GUEST_VMCS_LINK_POINTER_FULL                     0x00002800
#define VMCS_GUEST_IA32_DEBUGCTL_FULL                         0x00002802
///@}

/**
 * @name 32-Bit Control Fields
 * @anchor VMCS32BitControlFields
 * @see [Intel SDM, Vol. 3C, Appendix B.2.4](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_PIN_BASED_VM_EXECUTION_CONTROLS                  0x00004000
#define VMCS_PRIMARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS    0x00004002
#define VMCS_PRIMARY_VM_EXIT_CONTROLS                         0x0000400C
#define VMCS_VM_ENTRY_CONTROLS                                0x00004012
#define VMCS_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS  0x0000401E
///@}

/**
 * @name 32-Bit Read-Only Data Fields
 * @anchor VMCS32BitReadOnlyDataFields
 * @see [Intel SDM, Vol. 3C, Appendix B.3.2](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_EXIT_REASON                                      0x00004402
#define VMCS_VM_EXIT_INSTRUCTION_LENGTH                       0x0000440C
///@}

/**
 * @name 32-Bit Guest-State Fields
 * @anchor VMCS32BitGuestStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.3.3](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_GUEST_ES_LIMIT                                   0x00004800
#define VMCS_GUEST_CS_LIMIT                                   0x00004802
#define VMCS_GUEST_SS_LIMIT                                   0x00004804
#define VMCS_GUEST_DS_LIMIT                                   0x00004806
#define VMCS_GUEST_FS_LIMIT                                   0x00004808
#define VMCS_GUEST_GS_LIMIT                                   0x0000480A
#define VMCS_GUEST_LDTR_LIMIT                                 0x0000480C
#define VMCS_GUEST_TR_LIMIT                                   0x0000480E
#define VMCS_GUEST_GDTR_LIMIT                                 0x00004810
#define VMCS_GUEST_IDTR_LIMIT                                 0x00004812
#define VMCS_GUEST_ES_ACCESS_RIGHTS                           0x00004814
#define VMCS_GUEST_CS_ACCESS_RIGHTS                           0x00004816
#define VMCS_GUEST_SS_ACCESS_RIGHTS                           0x00004818
#define VMCS_GUEST_DS_ACCESS_RIGHTS                           0x0000481A
#define VMCS_GUEST_FS_ACCESS_RIGHTS                           0x0000481C
#define VMCS_GUEST_GS_ACCESS_RIGHTS                           0x0000481E
#define VMCS_GUEST_LDTR_ACCESS_RIGHTS                         0x00004820
#define VMCS_GUEST_TR_ACCESS_RIGHTS                           0x00004822
#define VMCS_GUEST_IA32_SYSENTER_CS                           0x0000482A
///@}

/**
 * @name 32-Bit Host-State Fields
 * @anchor VMCS32BitHostStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.3.4](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_HOST_IA32_SYSENTER_CS                            0x00004C00
///@}

/**
 * @name Natural-Width Read-Only Data Fields
 * @anchor VMCSNaturalWidthReadOnlyDataFields
 * @see [Intel SDM, Vol. 3C, Appendix B.4.2](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_EXIT_QUALIFICATION                               0x00006400
///@}

/**
 * @name Natural-Width Guest-State Fields
 * @anchor VMCSNaturalWidthGuestStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.4.3](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_GUEST_CR0                                        0x00006800
#define VMCS_GUEST_CR3                                        0x00006802
#define VMCS_GUEST_CR4                                        0x00006804
#define VMCS_GUEST_ES_BASE                                    0x00006806
#define VMCS_GUEST_CS_BASE                                    0x00006808
#define VMCS_GUEST_SS_BASE                                    0x0000680A
#define VMCS_GUEST_DS_BASE                                    0x0000680C
#define VMCS_GUEST_FS_BASE                                    0x0000680E
#define VMCS_GUEST_GS_BASE                                    0x00006810
#define VMCS_GUEST_LDTR_BASE                                  0x00006812
#define VMCS_GUEST_TR_BASE                                    0x00006814
#define VMCS_GUEST_GDTR_BASE                                  0x00006816
#define VMCS_GUEST_IDTR_BASE                                  0x00006818
#define VMCS_GUEST_DR7                                        0x0000681A
#define VMCS_GUEST_RSP                                        0x0000681C
#define VMCS_GUEST_RIP                                        0x0000681E
#define VMCS_GUEST_RFLAGS                                     0x00006820
#define VMCS_GUEST_A32_SYSENTER_ESP                           0x00006824
#define VMCS_GUEST_A32_SYSENTER_EIP                           0x00006826
///@}

/**
 * @name Natural-Width Host-State Fields
 * @anchor VMCSNaturalWidthHostStateFields
 * @see [Intel SDM, Vol. 3C, Appendix B.4.4](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define VMCS_HOST_CR0                                         0x00006C00
#define VMCS_HOST_CR3                                         0x00006C02
#define VMCS_HOST_CR4                                         0x00006C04
#define VMCS_HOST_FS_BASE                                     0x00006C06
#define VMCS_HOST_GS_BASE                                     0x00006C08
#define VMCS_HOST_TR_BASE                                     0x00006C0A
#define VMCS_HOST_GDTR_BASE                                   0x00006C0C
#define VMCS_HOST_IDTR_BASE                                   0x00006C0E
#define VMCS_HOST_IA32_SYSENTER_ESP                           0x00006C10
#define VMCS_HOST_IA32_SYSENTER_EIP                           0x00006C12
#define VMCS_HOST_RSP                                         0x00006C14
#define VMCS_HOST_RIP                                         0x00006C16
///@}


/**************************************************************************************************
* Type declarations
**************************************************************************************************/
#pragma warning(disable:4201)
#pragma pack(push, 1)
/**
 * @brief Union for Pin-Based VM-Execution Controls
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structures](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMCS_PinBasedVmExecutionControls
{
  UINT32 bits;
  struct
  {
    UINT32 externalInterruptExiting : 1;
    UINT32 _pad1 : 2;
    UINT32 nmiExiting : 1;
    UINT32 _pad2 : 1;
    UINT32 virtualNmis : 1;
    UINT32 activateVmxPreemptionTimer : 1;
    UINT32 processPostedInterrupts : 1;
    UINT32 _pad3 : 24;
  };
} VMCS_PinBasedVmExecutionControls;


/**
 * @brief Union for Primary Processor-Based VM-Execution Controls
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structures](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMCS_PrimaryProcessorBasedVmExecutionControls
{
  UINT32 bits;
  struct
  {
    UINT32 _pad1 : 2;
    UINT32 interruptWindowExiting : 1;
    UINT32 useTscOffsetting : 1;
    UINT32 _pad2 : 3;
    UINT32 hltExiting : 1;
    UINT32 _pad3 : 1;
    UINT32 invlpgExiting : 1;
    UINT32 mwaitExiting : 1;
    UINT32 rdpmcExiting : 1;
    UINT32 rdtscExiting : 1;
    UINT32 _pad4 : 2;
    UINT32 cr3LoadExiting : 1;
    UINT32 cr3StoreExiting : 1;
    UINT32 activateTertiaryControls : 1;
    UINT32 _pad5 : 1;
    UINT32 cr8LoadExiting : 1;
    UINT32 cr8StoreExiting : 1;
    UINT32 useTprShadow : 1;
    UINT32 nmiWindowExiting : 1;
    UINT32 movDrExiting : 1;
    UINT32 unconditionalIoExiting : 1;
    UINT32 useIoBitmaps : 1;
    UINT32 _pad6 : 1;
    UINT32 monitorTrapFlag : 1;
    UINT32 useMsrBitmaps : 1;
    UINT32 monitorExiting : 1;
    UINT32 pauseExiting : 1;
    UINT32 activateSecondaryControls : 1;
  };
} VMCS_PrimaryProcessorBasedVmExecutionControls;


/**
 * @brief Union for Secondary Processor-Based VM-Execution Controls
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structures](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMCS_SecondaryProcessorBasedVmExecutionControls
{
  UINT32 bits;
  struct
  {
    UINT32 virtualizeApicAccesses : 1;
    UINT32 enableEpt : 1;
    UINT32 descriptorTableExiting : 1;
    UINT32 enableRdtscp : 1;
    UINT32 virtualizeX2apicMode : 1;
    UINT32 enableVpid : 1;
    UINT32 wbinvdExiting : 1;
    UINT32 unrestrictedGuest : 1;
    UINT32 apicRegisterVirtualization : 1;
    UINT32 virtualInterruptDelivery : 1;
    UINT32 pauseLoopExiting : 1;
    UINT32 rdrandExiting : 1;
    UINT32 enableInvpcid : 1;
    UINT32 enableVmFunctions : 1;
    UINT32 vmcsShadowing : 1;
    UINT32 enableEnclsExiting : 1;
    UINT32 rdseedExiting : 1;
    UINT32 enablePML : 1;
    UINT32 eptViolationVe : 1;
    UINT32 concealVmxFromPt : 1;
    UINT32 enableXsavesXrstors : 1;
    UINT32 pasidTranslation : 1;
    UINT32 modeBasedExecuteControlForEpt : 1;
    UINT32 subPageWritePermissionsForEpt : 1;
    UINT32 intelPtUsesGuestPhysicalAddresses : 1;
    UINT32 useTscScaling : 1;
    UINT32 enableUserWaitAndPause : 1;
    UINT32 enablePconfig : 1;
    UINT32 enableEnclvExiting : 1;
    UINT32 _pad1 : 1;
    UINT32 vmmBusLockDetection : 1;
    UINT32 instructionTimeout : 1;
  };
} VMCS_SecondaryProcessorBasedVmExecutionControls;


/**
 * @brief Union for VM-Exit Controls
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structures](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMCS_PrimaryVmExitControls
{
  UINT32 bits;
  struct
  {
    UINT32 _pad1 : 2;
    UINT32 saveDebugControls : 1;
    UINT32 _pad2 : 6;
    UINT32 hostAddressSpaceSize : 1;
    UINT32 _pad3 : 2;
    UINT32 loadIa32PerfGlobalCtrl : 1;
    UINT32 _pad4 : 2;
    UINT32 acknowledgeInterruptOnExit : 1;
    UINT32 _pad5 : 2;
    UINT32 saveIa32Pat : 1;
    UINT32 loadIa32Pat : 1;
    UINT32 saveIa32Efer : 1;
    UINT32 loadIa32Efer : 1;
    UINT32 saveVmxPreemptionTimerValue : 1;
    UINT32 clearIa32Bndcfgs : 1;
    UINT32 concealVmxFromPt : 1;
    UINT32 clearIa32RtitCtl : 1;
    UINT32 clearIa32LbrCtl : 1;
    UINT32 clearUinv : 1;
    UINT32 loadCetState : 1;
    UINT32 loadPkrs : 1;
    UINT32 saveIa32PerfGlobalCtl : 1;
    UINT32 activateSecondaryControls : 1;
  };
} VMCS_PrimaryVmExitControls;


/**
 * @brief Union for VM-Entry Controls
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structures](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMCS_VmEntryControls
{
  UINT32 bits;
  struct
  {
    UINT32 _pad1 : 2;
    UINT32 loadDebugControls : 1;
    UINT32 _pad2 : 6;
    UINT32 ia32eModeGuest : 1;
    UINT32 entryToSmm : 1;
    UINT32 deactivateDualMonitorTreatment : 1;
    UINT32 _pad3 : 1;
    UINT32 loadIa32PerfGlobalCtrl : 1;
    UINT32 loadIa32Pat : 1;
    UINT32 loadIa32Efer : 1;
    UINT32 loadIa32Bndcfgs : 1;
    UINT32 concealVmxFromPt : 1;
    UINT32 loadIa32TritCtl : 1;
    UINT32 loadUinv : 1;
    UINT32 loadCetState : 1;
    UINT32 loadGuestIa32LbrCtl : 1;
    UINT32 loadPkrs : 1;
  };
} VMCS_VmEntryControls;
#pragma pack(pop)
#pragma warning(default:4201)


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS VMCS_setup(VOID);


VOID VMCS_restore(VOID);
