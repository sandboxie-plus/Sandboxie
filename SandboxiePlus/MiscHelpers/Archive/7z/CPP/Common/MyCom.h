// MyCom.h

#ifndef __MYCOM_H
#define __MYCOM_H

#include "MyWindows.h"

#ifndef RINOK
#define RINOK(x) { HRESULT __result_ = (x); if (__result_ != S_OK) return __result_; }
#endif

template <class T>
class CMyComPtr
{
  T* _p;
public:
  // typedef T _PtrClass;
  CMyComPtr() { _p = NULL;}
  CMyComPtr(T* p) {if ((_p = p) != NULL) p->AddRef(); }
  CMyComPtr(const CMyComPtr<T>& lp)
  {
    if ((_p = lp._p) != NULL)
      _p->AddRef();
  }
  ~CMyComPtr() { if (_p) _p->Release(); }
  void Release() { if (_p) { _p->Release(); _p = NULL; } }
  operator T*() const {  return (T*)_p;  }
  // T& operator*() const {  return *_p; }
  T** operator&() { return &_p; }
  T* operator->() const { return _p; }
  T* operator=(T* p)
  {
    if (p != 0)
      p->AddRef();
    if (_p)
      _p->Release();
    _p = p;
    return p;
  }
  T* operator=(const CMyComPtr<T>& lp) { return (*this = lp._p); }
  bool operator!() const { return (_p == NULL); }
  // bool operator==(T* pT) const {  return _p == pT; }
  // Compare two objects for equivalence
  void Attach(T* p2)
  {
    Release();
    _p = p2;
  }
  T* Detach()
  {
    T* pt = _p;
    _p = NULL;
    return pt;
  }
  #ifdef _WIN32
  HRESULT CoCreateInstance(REFCLSID rclsid, REFIID iid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
  {
    return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, iid, (void**)&_p);
  }
  #endif
  /*
  HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
  {
    CLSID clsid;
    HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
    ATLASSERT(_p == NULL);
    if (SUCCEEDED(hr))
      hr = ::CoCreateInstance(clsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&_p);
    return hr;
  }
  */
  template <class Q>
  HRESULT QueryInterface(REFGUID iid, Q** pp) const
  {
    return _p->QueryInterface(iid, (void**)pp);
  }
};

//////////////////////////////////////////////////////////

inline HRESULT StringToBstr(LPCOLESTR src, BSTR *bstr)
{
  *bstr = ::SysAllocString(src);
  return (*bstr != 0) ? S_OK : E_OUTOFMEMORY;
}

class CMyComBSTR
{
public:
  BSTR m_str;
  CMyComBSTR(): m_str(NULL) {}
  CMyComBSTR(LPCOLESTR src) { m_str = ::SysAllocString(src); }
  // CMyComBSTR(int nSize) { m_str = ::SysAllocStringLen(NULL, nSize); }
  // CMyComBSTR(int nSize, LPCOLESTR sz) { m_str = ::SysAllocStringLen(sz, nSize);  }
  CMyComBSTR(const CMyComBSTR& src) { m_str = src.MyCopy(); }
  /*
  CMyComBSTR(REFGUID src)
  {
    LPOLESTR szGuid;
    StringFromCLSID(src, &szGuid);
    m_str = ::SysAllocString(szGuid);
    CoTaskMemFree(szGuid);
  }
  */
  ~CMyComBSTR() { ::SysFreeString(m_str); }
  CMyComBSTR& operator=(const CMyComBSTR& src)
  {
    if (m_str != src.m_str)
    {
      if (m_str)
        ::SysFreeString(m_str);
      m_str = src.MyCopy();
    }
    return *this;
  }
  CMyComBSTR& operator=(LPCOLESTR src)
  {
    ::SysFreeString(m_str);
    m_str = ::SysAllocString(src);
    return *this;
  }
  unsigned int Length() const { return ::SysStringLen(m_str); }
  operator BSTR() const { return m_str; }
  BSTR* operator&() { return &m_str; }
  BSTR MyCopy() const
  {
    int byteLen = ::SysStringByteLen(m_str);
    BSTR res = ::SysAllocStringByteLen(NULL, byteLen);
    memcpy(res, m_str, byteLen);
    return res;
  }
  /*
  void Attach(BSTR src) { m_str = src; }
  BSTR Detach()
  {
    BSTR s = m_str;
    m_str = NULL;
    return s;
  }
  */
  void Empty()
  {
    ::SysFreeString(m_str);
    m_str = NULL;
  }
  bool operator!() const {  return (m_str == NULL); }
};

//////////////////////////////////////////////////////////

class CMyUnknownImp
{
public:
  ULONG __m_RefCount;
  CMyUnknownImp(): __m_RefCount(0) {}
};

#define MY_QUERYINTERFACE_BEGIN STDMETHOD(QueryInterface) \
    (REFGUID iid, void **outObject) {

#define MY_QUERYINTERFACE_ENTRY(i) if (iid == IID_ ## i) \
    { *outObject = (void *)(i *)this; AddRef(); return S_OK; }

#define MY_QUERYINTERFACE_ENTRY_UNKNOWN(i) if (iid == IID_IUnknown) \
    { *outObject = (void *)(IUnknown *)(i *)this; AddRef(); return S_OK; }

#define MY_QUERYINTERFACE_BEGIN2(i) MY_QUERYINTERFACE_BEGIN \
    MY_QUERYINTERFACE_ENTRY_UNKNOWN(i) \
    MY_QUERYINTERFACE_ENTRY(i)

#define MY_QUERYINTERFACE_END return E_NOINTERFACE; }

#define MY_ADDREF_RELEASE \
STDMETHOD_(ULONG, AddRef)() { return ++__m_RefCount; } \
STDMETHOD_(ULONG, Release)() { if (--__m_RefCount != 0)  \
  return __m_RefCount; delete this; return 0; }

#define MY_UNKNOWN_IMP_SPEC(i) \
  MY_QUERYINTERFACE_BEGIN \
  i \
  MY_QUERYINTERFACE_END \
  MY_ADDREF_RELEASE


#define MY_UNKNOWN_IMP MY_QUERYINTERFACE_BEGIN \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(IUnknown) \
  MY_QUERYINTERFACE_END \
  MY_ADDREF_RELEASE

#define MY_UNKNOWN_IMP1(i) MY_UNKNOWN_IMP_SPEC( \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(i) \
  MY_QUERYINTERFACE_ENTRY(i) \
  )

#define MY_UNKNOWN_IMP2(i1, i2) MY_UNKNOWN_IMP_SPEC( \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(i1) \
  MY_QUERYINTERFACE_ENTRY(i1) \
  MY_QUERYINTERFACE_ENTRY(i2) \
  )

#define MY_UNKNOWN_IMP3(i1, i2, i3) MY_UNKNOWN_IMP_SPEC( \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(i1) \
  MY_QUERYINTERFACE_ENTRY(i1) \
  MY_QUERYINTERFACE_ENTRY(i2) \
  MY_QUERYINTERFACE_ENTRY(i3) \
  )

#define MY_UNKNOWN_IMP4(i1, i2, i3, i4) MY_UNKNOWN_IMP_SPEC( \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(i1) \
  MY_QUERYINTERFACE_ENTRY(i1) \
  MY_QUERYINTERFACE_ENTRY(i2) \
  MY_QUERYINTERFACE_ENTRY(i3) \
  MY_QUERYINTERFACE_ENTRY(i4) \
  )

#define MY_UNKNOWN_IMP5(i1, i2, i3, i4, i5) MY_UNKNOWN_IMP_SPEC( \
  MY_QUERYINTERFACE_ENTRY_UNKNOWN(i1) \
  MY_QUERYINTERFACE_ENTRY(i1) \
  MY_QUERYINTERFACE_ENTRY(i2) \
  MY_QUERYINTERFACE_ENTRY(i3) \
  MY_QUERYINTERFACE_ENTRY(i4) \
  MY_QUERYINTERFACE_ENTRY(i5) \
  )

#endif
