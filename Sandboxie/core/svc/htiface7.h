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

typedef interface IUri IUri;
typedef interface ITargetFramePriv2 ITargetFramePriv2;


EXTERN_C const IID IID_IUri;
EXTERN_C const IID IID_ITargetFramePriv2;


typedef enum {
    Uri_PROPERTY_ABSOLUTE_URI   = 0,
    Uri_PROPERTY_STRING_START   = Uri_PROPERTY_ABSOLUTE_URI,
    Uri_PROPERTY_AUTHORITY  = 1,
    Uri_PROPERTY_DISPLAY_URI    = 2,
    Uri_PROPERTY_DOMAIN = 3,
    Uri_PROPERTY_EXTENSION  = 4,
    Uri_PROPERTY_FRAGMENT   = 5,
    Uri_PROPERTY_HOST   = 6,
    Uri_PROPERTY_PASSWORD   = 7,
    Uri_PROPERTY_PATH   = 8,
    Uri_PROPERTY_PATH_AND_QUERY = 9,
    Uri_PROPERTY_QUERY  = 10,
    Uri_PROPERTY_RAW_URI    = 11,
    Uri_PROPERTY_SCHEME_NAME    = 12,
    Uri_PROPERTY_USER_INFO  = 13,
    Uri_PROPERTY_USER_NAME  = 14,
    Uri_PROPERTY_STRING_LAST    = Uri_PROPERTY_USER_NAME,
    Uri_PROPERTY_HOST_TYPE  = 15,
    Uri_PROPERTY_DWORD_START    = Uri_PROPERTY_HOST_TYPE,
    Uri_PROPERTY_PORT   = 16,
    Uri_PROPERTY_SCHEME = 17,
    Uri_PROPERTY_ZONE   = 18,
    Uri_PROPERTY_DWORD_LAST = Uri_PROPERTY_ZONE
}   Uri_PROPERTY;


//
//    GUID A39EE748-6A27-4817-A6F2-13914BEF5890
//

    typedef struct IUriVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            IUri * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */
            void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            IUri * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            IUri * This);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyBSTR )(
            IUri * This,
            /* [range][in] */ Uri_PROPERTY uriProp,
            /* [out] */ BSTR *pbstrProperty,
            /* [in] */ DWORD dwFlags);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyLength )(
            IUri * This,
            /* [range][in] */ Uri_PROPERTY uriProp,
            /* [out] */ DWORD *pcchProperty,
            /* [in] */ DWORD dwFlags);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyDWORD )(
            IUri * This,
            /* [range][in] */ Uri_PROPERTY uriProp,
            /* [out] */ DWORD *pdwProperty,
            /* [in] */ DWORD dwFlags);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *HasProperty )(
            IUri * This,
            /* [range][in] */ Uri_PROPERTY uriProp,
            /* [out] */ BOOL *pfHasProperty);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAbsoluteUri )(
            IUri * This,
            /* [out] */ BSTR *pbstrAbsoluteUri);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetAuthority )(
            IUri * This,
            /* [out] */ BSTR *pbstrAuthority);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDisplayUri )(
            IUri * This,
            /* [out] */ BSTR *pbstrDisplayString);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetDomain )(
            IUri * This,
            /* [out] */ BSTR *pbstrDomain);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetExtension )(
            IUri * This,
            /* [out] */ BSTR *pbstrExtension);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetFragment )(
            IUri * This,
            /* [out] */ BSTR *pbstrFragment);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetHost )(
            IUri * This,
            /* [out] */ BSTR *pbstrHost);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPassword )(
            IUri * This,
            /* [out] */ BSTR *pbstrPassword);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPath )(
            IUri * This,
            /* [out] */ BSTR *pbstrPath);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPathAndQuery )(
            IUri * This,
            /* [out] */ BSTR *pbstrPathAndQuery);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetQuery )(
            IUri * This,
            /* [out] */ BSTR *pbstrQuery);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetRawUri )(
            IUri * This,
            /* [out] */ BSTR *pbstrRawUri);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetSchemeName )(
            IUri * This,
            /* [out] */ BSTR *pbstrSchemeName);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetUserInfo )(
            IUri * This,
            /* [out] */ BSTR *pbstrUserInfo);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetUserName )(
            IUri * This,
            /* [out] */ BSTR *pbstrUserName);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetHostType )(
            IUri * This,
            /* [out] */ DWORD *pdwHostType);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetPort )(
            IUri * This,
            /* [out] */ DWORD *pdwPort);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetScheme )(
            IUri * This,
            /* [out] */ DWORD *pdwScheme);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetZone )(
            IUri * This,
            /* [out] */ DWORD *pdwZone);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *GetProperties )(
            IUri * This,
            /* [out] */ LPDWORD pdwFlags);

        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *IsEqual )(
            IUri * This,
            /* [in] */ IUri *pUri,
            /* [out] */ BOOL *pfEqual);

        END_INTERFACE
    } IUriVtbl;

    interface IUri
    {
        CONST_VTBL struct IUriVtbl *lpVtbl;
    };


#define IUri_QueryInterface(This,riid,ppvObject)    \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) )

#define IUri_AddRef(This)   \
    ( (This)->lpVtbl -> AddRef(This) )

#define IUri_Release(This)  \
    ( (This)->lpVtbl -> Release(This) )


#define IUri_GetPropertyBSTR(This,uriProp,pbstrProperty,dwFlags)    \
    ( (This)->lpVtbl -> GetPropertyBSTR(This,uriProp,pbstrProperty,dwFlags) )

#define IUri_GetPropertyLength(This,uriProp,pcchProperty,dwFlags)   \
    ( (This)->lpVtbl -> GetPropertyLength(This,uriProp,pcchProperty,dwFlags) )

#define IUri_GetPropertyDWORD(This,uriProp,pdwProperty,dwFlags) \
    ( (This)->lpVtbl -> GetPropertyDWORD(This,uriProp,pdwProperty,dwFlags) )

#define IUri_HasProperty(This,uriProp,pfHasProperty)    \
    ( (This)->lpVtbl -> HasProperty(This,uriProp,pfHasProperty) )

#define IUri_GetAbsoluteUri(This,pbstrAbsoluteUri)  \
    ( (This)->lpVtbl -> GetAbsoluteUri(This,pbstrAbsoluteUri) )

#define IUri_GetAuthority(This,pbstrAuthority)  \
    ( (This)->lpVtbl -> GetAuthority(This,pbstrAuthority) )

#define IUri_GetDisplayUri(This,pbstrDisplayString) \
    ( (This)->lpVtbl -> GetDisplayUri(This,pbstrDisplayString) )

#define IUri_GetDomain(This,pbstrDomain)    \
    ( (This)->lpVtbl -> GetDomain(This,pbstrDomain) )

#define IUri_GetExtension(This,pbstrExtension)  \
    ( (This)->lpVtbl -> GetExtension(This,pbstrExtension) )

#define IUri_GetFragment(This,pbstrFragment)    \
    ( (This)->lpVtbl -> GetFragment(This,pbstrFragment) )

#define IUri_GetHost(This,pbstrHost)    \
    ( (This)->lpVtbl -> GetHost(This,pbstrHost) )

#define IUri_GetPassword(This,pbstrPassword)    \
    ( (This)->lpVtbl -> GetPassword(This,pbstrPassword) )

#define IUri_GetPath(This,pbstrPath)    \
    ( (This)->lpVtbl -> GetPath(This,pbstrPath) )

#define IUri_GetPathAndQuery(This,pbstrPathAndQuery)    \
    ( (This)->lpVtbl -> GetPathAndQuery(This,pbstrPathAndQuery) )

#define IUri_GetQuery(This,pbstrQuery)  \
    ( (This)->lpVtbl -> GetQuery(This,pbstrQuery) )

#define IUri_GetRawUri(This,pbstrRawUri)    \
    ( (This)->lpVtbl -> GetRawUri(This,pbstrRawUri) )

#define IUri_GetSchemeName(This,pbstrSchemeName)    \
    ( (This)->lpVtbl -> GetSchemeName(This,pbstrSchemeName) )

#define IUri_GetUserInfo(This,pbstrUserInfo)    \
    ( (This)->lpVtbl -> GetUserInfo(This,pbstrUserInfo) )

#define IUri_GetUserName(This,pbstrUserName)    \
    ( (This)->lpVtbl -> GetUserName(This,pbstrUserName) )

#define IUri_GetHostType(This,pdwHostType)  \
    ( (This)->lpVtbl -> GetHostType(This,pdwHostType) )

#define IUri_GetPort(This,pdwPort)  \
    ( (This)->lpVtbl -> GetPort(This,pdwPort) )

#define IUri_GetScheme(This,pdwScheme)  \
    ( (This)->lpVtbl -> GetScheme(This,pdwScheme) )

#define IUri_GetZone(This,pdwZone)  \
    ( (This)->lpVtbl -> GetZone(This,pdwZone) )

#define IUri_GetProperties(This,pdwFlags)   \
    ( (This)->lpVtbl -> GetProperties(This,pdwFlags) )

#define IUri_IsEqual(This,pUri,pfEqual) \
    ( (This)->lpVtbl -> IsEqual(This,pUri,pfEqual) )


//
//    GUID B2C867E6-69D6-46F2-A611-DED9A4BD7FEF
//




#define ITargetFramePriv2_QueryInterface(This,riid,ppvObject)   \
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) )

#define ITargetFramePriv2_AddRef(This)  \
    ( (This)->lpVtbl -> AddRef(This) )

#define ITargetFramePriv2_Release(This) \
    ( (This)->lpVtbl -> Release(This) )


#define ITargetFramePriv2_FindFrameDownwards(This,pszTargetName,dwFlags,ppunkTargetFrame)   \
    ( (This)->lpVtbl -> FindFrameDownwards(This,pszTargetName,dwFlags,ppunkTargetFrame) )

#define ITargetFramePriv2_FindFrameInContext(This,pszTargetName,punkContextFrame,dwFlags,ppunkTargetFrame)  \
    ( (This)->lpVtbl -> FindFrameInContext(This,pszTargetName,punkContextFrame,dwFlags,ppunkTargetFrame) )

#define ITargetFramePriv2_OnChildFrameActivate(This,pUnkChildFrame) \
    ( (This)->lpVtbl -> OnChildFrameActivate(This,pUnkChildFrame) )

#define ITargetFramePriv2_OnChildFrameDeactivate(This,pUnkChildFrame)   \
    ( (This)->lpVtbl -> OnChildFrameDeactivate(This,pUnkChildFrame) )

#define ITargetFramePriv2_NavigateHack(This,grfHLNF,pbc,pibsc,pszTargetName,pszUrl,pszLocation) \
    ( (This)->lpVtbl -> NavigateHack(This,grfHLNF,pbc,pibsc,pszTargetName,pszUrl,pszLocation) )

#define ITargetFramePriv2_FindBrowserByIndex(This,dwID,ppunkBrowser)    \
    ( (This)->lpVtbl -> FindBrowserByIndex(This,dwID,ppunkBrowser) )


#define ITargetFramePriv2_AggregatedNavigation2(This,grfHLNF,pbc,pibsc,pszTargetName,pUri,pszLocation)  \
    ( (This)->lpVtbl -> AggregatedNavigation2(This,grfHLNF,pbc,pibsc,pszTargetName,pUri,pszLocation) )

