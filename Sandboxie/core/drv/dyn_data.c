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

#include "dyn_data.h"
#ifndef ISSIGNTOOL
#include "util.h"
#define KERNEL_MODE
#include "verify.h"
#endif

const wchar_t Parameters[] = L"\\Parameters";

#ifndef IMAGE_FILE_MACHINE_I386
#define IMAGE_FILE_MACHINE_I386              0x014c  // Intel 386.
#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#define IMAGE_FILE_MACHINE_ARM64             0xAA64  // ARM64 Little-Endian
#endif

#define WIN11_LATEST    27695 // <-----
#define SVR2025         26040
#define WIN11_FIRST     22000
#define SVR2022         20348

#define WIN10_LATEST    19045 // <-----
#define WIN10_FIRST     10240

#define WIN8_1          9600
#define WIN8            9200
#define WIN7SP1         7601
#define WIN7            7600
#define WINVISTA        6002
#define WINXP           2600

#ifndef ASSERT
#define ASSERT(x)
#endif

//#define DYN_DEBUG

BOOLEAN Dyndata_Active = FALSE;
SBIE_DYNCONFIG Dyndata_Config = { 0,0 };


#define INIT_DATA(arch, count) \
    const size_t DataCount = count; \
    DefaultSize = FIELD_OFFSET(SBIE_DYNDATA, Configs) + ((sizeof(USHORT) + sizeof(SBIE_DYNCONFIG)) * DataCount) + sizeof(USHORT); \
    Default = (PSBIE_DYNDATA)Pool_Alloc(Driver_Pool, DefaultSize); \
    memset(Default, 0x00, DefaultSize); \
    if(!Default) \
        return STATUS_INSUFFICIENT_RESOURCES; \
    Default->Format = DYNDATA_FORMAT; \
    Default->Version = DYNDATA_VERSION; \
    Default->Signature = DYNDATA_SIGN; \
    Default->Arch = arch; \
    Default->Size = sizeof(SBIE_DYNCONFIG); 

#define BEGIN_DATA \
    Default->Count = 0; \
    PSBIE_DYNCONFIG Data = (PSBIE_DYNCONFIG)(((UCHAR*)Default) + FIELD_OFFSET(SBIE_DYNDATA, Configs) + sizeof(USHORT) * DataCount + sizeof(USHORT)); 

#define NEXT_DATA \
    Default->Configs[Default->Count++] = (USHORT)((UCHAR*)Data++ - (UCHAR*)Default);

#define END_DATA \
    Default->Configs[Default->Count] = 0; \
    ASSERT(Default->Count == DataCount);


//---------------------------------------------------------------------------
// Dyn_InitData
//---------------------------------------------------------------------------


_FX NTSTATUS Dyndata_InitDefault(PSBIE_DYNDATA* pDefault, ULONG* pDefaultSize)
{
    PSBIE_DYNDATA Default = NULL;
    ULONG DefaultSize = 0;

#ifdef _M_ARM64

#define DATA_COUNT 3

    INIT_DATA(IMAGE_FILE_MACHINE_ARM64, DATA_COUNT)

    BEGIN_DATA

    // 22000+ - ... // W11 - ...
    Data->OsBuild_max = WIN11_LATEST;
    Data->OsBuild_min = SVR2025;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x518;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x1E0;
    Data->MitigationFlags_offset = 0xA90;
    Data->SignatureLevel_offset = 0x938;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    // 22000+ - ... // W11 - ...
    Data->OsBuild_max = 26020;
    Data->OsBuild_min = WIN11_FIRST;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x518;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x418;
    Data->MitigationFlags_offset = 0xA90;
    Data->SignatureLevel_offset = 0x938;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    // 18980+ - ... / W10 20H1 - W10 20H1
    Data->OsBuild_max = WIN10_LATEST;
    Data->OsBuild_min = 18980;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x4C8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x418;
    Data->MitigationFlags_offset = 0xA90;
    Data->SignatureLevel_offset = 0x938;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    END_DATA

#elif _WIN64

#define DATA_COUNT 20

    INIT_DATA(IMAGE_FILE_MACHINE_AMD64, DATA_COUNT)

    BEGIN_DATA

    // Server2025 / >= 26040
    Data->OsBuild_max = WIN11_LATEST;
    Data->OsBuild_min = SVR2025;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x538;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x1F0;
    Data->MitigationFlags_offset = 0x9d0;
    Data->SignatureLevel_offset = 0x878;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    // >= 20348  & <= 26020
    Data->OsBuild_max = 26020;
    Data->OsBuild_min = SVR2022;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x4f8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x460;
    Data->MitigationFlags_offset = 0x9d0;
    Data->SignatureLevel_offset = 0x878;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    // >= 19013 & <= 19045
    Data->OsBuild_max = WIN10_LATEST;
    Data->OsBuild_min = 19013;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x4a8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x460;
    Data->MitigationFlags_offset = 0x9d0;
    Data->SignatureLevel_offset = 0x878;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 18980 & <= 19008
    Data->OsBuild_max = 19008;
    Data->OsBuild_min = 18980;

    Data->Clipboard_offset = 0x80;

    Data->ImpersonationData_offset = 0x4a8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x460;
    Data->MitigationFlags_offset = 0x9d0;
    Data->SignatureLevel_offset = 0x879;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 18885 & <= 18975 / Windows 10 RS7 FR
    Data->OsBuild_max = 18975;
    Data->OsBuild_min = 18885;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x688;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x318;
    Data->MitigationFlags_offset = 0x890;
    Data->SignatureLevel_offset = 0x738;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 18836 & <= 18875 / Windows 10 May 2019 Update
    Data->OsBuild_max = 18875;
    Data->OsBuild_min = 18836;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x688;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x308;
    Data->MitigationFlags_offset = 0x850;
    Data->SignatureLevel_offset = 0x6f8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 18312 & <= 18363 / Windows 10 May 2019 Update
    Data->OsBuild_max = 18363;
    Data->OsBuild_min = 18312;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x678;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x308;
    Data->MitigationFlags_offset = 0x850;
    Data->SignatureLevel_offset = 0x6f8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 18290 & <= 18309 / Windows 10 RS6 FR
    Data->OsBuild_max = 18309;
    Data->OsBuild_min = 18290;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x668;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x308;
    Data->MitigationFlags_offset = 0x828;
    Data->SignatureLevel_offset = 0x6d0;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 17661 & <= 18282 / Windows 10 RS5 FR
    Data->OsBuild_max = 18282;
    Data->OsBuild_min = 17661;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x668;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x300;
    Data->MitigationFlags_offset = 0x820;
    Data->SignatureLevel_offset = 0x6c8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 16241 & <= 17655 / Flags4_offset in Windows 10 FCU
    Data->OsBuild_max = 17655;
    Data->OsBuild_min = 16241;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x668;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x300;
    Data->MitigationFlags_offset = 0x828; 
    Data->SignatureLevel_offset = 0x6c8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 15042 & <= 16237 / Windows 10,  64-bit
    Data->OsBuild_max = 16237;
    Data->OsBuild_min = 15042;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x668;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x300;  
    Data->SignatureLevel_offset = 0x6c8;
    Data->MitigationFlags_offset = 0x6cc;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 15031 & <= 15031 / Windows 10 Fast Ring build 14965+,  64-bit
    Data->OsBuild_max = 15031;
    Data->OsBuild_min = 15031;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x668;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x308;  
    Data->SignatureLevel_offset = 0x6d0;
    Data->MitigationFlags_offset = 0;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 14965 & <=  15025 / Windows 10 Fast Ring build 14965+,  64-bit
    Data->OsBuild_max = 15025;
    Data->OsBuild_min = 14965;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x660;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x308;  
    Data->SignatureLevel_offset = 0x6d0;
    Data->MitigationFlags_offset = 0;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 14316 & <= 14959 / Windows 10,  64-bit
    Data->OsBuild_max = 14959;
    Data->OsBuild_min = 14316;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x660;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x300;  
    Data->SignatureLevel_offset = 0x6c8;
    Data->MitigationFlags_offset = 0x6cc;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // >= 10240 & <= 14295 / Windows 10,  64-bit
    Data->OsBuild_max = 14295;
    Data->OsBuild_min = WIN10_FIRST;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x658;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x300;  
    Data->SignatureLevel_offset = 0x6c8;
    Data->MitigationFlags_offset = 0x6cc;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W8.1
    Data->OsBuild_max = WIN8_1;
    Data->OsBuild_min = WIN8_1;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x650;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x2F8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W8
    Data->OsBuild_max = WIN8;
    Data->OsBuild_min = WIN8;

    Data->Clipboard_offset = 0x60;

    Data->ImpersonationData_offset = 0x3C8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0xA0;
    Data->UserAndGroups_offset = 0x98;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = 0x2F8;

    Data->TrapFrame_offset = 0x90;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W7
    Data->OsBuild_max = WIN7SP1;
    Data->OsBuild_min = WIN7;

    Data->Clipboard_offset = 0x58;

    Data->ImpersonationData_offset = 0x3E0;

    Data->RestrictedSidCount_offset = 0x7C;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x90;

    Data->Flags2_offset = 0x43C;

    Data->TrapFrame_offset = 0x1d8;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // Vista
    Data->OsBuild_max = WINVISTA;
    Data->OsBuild_min = WINVISTA;

    Data->Clipboard_offset = 0x58;

    Data->ImpersonationData_offset = 0x3B0;

    Data->RestrictedSidCount_offset = 0x7C;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x90;

    Data->Flags2_offset = 0x36C;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // XP
    Data->OsBuild_max = WINXP;
    Data->OsBuild_min = WINXP;

    Data->Clipboard_offset = 0x58;

    Data->ImpersonationData_offset = 0;

    Data->RestrictedSidCount_offset = 0x7C;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x90;

    Data->Flags2_offset = 0x36C;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    END_DATA
        
#else ! _WIN64

#define DATA_COUNT 11

    INIT_DATA(IMAGE_FILE_MACHINE_I386, DATA_COUNT)

    BEGIN_DATA

    // 18980+ - ... / W10 20H1
    Data->OsBuild_max = WIN10_LATEST;
    Data->OsBuild_min = 18980;

    Data->Clipboard_offset = 0x40;

    Data->ImpersonationData_offset = 0x2c8;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;
    Data->SignatureLevel_offset = -1;
    Data->MitigationFlags_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // 18312+ - 18975 / W10 19H1 - W10 20H1
    Data->OsBuild_max = 18975;
    Data->OsBuild_min = 18312;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x3A0;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;
    Data->SignatureLevel_offset = -1;
    Data->MitigationFlags_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // 16237+ - 18309 / W10 RS2 - W10 19H1
    Data->OsBuild_max = 18309;
    Data->OsBuild_min = 16241;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x398;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;
    Data->SignatureLevel_offset = -1;
    Data->MitigationFlags_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // 14965+ - 16237 / W10 RS2 - W10 19H1
    Data->OsBuild_max = 16237;
    Data->OsBuild_min = 14965;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x398;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;
    Data->MitigationFlags_offset = -1;
    Data->SignatureLevel_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // 10240+ - 14959 / W10 - W10 RS2
    Data->OsBuild_max = 14959;
    Data->OsBuild_min = WIN10_FIRST;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x390;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;
    Data->MitigationFlags_offset = -1;
    Data->SignatureLevel_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W8.1
    Data->OsBuild_max = WIN8_1;
    Data->OsBuild_min = WIN8_1;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x380;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W8
    Data->OsBuild_max = WIN8;
    Data->OsBuild_min = WIN8;

    Data->Clipboard_offset = 0x30;

    Data->ImpersonationData_offset = 0x230;

    Data->RestrictedSidCount_offset = 0x80;
    Data->RestrictedSids_offset = 0x98;
    Data->UserAndGroups_offset = 0x94;
    Data->UserAndGroupCount_offset = 0x7c;

    Data->Flags2_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // W7
    Data->OsBuild_max = WIN7SP1;
    Data->OsBuild_min = WIN7;

    Data->Clipboard_offset = 0x2c;

    Data->ImpersonationData_offset = 0x248;

    Data->RestrictedSidCount_offset = 0x7C;
    Data->RestrictedSids_offset = 0x94;
    Data->UserAndGroups_offset = 0x90;

    Data->Flags2_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // Vista
    Data->OsBuild_max = WINVISTA;
    Data->OsBuild_min = WINVISTA;

    Data->Clipboard_offset = 0x2c;

    Data->ImpersonationData_offset = 0x228;

    Data->RestrictedSidCount_offset = 0x7C;
    Data->RestrictedSids_offset = 0x94;
    Data->UserAndGroups_offset = 0x90;

    Data->Flags2_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // Server 2003
    Data->OsBuild_max = 3790;
    Data->OsBuild_min = 3790;

    Data->Clipboard_offset = 0x2c;

    Data->ImpersonationData_offset = 0x204;

    Data->RestrictedSidCount_offset = 0x50;
    Data->RestrictedSids_offset = 0x6C;
    Data->UserAndGroups_offset = 0x68;

    Data->Flags2_offset = -1;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA
    
    // XP
    Data->OsBuild_max = WINXP;
    Data->OsBuild_min = WINXP;

    Data->Clipboard_offset = 0x2c;

    Data->ImpersonationData_offset = 0x20C;

    Data->RestrictedSidCount_offset = 0x50;
    Data->RestrictedSids_offset = 0x6C;
    Data->UserAndGroups_offset = 0x68;

    Data->Flags2_offset = 0x224;

    Data->ServiceTable_offset = -1;
    //

    NEXT_DATA

    END_DATA

#endif _WIN64

    *pDefault = Default;
    *pDefaultSize = DefaultSize;

    return STATUS_SUCCESS;
}

#ifndef ISSIGNTOOL

//---------------------------------------------------------------------------
// Dyndata_LoadData
//---------------------------------------------------------------------------


_FX NTSTATUS Dyndata_LoadData()
{
    NTSTATUS status;
    WCHAR* path = NULL;
    ULONG  path_len = 0;

    PSBIE_DYNDATA Default = NULL;
    ULONG DefaultSize = 0;
    PSBIE_DYNDATA Custom = NULL;
    ULONG CustomSize = 0;
    PSBIE_DYNDATA Dyndata = NULL;
    ULONG DyndataSize = 0;

    Dyndata_Active = FALSE;
    memset(&Dyndata_Config, 0, sizeof(Dyndata_Config));

    //
    // load dyn data from registry
    // 
    // L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\SbieDrv\\Parameters"
    // L"DynData" / L"DynDataSig"
    //

    path_len = wcslen(Driver_RegistryPath) * sizeof(WCHAR) + sizeof(Parameters);
    path = Pool_Alloc(Driver_Pool, path_len);
    if (!path) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto finish;
    }
    wcscpy(path, Driver_RegistryPath);
    wcscat(path, Parameters);
    //DbgPrint("Sbie Parameters: %S\r\n", path);

    status = GetRegValue(path, L"DynData", &Custom, &CustomSize);
    if (NT_SUCCESS(status))
    {
#ifdef DYN_DEBUG
        DbgPrint("Sbie loaded custom DYNDATA, Size %d\r\n", CustomSize);
#endif

        //
        // Check if the provided dyn data matches our system architecture
        //

        if (Custom->Format != DYNDATA_FORMAT || Custom->Signature != DYNDATA_SIGN) {
            status = STATUS_INVALID_IMAGE_FORMAT;
        }
#ifdef _M_ARM64
        else if (Custom->Arch != IMAGE_FILE_MACHINE_ARM64) {
#elif _WIN64
        else if (Custom->Arch != IMAGE_FILE_MACHINE_AMD64) {
#else
        else if (Custom->Arch != IMAGE_FILE_MACHINE_I386) {
#endif
            status = STATUS_IMAGE_MACHINE_TYPE_MISMATCH;
        }

        //
        // Check if the provided dyn data is properly signed,
        // but only when not in test signign mode
        //

        else if(!Driver_OsTestSigning) {
            UCHAR data_sig[128];
            PVOID sig_ptr = data_sig;
            ULONG sig_len = sizeof(data_sig);
            status = GetRegValue(path, L"DynDataSig", &sig_ptr, &sig_len);
            if (NT_SUCCESS(status))
                status = KphVerifyBuffer((UCHAR*)Custom, CustomSize, sig_ptr, sig_len);
        }

        if (!NT_SUCCESS(status)) {
            WCHAR err[11];
            RtlStringCbPrintfW(err, sizeof(err), L"0x%X", status);
            Log_Msg1(MSG_1205, err);
        }
    }

    //
    // select whichever version of dyndata is newer
    // if they are the same, prefer built-in version
    // when no dyndata was loaded, use the hardcoded defaults
    //

    if (NT_SUCCESS(status) && Custom && Custom->Version > DYNDATA_VERSION)
    {
        Dyndata = Custom;
        DyndataSize = CustomSize;

#ifdef DYN_DEBUG
        DbgPrint("Sbie selected DYNDATA version %d\r\n", Dyndata->Version);
#endif
    }
    else 
    {
#ifdef DYN_DEBUG
        DbgPrint("Sbie selected default DYNDATA\r\n");
#endif

        status = Dyndata_InitDefault(&Default, &DefaultSize);
        if (NT_SUCCESS(status)) 
        {
            Dyndata = Default;
            DyndataSize = DefaultSize;

//            SetRegValue(path, L"DynData", Dyndata, DyndataSize);
// 
//#ifdef _M_ARM64
//            const unsigned char DynDataSig[64] =  {};
//#elif _WIN64
//            const unsigned char DynDataSig[64] =  {};
//#else ! _WIN64
//            const unsigned char DynDataSig[64] =  {};
//#endif _WIN64
//            SetRegValue(path, L"DynDataSig", DynDataSig, sizeof(DynDataSig));
        }
    }

    //
    // find suitable dynamic config for the current OS Build
    //

    if (NT_SUCCESS(status))
    {
        status = STATUS_INCOMPATIBLE_FILE_MAP;

        PSBIE_DYNCONFIG DataMatch = NULL;
        PSBIE_DYNCONFIG DataExp = NULL;

        for (USHORT Index = 0; Index < Dyndata->Count; Index++)
        {
            USHORT Offset = Dyndata->Configs[Index];
            if (!Offset) break;
            PSBIE_DYNCONFIG Data = (PSBIE_DYNCONFIG)((UCHAR*)Dyndata + Offset);
            if ((UCHAR*)Data > (UCHAR*)Dyndata + DyndataSize) continue;

#ifdef DYN_DEBUG
                DbgPrint("Sbie testing DYNDATA %d <= %d <= %d\r\n", Data->OsBuild_min, Driver_OsBuild, Data->OsBuild_max);
#endif

            //
            // Find an exact match for the current Windows build
            //

            if (Driver_OsBuild >= Data->OsBuild_min && Driver_OsBuild <= Data->OsBuild_max)
            {
//#ifdef DYN_DEBUG
                DbgPrint("Sbie found DYNDATA %d <= %d <= %d\r\n", Data->OsBuild_min, Driver_OsBuild, Data->OsBuild_max);
//#endif
                DataMatch = Data;
                break;
            }

            //
            // Fallback: find the latest entry for which the current OS build is greater or equal to the minimal supported build of this entry
            //

            else if (Driver_OsBuild >= Data->OsBuild_min && DataExp == NULL)
                DataExp = Data;
        }

        if (!DataMatch && DataExp)
        {
            //
            // Try detecting insider build
            //

            if (GetRegDword(L"\\Registry\\Machine\\Software\\Microsoft\\WindowsSelfHost\\Applicability", L"EnablePreviewBuilds") != 0) 
            {
                DbgPrint("Sbie detected insider build %d\r\n", Driver_OsBuild);

                DataMatch = DataExp;
            }

            //
            // Allow the last offsets to be used with not yet known to be compatible Windows builds
            // 
            // L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Services\\SbieDrv\\Parameters"
            // L"AllowOutdatedOffsets"
            //

            else if(GetRegDword(path, L"AllowOutdatedOffsets") || Driver_OsTestSigning)
            {
                DataMatch = DataExp;
            }
        }

        if (DataMatch)
        {
            USHORT Size = Dyndata->Size;
            if (Size > sizeof(Dyndata_Config)) // if we have a bigger structure, ignore the new fields
                Size = sizeof(Dyndata_Config);
            memcpy(&Dyndata_Config, DataMatch, Size);

            if (DataMatch == DataExp) // set experimental flag if this DynData is not an exact match
                Dyndata_Config.Flags |= DYNDATA_FLAG_EXP;

            Dyndata_Active = TRUE;
            status = STATUS_SUCCESS;
        }
    }

    if (!Dyndata_Active) {
//#ifdef DYN_DEBUG
        DbgPrint("Sbie no compatible DYNDATA found, OsBuild: %d\r\n", Driver_OsBuild);
//#endif
        WCHAR info[12];
        RtlStringCbPrintfW(info, sizeof(info), L"%d", Driver_OsBuild);
        WCHAR err[11];
        RtlStringCbPrintfW(err, sizeof(err), L"0x%X", status);
        Log_Msg2(MSG_1206, info, err);
    }

finish:

    if(path)
        Pool_Free(path, path_len);

    if(Default)
        Pool_Free(Default, DefaultSize);

    if(Custom)
        Pool_Free(Custom, CustomSize);

    return status;
}


//---------------------------------------------------------------------------
// Dyndata_Init
//---------------------------------------------------------------------------


_FX BOOLEAN Dyndata_Init()
{
    NTSTATUS status;

    status = Dyndata_LoadData();

    return TRUE;
}

#endif

