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
// MyPool Memory Management
//---------------------------------------------------------------------------


#ifndef _MY_MYPOOL_H
#define _MY_MYPOOL_H


class CMyPool
{
private:

    void *m_pool;

public:

    CMyPool();
    ~CMyPool();

    void *Alloc(int size);
    void Free(void *ptr);

    char      *Alloc_char(int size)     {return (char *)Alloc(size);};
    UCHAR     *Alloc_UCHAR(int size)    {return (UCHAR *)Alloc(size);};
    WCHAR     *Alloc_WCHAR(int size)    {return (WCHAR *)Alloc(size);};
    ULONG_PTR *Alloc_ULONG_PTR(int size){return (ULONG_PTR *)Alloc(size);};
};


#endif // _MY_MYPOOL_H
