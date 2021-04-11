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

// Version Information

#ifndef _MY_VERSION_H
#define _MY_VERSION_H

#define MY_VERSION_BINARY       5,49,7
#define MY_VERSION_STRING       "5.49.7"
#define MY_VERSION_COMPAT		"5.49.0" // this refers to the driver ABI compatibility

// These #defines are used by either Resource Compiler, or by NSIC installer
#define SBIE_INSTALLER_PATH		"..\\Bin\\"
#define SBIE_INSTALLER_PATH_32  "..\\Bin\\Win32\\SandboxieInstall32.exe"
#define SBIE_INSTALLER_PATH_64  "..\\Bin\\x64\\SandboxieInstall64.exe"

#define MY_PRODUCT_NAME_STRING  "Sandboxie"
#define MY_COMPANY_NAME_STRING  "sandboxie-plus.com"
#define MY_COPYRIGHT_STRING     "Copyright © 2020-2021 by David Xanatos (xanasoft.com)"
#define MY_COPYRIGHT_STRING_OLD "Copyright © 2004-2020 by Sandboxie Holdings, LLC"

#define SANDBOXIE               L"Sandboxie"
#define SBIE                    L"SBIE"

#define SANDBOXIE_USER			L"Sandboxie"

#define SBIE_BOXED_             SBIE L"_BOXED_"
#define SBIE_BOXED_LEN          (4 + 7)

#define SANDBOXIE_INI           L"Sandboxie.ini"

#define SBIEDRV                 L"SbieDrv"
#define SBIEDRV_SYS             L"SbieDrv.sys"

#define SBIESVC                 L"SbieSvc"
#define SBIESVC_EXE             L"SbieSvc.exe"

#define SANDBOXIE_CONTROL       L"SandboxieControl"
#define SBIECTRL_EXE            L"SbieCtrl.exe"
#define SBIECTRL_               L"SbieCtrl_"

#define START_EXE               L"Start.exe"

// see also environment variable in session.bat
#define SBIEDLL                 L"SbieDll"

#define SBIEMSG_DLL             L"SbieMsg.dll"
#define SBIE_IN_MSGS            L"SBIE"

#define SBIEINI                 L"SbieIni"
#define SBIEINI_EXE             L"SbieIni.exe"

#define SANDBOX_VERB            L"sandbox"

#define MY_AUTOPLAY_CLSID_STR   "7E950284-E123-49F4-B32B-A806C090D747"
#define MY_AUTOPLAY_CLSID       0x7E950284, 0xE123, 0x49F4, \
                                { 0xB3, 0x2B, 0xA8,0x06, 0xC0, 0x90, 0xD7, 0x47 }

#define SBIECTRL_LOGO_IMAGE     "../res/MastheadLogo.jpg"

#define TITLE_SUFFIX_W          L" [#]"
#define TITLE_SUFFIX_A           " [#]"

#define FILTER_ALTITUDE         L"86900"

#define OPTIONAL_VALUE(x,y)     VALUE x, y

#endif	// _MY_VERSION_H
