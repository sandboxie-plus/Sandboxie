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
// Command Line Parser Utility
//---------------------------------------------------------------------------

#include "stdafx.h"

#include <windows.h>
#include "MyCmdLine.h"
#include "common/defines.h"


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CMyCmdLine::CMyCmdLine(const WCHAR *CommandLineString)
{
    if (CommandLineString)
        m_Next = CommandLineString;
    else
        m_Next = GetCommandLine();
    if (! m_Next)
        m_Next = L"";

    m_Prev = NULL;
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CMyCmdLine::~CMyCmdLine()
{
    if (m_Prev)
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, m_Prev);
}


//---------------------------------------------------------------------------
// EatString
//---------------------------------------------------------------------------


const WCHAR *CMyCmdLine::EatString(const WCHAR *str)
{
    bool quoted = false;

    while (*str == L' ')
        ++str;

    while (*str) {
        if (*str == L'\"')
            quoted = !quoted;
        else if (*str == L' ' && (! quoted))
            break;
        ++str;
    }

    while (*str == L' ')
        ++str;

    return str;
}


//---------------------------------------------------------------------------
// Count
//---------------------------------------------------------------------------


int CMyCmdLine::Count()
{
    const WCHAR *ptr = m_Next;
    int n = 0;
    do {
        ++n;
        ptr = EatString(ptr);
    } while (*ptr);
    return (n - 1);
}


//---------------------------------------------------------------------------
// Next
//---------------------------------------------------------------------------


const WCHAR *CMyCmdLine::Next()
{
    if (m_Prev) {
        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, m_Prev);
        m_Prev = NULL;
    }

    m_Next = EatString(m_Next);
    if (*m_Next) {

        const WCHAR *Next2 = m_Next;
        while (*Next2 && *Next2 != L' ')
            ++Next2;

        ULONG len = (ULONG)(Next2 - m_Next);
        m_Prev = (WCHAR *)HeapAlloc(GetProcessHeap(), 0,
                                    (len + 1) * sizeof(WCHAR));
        if (m_Prev) {
            wmemcpy(m_Prev, m_Next, len);
            m_Prev[len] = L'\0';
        }
    }

    return m_Prev;
}
