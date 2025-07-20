// AlpsFilterDriver.cpp - KMDF filter driver for ALPS PS/2 touchpad (logs input IRPs)

#include <ntddk.h>
#include <wdf.h>
#include <ntddmou.h>

extern "C" DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD AlpsEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL AlpsEvtIoDeviceControl;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config, AlpsEvtDeviceAdd);

    KdPrint(("[ALPS] DriverEntry\n"));

    return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
}

NTSTATUS
AlpsEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    KdPrint(("[ALPS] DeviceAdd\n"));

    WdfFdoInitSetFilter(DeviceInit);

    WDFDEVICE device;
    WDF_IO_QUEUE_CONFIG ioQueueConfig;

    NTSTATUS status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (!NT_SUCCESS(status)) return status;

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);
    ioQueueConfig.EvtIoInternalDeviceControl = AlpsEvtIoDeviceControl;

    return WdfIoQueueCreate(device, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
}

VOID
AlpsEvtIoDeviceControl(
    _In_ WDFQUEUE   Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t     OutputBufferLength,
    _In_ size_t     InputBufferLength,
    _In_ ULONG      IoControlCode
)
{
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    if (IoControlCode == IOCTL_INTERNAL_MOUSE_CONNECT ||
        IoControlCode == IOCTL_INTERNAL_MOUSE_DISCONNECT ||
        IoControlCode == IOCTL_INTERNAL_MOUSE_READ) {
        KdPrint(("[ALPS] Mouse IOCTL: 0x%X\n", IoControlCode));
    }

    WdfRequestForwardToIoQueue(Request, WdfDeviceGetDefaultQueue(WdfIoQueueGetDevice(Queue)));
}
