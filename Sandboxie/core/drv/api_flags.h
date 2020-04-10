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

//---------------------------------------------------------------------------
// Driver API -- flags
//---------------------------------------------------------------------------


#ifndef _MY_API_FLAGS_H
#define _MY_API_FLAGS_H


//---------------------------------------------------------------------------


#define CONF_GET_NO_GLOBAL          0x40000000L
#define CONF_GET_NO_EXPAND          0x20000000L
#define CONF_GET_NO_TEMPLS          0x10000000L


// from ntddk:
#define DUPLICATE_CLOSE_SOURCE      0x00000001
#define DUPLICATE_SAME_ACCESS       0x00000002
#define DUPLICATE_SAME_ATTRIBUTES   0x00000004
// and our addition:
#define DUPLICATE_INHERIT           0x00040000
#define DUPLICATE_INTO_OTHER        0x00080000  // otherwise DUP_FROM_OTHER


#define MONITOR_PIPE                0x011B
#define MONITOR_IPC                 0x022B
#define MONITOR_WINCLASS            0x033B
#define MONITOR_DRIVE               0x044B
#define MONITOR_COMCLASS            0x055B
#define MONITOR_IGNORE              0x066B
#define MONITOR_IMAGE               0x077B
#define MONITOR_FILE_OR_KEY         0x088B
#define MONITOR_OTHER				0x099B
#define MONITOR_OPEN                0x1000
#define MONITOR_DENY                0x2000


#define DISABLE_JUST_THIS_PROCESS   0x0123ABC0


#define SBIE_FLAG_VALID_PROCESS         0x00000001
#define SBIE_FLAG_FORCED_PROCESS        0x00000002
#define SBIE_FLAG_UNUSED_00000004       0x00000004
#define SBIE_FLAG_PROCESS_IS_START_EXE  0x00000008
#define SBIE_FLAG_PARENT_WAS_START_EXE  0x00000010
#define SBIE_FLAG_IMAGE_FROM_SBIE_DIR   0x00000020
#define SBIE_FLAG_IMAGE_FROM_SANDBOX    0x00000040
#define SBIE_FLAG_DROP_RIGHTS           0x00000080
#define SBIE_FLAG_RIGHTS_DROPPED        0x00000100
#define SBIE_FLAG_OPEN_ALL_WIN_CLASS    0x00002000
//#define SBIE_FLAG_BLOCK_FAKE_INPUT      0x00001000
//#define SBIE_FLAG_BLOCK_SYS_PARAM       0x00004000
#define SBIE_FLAG_PROCESS_IN_PCA_JOB    0x08000000
#define SBIE_FLAG_CREATE_CONSOLE_HIDE   0x10000000
#define SBIE_FLAG_CREATE_CONSOLE_SHOW   0x20000000
#define SBIE_FLAG_PROTECTED_PROCESS     0x40000000
#define SBIE_FLAG_HOST_INJECT_PROCESS   0x80000000


//---------------------------------------------------------------------------


#endif // _MY_API_FLAGS_H
