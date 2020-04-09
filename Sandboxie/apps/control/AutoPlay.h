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


#ifndef _MY_AUTOPLAY_H
#define _MY_AUTOPLAY_H


#include "shobjidl.h"


class CAutoPlay : public IQueryCancelAutoPlay
{

private:

    static CAutoPlay *m_instance;

    ULONG m_refcount;

    IMoniker *m_pMoniker;
    IRunningObjectTable *m_pRunningObjectTable;
    ULONG m_cookie;

    void LogAutoPlay(const WCHAR *boxname, WCHAR drive);

public:

    //
    // IUnknown methods
    //

    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    //
    // IQueryCancelAutoPlay methods
    //

    STDMETHOD(AllowAutoPlay)(
        const WCHAR *path, ULONG cttype, const WCHAR *label, ULONG sn);

    //
    // static methods
    //

    static void Install();
    static void Remove();
};


#endif // _MY_AUTOPLAY_H
