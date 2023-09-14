#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z

class CArchiveExtractor: public IArchiveExtractCallback, public ICryptoGetTextPassword, public CMyUnknownImp
{
	Z7_COM_UNKNOWN_IMP_2(IArchiveExtractCallback, ICryptoGetTextPassword)
public:

	CArchiveExtractor(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files);
	~CArchiveExtractor();

	// IProgress
	Z7_COM7F_IMF(SetTotal(UInt64 size));
	Z7_COM7F_IMF(SetCompleted(const UInt64 *completeValue));

	// IArchiveExtractCallback
	Z7_COM7F_IMF(PrepareOperation(Int32 askExtractMode));
	Z7_COM7F_IMF(GetStream(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode));
	Z7_COM7F_IMF(SetOperationResult(Int32 operationResult));

	// ICryptoGetTextPassword
	Z7_COM7F_IMF(CryptoGetTextPassword(BSTR *password));

protected:
	CArchive*				m_pArchive;

	QMap<int,CArchiveIO*>	m_Files;
	int						m_Index;
};

#endif