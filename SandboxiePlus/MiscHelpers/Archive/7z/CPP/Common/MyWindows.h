// MyWindows.h

#ifndef __MYWINDOWS_H
#define __MYWINDOWS_H

#ifdef _WIN32

#include <windows.h>

#else

#include <stddef.h> // for wchar_t
#include <string.h>

#include "MyGuidDef.h"

typedef char CHAR;
typedef unsigned char UCHAR;

#undef BYTE
typedef unsigned char BYTE;

typedef short SHORT;
typedef unsigned short USHORT;

#undef WORD
typedef unsigned short WORD;
typedef short VARIANT_BOOL;

typedef int INT;
typedef Int32 INT32;
typedef unsigned int UINT;
typedef UInt32 UINT32;
typedef INT32 LONG;   // LONG, ULONG and DWORD must be 32-bit
typedef UINT32 ULONG;

#undef DWORD
typedef UINT32 DWORD;

typedef Int64 LONGLONG;
typedef UInt64 ULONGLONG;

typedef struct LARGE_INTEGER { LONGLONG QuadPart; }LARGE_INTEGER;
typedef struct _ULARGE_INTEGER { ULONGLONG QuadPart;} ULARGE_INTEGER;

typedef const CHAR *LPCSTR;
typedef CHAR TCHAR;
typedef const TCHAR *LPCTSTR;
typedef wchar_t WCHAR;
typedef WCHAR OLECHAR;
typedef const WCHAR *LPCWSTR;
typedef OLECHAR *BSTR;
typedef const OLECHAR *LPCOLESTR;
typedef OLECHAR *LPOLESTR;

typedef struct _FILETIME
{
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
}FILETIME;

#define HRESULT LONG
#define FAILED(Status) ((HRESULT)(Status)<0)
typedef ULONG PROPID;
typedef LONG SCODE;

#define S_OK    ((HRESULT)0x00000000L)
#define S_FALSE ((HRESULT)0x00000001L)
#define E_NOTIMPL ((HRESULT)0x80004001L)
#define E_NOINTERFACE ((HRESULT)0x80004002L)
#define E_ABORT ((HRESULT)0x80004004L)
#define E_FAIL ((HRESULT)0x80004005L)
#define STG_E_INVALIDFUNCTION ((HRESULT)0x80030001L)
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#define E_INVALIDARG ((HRESULT)0x80070057L)

#ifdef _MSC_VER
#define STDMETHODCALLTYPE __stdcall
#else
#define STDMETHODCALLTYPE
#endif

#define STDMETHOD_(t, f) virtual t STDMETHODCALLTYPE f
#define STDMETHOD(f) STDMETHOD_(HRESULT, f)
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#define STDMETHODIMP STDMETHODIMP_(HRESULT)

#define PURE = 0

#define MIDL_INTERFACE(x) struct

#ifdef __cplusplus

DEFINE_GUID(IID_IUnknown,
0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
struct IUnknown
{
  STDMETHOD(QueryInterface) (REFIID iid, void **outObject) PURE;
  STDMETHOD_(ULONG, AddRef)() PURE;
  STDMETHOD_(ULONG, Release)() PURE;
  #ifndef _WIN32
  virtual ~IUnknown() {}
  #endif
};

typedef IUnknown *LPUNKNOWN;

#endif

#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)

enum VARENUM
{
  VT_EMPTY = 0,
  VT_NULL = 1,
  VT_I2 = 2,
  VT_I4 = 3,
  VT_R4 = 4,
  VT_R8 = 5,
  VT_CY = 6,
  VT_DATE = 7,
  VT_BSTR = 8,
  VT_DISPATCH = 9,
  VT_ERROR = 10,
  VT_BOOL = 11,
  VT_VARIANT = 12,
  VT_UNKNOWN = 13,
  VT_DECIMAL = 14,
  VT_I1 = 16,
  VT_UI1 = 17,
  VT_UI2 = 18,
  VT_UI4 = 19,
  VT_I8 = 20,
  VT_UI8 = 21,
  VT_INT = 22,
  VT_UINT = 23,
  VT_VOID = 24,
  VT_HRESULT = 25,
  VT_FILETIME = 64
};

typedef unsigned short VARTYPE;
typedef WORD PROPVAR_PAD1;
typedef WORD PROPVAR_PAD2;
typedef WORD PROPVAR_PAD3;

#ifdef __cplusplus

typedef struct tagPROPVARIANT
{
  VARTYPE vt;
  PROPVAR_PAD1 wReserved1;
  PROPVAR_PAD2 wReserved2;
  PROPVAR_PAD3 wReserved3;
  union
  {
    CHAR cVal;
    UCHAR bVal;
    SHORT iVal;
    USHORT uiVal;
    LONG lVal;
    ULONG ulVal;
    INT intVal;
    UINT uintVal;
    LARGE_INTEGER hVal;
    ULARGE_INTEGER uhVal;
    VARIANT_BOOL boolVal;
    SCODE scode;
    FILETIME filetime;
    BSTR bstrVal;
  };
} PROPVARIANT;

typedef PROPVARIANT tagVARIANT;
typedef tagVARIANT VARIANT;
typedef VARIANT VARIANTARG;

MY_EXTERN_C HRESULT VariantClear(VARIANTARG *prop);
MY_EXTERN_C HRESULT VariantCopy(VARIANTARG *dest, VARIANTARG *src);

#endif

MY_EXTERN_C BSTR SysAllocStringByteLen(LPCSTR psz, UINT len);
MY_EXTERN_C BSTR SysAllocString(const OLECHAR *sz);
MY_EXTERN_C void SysFreeString(BSTR bstr);
MY_EXTERN_C UINT SysStringByteLen(BSTR bstr);
MY_EXTERN_C UINT SysStringLen(BSTR bstr);

MY_EXTERN_C DWORD GetLastError();
MY_EXTERN_C LONG CompareFileTime(const FILETIME* ft1, const FILETIME* ft2);

#define CP_ACP    0
#define CP_OEMCP  1

typedef enum tagSTREAM_SEEK
{
  STREAM_SEEK_SET = 0,
  STREAM_SEEK_CUR = 1,
  STREAM_SEEK_END = 2
} STREAM_SEEK;

#endif
#endif
