/**
 * @file ia32.h
 * @brief IA32 definitions
 *
 * This file contains definitions of structures from Intel SDM.
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name IA32 MSR addresses
 * @brief Numeric values of IA32 MSR addresses
 * @anchor IA32MsrAddresses
 * @see [Intel SDM, Vol. 4, Chapter 2 Model-Specific Registers (MSRs)](https://software.intel.com/en-us/articles/intel-sdm)
 */
 ///@{
#define IA32_FEATURE_CONTROL             0x3A
#define IA32_MTRRCAP                     0xFE
#define IA32_SYSENTER_CS                 0x174
#define IA32_SYSENTER_ESP                0x175
#define IA32_SYSENTER_EIP                0x176
#define IA32_DEBUGCTL                    0x1D9
#define IA32_MTRR_PHYSBASE0              0x200
#define IA32_MTRR_PHYSMASK0              0x201
#define IA32_MTRR_FIX64K_00000           0x250
#define IA32_MTRR_FIX16K_80000           0x258
#define IA32_MTRR_FIX16K_A0000           0x259
#define IA32_MTRR_FIX4K_C0000            0x268
#define IA32_MTRR_FIX4K_C8000            0x269
#define IA32_MTRR_FIX4K_D0000            0x26A
#define IA32_MTRR_FIX4K_D8000            0x26B
#define IA32_MTRR_FIX4K_E0000            0x26C
#define IA32_MTRR_FIX4K_E8000            0x26D
#define IA32_MTRR_FIX4K_F0000            0x26E
#define IA32_MTRR_FIX4K_F8000            0x26F
#define IA32_MTRR_DEF_TYPE               0x2FF
#define IA32_VMX_BASIC                   0x480
#define IA32_VMX_CR0_FIXED0              0x486
#define IA32_VMX_CR0_FIXED1              0x487
#define IA32_VMX_CR4_FIXED0              0x488
#define IA32_VMX_CR4_FIXED1              0x489
#define IA32_VMX_PROCBASED_CTLS2         0x48B
#define IA32_VMX_TRUE_PINBASED_CTLS      0x48D
#define IA32_VMX_TRUE_PROCBASED_CTLS     0x48E
#define IA32_VMX_TRUE_EXIT_CTLS          0x48F
#define IA32_VMX_TRUE_ENTRY_CTLS         0x490
#define IA32_FS_BASE                     0xC0000100
#define IA32_GS_BASE                     0xC0000101
///@}

/**
 * @name IA32 CPUID function numbers
 * @brief CPUID function numbers for selected functions
 * @anchor IA32CpuidFunctionNumbers
 * @see [Intel SDM, Vol. 3A, Chapter 3 Instruction Set Reference, A-L](https://software.intel.com/en-us/articles/intel-sdm)
 */
///@{
#define IA32_CPUID_BASIC_INFORMATION_0   0x0
#define IA32_CPUID_BASIC_INFORMATION_1   0x1
#define IA32_CPUID_ADDRESS_BITS          0x80000008
 ///@}

/**
 * @name Others
 * @brief Additional constants
 * @anchor IA32Others
 */
///@{
#define IA32_MTRR_MEMORY_TYPE_UC         0x0
#define IA32_MTRR_MEMORY_TYPE_WC         0x1
#define IA32_MTRR_MEMORY_TYPE_WT         0x4
#define IA32_MTRR_MEMORY_TYPE_WP         0x5
#define IA32_MTRR_MEMORY_TYPE_WB         0x6
#define IA32_MTRR_MEMORY_TYPE_ERR        0xFFFF
#define IA32_MTRR_DISABLED_DEFAULT_TYPE  IA32_MTRR_MEMORY_TYPE_UC
#define IA32_MTRR_PHYSBASE0_INC          2
#define IA32_MTRR_PHYSMASK0_INC          2
#define IA32_MTRR_MAX_VMTRR_COUNT        256
 ///@}


/**************************************************************************************************
* Type declarations
**************************************************************************************************/
#pragma warning(disable:4201)
#pragma pack(push, 1)
/**
 * @brief Structure for IA32_VMX_BASIC MSR
 * @see [Intel SDM, Vol. 3C, Appendix A.1](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_VmxBasic
{
  UINT64 bits;
  struct
  {
    UINT64 revisionIdentifier : 31;
    UINT64 _pad1 : 1;
    UINT64 vmRegionSize : 13;
    UINT64 _pad2 : 3;
    UINT64 vmRegionPhysicalAddressBit : 1;
    UINT64 dualMonitorTreatmentSupport : 1;
    UINT64 vmcsMemoryType : 4;
    UINT64 insOutsExitInformation : 1;
    UINT64 trueControls : 1;
    UINT64 vmEntryHwExcepion : 1;
    UINT64 _pad3 : 7;
  };
} IA32_VmxBasic;


/**
 * @brief Structure for IA32_MTRRCAP MSR
 * @see [Intel SDM, Vol. 3A, Memory Cache Control](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_Mtrrcap
{
  UINT64 bits;
  struct
  {
    UINT64 variableRangeRegistersCount : 8;
    UINT64 fixedRangeRegistersSupported : 1;
    UINT64 _pad1 : 1;
    UINT64 wcSupported : 1;
    UINT64 smrrSupported : 1;
    UINT64 prmrrSupported : 1;
    UINT64 _pad2 : 51;
  };
} IA32_Mtrrcap;


/**
 * @brief Structure for IA32_MTRR_DEF_TYPE MSR
 * @see [Intel SDM, Vol. 3A, Memory Cache Control](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_MtrrDefType
{
  UINT64 bits;
  struct
  {
    UINT64 defaultMemoryType : 3;
    UINT64 _pad1 : 7;
    UINT64 fixedRangeMtrrEnable : 1;
    UINT64 mtrrEnable : 1;
    UINT64 _pad2 : 52;
  };
} IA32_MtrrDefType;


/**
 * @brief Structure for IA32_MTRR_PHYSBASE MSR
 * @see [Intel SDM, Vol. 3A, Memory Cache Control](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_MtrrPhysbase
{
  struct
  {
    UINT64 memoryType : 8;
    UINT64 _pad1 : 4;
    UINT64 physBase : 52;
  };
  UINT64 bits;
} IA32_MtrrPhysbase;


/**
 * @brief Structure for IA32_MTRR_PHYSMASK MSR
 * @see [Intel SDM, Vol. 3A, Memory Cache Control](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_MtrrPhysmask
{
  UINT64 bits;
  struct
  {
    UINT64 _pad1 : 11;
    UINT64 valid : 1;
    UINT64 physMask : 52;
  };
} IA32_MtrrPhysmask;


/**
 * @brief Structure for CR0 register
 * @see [Intel SDM, Vol. 3A, System Architecture Overview](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_Cr0 {
  UINT64 bits;
  struct
  {
    UINT64 protectionEnable : 1;
    UINT64 monitorCoprocessor : 1;
    UINT64 emultion : 1;
    UINT64 taskSwitched : 1;
    UINT64 extensionType : 1;
    UINT64 numericError : 1;
    UINT64 _pad1 : 10;
    UINT64 writeProtect : 1;
    UINT64 _pad2 : 1;
    UINT64 alignmentMask : 1;
    UINT64 _pad3 : 10;
    UINT64 notWriteThrough : 1;
    UINT64 cacheDisable : 1;
    UINT64 paging : 1;
    UINT64 _pad4 : 32;
  };
} IA32_Cr0;


/**
 * @brief Structure for CR3 register
 * @see [Intel SDM, Vol. 3A, System Architecture Overview](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_Cr3 {
  UINT64 bits;
  struct
  {
    UINT64 _pad1 : 3;
    UINT64 pageLevelWriteThrough : 1;
    UINT64 pageLevelCacheDisable : 1;
    UINT64 _pad2 : 7;
    UINT64 pageDirectoryBase : 52;
  };
} IA32_Cr3;


/**
 * @brief Structure for CR4 register
 * @see [Intel SDM, Vol. 3A, System Architecture Overview](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_Cr4 {
  UINT64 bits;
  struct
  {
    UINT64 virtualModeExtensions : 1;
    UINT64 protectedModeVirtualInterrupts : 1;
    UINT64 timeStampDisable : 1;
    UINT64 debuggingExtensions : 1;
    UINT64 pageSizeExtensions : 1;
    UINT64 physicalAddressExtension : 1;
    UINT64 machineCheckEnable : 1;
    UINT64 pageGlobalEnable : 1;
    UINT64 performanceCounterEnable : 1;
    UINT64 osSupportFxsaveFxrstor : 1;
    UINT64 osSupportUnmaskedSimdFPExceptions : 1;
    UINT64 userModeInstructionPrevention : 1;
    UINT64 linearAddres57Bit : 1;
    UINT64 vmxEnableBit : 1;
    UINT64 smxEnableBit : 1;
    UINT64 _pad2 : 1;
    UINT64 fsgsbaseEnableBit : 1;
    UINT64 pcidEnableBit : 1;
    UINT64 xsaveAndProcExtStateEnableBit : 1;
    UINT64 keyLockerEnableBit : 1;
    UINT64 smepEnableBit : 1;
    UINT64 smapEnableBit : 1;
    UINT64 enableProtKeysUserMode : 1;
    UINT64 controlFlowEnforcementTechnology : 1;
    UINT64 enableProtKeysSupervisorModePages : 1;
    UINT64 userInterruptsEnableBit : 1;
    UINT64 _pad3 : 38;
  };
} IA32_Cr4;


/**
 * @brief Structure for CPUID.0h
 * @see [Intel SDM, Vol. 3A, Chapter 3 Instruction Set Reference, A-L](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_CpuidBasicInformation0 {
  INT32 registers[4];
  struct
  {
    INT32 maxValueBasicInformation;
    INT32 vendor1;
    INT32 vendor3;
    INT32 vendor2;
  };
} IA32_CpuidBasicInformation0;


/**
 * @brief Structure for CPUID.1h
 * @see [Intel SDM, Vol. 3A, Chapter 3 Instruction Set Reference, A-L](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_CpuidBasicInformation1 {
  INT32 registers[4];
  struct
  {
    INT32 steppingId : 4;
    INT32 model : 4;
    INT32 familyId : 4;
    INT32 processorType : 2;
    INT32 _pad1 : 2;
    INT32 extendedModel : 4;
    INT32 extendedFamilyId : 8;
    INT32 _pad2 : 4;

    INT32 brandIndex : 8;
    INT32 cflushLineSize : 8;
    INT32 maximumAddressableIds : 8;
    INT32 initialApicId : 8;

    // There is a lot of features, see Intel SDM
    INT32 _otherFeatures1 : 5;
    INT32 vmx : 1;
    INT32 _otherFeatures2 : 25;
    INT32 hypervisorPresentBit : 1;

    INT32 _otherFeatures3 : 12;
    INT32 mtrr : 1;
    INT32 _otherFeatures4 : 19;
  };
} IA32_CpuidBasicInformation1;


/**
 * @brief Structure for CPUID.80000008h
 * @see [Intel SDM, Vol. 3A, Chapter 3 Instruction Set Reference, A-L](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_CpuidAddressBits
{
  INT32 registers[4];
  struct
  {
    INT32 physicalAddressBits : 8;
    INT32 linearAddressBits : 8;
    INT32 _pad1 : 16;

    INT32 _pad2 : 9;
    INT32 wbnoinvdIfOne : 1;
    INT32 _pad3 : 22;

    INT32 _pad4;
    INT32 _pad5;
  };
} IA32_CpuidAddressBits;


/**
 * @brief Structure for IA32_FEATURE_CONTROL MSR
 * @see [Intel SDM, Vol. 4, Chapter 2 Model-Specific Registers (MSRs)](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union IA32_FeatureControl
{
  UINT64 bits;
  struct
  {
    UINT64 lockBit : 1;
    UINT64 enableVmxInsideSmx : 1;
    UINT64 enableVmxOutsideSmx : 1;
    UINT64 _pad1 : 5;
    UINT64 senterLocalFunctionEnables : 7;
    UINT64 senterGlobalEnable : 1;
    UINT64 _pad2 : 1;
    UINT64 sgxLaunchControlEnable : 1;
    UINT64 sgxGlobalEnable : 1;
    UINT64 _pad3 : 1;
    UINT64 lmceOn : 1;
    UINT64 _pad4 : 43;
  };
}IA32_FeatureControl;
#pragma pack(pop)
#pragma warning(default:4201)
