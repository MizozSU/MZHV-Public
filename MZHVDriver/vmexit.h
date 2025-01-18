/**
 * @file vmexit.h
 * @brief VMEXIT structures and definitions
 */



#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name VMEXIT reason definitions
 * @anchor VMEXITReasons
 * @see [Intel SDM, Vol. 3C, Appendix C](https://software.intel.com/en-us/articles/intel-sdm)
 */
 ///@{
#define VMEXIT_CPUID          10
#define VMEXIT_VMCALL         18
#define VMEXIT_EPT_VIOLATION  48
///@}

/**
 * @name VMCALL codes
 * @brief Hypervisor specific VMCALL codes
 * @anchor VMCALLCodes
 */
///@{
#define VMEXIT_VMCALL_INITIATE_SHUTDOWN  0xFFFFFFFF00000000
#define VMEXIT_VMCALL_MAP_PAGE           0xF1337
#define VMEXIT_VMCALL_UNMAP_PAGE         0xF2137
///@}


/**************************************************************************************************
* Type declarations
**************************************************************************************************/
#pragma warning(disable:4201)
#pragma pack(push, 1)
/**
 * @brief Guest register state
 *
 * @note This structure is used to save and restore guest register state
 */
typedef struct VMEXIT_Registers
{
  UINT64 R15;
  UINT64 R14;
  UINT64 R13;
  UINT64 R12;
  UINT64 R11;
  UINT64 R10;
  UINT64 R9;
  UINT64 R8;
  UINT64 RDI;
  UINT64 RSI;
  UINT64 RSP;
  UINT64 RBP;
  UINT64 RDX;
  UINT64 RCX;
  UINT64 RBX;
  UINT64 RAX;
} VMEXIT_Registers;


/**
 * @brief Structure for VMEXIT reason
 * @see [Intel SDM, Vol. 3C, Chapter 28 VM Exits](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMEXIT_ExitReason
{
  UINT32 bits;
  struct
  {
    UINT32 basicExitReason : 16;
    UINT32 _pad1 : 11;
    UINT32 enclaveMode : 1;
    UINT32 pendingMtf : 1;
    UINT32 exitFromVmxRoot : 1;
    UINT32 _pad2 : 1;
    UINT32 vmEntryFailure : 1;
  };
} VMEXIT_ExitReason;


/**
 * @brief Structure for VMEXIT qualification
 * @see [Intel SDM, Vol. 3C, Chapter 28 VM Exits](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union VMEXIT_EptViolation
{
  UINT64 bits;
  struct
  {
    UINT64 dataRead : 1;
    UINT64 dataWrite : 1;
    UINT64 instructionFetch : 1;
    UINT64 addressReadable : 1;
    UINT64 addresWriteable : 1;
    UINT64 addresExecutable : 1;
    // There are more advanced fields in Exit Qualification for EPT Violations
    // See Table 28-7 in Intel SDM for details
    UINT64 _pad1 : 58;
  };
} VMEXIT_EptViolation;
#pragma pack(pop)
#pragma warning(default:4201)

/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
BOOLEAN VMEXIT_handler(VMEXIT_Registers* registers);
