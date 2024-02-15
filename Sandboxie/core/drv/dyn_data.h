/*
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
// Dynamic Data
//---------------------------------------------------------------------------


#ifndef _MY_DYN_DATA_H
#define _MY_DYN_DATA_H

#ifndef ISSIGNTOOL
#include "driver.h"
#endif

#define DYNDATA_FORMAT      1
#define DYNDATA_VERSION     1
#define DYNDATA_SIGN        'eibs'

#define DYNDATA_FLAG_EXP    0x80000000

//
// Special values:
//  OsBuild_max can be set to -1 indicating no upper bound
//  Offset, 0 means not available/disabled, -1 means use dynamic resolution
//

typedef struct _SBIE_DYNCONFIG
{
    ULONG OsBuild_min;
    ULONG OsBuild_max;
    ULONG Flags;
    ULONG Reserved;

    USHORT Clipboard_offset;

    USHORT ImpersonationData_offset;    // ETHREAD.PS_CLIENT_SECURITY_CONTEXT.ImpersonationData

    USHORT RestrictedSidCount_offset;   // TOKEN.RestrictedSidCount
    USHORT RestrictedSids_offset;       // TOKEN.RestrictedSids
    USHORT UserAndGroups_offset;        // TOKEN.UserAndGroups
    USHORT UserAndGroupCount_offset;    // TOKEN.UserAndGroupCount

    USHORT Flags2_offset;               // EPROCESS.Flags2
    USHORT SignatureLevel_offset;       // EPROCESS.SignatureLevel
    USHORT MitigationFlags_offset;      // EPROCESS.MitigationFlags

    USHORT TrapFrame_offset;            // AMD64 only

    ULONG  ServiceTable_offset;

} SBIE_DYNCONFIG, *PSBIE_DYNCONFIG;

//const int x = sizeof(SBIE_DYNCONFIG);

typedef struct _SBIE_DYNDATA
{
    USHORT Format;
    USHORT Version;
    ULONG Signature;
    USHORT Arch;        // IMAGE_FILE_MACHINE_AMD64, IMAGE_FILE_MACHINE_ARM64, IMAGE_FILE_MACHINE_I386
    USHORT Reserved;
    USHORT Size;        // size of one entry
    USHORT Count;
    USHORT Configs[1];  // offsets, allows to extend SBIE_DYNCONFIG with new fields

} SBIE_DYNDATA, *PSBIE_DYNDATA;

//const int x = sizeof(SBIE_DYNDATA);
//const int y = FIELD_OFFSET(SBIE_DYNDATA, Configs);

extern BOOLEAN Dyndata_Active;
extern SBIE_DYNCONFIG Dyndata_Config;

BOOLEAN Dyndata_Init();

#endif // _MY_DYN_DATA_H