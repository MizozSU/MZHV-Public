/**
 * @file page_swapper.h
 * @brief Page swapper header file
 */

#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS PAGE_SWAPPER_map(
  VOID* const pageToMapVirtualAddress,
  VOID* const rwPageVirtualAddress,
  VOID* const fPageVirtualAddress);


NTSTATUS PAGE_SWAPPER_unmap(VOID* const pageToUnmapVirtualAddress);
