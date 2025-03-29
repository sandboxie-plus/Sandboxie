/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2024 David Xanatos, xanasoft.com
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
// Driver API -- flags
//---------------------------------------------------------------------------


#ifndef _MY_API_FLAGS_H
#define _MY_API_FLAGS_H


//---------------------------------------------------------------------------


#define CONF_JUST_EXPAND				0x80000000L
#define CONF_GET_NO_GLOBAL				0x40000000L
#define CONF_GET_NO_EXPAND				0x20000000L
#define CONF_GET_NO_TEMPLS				0x10000000L


// from ntddk:
#define DUPLICATE_CLOSE_SOURCE			0x00000001
#define DUPLICATE_SAME_ACCESS			0x00000002
#define DUPLICATE_SAME_ATTRIBUTES		0x00000004
// and our addition:
#define DUPLICATE_INHERIT				0x00040000
#define DUPLICATE_INTO_OTHER			0x00080000  // otherwise DUP_FROM_OTHER


#define MONITOR_SYSCALL					0x00000001
#define MONITOR_PIPE					0x00000002
#define MONITOR_IPC						0x00000003
#define MONITOR_WINCLASS				0x00000004
#define MONITOR_DRIVE					0x00000005
#define MONITOR_COMCLASS				0x00000006
#define MONITOR_RTCLASS					0x00000007
#define MONITOR_IGNORE					0x00000008
#define MONITOR_IMAGE					0x00000009
#define MONITOR_FILE					0x0000000A
#define MONITOR_KEY						0x0000000B
#define MONITOR_OTHER					0x0000000C
#define MONITOR_NETFW					0x0000000D
#define MONITOR_SCM						0x0000000E // Service Control Manager
#define MONITOR_APICALL					0x0000000F
#define MONITOR_RPC						0x00000010
#define MONITOR_DNS						0x00000011
#define MONITOR_HOOK					0x00000012
#define MONITOR_TYPE_MASK				0x000000FF

#define MONITOR_RESERVED				0x0000FF00

#define MONITOR_OPEN					0x00010000
#define MONITOR_DENY					0x00020000 // CLOSED
#define MONITOR_DISPOSITION_MASK		0x000F0000

#define MONITOR_ALLOWED					0x00100000
#define MONITOR_DENIDED					0x00200000
#define MONITOR_SUCCESS					0x00400000
#define MONITOR_FAILURE					0x00800000
#define MONITOR_RESULT_MASK				0x00F00000

#define MONITOR_UNUSED					0x3F000000

#define MONITOR_TRACE					0x40000000
#define MONITOR_USER					0x80000000



#define DISABLE_JUST_THIS_PROCESS		0x0123ABC0


#define SBIE_FLAG_VALID_PROCESS         0x00000001
#define SBIE_FLAG_FORCED_PROCESS        0x00000002
#define SBIE_FLAG_UNUSED_00000004       0x00000004
#define SBIE_FLAG_PROCESS_IS_START_EXE  0x00000008
#define SBIE_FLAG_PARENT_WAS_START_EXE  0x00000010
#define SBIE_FLAG_IMAGE_FROM_SBIE_DIR   0x00000020
#define SBIE_FLAG_IMAGE_FROM_SANDBOX    0x00000040
#define SBIE_FLAG_DROP_RIGHTS           0x00000080
#define SBIE_FLAG_RIGHTS_DROPPED        0x00000100
									  //0x00000200
									  //0x00000400
									  //0x00000800
//#define SBIE_FLAG_BLOCK_FAKE_INPUT    0x00001000
#define SBIE_FLAG_OPEN_ALL_WIN_CLASS    0x00002000
//#define SBIE_FLAG_BLOCK_SYS_PARAM     0x00004000
#define SBIE_FLAG_WIN32K_HOOKABLE		0x00008000

									  //0x00010000
									  //0x00020000
									  //0x00040000
									  //0x00080000
									  //0x00100000
									  //0x00200000
									  //0x00400000
									  //0x00800000

#define SBIE_FLAG_APP_COMPARTMENT		0x01000000
#define SBIE_FLAG_PRIVACY_MODE			0x02000000
#define SBIE_FLAG_RULE_SPECIFICITY		0x04000000
#define SBIE_FLAG_PROCESS_IN_PCA_JOB    0x08000000
#define SBIE_FLAG_CREATE_CONSOLE_HIDE   0x10000000
#define SBIE_FLAG_CREATE_CONSOLE_SHOW   0x20000000
#define SBIE_FLAG_PROTECTED_PROCESS     0x40000000
#define SBIE_FLAG_HOST_INJECT_PROCESS   0x80000000


#define SBIE_CONF_FLAG_RECONFIGURE		0x00000001
#define SBIE_CONF_FLAG_RELOAD_CERT		0x80000000


#define SBIE_FEATURE_FLAG_WFP   		0x00000001
#define SBIE_FEATURE_FLAG_OB_CALLBACKS  0x00000002
#define SBIE_FEATURE_FLAG_PRIVACY_MODE  0x00000004
#define SBIE_FEATURE_FLAG_COMPARTMENTS	0x00000008
#define SBIE_FEATURE_FLAG_SBIE_LOGIN    0x00000010
#define SBIE_FEATURE_FLAG_WIN32K_HOOK   0x00000020
#define SBIE_FEATURE_FLAG_SECURITY_MODE	0x00000040
#define SBIE_FEATURE_FLAG_ENCRYPTION	0x00000080
#define SBIE_FEATURE_FLAG_NET_PROXY		0x00000100
//...
#define SBIE_FEATURE_FLAG_MAX			0x00008000
#define SBIE_FEATURE_FLAG_NO_SIG		0x00010000

#define SBIE_FEATURE_FLAG_DYNDATA_OK	0x10000000
#define SBIE_FEATURE_FLAG_DYNDATA_EXP	0x20000000
#define SBIE_FEATURE_FLAG_NEW_ARCH		0x40000000
#define SBIE_FEATURE_FLAG_CERTIFIED		0x80000000

//---------------------------------------------------------------------------


#endif // _MY_API_FLAGS_H
