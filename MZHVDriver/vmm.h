/**
 * @file vmm.h
 * @brief Function declarations
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS VMM_enable(VOID);


VOID VMM_disable(VOID);
