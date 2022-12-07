#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveUpdater: public IArchiveUpdateCallback2, public ICryptoGetTextPassword2, public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

	CArchiveUpdater(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files);
	~CArchiveUpdater();

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);

	// IArchiveUpdateCallback2
	STDMETHOD(GetUpdateItemInfo)(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
	STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
	STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
	STDMETHOD(SetOperationResult)(Int32 operationResult)							{return S_OK;}

	// deprecated 
	STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size)							{return S_FALSE;}
	STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream)	{return S_FALSE;}

	// ICryptoGetTextPassword2
	STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

protected:
	CArchive*				m_pArchive;

	QMap<int,CArchiveIO*>	m_Files;
};

#endif
