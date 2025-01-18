/**
 * @file memory.h
 * @brief Memory mangement tag and function declarations
 */

#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/*
 * @brief Memory pool tag
 */
#define MEMORY_POOL_TAG  'MZHV'


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
VOID* Memory_allocate(const SIZE_T noOfBytes, const BOOLEAN aligned);


VOID Memory_copy(VOID* const destination, const VOID* const source, const SIZE_T length);


BOOLEAN Memory_compare(const VOID* const source1, const VOID* const source2, const SIZE_T length);


UINT64 Memory_getPhysicalAddress(VOID* const virtualAddress);


VOID* Memory_getVirtualAddress(const UINT64 physicalAddress);


VOID Memory_free(VOID* const address);
