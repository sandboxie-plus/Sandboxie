/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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
// SbieSvc Messages
//---------------------------------------------------------------------------


#ifndef _MY_MSGIDS_H
#define _MY_MSGIDS_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define MSGID_PSTORE                            0x1100
#define MSGID_PSTORE_GET_TYPE_INFO              0x1101
#define MSGID_PSTORE_GET_SUBTYPE_INFO           0x1102
#define MSGID_PSTORE_READ_ITEM                  0x1103
#define MSGID_PSTORE_ENUM_TYPES                 0x1104
#define MSGID_PSTORE_ENUM_ITEMS                 0x1105

#define MSGID_PROCESS                           0x1200
#define MSGID_PROCESS_CHECK_INIT_COMPLETE       0x1201
#define MSGID_PROCESS_GET_WORK_DEPRECATED       0x1202
#define MSGID_PROCESS_KILL_ONE                  0x1203
#define MSGID_PROCESS_KILL_ALL                  0x1204
#define MSGID_PROCESS_RUN_SANDBOXED             0x1205
#define MSGID_PROCESS_SET_DEVICE_MAP            0x1206
#define MSGID_PROCESS_OPEN_DEVICE_MAP           0x1207
#define MSGID_PROCESS_RUN_UPDATER               0x1208
#define MSGID_PROCESS_GET_INFO                  0x1209
#define MSGID_PROCESS_SUSPEND_RESUME_ONE        0x120A
#define MSGID_PROCESS_SUSPEND_RESUME_ALL        0x120B

#define MSGID_SERVICE                           0x1300
#define MSGID_SERVICE_START                     0x1301
#define MSGID_SERVICE_QUERY                     0x1302
#define MSGID_SERVICE_LIST                      0x1303
#define MSGID_SERVICE_RUN                       0x1304
#define MSGID_SERVICE_UAC                       0x1305

#define MSGID_TERMINAL                          0x1400
#define MSGID_TERMINAL_QUERY_INFO               0x1411
#define MSGID_TERMINAL_CHECK_TYPE               0x1412
#define MSGID_TERMINAL_GET_NAME                 0x1413
#define MSGID_TERMINAL_GET_PROPERTY             0x1414
#define MSGID_TERMINAL_DISCONNECT               0x1415
#define MSGID_TERMINAL_GET_USER_TOKEN           0x1416

#define MSGID_NAMED_PIPE                        0x1500
#define MSGID_NAMED_PIPE_OPEN                   0x1501
#define MSGID_NAMED_PIPE_CLOSE                  0x1502
#define MSGID_NAMED_PIPE_SET                    0x1503
#define MSGID_NAMED_PIPE_READ                   0x1504
#define MSGID_NAMED_PIPE_WRITE                  0x1505
#define MSGID_NAMED_PIPE_LPC_CONNECT            0x1506
#define MSGID_NAMED_PIPE_LPC_REQUEST            0x1507
#define MSGID_NAMED_PIPE_ALPC_REQUEST           0x1508
#define MSGID_NAMED_PIPE_NOTIFICATION           0x15FF

#define MSGID_FILE                              0x1700
#define MSGID_FILE_SET_ATTRIBUTES               0x1701
#define MSGID_FILE_SET_SHORT_NAME               0x1702
#define MSGID_FILE_LOAD_KEY                     0x1703
#define MSGID_FILE_GET_ALL_HANDLES              0x1704
#define MSGID_FILE_IMPERSONATION_REQUESTS       0x1780
#define MSGID_FILE_SET_REPARSE_POINT            0x1781
#define MSGID_FILE_OPEN_WOW64_KEY               0x1782
#define MSGID_FILE_CHECK_KEY_EXISTS             0x1783

#define MSGID_SBIE_INI                          0x1800
#define MSGID_SBIE_INI_GET_USER                 0x1801
#define MSGID_SBIE_INI_GET_PATH                 0x1802
#define MSGID_SBIE_INI_TEMPLATE                 0x1806
#define MSGID_SBIE_INI_SET_PASSWORD             0x1807
#define MSGID_SBIE_INI_TEST_PASSWORD            0x1808
#define MSGID_SBIE_INI_GET_SETTING              0x1810
#define MSGID_SBIE_INI_SET_SETTING              0x1811
#define MSGID_SBIE_INI_ADD_SETTING              0x1812
#define MSGID_SBIE_INI_INS_SETTING              0x1813
#define MSGID_SBIE_INI_DEL_SETTING              0x1814
#define MSGID_SBIE_INI_GET_VERSION              0x18AA
#define MSGID_SBIE_INI_GET_WAIT_HANDLE          0x18AB
#define MSGID_SBIE_INI_RUN_SBIE_CTRL            0x180A
#define MSGID_SBIE_INI_RC4_CRYPT                0x180F
#define MSGID_SBIE_INI_SET_DAT                  0x18D1
#define MSGID_SBIE_INI_GET_DAT                  0x18D2

//#define MSGID_SBIE_MGR                          0x1900

#define MSGID_NETAPI                            0x1A00
#define MSGID_NETAPI_USE_ADD                    0x1A01
//#define MSGID_NETAPI_WKSTA_GET_INFO             0x1A01
//#define MSGID_NETAPI_SERVER_GET_INFO            0x1A02

#define MSGID_COM                               0x1B00
#define MSGID_COM_GET_CLASS_OBJECT              0x1B01
#define MSGID_COM_CREATE_INSTANCE               0x1B02
#define MSGID_COM_QUERY_INTERFACE               0x1B03
#define MSGID_COM_ADD_REF_RELEASE               0x1B04
#define MSGID_COM_INVOKE_METHOD                 0x1B05
#define MSGID_COM_UNMARSHAL_INTERFACE           0x1B06
#define MSGID_COM_MARSHAL_INTERFACE             0x1B07
#define MSGID_COM_QUERY_BLANKET                 0x1B08
#define MSGID_COM_SET_BLANKET                   0x1B09
#define MSGID_COM_COPY_PROXY                    0x1B0A
#define MSGID_COM_CRYPT_PROTECT_DATA            0x1B81
#define MSGID_COM_NOTIFICATION                  0x1BFF

#define MSGID_IPHLP                             0x1C00
#define MSGID_IPHLP_CREATE_FILE                 0x1C01
#define MSGID_IPHLP_CLOSE_HANDLE                0x1C02
#define MSGID_IPHLP_SEND_ECHO                   0x1C03
#define MSGID_IPHLP_NOTIFICATION                0x1CFF

#define MSGID_IMBOX                             0x1D00
#define MSGID_IMBOX_CREATE                      0x1D01
#define MSGID_IMBOX_MOUNT                       0x1D02
#define MSGID_IMBOX_UNMOUNT                     0x1D03
#define MSGID_IMBOX_ENUM                        0x1D04
#define MSGID_IMBOX_QUERY                       0x1D05
#define MSGID_IMBOX_UPDATE                      0x1D06

#define MSGID_QUEUE                             0x1E00
#define MSGID_QUEUE_CREATE                      0x1E01
#define MSGID_QUEUE_GETREQ                      0x1E02
#define MSGID_QUEUE_PUTRPL                      0x1E03
#define MSGID_QUEUE_PUTREQ                      0x1E04
#define MSGID_QUEUE_GETRPL                      0x1E05
#define MSGID_QUEUE_STARTUP                     0x1E10
#define MSGID_QUEUE_NOTIFICATION                0x1EFF

#define MSGID_EPMAPPER                          0x1F00
#define MSGID_EPMAPPER_GET_PORT_NAME            0x1F01

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


typedef struct _MSG_HEADER {

    ULONG length;
    union {
        ULONG msgid;
        ULONG status;
    };

} MSG_HEADER;


//---------------------------------------------------------------------------


#endif // _MY_MSGIDS_H
