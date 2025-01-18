/**
 * @file memory.c
 * @brief Memory management functions.
 *
 * Functions in this file are used to allocate, copy, compare and free memory. They are an overlay
 * for functions provided by the Windows Driver Kit (WDK). This overlay is used to provide
 * additional functionality, such as alignment checks.
 */


#include "bsod.h"
#include "memory.h"


#if 4096 != PAGE_SIZE
#error Intel requires 4KB alignment of specific regions, memory.c allocator only provides PAGE_SIZE alignment
#endif


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Allocates memory.
 * 
 * Allocates memory of NON_PAGED_EXECUTE type. If the aligned flag is set, the memory
 * is aligned to PAGE_SIZE.
 * 
 * @param noOfBytes Number of bytes to allocate.
 * @param aligned   Whether memory should be aligned to PAGE_SIZE.
 * 
 * @return Allocated memory or NULL if the allocation failed.
 */
VOID* Memory_allocate(const SIZE_T noOfBytes, const BOOLEAN aligned)
{
  const SIZE_T noOfBytesToAllocate = (noOfBytes < PAGE_SIZE && aligned) ? PAGE_SIZE : noOfBytes;

  // Must be POOL_FLAG_NON_PAGED_EXECUTE so MmGetVirtualForPhysical works
  VOID* const allocatedMemory =
    ExAllocatePool2(POOL_FLAG_NON_PAGED_EXECUTE, noOfBytesToAllocate, MEMORY_POOL_TAG);
  if (allocatedMemory == NULL)
  {
    return NULL;
  }

  if (aligned)
  {
    const BOOLEAN virtualAddressAligned = (allocatedMemory == PAGE_ALIGN(allocatedMemory));
    const BOOLEAN physicalAddressAligned =
      ((Memory_getPhysicalAddress(allocatedMemory) & (PAGE_SIZE - 1)) == 0);

    if (!virtualAddressAligned || !physicalAddressAligned)
    {
      ExFreePoolWithTag(allocatedMemory, MEMORY_POOL_TAG);
      return NULL;
    }
  }

  return allocatedMemory;
}


/**
 * @brief Copies memory.
 * 
 * Copies memory from source to destination.
 * 
 * @param destination Destination address.
 * @param source      Source address.
 * @param length      Number of bytes to copy.
 * 
 * @return VOID
 */
VOID Memory_copy(VOID* const destination, const VOID* const source, const SIZE_T length)
{
  RtlCopyMemory(destination, source, length);
}


/**
 * @brief Compares memory.
 * 
 * Compares memory from source1 to source2.
 * 
 * @param source1 First address.
 * @param source2 Second address.
 * @param length  Number of bytes to compare.
 * 
 * @return TRUE if memory is equal, FALSE otherwise.
 */
BOOLEAN Memory_compare(const VOID* const source1, const VOID* const source2, const SIZE_T length)
{
  return RtlCompareMemory(source1, source2, length) == length;
}


/**
 * @brief Returns physical address.
 * 
 * Returns physical address of a virtual address.
 * 
 * @param virtualAddress Virtual address.
 * 
 * @return Physical address.
 */
UINT64 Memory_getPhysicalAddress(VOID* const virtualAddress)
{
  return MmGetPhysicalAddress(virtualAddress).QuadPart;
}


/**
* @brief Returns virtual address.
  * 
  * Returns virtual address of a physical address.
  * 
  * @param physicalAddress Physical address.
  * 
  * @return Virtual address.
  */
VOID* Memory_getVirtualAddress(const UINT64 physicalAddress)
{
  VOID* const virtualAddress =
    MmGetVirtualForPhysical((PHYSICAL_ADDRESS) { .QuadPart = physicalAddress });
  if (virtualAddress == NULL)
  {
    KeBugCheck(BSOD_MEMORY_VA_CONVERSION);
  }

  return virtualAddress;
}


/**
 * @brief Frees memory.
 * 
 * Deallocates memory.
 * 
 * @param address Address to deallocate.
 * 
 * @return VOID
 */
VOID Memory_free(VOID* const address)
{
  ExFreePoolWithTag(address, MEMORY_POOL_TAG);
}
