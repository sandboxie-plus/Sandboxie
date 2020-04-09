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
// propsys.h from Windows Vista/7 PSDK
//---------------------------------------------------------------------------


#ifndef _MY_PROPSYS_H
#define _MY_PROPSYS_H


#define COBJMACROS
#include <objbase.h>


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------



typedef const PROPERTYKEY *REFPROPERTYKEY;

typedef interface IPropertyStore IPropertyStore;


typedef struct IPropertyStoreVtbl
{
    BEGIN_INTERFACE

    HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
        IPropertyStore * This,
        REFIID riid,
        void **ppvObject);

    ULONG ( STDMETHODCALLTYPE *AddRef )(
        IPropertyStore * This);

    ULONG ( STDMETHODCALLTYPE *Release )(
        IPropertyStore * This);

    HRESULT ( STDMETHODCALLTYPE *GetCount )(
        IPropertyStore * This,
        DWORD *cProps);

    HRESULT ( STDMETHODCALLTYPE *GetAt )(
        IPropertyStore * This,
        DWORD iProp,
        PROPERTYKEY *pkey);

    HRESULT ( STDMETHODCALLTYPE *GetValue )(
        IPropertyStore * This,
        REFPROPERTYKEY key,
        PROPVARIANT *pv);

    HRESULT ( STDMETHODCALLTYPE *SetValue )(
        IPropertyStore * This,
        REFPROPERTYKEY key,
        REFPROPVARIANT propvar);

    HRESULT ( STDMETHODCALLTYPE *Commit )(
        IPropertyStore * This);

    END_INTERFACE
} IPropertyStoreVtbl;

interface IPropertyStore
{
    CONST_VTBL struct IPropertyStoreVtbl *lpVtbl;
};


//---------------------------------------------------------------------------
// Property Keys
//---------------------------------------------------------------------------


#ifdef PROPSYS_INITGUID
#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
#else
#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY name
#endif // PROPSYS_INITGUID


DEFINE_PROPERTYKEY(PKEY_AppUserModel_ID, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 5);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_RelaunchCommand, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 2);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_RelaunchDisplayNameResource, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 4);
DEFINE_PROPERTYKEY(PKEY_AppUserModel_RelaunchIconResource, 0x9F4C2855, 0x9F79, 0x4B39, 0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3, 3);


//---------------------------------------------------------------------------


#endif /* _MY_PROPSYS_H */
