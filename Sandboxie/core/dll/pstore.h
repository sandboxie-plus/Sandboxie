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



/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* Compiler settings for pstore.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __pstore_h__
#define __pstore_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IEnumPStoreItems_FWD_DEFINED__
#define __IEnumPStoreItems_FWD_DEFINED__
typedef interface IEnumPStoreItems IEnumPStoreItems;
#endif 	/* __IEnumPStoreItems_FWD_DEFINED__ */


#ifndef __IEnumPStoreTypes_FWD_DEFINED__
#define __IEnumPStoreTypes_FWD_DEFINED__
typedef interface IEnumPStoreTypes IEnumPStoreTypes;
#endif 	/* __IEnumPStoreTypes_FWD_DEFINED__ */


#ifndef __IPStore_FWD_DEFINED__
#define __IPStore_FWD_DEFINED__
typedef interface IPStore IPStore;
#endif 	/* __IPStore_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_pstore_0000 */
/* [local] */ 

#define PST_KEY_CURRENT_USER    0
#define PST_KEY_LOCAL_MACHINE   1
#define PST_E_OK                     0x00000000L
#define PST_E_TYPE_EXISTS            0x800C0004L
#define PST_E_TYPE_NO_EXISTS		0x800C0005L
#define PST_E_ITEM_EXISTS            0x800C000FL
#define PST_E_ITEM_NO_EXISTS		0x800C0010L
#define PST_NO_OVERWRITE             0x00000002L
#define PST_UNRESTRICTED_ITEMDATA    0x00000004L
#define PST_AUTHENTICODE        1
#define PST_BINARY_CHECK        2
#define PST_SECURITY_DESCRIPTOR 4
typedef DWORD PST_ACCESSMODE;

typedef DWORD PST_ACCESSCLAUSETYPE;

typedef DWORD PST_KEY;

typedef DWORD PST_PROVIDERCAPABILITIES;

typedef GUID PST_PROVIDERID;

typedef GUID *PPST_PROVIDERID;

typedef struct _PST_PROVIDERINFO
    {
    DWORD cbSize;
    PST_PROVIDERID ID;
    PST_PROVIDERCAPABILITIES Capabilities;
    LPWSTR szProviderName;
    } 	PST_PROVIDERINFO;

typedef struct _PST_PROVIDERINFO *PPST_PROVIDERINFO;

typedef /* [public][public][public][public][public][public][public][public] */ struct __MIDL___MIDL_itf_pstore_0000_0001
    {
    DWORD cbSize;
    PST_ACCESSCLAUSETYPE ClauseType;
    DWORD cbClauseData;
    BYTE *pbClauseData;
    } 	PST_ACCESSCLAUSE;

typedef struct __MIDL___MIDL_itf_pstore_0000_0001 *PPST_ACCESSCLAUSE;

typedef /* [public][public][public][public][public][public] */ struct __MIDL___MIDL_itf_pstore_0000_0002
    {
    DWORD cbSize;
    PST_ACCESSMODE AccessModeFlags;
    DWORD cClauses;
    PST_ACCESSCLAUSE *rgClauses;
    } 	PST_ACCESSRULE;

typedef struct __MIDL___MIDL_itf_pstore_0000_0002 *PPST_ACCESSRULE;

typedef /* [public] */ struct __MIDL___MIDL_itf_pstore_0000_0003
    {
    DWORD cbSize;
    DWORD cClause;
    PST_ACCESSRULE *rgRules;
    } 	PST_ACCESSRULESET;

typedef struct __MIDL___MIDL_itf_pstore_0000_0003 *PPST_ACCESSRULESET;

typedef /* [public] */ struct __MIDL___MIDL_itf_pstore_0000_0004
    {
    DWORD cbSize;
    LPWSTR szDisplayName;
    } 	PST_TYPEINFO;

typedef struct __MIDL___MIDL_itf_pstore_0000_0004 *PPST_TYPEINFO;

typedef /* [public] */ struct __MIDL___MIDL_itf_pstore_0000_0005
    {
    DWORD cbSize;
    DWORD dwPromptFlags;
    DWORD *hwndApp;
    LPCWSTR szPrompt;
    } 	PST_PROMPTINFO;

typedef struct __MIDL___MIDL_itf_pstore_0000_0005 *PPST_PROMPTINFO;



extern RPC_IF_HANDLE __MIDL_itf_pstore_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_pstore_0000_v0_0_s_ifspec;

#ifndef __IEnumPStoreItems_INTERFACE_DEFINED__
#define __IEnumPStoreItems_INTERFACE_DEFINED__

/* interface IEnumPStoreItems */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IEnumPStoreItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C83B307-0B70-4726-8F75-396EBBDAA401")
    IEnumPStoreItems : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ DWORD celt,
            /* [in] */ LPWSTR *rgelt,
            /* [in] */ DWORD *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ DWORD celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [in] */ IEnumPStoreItems **ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumPStoreItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPStoreItems * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPStoreItems * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPStoreItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPStoreItems * This,
            /* [in] */ DWORD celt,
            /* [in] */ LPWSTR *rgelt,
            /* [in] */ DWORD *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPStoreItems * This,
            /* [in] */ DWORD celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPStoreItems * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPStoreItems * This,
            /* [in] */ IEnumPStoreItems **ppenum);
        
        END_INTERFACE
    } IEnumPStoreItemsVtbl;

    interface IEnumPStoreItems
    {
        CONST_VTBL struct IEnumPStoreItemsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPStoreItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPStoreItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPStoreItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPStoreItems_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPStoreItems_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPStoreItems_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPStoreItems_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Next_Proxy( 
    IEnumPStoreItems * This,
    /* [in] */ DWORD celt,
    /* [in] */ LPWSTR *rgelt,
    /* [in] */ DWORD *pceltFetched);


void __RPC_STUB IEnumPStoreItems_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Skip_Proxy( 
    IEnumPStoreItems * This,
    /* [in] */ DWORD celt);


void __RPC_STUB IEnumPStoreItems_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Reset_Proxy( 
    IEnumPStoreItems * This);


void __RPC_STUB IEnumPStoreItems_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreItems_Clone_Proxy( 
    IEnumPStoreItems * This,
    /* [in] */ IEnumPStoreItems **ppenum);


void __RPC_STUB IEnumPStoreItems_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumPStoreItems_INTERFACE_DEFINED__ */


#ifndef __IEnumPStoreTypes_INTERFACE_DEFINED__
#define __IEnumPStoreTypes_INTERFACE_DEFINED__

/* interface IEnumPStoreTypes */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IEnumPStoreTypes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C83B307-0B70-4726-8F75-396EBBDAA402")
    IEnumPStoreTypes : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ DWORD celt,
            /* [in] */ GUID *rgelt,
            /* [in] */ DWORD *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ DWORD celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
            /* [in] */ IEnumPStoreTypes **ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEnumPStoreTypesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumPStoreTypes * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumPStoreTypes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumPStoreTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumPStoreTypes * This,
            /* [in] */ DWORD celt,
            /* [in] */ GUID *rgelt,
            /* [in] */ DWORD *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumPStoreTypes * This,
            /* [in] */ DWORD celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumPStoreTypes * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumPStoreTypes * This,
            /* [in] */ IEnumPStoreTypes **ppenum);
        
        END_INTERFACE
    } IEnumPStoreTypesVtbl;

    interface IEnumPStoreTypes
    {
        CONST_VTBL struct IEnumPStoreTypesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumPStoreTypes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumPStoreTypes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumPStoreTypes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumPStoreTypes_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumPStoreTypes_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumPStoreTypes_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumPStoreTypes_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Next_Proxy( 
    IEnumPStoreTypes * This,
    /* [in] */ DWORD celt,
    /* [in] */ GUID *rgelt,
    /* [in] */ DWORD *pceltFetched);


void __RPC_STUB IEnumPStoreTypes_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Skip_Proxy( 
    IEnumPStoreTypes * This,
    /* [in] */ DWORD celt);


void __RPC_STUB IEnumPStoreTypes_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Reset_Proxy( 
    IEnumPStoreTypes * This);


void __RPC_STUB IEnumPStoreTypes_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumPStoreTypes_Clone_Proxy( 
    IEnumPStoreTypes * This,
    /* [in] */ IEnumPStoreTypes **ppenum);


void __RPC_STUB IEnumPStoreTypes_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IEnumPStoreTypes_INTERFACE_DEFINED__ */


#ifndef __IPStore_INTERFACE_DEFINED__
#define __IPStore_INTERFACE_DEFINED__

/* interface IPStore */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IPStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C83B307-0B70-4726-8F75-396EBBDAA403")
    IPStore : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInfo( 
            /* [in] */ PPST_PROVIDERINFO *ppProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProvParam( 
            /* [in] */ DWORD dwParam,
            /* [out] */ DWORD *pcbData,
            /* [out] */ BYTE **ppbData,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProvParam( 
            /* [in] */ DWORD dwParam,
            /* [in] */ DWORD cbData,
            /* [in] */ BYTE *pbData,
            /* [in] */ DWORD *dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateType( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ PPST_TYPEINFO **ppInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteType( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateSubtype( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET pRules,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSubtypeInfo( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO **ppInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteSubtype( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadAccessRuleset( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET **ppRules,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteAccessRuleset( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET pRules,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumTypes( 
            /* [in] */ PST_KEY Key,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreTypes **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumSubtypes( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreTypes **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteItem( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubType,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadItem( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ DWORD *cbData,
            /* [in] */ BYTE **ppbData,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteItem( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ DWORD cbData,
            /* [size_is][in] */ BYTE *pbData,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwDefaultConfirmationStyle,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenItem( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ PST_ACCESSMODE ModeFlags,
            /* [in] */ PPST_PROMPTINFO pProomptInfo,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseItem( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR *szItemName,
            /* [in] */ DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumItems( 
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreItems **ppenum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPStore * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPStore * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPStore * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetInfo )( 
            IPStore * This,
            /* [in] */ PPST_PROVIDERINFO *ppProperties);
        
        HRESULT ( STDMETHODCALLTYPE *GetProvParam )( 
            IPStore * This,
            /* [in] */ DWORD dwParam,
            /* [out] */ DWORD *pcbData,
            /* [out] */ BYTE **ppbData,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetProvParam )( 
            IPStore * This,
            /* [in] */ DWORD dwParam,
            /* [in] */ DWORD cbData,
            /* [in] */ BYTE *pbData,
            /* [in] */ DWORD *dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CreateType )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ PPST_TYPEINFO **ppInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteType )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CreateSubtype )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET pRules,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetSubtypeInfo )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO **ppInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteSubtype )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ReadAccessRuleset )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET **ppRules,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *WriteAccessRuleset )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ const GUID *pSubtype,
            /* [in] */ PPST_TYPEINFO pInfo,
            /* [in] */ PPST_ACCESSRULESET pRules,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumTypes )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreTypes **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumSubtypes )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pType,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreTypes **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteItem )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubType,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *ReadItem )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ DWORD *cbData,
            /* [in] */ BYTE **ppbData,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *WriteItem )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ DWORD cbData,
            /* [size_is][in] */ BYTE *pbData,
            /* [in] */ PPST_PROMPTINFO pPromptInfo,
            /* [in] */ DWORD dwDefaultConfirmationStyle,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *OpenItem )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR szItemName,
            /* [in] */ PST_ACCESSMODE ModeFlags,
            /* [in] */ PPST_PROMPTINFO pProomptInfo,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *CloseItem )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ LPCWSTR *szItemName,
            /* [in] */ DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *EnumItems )( 
            IPStore * This,
            /* [in] */ PST_KEY Key,
            /* [in] */ const GUID *pItemType,
            /* [in] */ const GUID *pItemSubtype,
            /* [in] */ DWORD dwFlags,
            /* [in] */ IEnumPStoreItems **ppenum);
        
        END_INTERFACE
    } IPStoreVtbl;

    interface IPStore
    {
        CONST_VTBL struct IPStoreVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPStore_GetInfo(This,ppProperties)	\
    (This)->lpVtbl -> GetInfo(This,ppProperties)

#define IPStore_GetProvParam(This,dwParam,pcbData,ppbData,dwFlags)	\
    (This)->lpVtbl -> GetProvParam(This,dwParam,pcbData,ppbData,dwFlags)

#define IPStore_SetProvParam(This,dwParam,cbData,pbData,dwFlags)	\
    (This)->lpVtbl -> SetProvParam(This,dwParam,cbData,pbData,dwFlags)

#define IPStore_CreateType(This,Key,pType,pInfo,dwFlags)	\
    (This)->lpVtbl -> CreateType(This,Key,pType,pInfo,dwFlags)

#define IPStore_GetTypeInfo(This,Key,pType,ppInfo,dwFlags)	\
    (This)->lpVtbl -> GetTypeInfo(This,Key,pType,ppInfo,dwFlags)

#define IPStore_DeleteType(This,Key,pType,dwFlags)	\
    (This)->lpVtbl -> DeleteType(This,Key,pType,dwFlags)

#define IPStore_CreateSubtype(This,Key,pType,pSubtype,pInfo,pRules,dwFlags)	\
    (This)->lpVtbl -> CreateSubtype(This,Key,pType,pSubtype,pInfo,pRules,dwFlags)

#define IPStore_GetSubtypeInfo(This,Key,pType,pSubtype,ppInfo,dwFlags)	\
    (This)->lpVtbl -> GetSubtypeInfo(This,Key,pType,pSubtype,ppInfo,dwFlags)

#define IPStore_DeleteSubtype(This,Key,pType,pSubtype,dwFlags)	\
    (This)->lpVtbl -> DeleteSubtype(This,Key,pType,pSubtype,dwFlags)

#define IPStore_ReadAccessRuleset(This,Key,pType,pSubtype,pInfo,ppRules,dwFlags)	\
    (This)->lpVtbl -> ReadAccessRuleset(This,Key,pType,pSubtype,pInfo,ppRules,dwFlags)

#define IPStore_WriteAccessRuleset(This,Key,pType,pSubtype,pInfo,pRules,dwFlags)	\
    (This)->lpVtbl -> WriteAccessRuleset(This,Key,pType,pSubtype,pInfo,pRules,dwFlags)

#define IPStore_EnumTypes(This,Key,dwFlags,ppenum)	\
    (This)->lpVtbl -> EnumTypes(This,Key,dwFlags,ppenum)

#define IPStore_EnumSubtypes(This,Key,pType,dwFlags,ppenum)	\
    (This)->lpVtbl -> EnumSubtypes(This,Key,pType,dwFlags,ppenum)

#define IPStore_DeleteItem(This,Key,pItemType,pItemSubType,szItemName,pPromptInfo,dwFlags)	\
    (This)->lpVtbl -> DeleteItem(This,Key,pItemType,pItemSubType,szItemName,pPromptInfo,dwFlags)

#define IPStore_ReadItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,ppbData,pPromptInfo,dwFlags)	\
    (This)->lpVtbl -> ReadItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,ppbData,pPromptInfo,dwFlags)

#define IPStore_WriteItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,pbData,pPromptInfo,dwDefaultConfirmationStyle,dwFlags)	\
    (This)->lpVtbl -> WriteItem(This,Key,pItemType,pItemSubtype,szItemName,cbData,pbData,pPromptInfo,dwDefaultConfirmationStyle,dwFlags)

#define IPStore_OpenItem(This,Key,pItemType,pItemSubtype,szItemName,ModeFlags,pProomptInfo,dwFlags)	\
    (This)->lpVtbl -> OpenItem(This,Key,pItemType,pItemSubtype,szItemName,ModeFlags,pProomptInfo,dwFlags)

#define IPStore_CloseItem(This,Key,pItemType,pItemSubtype,szItemName,dwFlags)	\
    (This)->lpVtbl -> CloseItem(This,Key,pItemType,pItemSubtype,szItemName,dwFlags)

#define IPStore_EnumItems(This,Key,pItemType,pItemSubtype,dwFlags,ppenum)	\
    (This)->lpVtbl -> EnumItems(This,Key,pItemType,pItemSubtype,dwFlags,ppenum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IPStore_GetInfo_Proxy( 
    IPStore * This,
    /* [in] */ PPST_PROVIDERINFO *ppProperties);


void __RPC_STUB IPStore_GetInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetProvParam_Proxy( 
    IPStore * This,
    /* [in] */ DWORD dwParam,
    /* [out] */ DWORD *pcbData,
    /* [out] */ BYTE **ppbData,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_GetProvParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_SetProvParam_Proxy( 
    IPStore * This,
    /* [in] */ DWORD dwParam,
    /* [in] */ DWORD cbData,
    /* [in] */ BYTE *pbData,
    /* [in] */ DWORD *dwFlags);


void __RPC_STUB IPStore_SetProvParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CreateType_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_CreateType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetTypeInfo_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ PPST_TYPEINFO **ppInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteType_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_DeleteType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CreateSubtype_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ const GUID *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET pRules,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_CreateSubtype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_GetSubtypeInfo_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ const GUID *pSubtype,
    /* [in] */ PPST_TYPEINFO **ppInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_GetSubtypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteSubtype_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ const GUID *pSubtype,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_DeleteSubtype_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_ReadAccessRuleset_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ const GUID *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET **ppRules,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_ReadAccessRuleset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_WriteAccessRuleset_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ const GUID *pSubtype,
    /* [in] */ PPST_TYPEINFO pInfo,
    /* [in] */ PPST_ACCESSRULESET pRules,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_WriteAccessRuleset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumTypes_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreTypes **ppenum);


void __RPC_STUB IPStore_EnumTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumSubtypes_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pType,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreTypes **ppenum);


void __RPC_STUB IPStore_EnumSubtypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_DeleteItem_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubType,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_DeleteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_ReadItem_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ DWORD *cbData,
    /* [in] */ BYTE **ppbData,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_ReadItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_WriteItem_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ DWORD cbData,
    /* [size_is][in] */ BYTE *pbData,
    /* [in] */ PPST_PROMPTINFO pPromptInfo,
    /* [in] */ DWORD dwDefaultConfirmationStyle,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_WriteItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_OpenItem_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubtype,
    /* [in] */ LPCWSTR szItemName,
    /* [in] */ PST_ACCESSMODE ModeFlags,
    /* [in] */ PPST_PROMPTINFO pProomptInfo,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_OpenItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_CloseItem_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubtype,
    /* [in] */ LPCWSTR *szItemName,
    /* [in] */ DWORD dwFlags);


void __RPC_STUB IPStore_CloseItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPStore_EnumItems_Proxy( 
    IPStore * This,
    /* [in] */ PST_KEY Key,
    /* [in] */ const GUID *pItemType,
    /* [in] */ const GUID *pItemSubtype,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IEnumPStoreItems **ppenum);


void __RPC_STUB IPStore_EnumItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPStore_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


