/**
 * @file asmproc.h
 * @brief Declarations of procedures defined in asmproc.asm
 * 
 * This file contains the declarations of asm procedures defined in asmproc.asm.
 */

#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
/**
 * @brief Entry point of VMX root mode
 * 
 * This function is the entry point of VMX root mode. It is called after each VM exit.
 * 
 * @return VOID
 */
VOID ASMPROC_vmExitHandler(VOID);


/**
 * @brief Switch to VMX non-root mode
 * 
 * This function switches to VMX non-root mode, entering a VM with VMLAUNCH.
 * 
 * @return STATUS_SUCCESS if successful, STATUS_UNSUCCESSFUL otherwise
 */
NTSTATUS ASMPROC_enterVmcs(VOID);


/**
 * @brief VMX non-root mode entryPoint
 * 
 * This function is the entry point of VMX non-root mode. It is called after VMLAUNCH caused by
 * ASMPROC_enterVmcs function. It is a second part of ASMPROC_enterVmcs function, returning
 * STATUS_SUCCESS.
 * 
 * @return STATUS_SUCCESS
 */
NTSTATUS ASMPROC_vmcsEntryPoint(VOID);
