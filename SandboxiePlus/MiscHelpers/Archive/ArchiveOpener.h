#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveOpener: public IArchiveOpenCallback, public IArchiveOpenVolumeCallback, public ICryptoGetTextPassword, public CMyUnknownImp
{
	Z7_COM_UNKNOWN_IMP_3(IArchiveOpenCallback, IArchiveOpenVolumeCallback, ICryptoGetTextPassword)
public:

	CArchiveOpener(CArchive* pArchive);
	~CArchiveOpener();

	// IArchiveOpenCallback
	Z7_COM7F_IMF(SetTotal(const UInt64 *files, const UInt64 *bytes))					{return S_OK;}
	Z7_COM7F_IMF(SetCompleted(const UInt64 *files, const UInt64 *bytes))				{return S_OK;}

	// IArchiveOpenVolumeCallback
	Z7_COM7F_IMF(GetProperty(PROPID propID, PROPVARIANT *value));
	Z7_COM7F_IMF(GetStream(const wchar_t *name, IInStream **inStream));

	// ICryptoGetTextPassword2
	Z7_COM7F_IMF(CryptoGetTextPassword(BSTR *password));

protected:
	CArchive*				m_pArchive;

	QList<CArchiveIO*>		m_Files;
};

#endif