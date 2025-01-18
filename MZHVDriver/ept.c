/**
 * @file ept.c
 * @brief EPT functions implementation
 * 
 * Functions related to EPT - setting up, changing and destroying mappings.
 */


#include <intrin.h>
#include "context.h"
#include "ept.h"
#include "ia32.h"
#include "memory.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static UINT64 getPml4Count(VOID);


static NTSTATUS setupPml4Entry(const UINT64 pml4EntryIndex, EPT_Pml4E* const pml4e);


static VOID fillVariableMtrrsCache(
  const UINT64 noOfVariableRangeRegisters,
  EPT_VariableMtrr* const variableMtrrs);


static NTSTATUS adjustPml4WithVariableMtrrs(
  const UINT64 noOfVariableRangeRegisters,
  const EPT_VariableMtrr* const variableMtrrs,
  const UINT64 defaultMemoryType,
  EPT_Pml4E* const pml4e);


static UINT64 searchVariableMtrrs(
  const UINT64 noOfVariableMtrrs,
  const EPT_VariableMtrr variableMtrrs[],
  const UINT64 defaultMemoryType,
  const UINT64 physicalAddress);


static NTSTATUS splitPage(EPT_PdE* const pde);


static VOID fillFixedMtrr(
  const UINT32 msrIndex,
  const UINT64 sizeInKBytes,
  EPT_PtE** const ptIteratorPointer);


static NTSTATUS overrideFirstPml4EntryWithFixedMtrrs(EPT_Pml4E* pml4);


static VOID destroyPml4(const UINT64 noOfPml4Entries, EPT_Pml4E* const pml4);


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Creates Extended Page Table Pointer with default (1-1) mapping.
 * 
 * This function allocates and initializez EPT structures, sets their memory type and returns
 * Extended Page Table Pointer bits.
 * 
 * @param eptpBits Pointer to variable where EPTP bits should be stored.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
NTSTATUS EPT_setupDefaltStructures(UINT64* const eptpBits)
{
  EPT_Pml4E* const pml4 = Memory_allocate(sizeof(EPT_Pml4E[EPT_PML4_ENTRIES]), TRUE);
  if (pml4 == NULL)
  {
    return STATUS_UNSUCCESSFUL;
  }

  NTSTATUS ntStatus;
  const UINT64 pml4Count = getPml4Count();
  for (UINT64 pml4Index = 0; pml4Index < pml4Count; pml4Index++)
  {
    ntStatus = setupPml4Entry(pml4Index, &pml4[pml4Index]);
    if (!NT_SUCCESS(ntStatus))
    {
      destroyPml4(pml4Index, pml4);
      return ntStatus;
    }
  }

  IA32_CpuidBasicInformation1 cpuid = { 0 };
  __cpuid(cpuid.registers, IA32_CPUID_BASIC_INFORMATION_1);
  if (cpuid.mtrr)
  {
    const IA32_MtrrDefType mtrrDefType = { .bits = __readmsr(IA32_MTRR_DEF_TYPE) };
    if (mtrrDefType.mtrrEnable)
    {
      EPT_VariableMtrr variableMtrrs[IA32_MTRR_MAX_VMTRR_COUNT] = { 0 };
      const IA32_Mtrrcap mtrrCap = { .bits = __readmsr(IA32_MTRRCAP) };
      fillVariableMtrrsCache(mtrrCap.variableRangeRegistersCount, variableMtrrs);

      for (UINT64 pml4EntryIndex = 0; pml4EntryIndex < pml4Count; pml4EntryIndex++)
      {
        ntStatus = adjustPml4WithVariableMtrrs(
          mtrrCap.variableRangeRegistersCount,
          variableMtrrs, 
          mtrrDefType.defaultMemoryType,
          &pml4[pml4EntryIndex]);
        if (!NT_SUCCESS(ntStatus))
        {
          destroyPml4(pml4Count, pml4);
          return ntStatus;
        }
      }

      if (mtrrCap.fixedRangeRegistersSupported && mtrrDefType.fixedRangeMtrrEnable)
      {
        ntStatus = overrideFirstPml4EntryWithFixedMtrrs(pml4);
        if (!NT_SUCCESS(ntStatus))
        {
          destroyPml4(pml4Count, pml4);
          return ntStatus;
        }
      }
    }
  }

  *eptpBits = (EPT_EptP)
  {
    .memoryType = EPT_PAGING_STRUCTURE_MEMORY_TYPE_WB,
    .oneLessPageWalkLen = EPT_PAGE_WALK_LEN - 1,
    .pageFrameNumber = (EPT_Address) {.address = Memory_getPhysicalAddress(pml4) }.pageFrameNumber4KB
  }.bits;

  return STATUS_SUCCESS;
}


/**
 * @brief Changes mapping of a given address.
 * 
 * Changes mapping of a given source address to a given target address. It also
 * sets read/write and fetch permissions.
 * 
 * @param sourceAddress Source address.
 * @param targetAddress Target address.
 * @param rw Read/write permissions.
 * @param fetch Fetch permissions.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
NTSTATUS EPT_changeMapping(
  const UINT64 sourceAddress,
  const UINT64 targetAddress,
  const BOOLEAN rw,
  const BOOLEAN fetch)
{
  const EPT_EptP eptp = (EPT_EptP){ .bits = Context_getLogicalCore()->eptp };

  const EPT_Address eptAddress = { .address = sourceAddress };

  const EPT_Pml4E* const pml4 = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = eptp.pageFrameNumber
  }.address);
  const EPT_PdptE* const pdpt = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pml4[eptAddress.pml4Entry].pageFrameNumber
  }.address);
  EPT_PdE* const pd = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pdpt[eptAddress.pdptEntry].pageFrameNumber
  }.address);
  EPT_PdE* const pde = &pd[eptAddress.pdEntry];

  if (pde->largePage.isLargePage)
  {
    NTSTATUS ntStatus = splitPage(pde);
    if (!NT_SUCCESS(ntStatus))
    {
      return ntStatus;
    }
  }

  EPT_PtE* const pt = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pde->standard.pageFrameNumber
  }.address);

  pt[eptAddress.ptEntry].pageFrameNumber = (EPT_Address){
    .address = targetAddress
  }.pageFrameNumber4KB;
  pt[eptAddress.ptEntry].readAccess = rw;
  pt[eptAddress.ptEntry].writeAccess = rw;
  pt[eptAddress.ptEntry].fetchAccess = fetch;

  return STATUS_SUCCESS;
}


/**
 * @brief Frees EPT structures.
 * 
 * Frees memory allocated for EPT structures allocated with EPT_setupDefaltStructures.
 * This function is designed for external use.
 * 
 * @param eptpBits Extended Page Table Pointer bits.
 * 
 * @return VOID
 */
VOID EPT_destroyEPTStructure(const UINT64 eptpBits)
{
  const EPT_EptP eptp = (EPT_EptP){ .bits = eptpBits };
  EPT_Pml4E* const pml4 = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = eptp.pageFrameNumber
  }.address);
  const UINT64 pml4Count = getPml4Count();
  destroyPml4(pml4Count, pml4);
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Returns number of PML4 entries.
 * 
 * Calculates required number of PML4 entries based on CPUID information.
 * 
 * @return Number of PML4 entries.
 */
static UINT64 getPml4Count(VOID)
{
  IA32_CpuidAddressBits cpuid = { 0 };
  __cpuid(cpuid.registers, IA32_CPUID_ADDRESS_BITS);

  // Memory range mapped by 1 PML4 entry = 512 GB (2^39)
  const UINT64 shiftValue = max(cpuid.physicalAddressBits - 39, 0);
  const UINT64 pml4Count = min((1U << shiftValue), EPT_WINDOWS_MAX_PML4_COUNT);

  return pml4Count;
}


/**
 * @brief Initializes PML4 entry.
 * 
 * This function initializes PML4 entry, allocating memory for PDPT and PD structures.
 * The memory type is set to UC.
 * 
 * @param pml4EntryIndex Index of PML4 entry to initialize.
 * @param pml4e Pointer to PML4 entry to initialize.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
static NTSTATUS setupPml4Entry(const UINT64 pml4EntryIndex, EPT_Pml4E* const pml4e)
{
  EPT_PdptE* const pdpt = Memory_allocate(sizeof(EPT_PdptE[EPT_PDPT_ENTRIES]), TRUE);
  if (pdpt == NULL)
  {
    return STATUS_UNSUCCESSFUL;
  }

  *pml4e = (EPT_Pml4E)
  {
    .readAccess = TRUE,
    .writeAccess = TRUE,
    .fetchAccess = TRUE,
    .pageFrameNumber = (EPT_Address) {.address = Memory_getPhysicalAddress(pdpt) }.pageFrameNumber4KB
  };

  // Allocate 512 Page Directories at once
  EPT_PdE* const pdArray = Memory_allocate(EPT_PDPT_ENTRIES * sizeof(EPT_PdE[EPT_PD_ENTRIES]), TRUE);
  if (pdArray == NULL)
  {
    Memory_free(pdpt);
    return STATUS_UNSUCCESSFUL;
  }

  for (UINT64 pdptEntryIndex = 0; pdptEntryIndex < EPT_PDPT_ENTRIES; pdptEntryIndex++)
  {
    EPT_PdE* const pd = &pdArray[pdptEntryIndex * EPT_PD_ENTRIES];
    pdpt[pdptEntryIndex] = (EPT_PdptE)
    {
      .readAccess = TRUE,
      .writeAccess = TRUE,
      .fetchAccess = TRUE,
      .pageFrameNumber = (EPT_Address)
        {.address = Memory_getPhysicalAddress(pd) }.pageFrameNumber4KB
    };

    for (UINT64 pdEntryIndex = 0; pdEntryIndex < EPT_PD_ENTRIES; pdEntryIndex++)
    {
      const EPT_Address address = (EPT_Address)
      {
          .pml4Entry = pml4EntryIndex, .pdptEntry = pdptEntryIndex, .pdEntry = pdEntryIndex
      };

      pd[pdEntryIndex].largePage = (EPT_PdE2MB)
      {
        .readAccess = TRUE,
        .writeAccess = TRUE,
        .fetchAccess = TRUE,
        .isLargePage = TRUE,
        .memoryType = IA32_MTRR_DISABLED_DEFAULT_TYPE,
        .pageFrameNumber = address.pageFrameNumber2MB
      };
    }
  }

  return STATUS_SUCCESS;
}


/**
 * @brief Fills variable MTRRs cache.
 * 
 * This function caches MTRR information in a local array.
 * 
 * @param noOfVariableRangeRegisters Number of variable range registers.
 * @param variableMtrrs Array where MTRR information will be stored.
 * 
 * @return VOID
 */
static VOID fillVariableMtrrsCache(
  const UINT64 noOfVariableRangeRegisters,
  EPT_VariableMtrr* const variableMtrrs)
{
  for (UINT32 mtrrIndex = 0; mtrrIndex < noOfVariableRangeRegisters; mtrrIndex++)
  {
    const IA32_MtrrPhysbase physbase =
    { .bits = __readmsr(IA32_MTRR_PHYSBASE0 + IA32_MTRR_PHYSBASE0_INC * mtrrIndex) };
    const IA32_MtrrPhysmask physmask =
    { .bits = __readmsr(IA32_MTRR_PHYSMASK0 + IA32_MTRR_PHYSMASK0_INC * mtrrIndex) };

    if (!physmask.valid)
    {
      continue;
    }

    ULONG firstOneIndex = { 0 };
    BOOLEAN isNonzero = _BitScanForward64(&firstOneIndex, (EPT_Address)
    {
      .pageFrameNumber4KB = physmask.physMask
    }.address);

    if (!isNonzero)
    {
      continue;
    }

    variableMtrrs[mtrrIndex] = (EPT_VariableMtrr)
    {
      .baseAddress = (EPT_Address) {.pageFrameNumber4KB = physbase.physBase }.address,
      .length = 1LLU << firstOneIndex,
      .memoryType = physbase.memoryType,
      .valid = TRUE,
    };
  }
}


/**
 * @brief Adjusts PML4 with variable MTRRs.
 * 
 * This function adjusts memory type of PML4 entry with variable MTRR values.
 * 
 * @param noOfVariableRangeRegisters Number of variable range registers.
 * @param variableMtrrs Array with MTRR information.
 * @param defaultMemoryType Default memory type.
 * @param pml4e Pointer to PML4 entry to adjust.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
static NTSTATUS adjustPml4WithVariableMtrrs(
  const UINT64 noOfVariableRangeRegisters,
  const EPT_VariableMtrr* const variableMtrrs,
  const UINT64 defaultMemoryType,
  EPT_Pml4E* const pml4e)
{
  const EPT_PdptE* const pdpt = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pml4e->pageFrameNumber
  }.address);

  for (UINT64 pdptEntryIndex = 0; pdptEntryIndex < EPT_PDPT_ENTRIES; pdptEntryIndex++)
  {
    EPT_PdE* const pd = Memory_getVirtualAddress((EPT_Address) {
      .pageFrameNumber4KB = pdpt[pdptEntryIndex].pageFrameNumber
    }.address);

    for (UINT64 pdEntryIndex = 0; pdEntryIndex < EPT_PD_ENTRIES; pdEntryIndex++)
    {
      const UINT64 address = (EPT_Address){
        .pageFrameNumber2MB = pd[pdEntryIndex].largePage.pageFrameNumber
      }.address;

      pd[pdEntryIndex].largePage.memoryType =
        searchVariableMtrrs(noOfVariableRangeRegisters, variableMtrrs, defaultMemoryType, address);
      if (pd[pdEntryIndex].largePage.memoryType == IA32_MTRR_MEMORY_TYPE_ERR)
      {
        return STATUS_UNSUCCESSFUL;
      }
    }
  }

  return STATUS_SUCCESS;
}


/**
 * @brief Searches MTRRs for given address' memory type.
 * 
 * @param noOfVariableMtrrs Number of variable MTRRs.
 * @param variableMtrrs Array with MTRR information.
 * @param defaultMemoryType Default memory type.
 * @param physicalAddress Physical address.
 * 
 * @return Memory type, or IA32_MTRR_MEMORY_TYPE_ERR on error.
 */
static UINT64 searchVariableMtrrs(
  const UINT64 noOfVariableMtrrs,
  const EPT_VariableMtrr variableMtrrs[],
  const UINT64 defaultMemoryType,
  const UINT64 physicalAddress)
{
  EPT_FoundMemoryTypes foundTypes = { 0 };

  for (UINT64 mtrrIndex = 0; mtrrIndex < noOfVariableMtrrs; mtrrIndex++)
  {
    const EPT_VariableMtrr* currentMtrr = &variableMtrrs[mtrrIndex];
    if (!currentMtrr->valid)
    {
      continue;
    }

    if (!((currentMtrr->baseAddress <= physicalAddress) && (physicalAddress < (currentMtrr->baseAddress + currentMtrr->length))))
    {
      continue;
    }

    switch (currentMtrr->memoryType)
    {
    case IA32_MTRR_MEMORY_TYPE_UC:
    {
      foundTypes.uncacheable = 1;
      break;
    }
    case IA32_MTRR_MEMORY_TYPE_WC:
    {
      foundTypes.writeCombining = 1;
      break;
    }
    case IA32_MTRR_MEMORY_TYPE_WT:
    {
      foundTypes.writeThrough = 1;
      break;
    }
    case IA32_MTRR_MEMORY_TYPE_WP:
    {
      foundTypes.writeProtected = 1;
      break;
    }
    case IA32_MTRR_MEMORY_TYPE_WB:
    {
      foundTypes.writeback = 1;
      break;
    }
    default:
    {
      return IA32_MTRR_MEMORY_TYPE_ERR;
      break;
    }
    }
  }

  if ((foundTypes.uncacheable + foundTypes.writeCombining + foundTypes.writeThrough + foundTypes.writeProtected + foundTypes.writeback) == 1)
  {
    if (foundTypes.uncacheable)
      return IA32_MTRR_MEMORY_TYPE_UC;
    if (foundTypes.writeCombining)
      return IA32_MTRR_MEMORY_TYPE_WC;
    if (foundTypes.writeThrough)
      return IA32_MTRR_MEMORY_TYPE_WT;
    if (foundTypes.writeProtected)
      return IA32_MTRR_MEMORY_TYPE_WP;
    if (foundTypes.writeback)
      return IA32_MTRR_MEMORY_TYPE_WB;
  }

  if (foundTypes.uncacheable)
  {
    return IA32_MTRR_MEMORY_TYPE_UC;
  }

  if (foundTypes.writeThrough == 1 && foundTypes.writeback == 1 && ((foundTypes.uncacheable + foundTypes.writeCombining + foundTypes.writeProtected) == 0))
  {
    return IA32_MTRR_MEMORY_TYPE_WT;
  }

  if ((foundTypes.uncacheable + foundTypes.writeCombining + foundTypes.writeThrough + foundTypes.writeProtected + foundTypes.writeback) != 0)
  {
    return IA32_MTRR_MEMORY_TYPE_ERR;
  }

  return defaultMemoryType;
}


/**
 * @brief Splits large page into small pages.
 * 
 * This function splits large (2MB) mapped by this PDE into 512 small (4KB) pages
 * mapped by a single PT.
 * 
 * @param pde Pointer to Page Directory Entry.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
static NTSTATUS splitPage(EPT_PdE* const pde)
{
  Context_EptMappingsData* const mappingData = &Context_getLogicalCore()->eptMappingData;
  if (mappingData->noOfUsedSplits >= CONTEXT_EPT_NO_SPLITS)
  {
    return STATUS_UNSUCCESSFUL;
  }

  EPT_PtE* const pte =
    (EPT_PtE*)&mappingData->splitBuffer[CONTEXT_EPT_SPLIT_SIZE * mappingData->noOfUsedSplits];
  mappingData->noOfUsedSplits++;

  const UINT64 memoryType = pde->largePage.memoryType;
  const UINT64 pageFrameNumber = pde->largePage.pageFrameNumber;

  for (UINT64 ptEntryIndex = 0; ptEntryIndex < EPT_PT_ENTRIES; ptEntryIndex++)
  {
    pte[ptEntryIndex] = (EPT_PtE)
    {
      .readAccess = TRUE,
      .writeAccess = TRUE,
      .fetchAccess = TRUE,
      .memoryType = memoryType,
      .pageFrameNumber = EPT_PT_ENTRIES * pageFrameNumber + ptEntryIndex
    };
  }

  pde->standard = (EPT_PagingStructure)
  {
    .writeAccess = TRUE,
    .readAccess = TRUE,
    .fetchAccess = TRUE,
    .pageFrameNumber = (EPT_Address)
    {.address = Memory_getPhysicalAddress(pte) }.pageFrameNumber4KB
  };

  return STATUS_SUCCESS;
}


/**
 * @brief Fills PTE with fixed MTRRs.
 * 
 * This function fills PTE memory types with fixed MTRR's values. Helper function used by
 * overrideFirstPml4EntryWithFixedMtrrs.
 * 
 * @param msrIndex MSR index.
 * @param sizeInKBytes Size in KB.
 * @param ptIteratorPointer Pointer to PTE iterator.
 * 
 * @return VOID
 */
static VOID fillFixedMtrr(
  const UINT32 msrIndex,
  const UINT64 sizeInKBytes,
  EPT_PtE** const ptIteratorPointer)
{
  const UINT64 msr = __readmsr(msrIndex);
  for (UINT64 msrByteIndex = 0; msrByteIndex < 8; msrByteIndex++)
  {
    const UINT64 memoryType = (msr >> (msrByteIndex * 8)) & 0xFF;
    for (UINT64 mappedPageCounter = 0; mappedPageCounter < (sizeInKBytes / 4); mappedPageCounter++)
    {
      (*ptIteratorPointer)->memoryType = memoryType;
      *ptIteratorPointer = *ptIteratorPointer + 1;
    }
  }
}


/**
 * @brief Overrides first 1MB of memory with fixed MTRRs.
 * 
 * This function overrides first 1MB of memory' memory type with fixed MTRR's values.
 * 
 * @param pml4 Pointer to PML4.
 * 
 * @return STATUS_SUCCESS on success, error code otherwise.
 */
static NTSTATUS overrideFirstPml4EntryWithFixedMtrrs(EPT_Pml4E* pml4)
{
  const EPT_PdptE* const pdpt = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pml4[0].pageFrameNumber
  }.address);
  EPT_PdE* const pd = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pdpt[0].pageFrameNumber
  }.address);
  EPT_PdE* const pde = &pd[0];

  NTSTATUS ntStatus = splitPage(pde);
  if (!NT_SUCCESS(ntStatus))
  {
    return ntStatus;
  }

  EPT_PtE* const pt = Memory_getVirtualAddress((EPT_Address) {
    .pageFrameNumber4KB = pde->standard.pageFrameNumber
  }.address);
  EPT_PtE* ptIterator = &pt[0];

  fillFixedMtrr(IA32_MTRR_FIX64K_00000, 64, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX16K_80000, 16, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX16K_A0000, 16, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_C0000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_C8000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_D0000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_D8000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_E0000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_E8000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_F0000, 4, &ptIterator);
  fillFixedMtrr(IA32_MTRR_FIX4K_F8000, 4, &ptIterator);

  return STATUS_SUCCESS;
}


/**
 * @brief Destroys PML4 for internal use.
 * 
 * Internal function that frees memory allocated for PML4 and all underlying structures.
 * The noOfPml4Entries parameter limits number of PML4 entries to destroy (since not all
 * of them may have been allocated).
 * 
 * @param noOfPml4Entries Number of PML4 entries.
 * @param pml4 Pointer to PML4.
 * 
 * @return VOID
 */
static VOID destroyPml4(const UINT64 noOfPml4Entries, EPT_Pml4E* const pml4)
{
  for (UINT64 pml4EntryIndex = 0; pml4EntryIndex < noOfPml4Entries; pml4EntryIndex++)
  {
    EPT_PdptE* const pdpt = Memory_getVirtualAddress(
      (EPT_Address) {
      .pageFrameNumber4KB = pml4[pml4EntryIndex].pageFrameNumber
    }.address);
    EPT_PdE* const pdArray = Memory_getVirtualAddress(
      (EPT_Address) {
      .pageFrameNumber4KB = pdpt[0].pageFrameNumber
    }.address);

    Memory_free(pdArray);
    Memory_free(pdpt);
  }

  Memory_free(pml4);
}
