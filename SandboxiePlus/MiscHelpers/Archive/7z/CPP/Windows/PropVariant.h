// Windows/PropVariant.h

#ifndef __WINDOWS_PROPVARIANT_H
#define __WINDOWS_PROPVARIANT_H

#include "../Common/MyWindows.h"
#include "../Common/Types.h"

namespace NWindows {
namespace NCOM {

class CPropVariant : public tagPROPVARIANT
{
public:
  CPropVariant() { vt = VT_EMPTY; wReserved1 = 0; }
  ~CPropVariant() { Clear(); }
  CPropVariant(const PROPVARIANT &varSrc);
  CPropVariant(const CPropVariant &varSrc);
  CPropVariant(BSTR bstrSrc);
  CPropVariant(LPCOLESTR lpszSrc);
  CPropVariant(bool bSrc) { vt = VT_BOOL; wReserved1 = 0; boolVal = (bSrc ? VARIANT_TRUE : VARIANT_FALSE); };
  CPropVariant(Byte value) { vt = VT_UI1; wReserved1 = 0; bVal = value; }
  CPropVariant(Int16 value) { vt = VT_I2; wReserved1 = 0; iVal = value; }
  CPropVariant(Int32 value) { vt = VT_I4; wReserved1 = 0; lVal = value; }
  CPropVariant(UInt32 value) { vt = VT_UI4; wReserved1 = 0; ulVal = value; }
  CPropVariant(UInt64 value) { vt = VT_UI8; wReserved1 = 0; uhVal.QuadPart = value; }
  CPropVariant(const FILETIME &value) { vt = VT_FILETIME; wReserved1 = 0; filetime = value; }

  CPropVariant& operator=(const CPropVariant &varSrc);
  CPropVariant& operator=(const PROPVARIANT &varSrc);
  CPropVariant& operator=(BSTR bstrSrc);
  CPropVariant& operator=(LPCOLESTR lpszSrc);
  CPropVariant& operator=(const char *s);
  CPropVariant& operator=(bool bSrc);
  CPropVariant& operator=(Byte value);
  CPropVariant& operator=(Int16 value);
  CPropVariant& operator=(Int32 value);
  CPropVariant& operator=(UInt32 value);
  CPropVariant& operator=(Int64 value);
  CPropVariant& operator=(UInt64 value);
  CPropVariant& operator=(const FILETIME &value);

  HRESULT Clear();
  HRESULT Copy(const PROPVARIANT *pSrc);
  HRESULT Attach(PROPVARIANT *pSrc);
  HRESULT Detach(PROPVARIANT *pDest);

  HRESULT InternalClear();
  void InternalCopy(const PROPVARIANT *pSrc);

  int Compare(const CPropVariant &a1);
};

}}

#endif
