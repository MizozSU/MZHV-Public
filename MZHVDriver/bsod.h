/**
 * @file bsod.h
 * @brief Blue Screen of Death Codes definitions.
 */

#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name Blue Screen of Death Codes
 * @brief These codes are used to identify the cause of BSOD which occurred in VMX root operation.
 * 
 * @anchor BSODCodes
 */
///@{
#define BSOD_MEMORY_VA_CONVERSION     0x1000
#define BSOD_VMEXIT_UNKNOWN           0x1001
#define BSOD_VMEXIT_EPT_NO_MAPPING    0x1002
#define BSOD_VMEXIT_EPT_UNKNOWN       0x1003
///@}
