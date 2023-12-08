#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveUpdater: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, public CMyUnknownImp
{
	Z7_COM_UNKNOWN_IMP_2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)
public:

	CArchiveUpdater(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files);
	~CArchiveUpdater();

	// IProgress
	Z7_COM7F_IMF(SetTotal(UInt64 size));
	Z7_COM7F_IMF(SetCompleted(const UInt64 *completeValue));

	// IArchiveUpdateCallback2
	Z7_COM7F_IMF(GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive));
	Z7_COM7F_IMF(GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value));
	Z7_COM7F_IMF(GetStream(UInt32 index, ISequentialInStream **inStream));
	Z7_COM7F_IMF(SetOperationResult(Int32 operationResult))							{return S_OK;}

	// deprecated 
	Z7_COM7F_IMF(GetVolumeSize(UInt32 index, UInt64 *size))							{return S_FALSE;}
	Z7_COM7F_IMF(GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)){return S_FALSE;}

	// ICryptoGetTextPassword2
	Z7_COM7F_IMF(CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password));

protected:
	CArchive*				m_pArchive;

	QMap<int,CArchiveIO*>	m_Files;
};

#endif
