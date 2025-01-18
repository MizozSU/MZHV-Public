/**
 * @file driver.c
 * @brief Driver implementation
 * 
 * Implements driver entry point and driver's functions.
 */


#include "context.h"
#include "driver.h"
#include "page_swapper.h"
#include "memory.h"
#include "vmm.h"


/**************************************************************************************************
* Local function declarations
**************************************************************************************************/
static NTSTATUS DeviceCreate(PDEVICE_OBJECT deviceObject, PIRP irp);


static NTSTATUS DeviceClose(PDEVICE_OBJECT deviceObject, PIRP irp);


static NTSTATUS DeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp);


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Driver entry point
 * 
 * Called when driver is loaded. It initializes driver, creates a device and boots VMM.
 * 
 * @param driverObject Driver object
 * @param registryPath Registry path
 * 
 * @return Status code
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath)
{
  UNREFERENCED_PARAMETER(registryPath);
  DbgPrint("DriverEntry\n");

  UNICODE_STRING ntDeviceName;
  UNICODE_STRING dosDeviceName;
  RtlInitUnicodeString(&ntDeviceName, DRIVER_NT_NAME);
  RtlInitUnicodeString(&dosDeviceName, DRIVER_DOS_NAME);

  PDEVICE_OBJECT deviceObject;
  NTSTATUS ntStatus = IoCreateDevice(driverObject, 0, &ntDeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);
  if (!NT_SUCCESS(ntStatus))
  {
    DbgPrint("DriverEntry: IoCreateDevice error=%ld\n", ntStatus);
    return ntStatus;
  }

  ntStatus = IoCreateSymbolicLink(&dosDeviceName, &ntDeviceName);
  if (!NT_SUCCESS(ntStatus))
  {
    DbgPrint("DriverEntry: IoCreateSymbolicLink error=%ld\n", ntStatus);
    IoDeleteDevice(deviceObject);
    return ntStatus;
  }

  driverObject->DriverUnload = &DriverUnload;
  driverObject->MajorFunction[IRP_MJ_CREATE] = &DeviceCreate;
  driverObject->MajorFunction[IRP_MJ_CLOSE] = &DeviceClose;
  driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DeviceControl;

  ntStatus = Context_init();
  if (!NT_SUCCESS(ntStatus))
  {
    DbgPrint("DriverEntry: Context_init error=%ld\n", ntStatus);
    IoDeleteSymbolicLink(&dosDeviceName);
    IoDeleteDevice(driverObject->DeviceObject);
    return ntStatus;
  }

  ntStatus = VMM_enable();
  if (!NT_SUCCESS(ntStatus))
  {
    DbgPrint("DriverEntry: VMM_enable error=%ld\n", ntStatus);
    Context_destroy();
    IoDeleteSymbolicLink(&dosDeviceName);
    IoDeleteDevice(driverObject->DeviceObject);
    return ntStatus;
  }

  DbgPrint("DriverEntry: Success\n");

  return STATUS_SUCCESS;
}


/**
 * @brief Driver unload function
 * 
 * Called when driver is unloaded. It disables VMM and destroys context.
 * 
 * @param driverObject Driver object
 */
VOID DriverUnload(PDRIVER_OBJECT driverObject)
{
  UNREFERENCED_PARAMETER(driverObject);
  DbgPrint("DriverUnload\n");

  if (Context_getContext() != NULL)
  {
    VMM_disable();
    Context_destroy();
  }

  UNICODE_STRING dosDeviceName;
  RtlInitUnicodeString(&dosDeviceName, DRIVER_DOS_NAME);
  IoDeleteSymbolicLink(&dosDeviceName);
  if (driverObject->DeviceObject != NULL)
  {
    IoDeleteDevice(driverObject->DeviceObject);
  }
}


/**************************************************************************************************
* Local function definitions
**************************************************************************************************/
/**
 * @brief Device create function
 * 
 * Called when device is created, does nothing.
 * 
 * @param deviceObject Device object
 * @param irp I/O request packet
 * 
 * @return STATUS_SUCCESS
 */
static NTSTATUS DeviceCreate(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  UNREFERENCED_PARAMETER(deviceObject);
  DbgPrint("DeviceCreate\n");

  irp->IoStatus.Status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}


/**
 * @brief Device close function
 * 
 * Called when the device is closed, does nothing.
 * 
 * @param deviceObject Device object
 * @param irp I/O request packet
 * 
 * @return STATUS_SUCCESS
 */
static NTSTATUS DeviceClose(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  UNREFERENCED_PARAMETER(deviceObject);
  DbgPrint("DeviceClose\n");

  irp->IoStatus.Status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return STATUS_SUCCESS;
}


/**
 * @brief Device control function
 * 
 * Called when device's function is invoked. It handles DRIVER_MAP and DRIVER_UNMAP function
 * forwarding to the page swapper.
 * 
 * @param deviceObject Device object
 * @param irp I/O request packet
 * 
 * @return Status code
 */
static NTSTATUS DeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp)
{
  UNREFERENCED_PARAMETER(deviceObject);
  DbgPrint("DeviceControl\n");

  NTSTATUS status = STATUS_SUCCESS;

  PIO_STACK_LOCATION  ioStackLocation = IoGetCurrentIrpStackLocation(irp);
  switch (ioStackLocation->Parameters.DeviceIoControl.IoControlCode)
  {
  case DRIVER_MAP:
  {
    if (ioStackLocation->Parameters.DeviceIoControl.InputBufferLength < 3 * sizeof(PVOID))
    {
      status = STATUS_INVALID_PARAMETER;
      break;
    }
    PVOID arguments[3] = { 0 };
    Memory_copy(arguments, irp->AssociatedIrp.SystemBuffer, sizeof(arguments));
    status = PAGE_SWAPPER_map(arguments[0], arguments[1], arguments[2]);

    break;
  }
  case DRIVER_UNMAP:
  {
    if (ioStackLocation->Parameters.DeviceIoControl.InputBufferLength < sizeof(PVOID))
    {
      status = STATUS_INVALID_PARAMETER;
      break;
    }
    PVOID argument = { 0 };
    Memory_copy(&argument, irp->AssociatedIrp.SystemBuffer, sizeof(argument));
    status = PAGE_SWAPPER_unmap(argument);

    break;
  }
  default:
  {
    status = STATUS_INVALID_DEVICE_REQUEST;
    break;
  }
  }

  irp->IoStatus.Status = status;
  IoCompleteRequest(irp, IO_NO_INCREMENT);

  DbgPrint("DeviceControl: status=%ld\n", status);

  return status;
}
