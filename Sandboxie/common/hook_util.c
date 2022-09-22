/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
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
// Variouse generic hooking helpers
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// Hook_CheckChromeHook
//---------------------------------------------------------------------------


#ifdef _WIN64
#define MAX_FUNC_SIZE 0x76
ULONGLONG * findChromeTarget(unsigned char* addr)
{
    int i = 0;
    ULONGLONG target;
    ULONGLONG * ChromeTarget = NULL;
    if (!addr) return NULL;

    //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
    //So far the offset has been positive between 0xa00000 and 0xb00000 bytes;
    //This may change in a future version of chrome
    for (i = 0; i < MAX_FUNC_SIZE; i++) {

        // some chromium 90+ derivatives replace the function with a return 1 stub
        // mov eax,1
        // ret
        // int 3
        if (addr[i] == 0xB8 && addr[i + 5] == 0xC3 && addr[i + 6] == 0xCC)
            return NULL;

        if ((*(USHORT *)&addr[i] == 0x8b48)) {

            //Look for mov rcx,[target 4 byte offset] or in some cases mov rax,[target 4 byte offset]
            if ((addr[i + 2] == 0x0d || addr[i + 2] == 0x05)) {
                LONG delta;
                target = (ULONG_PTR)(addr + i + 7);
                delta = *(LONG *)&addr[i + 3];

                //check if offset is close to the expected value (is positive and less than 0x100000 as of chrome 64) 
        //      if (delta > 0 && delta < 0x100000 )  { //may need to check delta in a future version of chrome
                target += delta;
                ChromeTarget = *(ULONGLONG **)target;

                // special case when compiled using mingw toolchain
                // mov rcx,qword ptr [rax+offset] or mov rcx,qword ptr [rcx+offset]
                if ((*(USHORT *)&addr[i + 7] == 0x8B48)) 
                {
                    if (addr[i + 9] == 0x48 || addr[i + 9] == 0x49)
                        delta = addr[i + 10];
                    else if (addr[i + 9] == 0x88 || addr[i + 9] == 0x89)
                        delta = *(ULONG*)&addr[i + 10];
                    else
                        break;
                    target = (ULONGLONG)ChromeTarget + delta;
                    ChromeTarget = *(ULONGLONG **)target;
                }

        //      }
                break;
            }
        }
    }

    return ChromeTarget;
}
#endif

_FX void* Hook_CheckChromeHook(void *SourceFunc)
{
    if (!SourceFunc)
        return NULL;
#ifdef  _WIN64
    UCHAR *func = (UCHAR *)SourceFunc;
    ULONGLONG *chrome64Target = NULL;

    if (func[0] == 0x50 &&	//push rax
        func[1] == 0x48 &&	//mov rax,?
        func[2] == 0xb8) {
        ULONGLONG *longlongs = *(ULONGLONG **)&func[3];
        chrome64Target = findChromeTarget((unsigned char *)longlongs);
    }
    // Chrome 49+ 64bit hook
    // mov rax, <target> 
    // jmp rax 
    else if (func[0] == 0x48 && //mov rax,<target>
        func[1] == 0xb8 &&
        *(USHORT *)&func[10] == 0xe0ff) /* jmp rax */ {
        ULONGLONG *longlongs = *(ULONGLONG **)&func[2];
        chrome64Target = findChromeTarget((unsigned char *)longlongs);
    }
    if (chrome64Target) {
        SourceFunc = chrome64Target;
    }
    /*sboxie 64bit jtable hook signature */
        /* // use this to hook jtable location (useful for debugging)
        //else if(func[0] == 0x51 && func[1] == 0x48 && func[2] == 0xb8 ) {
        else if(func[0] == 0x90 && func[1] == 0x48 && func[2] == 0xb8 ) {
            long long addr;
            addr = (ULONG_PTR) *(ULONGLONG **)&func[3] ;
            SourceFunc = (void *) addr;
        }
        */
#else
    UCHAR *func = (UCHAR *)SourceFunc;
    if (func[0] == 0xB8 &&                  // mov eax,?
        func[5] == 0xBA &&                  // mov edx,?
        *(USHORT *)&func[10] == 0xE2FF)     // jmp edx
    {
        ULONG i = 0;
        ULONG *longs = *(ULONG **)&func[6];

        for (i = 0; i < 20; i++, longs++)
        {
            if (longs[0] == 0x5208EC83 && longs[1] == 0x0C24548B &&
                longs[2] == 0x08245489 && longs[3] == 0x0C2444C7 &&
                longs[5] == 0x042444C7)
            {
                SourceFunc = (void *)longs[4];
                break;
            }
        }
    }
#endif ! _WIN64
    return SourceFunc;
}