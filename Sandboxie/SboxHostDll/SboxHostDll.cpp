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

// SboxHostDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "core/dll/sbiedll.h"

typedef BOOL (*P_OpenProcessToken)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE phTokenOut);

static P_OpenProcessToken   __sys_OpenProcessToken = NULL;


BOOL SboxHostDll_OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE phTokenOut)
{
    BOOL bRet = FALSE;
    HANDLE hToken = 0;
    std::wstring wsLogonSid;

    // if caller provide null-phTokenOut, pass it to the original call to have the same last error.
    if (__sys_OpenProcessToken)
    {
        bRet = __sys_OpenProcessToken(ProcessHandle, DesiredAccess, phTokenOut? &hToken:phTokenOut);
    }

    if (bRet)
    {
        if (SbieApi_QueryProcessInfo((HANDLE)GetProcessId(ProcessHandle), 0) & SBIE_FLAG_VALID_PROCESS)
        {
            BOOL bNeedAnotherValidToken = FALSE;
            LUID idLogonSession = {0};
            ATL::CAccessToken token;
            ATL::CSid userSid;
            ATL::CSid logonSid;
            token.Attach(hToken);

            //if(token.IsTokenRestricted())
            //{
                if(token.GetUser(&userSid))
                {
                    // this token is usable for clicktorun service
                    if (lstrcmpi(userSid.Sid(), _T("S-1-5-7"))==0)
                    {
                        if(token.GetLogonSid(&logonSid))
                        {
                            wsLogonSid = logonSid.Sid();
                            bNeedAnotherValidToken = TRUE;
                        }
                    }
                }
            //}

            token.Detach();

            if (bNeedAnotherValidToken)
            {
                // get a host process token with the same wsLogonSid.
                HANDLE hAltToken = 0;
                DWORD aPid[1024], cbReturned, nPidCount;

                if (EnumProcesses( aPid, sizeof(aPid), &cbReturned ) )
                {
                    nPidCount = cbReturned / sizeof(DWORD);
                    for ( UINT i = 0; i < nPidCount; i++ )
                    {
                        if (!SbieApi_QueryProcessInfo((HANDLE)aPid[i], 0) & SBIE_FLAG_VALID_PROCESS)
                        {
                            HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, aPid[i]);
                            if (hProc)
                            {
                                if (__sys_OpenProcessToken(hProc, DesiredAccess, &hAltToken))
                                {
                                    ATL::CAccessToken altToken;
                                    altToken.Attach(hAltToken);

                                    if(altToken.GetLogonSid(&logonSid))
                                    {
                                        std::wstring wsAltLogonSid = logonSid.Sid();
                                        if (wsAltLogonSid == wsLogonSid)
                                        {
                                            altToken.Detach();
                                            CloseHandle(hProc);
                                            break;
                                        }
                                    }

                                    altToken.Detach();

                                    CloseHandle(hAltToken);
                                    hAltToken = 0;
                                }
                                CloseHandle(hProc);
                            }
                        }
                    }
                }

                if (hAltToken)
                {
                    CloseHandle(hToken);
                    hToken = hAltToken;
                    bRet = TRUE;
                }
            }
        }

        if (phTokenOut)
            *phTokenOut = hToken;
    }

    return bRet;
}

BOOLEAN InitHook( HINSTANCE hSbieDll )
{
    if (hSbieDll)
    {
        HMODULE hAdvapi32 = GetModuleHandle(L"Advapi32.dll");

        void *OpenProcessToken = (P_OpenProcessToken)GetProcAddress(hAdvapi32, "OpenProcessToken");

        if (OpenProcessToken)
            SBIEDLL_HOOK(SboxHostDll_, OpenProcessToken);
    }
    return TRUE;
}