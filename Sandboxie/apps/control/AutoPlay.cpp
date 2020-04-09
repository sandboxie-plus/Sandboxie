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
// AutoPlay
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "MyApp.h"
#include "AutoPlay.h"

#include "SbieIni.h"
#include "Boxes.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CAutoPlay *CAutoPlay::m_instance = NULL;


static GUID CLSID_MyAutoPlay = { MY_AUTOPLAY_CLSID };


//---------------------------------------------------------------------------
// QueryInterface
//---------------------------------------------------------------------------


HRESULT CAutoPlay::QueryInterface(REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IQueryCancelAutoPlay)) {

        ++m_refcount;
        *ppv = this;
        return S_OK;
    }

    return E_NOINTERFACE;
}


//---------------------------------------------------------------------------
// AddRef
//---------------------------------------------------------------------------


ULONG CAutoPlay::AddRef()
{
    return ++m_refcount;
}


//---------------------------------------------------------------------------
// Release
//---------------------------------------------------------------------------


ULONG CAutoPlay::Release()
{
    return --m_refcount;
}


//---------------------------------------------------------------------------
// AllowAutoPlay
//---------------------------------------------------------------------------


HRESULT CAutoPlay::AllowAutoPlay(
    const WCHAR *path, ULONG cttype, const WCHAR *label, ULONG sn)
{

    ULONG state;
    SbieApi_DisableForceProcess(NULL, &state);
    if (state != 0)
        return S_OK;

    WCHAR drive = towupper(path[0]);

    CBoxes &boxes = CBoxes::GetInstance();
    for (int i = (int)boxes.GetSize() - 1; i >= 1; --i) {

        CBox &box = boxes.GetBox(i);
        CStringList folders;
        box.GetProcessList(L'O', folders);

        while (! folders.IsEmpty()) {
            CString fol = folders.RemoveHead();
            ULONG len = fol.GetLength();
            if (len <= 3 && fol.GetAt(1) == L':') {
                if (towupper(fol.GetAt(0)) == drive) {

                    LogAutoPlay(box.GetName(), drive);
                    return S_FALSE;
                }
            }
        }
    }

    return S_OK;
}


//---------------------------------------------------------------------------
// LogAutoPlay
//---------------------------------------------------------------------------


void CAutoPlay::LogAutoPlay(const WCHAR *boxname, WCHAR drive)
{
    WCHAR *tmp = new WCHAR[(wcslen(boxname) + 16)];
    if (! tmp)
        return;

    wcscpy(tmp, boxname);
    WCHAR *tmp2 = tmp + wcslen(tmp);
    *tmp2 = L' ';
    ++tmp2;
    wcscpy(tmp2, L"*AUTOPLAY*");
    tmp2 += 10;
    *tmp2 = drive;
    ++tmp2;
    *tmp2 = L'\0';

    SbieApi_Log(2199, tmp);

    delete tmp;
}


//---------------------------------------------------------------------------
// Install
//---------------------------------------------------------------------------


void CAutoPlay::Install()
{
    HRESULT hr;
    IMoniker *pMoniker;
    IRunningObjectTable *pRunningObjectTable;
    ULONG cookie;

    if (CMyApp::m_Windows2000)
        return;

    CAutoPlay *pAutoPlay = new CAutoPlay();
    if (! pAutoPlay)
        return;
    CAutoPlay::m_instance = pAutoPlay;

    pAutoPlay->m_refcount = 1;
    pAutoPlay->m_pMoniker = NULL;
    pAutoPlay->m_pRunningObjectTable = NULL;
    pAutoPlay->m_cookie = 0;

    hr = CreateClassMoniker(CLSID_MyAutoPlay, &pMoniker);
    if (FAILED(hr))
        return;
    pAutoPlay->m_pMoniker = pMoniker;

    hr = GetRunningObjectTable(0, &pRunningObjectTable);
    if (FAILED(hr))
        return;
    pAutoPlay->m_pRunningObjectTable = pRunningObjectTable;

    hr = pRunningObjectTable->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE,
                                       pAutoPlay,
                                       pMoniker,
                                       &cookie);
    if (FAILED(hr))
        return;
    pAutoPlay->m_cookie = cookie;
}


//---------------------------------------------------------------------------
// Remove
//---------------------------------------------------------------------------


void CAutoPlay::Remove()
{
    CAutoPlay *pAutoPlay = CAutoPlay::m_instance;
    if (! pAutoPlay)
        return;

    if (pAutoPlay->m_cookie) {

        pAutoPlay->m_pRunningObjectTable->Revoke(pAutoPlay->m_cookie);
        pAutoPlay->m_cookie = 0;
    }

    if (pAutoPlay->m_pRunningObjectTable)
        pAutoPlay->m_pRunningObjectTable->Release();

    if (pAutoPlay->m_pMoniker)
        pAutoPlay->m_pMoniker->Release();
}
