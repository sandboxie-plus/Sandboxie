#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveOpener: public IArchiveOpenCallback, public IArchiveOpenVolumeCallback, public ICryptoGetTextPassword, public CMyUnknownImp
{
public:
	CArchiveOpener(CArchive* pArchive);

	MY_UNKNOWN_IMP3(IArchiveOpenCallback, IArchiveOpenVolumeCallback, ICryptoGetTextPassword)

	// IArchiveOpenCallback
	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes)					{return S_OK;}
	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes)				{return S_OK;}

	// IArchiveOpenVolumeCallback
	STDMETHOD(GetProperty)(PROPID propID, PROPVARIANT *value);
	STDMETHOD(GetStream)(const wchar_t *name, IInStream **inStream);

	// ICryptoGetTextPassword2
	STDMETHOD(CryptoGetTextPassword)(BSTR *password);

protected:
	CArchive*				m_pArchive;
};

#endif