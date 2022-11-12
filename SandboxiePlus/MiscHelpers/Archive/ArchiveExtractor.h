#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveExtractor: public IArchiveExtractCallback, public ICryptoGetTextPassword, public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP2(IArchiveExtractCallback, ICryptoGetTextPassword)

	CArchiveExtractor(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files);
	~CArchiveExtractor();

	// IProgress
	STDMETHOD(SetTotal)(UInt64 size);
	STDMETHOD(SetCompleted)(const UInt64 *completeValue);

	// IArchiveExtractCallback
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 operationResult);

	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *password);

protected:
	CArchive*				m_pArchive;

	QMap<int,CArchiveIO*>	m_Files;
	int						m_Index;
};

#endif