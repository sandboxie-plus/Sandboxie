/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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

//
// alpc.h
// 

#ifndef _ALPC_H
#define _ALPC_H

#define MAX_PORTMSG_LENGTH 328

typedef void *PPORT_VIEW;
typedef void *PREMOTE_PORT_VIEW;

typedef enum _LPC_TYPE {
    LPC_NEW_MESSAGE,            // 0
    LPC_REQUEST,                // 1
    LPC_REPLY,                  // 2
    LPC_DATAGRAM,               // 3
    LPC_LOST_REPLY,             // 4
    LPC_PORT_CLOSED,            // 5
    LPC_CLIENT_DIED,            // 6
    LPC_EXCEPTION,              // 7
    LPC_DEBUG_EVENT,            // 8
    LPC_ERROR_EVENT,            // 9
    LPC_CONNECTION_REQUEST,     // 10
    LPC_CONNECTION_REFUSED,     // 11
    LPC_MAXIMUM,
    LPC_KERNELMODE_MESSAGE = 0x8000
} LPC_TYPE;


typedef struct _LPC_SECTION_MEMORY {
    ULONG   Length;
    ULONG   ViewSize;
    PVOID   ViewBase;
} LPC_SECTION_MEMORY, *PLPC_SECTION_MEMORY;


typedef struct _LPC_SECTION_OWNER_MEMORY {
    ULONG   Length;
    HANDLE  SectionHandle;
    ULONG   OffsetInSection;
    ULONG   ViewSize;
    PVOID   ViewBase;
    PVOID   OtherSideViewBase;
} LPC_SECTION_OWNER_MEMORY, *PLPC_SECTION_OWNER_MEMORY;


#pragma warning( push )
#pragma warning( disable : 4200)

typedef struct _PORT_MESSAGE {
    union {
        struct {
            USHORT DataLength;
            USHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            USHORT Type;
            USHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        CLIENT_ID ClientId;
        double DoNotUseThisField;       // force quadword alignment
    };
    ULONG MessageId;
    union {
        ULONG_PTR ClientViewSize;       // for LPC_CONNECTION_REQUEST message
        ULONG CallbackId;               // for LPC_REQUEST message
    };
    //UCHAR Data[0];
} PORT_MESSAGE, *PPORT_MESSAGE;

#pragma warning( pop )

typedef struct _PORT_DATA_INFO {

    ULONG NumDataInfo;
    UCHAR *Buffer;
    ULONG BufferLen;

} PORT_DATA_INFO;


// begin ALPC_INFO structure from LPC-ALPC-paper.pdf

#define PORT_INFO_LPCMODE               0x001000    // Behave like an LPC port
#define PORT_INFO_CANIMPERSONATE        0x010000    // Accept impersonation
#define PORT_INFO_REQUEST_ALLOWED       0x020000    // Allow messages
#define PORT_INFO_SEMAPHORE             0x040000    // Synchronization system
#define PORT_INFO_HANDLE_EXPOSE         0x080000    // Accept handle expose
#define PORT_INFO_PARENT_SYSTEM_PROCESS 0x100000    // Kernel ALPC interface
#define PORT_INFO_WOW64_PROCESS         0x40000000  // WOW64 process

#define ALPC_SYNC_CONNECTION            0x020000    // Synchronous connection request
#define ALPC_USER_WAIT_MODE             0x100000    // Wait in user mode
#define ALPC_WAIT_IS_ALERTABLE          0x200000    // Wait in alertable mode
#define ALPC_MESSAGE_FLAG_VIEW          0x40000000  // Message buffer type for section view

typedef struct _ALPC_PORT_ATTRIBUTES {

    ULONG       Flags;
    SECURITY_QUALITY_OF_SERVICE SecurityQos;
    ULONG       MaxMessageLength;
    ULONG_PTR   MemoryBandwidth;
    ULONG_PTR   MaxPoolUsage;
    ULONG_PTR   MaxSectionSize;
    ULONG_PTR   MaxViewSize;
    ULONG_PTR   MaxTotalSectionSize;
    ULONG_PTR   DupObjectTypes;

} ALPC_PORT_ATTRIBUTES;

typedef struct _ALPC_MESSAGE_VIEW {

    ULONG               SendFlags;
    ULONG               ReceiveFlags;
    union {
        ULONG           Unknowns[12];
        struct {
            ULONG       ReplyLength;
            ULONG       Unknown1;
            ULONG       Unknown2;
#ifdef _WIN64
            ULONG       Unknown3;
            ULONG       Unknown4;
#endif _WIN64
            ULONG       MessageId;
            ULONG       CallbackId;
        } s1;
        struct {
            ULONG       ViewAttrs;
            ULONG       Unknown1;
#ifdef _WIN64
            ULONG       Unknown2;
            ULONG       Unknown3;
#endif _WIN64
            ULONG_PTR   ViewBase;
            ULONG       ViewSize;
        } s2;
    } u;

} ALPC_MESSAGE_VIEW;


// end ALPC_INFO structure from LPC-ALPC-paper.pdf

__declspec(dllimport) NTSTATUS NtCreatePort(
    OUT PHANDLE PortHandle,
    IN  POBJECT_ATTRIBUTES ObjectAttributes,
    IN  ULONG MaxConnectInfoLength,
    IN  ULONG MaxMsgLength,
    IN  OUT PULONG Reserved OPTIONAL);

__declspec(dllimport) NTSTATUS NtConnectPort(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

__declspec(dllimport) NTSTATUS NtSecureConnectPort(
    OUT PHANDLE ClientPortHandle,
    IN  PUNICODE_STRING ServerPortName,
    IN  PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN  OUT PLPC_SECTION_OWNER_MEMORY ClientSharedMemory OPTIONAL,
    IN  PSID ServerSid OPTIONAL,
    OUT PLPC_SECTION_MEMORY ServerSharedMemory OPTIONAL,
    OUT PULONG MaximumMessageLength OPTIONAL,
    IN  OUT PVOID ConnectionInfo OPTIONAL,
    IN  OUT PULONG ConnectionInfoLength OPTIONAL);

__declspec(dllimport) NTSTATUS NtAcceptConnectPort(
    OUT PHANDLE PortHandle,
    IN PVOID PortContext OPTIONAL,
    IN PPORT_MESSAGE ConnectionRequest,
    IN BOOLEAN AcceptConnection,
    IN OUT PPORT_VIEW ServerView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ClientView OPTIONAL);

__declspec(dllimport) NTSTATUS NtCompleteConnectPort(
    IN  HANDLE PortHandle);

__declspec(dllimport) NTSTATUS NtRegisterThreadTerminatePort(
    IN  HANDLE PortHandle);

__declspec(dllimport) NTSTATUS NtRequestPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage);

__declspec(dllimport) NTSTATUS NtReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE ReplyMessage);

__declspec(dllimport) NTSTATUS NtRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage);

__declspec(dllimport) NTSTATUS NtReplyWaitReceivePort(
    IN  HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN  PPORT_MESSAGE ReplyMessage OPTIONAL,
    OUT PPORT_MESSAGE ReceiveMessage);

__declspec(dllimport) NTSTATUS NtImpersonateClientOfPort(
    IN  HANDLE PortHandle,
    IN  PPORT_MESSAGE PortMessage);

#define NTOS_API(type)  NTSYSAPI type NTAPI
#define NTOS_NTSTATUS   NTOS_API(NTSTATUS)

NTOS_NTSTATUS   LpcRequestPort(
    IN PVOID            PortObject,
    IN PVOID            RequestMessage);


extern POBJECT_TYPE *LpcPortObjectType;

#endif // _ALPC_H
