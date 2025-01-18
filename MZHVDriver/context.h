/**
 * @file context.h
 * @brief Defines context data structures and declares its functions.
 * 
 * Context is a global structure that contains processor information.
 * It is used to store VMXON, VMCS regions, MSR bitmaps, mappings data, etc.
 */


#pragma once


#include <ntddk.h>
#include "ia32.h"


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name Size and alignment
 * @brief Size and alignment requirements for context data structures.
 * @anchor CONTEXTSizeAndAlignment
 */
 ///@{
#define CONTEXT_ALIGN_REQUIREMENT     4096
#define CONTEXT_VMXON_REGION_SIZE     4096
#define CONTEXT_VMCS_REGION_SIZE      4096
#define CONTEXT_MSR_BITMAP_SIZE       4096
#define CONTEXT_ROOT_MODE_STACK_SIZE  32768
///@}

/**
 * @name Split limits
 * @brief Page split limits for EPT mappings.
 * @anchor CONTEXTSplitLimits
 */
 ///@{
#define CONTEXT_EPT_SPLIT_SIZE        4096
#define CONTEXT_EPT_NO_SPLITS         32
#define CONTEXT_EPT_MAX_MAPPINGS      32
///@}

/**************************************************************************************************
* Type declarations
**************************************************************************************************/
/**
 * @brief Structure defining a single changed EPT mapping.
 * 
 * @param guestAddress Guest physical address.
 * @param hostRwAddress Physical address for read/write access.
 * @param hostFetchAddress Physical address for fetch access.
 * @param valid TRUE if mapping is valid, FALSE otherwise.
 */
typedef struct Context_EptChangedMapping
{
  UINT64 guestAddress;
  UINT64 hostRwAddress;
  UINT64 hostFetchAddress;
  BOOLEAN valid;
} Context_EptChangedMapping;


/**
 * @brief Structure that combines the split buffer and changed mappings.
 * 
 * @param splitBuffer Buffer used to split EPT large pages.
 * @param noOfUsedSplits Number of already used splits.
 * @param changedMappings Array of changed mappings.
 */
typedef struct Context_EptMappingsData
{
  __declspec(align(CONTEXT_ALIGN_REQUIREMENT)) CHAR
    splitBuffer[CONTEXT_EPT_SPLIT_SIZE * CONTEXT_EPT_NO_SPLITS];
  UINT64 noOfUsedSplits;
  Context_EptChangedMapping changedMappings[CONTEXT_EPT_MAX_MAPPINGS];
} Context_EptMappingsData;


/**
 * @brief Structure defining a single core's context.
 * 
 * @param vmxonRegion VMXON region.
 * @param vmcsRegion MCS region.
 * @param msrBitmap MSR bitmap.
 * @param rootModeStack Root mode stack.
 * @param eptMappingData EPT mappings data.
 * @param eptp Extended Page Table Pointer bit representation.
 * @param isVirtualized TRUE if logical core is virtualized, FALSE otherwise.
 */
typedef struct Context_LogicalCore
{
  __declspec(align(CONTEXT_ALIGN_REQUIREMENT)) CHAR vmxonRegion[CONTEXT_VMXON_REGION_SIZE];
  __declspec(align(CONTEXT_ALIGN_REQUIREMENT)) CHAR vmcsRegion[CONTEXT_VMCS_REGION_SIZE];
  __declspec(align(CONTEXT_ALIGN_REQUIREMENT)) CHAR msrBitmap[CONTEXT_MSR_BITMAP_SIZE];
  __declspec(align(CONTEXT_ALIGN_REQUIREMENT)) CHAR rootModeStack[CONTEXT_ROOT_MODE_STACK_SIZE];
  Context_EptMappingsData eptMappingData;
  UINT64 eptp;
  BOOLEAN isVirtualized;
} Context_LogicalCore;


/**
 * @brief Structure defining whole processor context.
 *
 * @param systemCR3 CR3 value of the system.
 * @param noOfLogicalCores Number of logical cores in the system.
 * @param logicalCores Array of logical cores' contexts.
 */
#pragma warning(disable:4200)
typedef struct Context_Context
{
  IA32_Cr3 systemCR3;
  UINT64 noOfLogicalCores;
  Context_LogicalCore logicalCores[];
} Context_Context;
#pragma warning(default:4200)


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS Context_init(VOID);


Context_Context* Context_getContext(VOID);


Context_LogicalCore* Context_getLogicalCore(VOID);


VOID Context_destroy(VOID);
