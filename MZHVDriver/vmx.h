/**
 * @file vmx.h
 * @brief Declarations for VMX assembly functions
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
/**
 * @brief Performs VMCALL instruction
 * 
 * This function is used to perform VMCALL instruction. It takes 4 parameters and returns NTSTATUS.
 * 
 * @param rcx 1st parameter
 * @param rdx 2nd parameter
 * @param r8 3rd parameter
 * @param r9 4th parameter
 * 
 * @return VMCALL return value
 */
NTSTATUS VMX_vmcall(const UINT64 rcx, const UINT64 rdx, const UINT64 r8, const UINT64 r9);

/**
 * @brief Performs INVEPT instruction
 * 
 * This function performs INVEPT instruction, invalidating EPT cache.
 * 
 * @return VOID
 */
VOID VMX_inveptAll(VOID);
