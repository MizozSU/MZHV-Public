/**
 * @file ept.h
 * @brief EPT data types definitions and function declarations.
 * 
 * This file contains definitions of structures from Intel SDM.
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name EPT constants
 * @brief EPT memory types and page structure size.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 * @anchor EPTConstants
 */
///@{
#define EPT_PAGING_STRUCTURE_MEMORY_TYPE_UC   0
#define EPT_PAGING_STRUCTURE_MEMORY_TYPE_WB   6
#define EPT_PAGE_WALK_LEN                     4
///@}

/**
 * @name EPT entries per table
 * @anchor EPTEntriesPerTable
 * @brief Number of entries per table.
 */
///@{
#define EPT_PML4_ENTRIES                      512
#define EPT_PDPT_ENTRIES                      512
#define EPT_PD_ENTRIES                        512
#define EPT_PT_ENTRIES                        512
///@}

/**
 * @name EPT-Windows constant
 * @brief The upper limit for PML4 entries, because Windows can manage up to 2TB of memory.
 * @anchor EPTWindowsConstant
 */
///@{
#define EPT_WINDOWS_MAX_PML4_COUNT            4
///@}


/**************************************************************************************************
* Type declarations
**************************************************************************************************/
/**
 * @brief Variable MTRR cache helper structure.
 * 
 * @param baseAddress Base address
 * @param length Length
 * @param memoryType Memory type
 * @param valid TRUE if valid
 */
#pragma warning(disable:4201)
#pragma pack(push, 1)
typedef struct EPT_VariableMtrr
{
  UINT64 baseAddress;
  UINT64 length;
  UINT64 memoryType;
  BOOLEAN valid;
} EPT_VariableMtrr;


/**
 * @brief Helper structure for found memory types.
 * 
 * @param uncacheable TRUE if uncacheable
 * @param writeCombining TRUE if write combining
 * @param writeThrough TRUE if write through
 * @param writeProtected TRUE if write protected
 * @param writeback TRUE if writeback
 */
typedef struct EPT_FoundMemoryTypes
{
  UINT8 uncacheable : 1;
  UINT8 writeCombining : 1;
  UINT8 writeThrough : 1;
  UINT8 writeProtected : 1;
  UINT8 writeback : 1;
  UINT8 _pad1 : 3;
} EPT_FoundMemoryTypes;


/**
 * @brief Common EPT paging structure.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef struct EPT_PagingStructure
{
  UINT64 readAccess : 1;
  UINT64 writeAccess : 1;
  UINT64 fetchAccess : 1;
  UINT64 _pad1 : 5;
  UINT64 accessed : 1;
  UINT64 _pad2 : 1;
  UINT64 fetchAccessUserMode : 1;
  UINT64 _pad3 : 1;
  UINT64 pageFrameNumber : 40;
  UINT64 _pad4 : 12;
} EPT_PagingStructure;


/**
 * @brief Union for PML4 entry.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_Pml4E
{
  UINT64 bits;
  EPT_PagingStructure;
} EPT_Pml4E;


/**
 * @brief Union for PDPT entry.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_PdptE
{
  UINT64 bits;
  EPT_PagingStructure;
} EPT_PdptE;


/**
 * @brief EPT PD entry for 2MB pages.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_PdE2MB
{
  UINT64 bits;
  struct
  {
    UINT64 readAccess : 1;
    UINT64 writeAccess : 1;
    UINT64 fetchAccess : 1;
    UINT64 memoryType : 3;
    UINT64 ignorePATMemoryType : 1;
    UINT64 isLargePage : 1;
    UINT64 accessed : 1;
    UINT64 dirty : 1;
    UINT64 fetchAccessUserMode : 1;
    UINT64 _pad1 : 10;
    UINT64 pageFrameNumber : 31;
    UINT64 _pad2 : 5;
    UINT64 verifyGuestPaging : 1;
    UINT64 pagingWriteAccess : 1;
    UINT64 _pad3 : 1;
    UINT64 supervisorShadowStack : 1;
    UINT64 _pad4 : 2;
    UINT64 supressVE : 1;
  };
} EPT_PdE2MB;


/**
 * @brief Union for PD entry.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_PdE
{
  UINT64 bits;
  EPT_PagingStructure standard;
  EPT_PdE2MB largePage;
} EPT_PdE;


/**
 * @brief EPT PT entry mapping 4KB page.
 * @see [Intel SDM, Vol. 3C, Chapter 29 VMX Support for Address Translation](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_PtE
{
  UINT64 bits;
  struct
  {
    UINT64 readAccess : 1;
    UINT64 writeAccess : 1;
    UINT64 fetchAccess : 1;
    UINT64 memoryType : 3;
    UINT64 ignorePATMemoryType : 1;
    UINT64 _pad1 : 1;
    UINT64 accessed : 1;
    UINT64 dirty : 1;
    UINT64 fetchAccessUserMode : 1;
    UINT64 _pad2 : 1;
    UINT64 pageFrameNumber : 40;
    UINT64 _pad3 : 5;
    UINT64 verifyGuestPaging : 1;
    UINT64 pagingWriteAccess : 1;
    UINT64 _pad4 : 1;
    UINT64 supervisorShadowStack : 1;
    UINT64 subPageWritePermissions : 1;
    UINT64 _pad5 : 1;
    UINT64 supressVE : 1;
  };
} EPT_PtE;


/**
 * @brief Represents the EPT pointer.
 * @see [Intel SDM, Vol. 3C, Chapter 25 Virtual Machine Control Structure](https://software.intel.com/en-us/articles/intel-sdm)
 */
typedef union EPT_EptP
{
  UINT64 bits;
  struct
  {
    UINT64 memoryType : 3;
    UINT64 oneLessPageWalkLen : 3;
    UINT64 accessedDirtyEnable : 1;
    UINT64 accessRightsEnforcement : 1;
    UINT64 _pad1 : 4;
    UINT64 pageFrameNumber : 40;
    UINT64 _pad2 : 12;
  };
} EPT_EptP;


/**
 * @brief Helper union for converting addresses for EPT purposes.
 * 
 * @param address Whole address
 * @param offset Offset
 * @param ptEntry PT entry Index
 * @param pdEntry PD entry Index
 * @param pdptEntry PDPT entry Index
 * @param pml4Entry PML4 entry Index
 * 
 * @param pageFrameNumber4KB Page frame number for 4KB page
 * @param pageFrameNumber2MB Page frame number for 2MB page
 */
typedef union EPT_Address
{
  UINT64 address;
  struct
  {
    UINT64 offset : 12;
    UINT64 ptEntry : 9;
    UINT64 pdEntry : 9;
    UINT64 pdptEntry : 9;
    UINT64 pml4Entry : 9;
    UINT64 _pad1 : 16;
  };
  struct
  {
    UINT64 _pad2 : 12;
    UINT64 pageFrameNumber4KB : 52;
  };
  struct
  {
    UINT64 _pad3 : 21;
    UINT64 pageFrameNumber2MB : 43;
  };
} EPT_Address;
#pragma pack(pop)
#pragma warning(default:4201)


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS EPT_setupDefaltStructures(UINT64* const eptpBits);


NTSTATUS EPT_changeMapping(
  const UINT64 sourceAddress,
  const UINT64 targetAddress,
  const BOOLEAN rw,
  const BOOLEAN fetch);


VOID EPT_destroyEPTStructure(const UINT64 eptpBits);
