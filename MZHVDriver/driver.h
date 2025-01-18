/**
 * @file driver.h
 * @brief Defines driver name, IOCTLs and entry point
 */
#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Defines
**************************************************************************************************/
/**
 * @name Driver name
 * @brief Macros defining the driver name for NT/DOS
 * @anchor DRIVERName
 */
 ///@{
#define DRIVER_NT_NAME   L"\\Device\\MZHV"
#define DRIVER_DOS_NAME  L"\\DosDevices\\MZHV"
///@}

/**
 * @name Driver IOCTLs
 * @brief Macros defining the IOCTLs for the driver (function codes)
 * @anchor DRIVERName
 */
 ///@{
#define DRIVER_MAP       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define DRIVER_UNMAP     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2137, METHOD_BUFFERED, FILE_ANY_ACCESS)
///@}

/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);


VOID DriverUnload(PDRIVER_OBJECT driverObject);
