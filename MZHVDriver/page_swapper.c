/**
 * @file page_swapper.c
 * @brief Implements driver page swapper.
 * 
 * Provides delegation mechanism for mapping and unmapping pages. It allows the driver
 * to communicate with all logical cores to perform EPT structure changes.
 */


#include "context.h"
#include "memory.h"
#include "page_swapper.h"
#include "vmexit.h"
#include "vmx.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static KIPI_BROADCAST_WORKER PAGE_SWAPPER_mapIpi;


static KIPI_BROADCAST_WORKER PAGE_SWAPPER_unmapIpi;


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Changes page mapping in EPT.
 * 
 * @param pageToMapVirtualAddress Source page virtual address.
 * @param rwPageVirtualAddress Target read-write page virtual address.
 * @param fPageVirtualAddress Target fetch page virtual address.
 * 
 * @return STATUS_SUCCESS if successful, error code otherwise.
 */
NTSTATUS PAGE_SWAPPER_map(
  VOID* const pageToMapVirtualAddress,
  VOID* const rwPageVirtualAddress,
  VOID* const fPageVirtualAddress)
{
  const Context_EptChangedMapping mapping = (Context_EptChangedMapping)
  {
    .guestAddress = Memory_getPhysicalAddress(pageToMapVirtualAddress),
    .hostRwAddress = Memory_getPhysicalAddress(rwPageVirtualAddress),
    .hostFetchAddress = Memory_getPhysicalAddress(fPageVirtualAddress),
    .valid = TRUE
  };

  return (NTSTATUS)KeIpiGenericCall(PAGE_SWAPPER_mapIpi, (ULONG_PTR)&mapping);
}


/**
 * @brief Removes page mapping change in EPT.
 * 
 * @param pageToUnmapVirtualAddress Source page virtual address.
 * 
 * @return STATUS_SUCCESS if successful, error code otherwise.
 */
NTSTATUS PAGE_SWAPPER_unmap(VOID* const pageToUnmapVirtualAddress)
{
  const Context_EptChangedMapping mapping = (Context_EptChangedMapping)
  {
    .guestAddress = Memory_getPhysicalAddress(pageToUnmapVirtualAddress),
    .hostRwAddress = 0,
    .hostFetchAddress = 0,
    .valid = TRUE
  };

  return (NTSTATUS)KeIpiGenericCall(PAGE_SWAPPER_unmapIpi, (ULONG_PTR)&mapping);
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Changes the page's mapping on all logical cores.
 * 
 * @param argument Context_EptChangedMapping structure.
 * 
 * @return STATUS_SUCCESS if successful, error code otherwise.
 */
_Use_decl_annotations_
static ULONG_PTR PAGE_SWAPPER_mapIpi(ULONG_PTR argument)
{
  const Context_EptChangedMapping* const mapping = (const Context_EptChangedMapping*)argument;
  return VMX_vmcall(
    VMEXIT_VMCALL_MAP_PAGE,
    mapping->guestAddress,
    mapping->hostRwAddress,
    mapping->hostFetchAddress);
}


/**
 * @brief Removes the page mapping change.
 *
 * @param argument Context_EptChangedMapping structure.
 *
 * @return STATUS_SUCCESS if successful, error code otherwise.
 */
_Use_decl_annotations_
static ULONG_PTR PAGE_SWAPPER_unmapIpi(ULONG_PTR argument)
{
  const Context_EptChangedMapping* const mapping = (const Context_EptChangedMapping*)argument;
  return VMX_vmcall(VMEXIT_VMCALL_UNMAP_PAGE, mapping->guestAddress, 0, 0);
}
