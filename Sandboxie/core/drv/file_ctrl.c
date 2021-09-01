/*
 * Copyright 2021 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 //---------------------------------------------------------------------------
 // DeviceIoControlFile filter
 //---------------------------------------------------------------------------

//#define CTL_CODE( DeviceType, Function, Method, Access ) (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))

#define DEVICE_TYPE_FROM_CTL_CODE(ctrlCode)     (((ULONG)(ctrlCode & 0xffff0000)) >> 16)

#define FUNCTION_FROM_CTL_CODE(ctrlCode)        (((ULONG)(ctrlCode & 0x3ffc)) >> 2)

#define METHOD_FROM_CTL_CODE(ctrlCode)          ((ULONG)(ctrlCode & 3))

_FX NTSTATUS Syscall_DeviceIoControlFile(
    PROCESS *proc, SYSCALL_ENTRY *syscall_entry, ULONG_PTR *user_args)
{
    // filter out dangerous driver calls

    ULONG IoControlCode = (ULONG)user_args[5];

    if (DEVICE_TYPE_FROM_CTL_CODE(IoControlCode) == 0x6d)    //MOUNTMGRCONTROLTYPE 'm'   \Device\MountPointManager
    {
        ULONG function;
        function = FUNCTION_FROM_CTL_CODE(IoControlCode);
        //DbgPrint("DeviceIoContoleFile, func = %d, p=%06d t=%06d, %S\n", function, PsGetCurrentProcessId(), PsGetCurrentThreadId(), proc->image_name);
        if (function == 0 ||        // IOCTL_MOUNTMGR_CREATE_POINT
            function == 1 ||        // IOCTL_MOUNTMGR_DELETE_POINTS (DeleteVolumeMountPoint())
            function == 3 ||        // IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY
            function == 6 ||        // IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_CREATED
            function == 7 ||        // IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_DELETED
            function == 9)          // IOCTL_MOUNTMGR_KEEP_LINKS_WHEN_OFFLINE
            return STATUS_ACCESS_DENIED;
    }
    
    if (DEVICE_TYPE_FROM_CTL_CODE(IoControlCode) == 0x47)    //CMApi(?)CONTROLTYPE 0x47   \Device\DeviceApi\CMApi
    {
        if (!proc->file_open_devapi_cmapi)
        {
            ULONG function = FUNCTION_FROM_CTL_CODE(IoControlCode);
            /*WCHAR path[256] = { 0 };
            FILE_OBJECT* object;
            NTSTATUS status = ObReferenceObjectByHandle((HANDLE)user_args[0], 0, *IoFileObjectType, UserMode, &object, NULL);
            if (NT_SUCCESS(status))
            {
                memcpy(path, object->FileName.Buffer, object->FileName.Length);
                path[object->FileName.Length / sizeof(WCHAR)] = 0;
                ObDereferenceObject(object);
            }*/

            BOOLEAN filter = FALSE;
            switch (function)
            {
            //case 0x0: 
            //case 0x100: //?SwDeviceCreate - Initiates the enumeration of a software device.
            case 0x101: //?SwDevicePropertySet
            case 0x102: //?SwDeviceInterfaceRegister
            case 0x103: //?SwDeviceInterfaceSetState
            case 0x104: //?SwDeviceInterfacePropertySet
            case 0x105: //?SwDeviceSetLifetime
            //case 0x106: //?SwDeviceGetLifetime

            //case 0x200: //?CMP_Register_Notification,CM_Get_Device_ID_List,CM_Get_Device_ID_List_Size
            //case 0x201: //?CmNotifyWnfNotificationCallback,CM_Get_Device_Interface_List_Size
            //case 0x202: //?ProcessPlugPlayEventCallback,CM_Enumerate_Classes,CM_Enumerate_Enumerators
            //case 0x203: //?UnregisterClient,CM_Get_Class_Property_Keys,CM_Get_DevNode_Property_Keys,CM_Get_Device_Interface_Property_Keys
            //case 0x204: //GetObjectProperty,GetObjectProperty
            case 0x205: //CM_Set_DevNode_Property,CM_Set_Device_Interface_Property,CM_Set_Class_Property
            //case 0x206: //GetRegistryProperty,GetRegistryProperty
            case 0x207: //CM_Set_Class_Registry_Property,CM_Set_DevNode_Registry_Property
            //case 0x209: //CM_Get_DevNode_Status
            //case 0x20a: //CM_Get_Depth
            case 0x20b: //CM_Set_DevNode_Problem
            case 0x20c: //CM_Disable_DevNode,CM_Query_And_Remove_SubTree
            case 0x20d: //CM_Register_Device_Interface
            case 0x20e: //CM_Unregister_Device_Interface
            //case 0x20f: //CM_Get_Device_Interface_Alias
            //case 0x210: //Validate_Device_Instance
            case 0x211: //CM_Create_DevNode
            case 0x212: //CM_Uninstall_DevNode
            //case 0x213: //CM_Reenumerate_DevNode,CM_Setup_DevNode
            //case 0x214: //CM_Open_Device_Interface_Key
            case 0x215: //CM_Delete_Device_Interface_Key
            //case 0x216: //OpenLogConfKey,OpenLogConfKey,CM_Open_DevNode_Key,CM_Get_DevNode_Custom_Property,OpenDeviceHwProfileKey
            case 0x217: //CM_Delete_DevNode_Key
            //case 0x218: //CM_Open_Class_Key
            case 0x219: //CM_Delete_Class_Key
                
                filter = TRUE;
            }

            /*
            WCHAR msg_str[240];
            RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"DeviceIoContoleFile, CMApi, func = 0x%X, filter=%d, p=%06d t=%06d, %s\n",
                function, filter, PsGetCurrentProcessId(), PsGetCurrentThreadId(), proc->image_name);
            Log_Debug_Msg(MONITOR_OTHER | MONITOR_TRACE, msg_str, NULL);*/

            if (Session_MonitorCount && (proc->ipc_trace & (TRACE_ALLOW | TRACE_DENY))) {

                ULONG mon_type = MONITOR_IPC;

                if (filter && (proc->ipc_trace & TRACE_DENY))
                    mon_type |= MONITOR_DENY;
                else if (!filter && (proc->ipc_trace & TRACE_ALLOW))
                    mon_type |= MONITOR_OPEN;
                else
                    mon_type = 0;

                if (mon_type) {
                    WCHAR msg_str[24];
                    RtlStringCbPrintfW(msg_str, sizeof(msg_str), L"Func: %02X", (ULONG)function);
                    Log_Debug_Msg(mon_type, L"\\Device\\DeviceApi\\CMApi", msg_str);
                }
            }

            if(filter)
                return STATUS_ACCESS_DENIED;
        }
    }

    return NtDeviceIoControlFile(
        (HANDLE)user_args[0],       // FileHandle
        (HANDLE)user_args[1],       // Event
        (PIO_APC_ROUTINE)user_args[2],  // ApcRoutine
        (PVOID)user_args[3],        // ApcContext
        (PIO_STATUS_BLOCK)user_args[4], // IoStatusBlock
        (ULONG)user_args[5],        // IoControlCode
        (PVOID)user_args[6],        // InputBuffer
        (ULONG)user_args[7],        // InputBufferLength
        (PVOID)user_args[8],        // OutBuffer
        (ULONG)user_args[9]);       // OutputBufferLength
}
