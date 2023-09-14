#include "stdafx.h"
#include "ArchiveExtractor.h"
#include "Archive.h"

#ifdef USE_7Z

CArchiveExtractor::CArchiveExtractor(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files)
{
	m_pArchive = pArchive;
	m_Index = -1;
	m_Files = Files;
}

CArchiveExtractor::~CArchiveExtractor()
{
	foreach(CArchiveIO* pFile, m_Files)
		delete pFile;
}

STDMETHODIMP CArchiveExtractor::SetTotal(UInt64 size)
{
	m_pArchive->m_Progress.SetTotal(size);
	return S_OK;
}

STDMETHODIMP CArchiveExtractor::SetCompleted(const UInt64 *completeValue)
{
	ASSERT(completeValue);
	m_pArchive->m_Progress.SetCompleted(*completeValue);
	return S_OK;
}

STDMETHODIMP CArchiveExtractor::PrepareOperation (Int32 askExtractMode)
{
	switch (askExtractMode)
	{
		case NArchive::NExtract::NAskMode::kExtract:	
			//TRACE(L"Extracting ...");		
			break;
		case NArchive::NExtract::NAskMode::kTest:		
			//TRACE(L"Testing ...");			
			break;
		case NArchive::NExtract::NAskMode::kSkip: 		
			//TRACE(L"Skipping ...");			
			break;
		default: Q_ASSERT(0);
	}
	return S_OK;
}

STDMETHODIMP CArchiveExtractor::GetStream (UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode)
{
	*outStream = NULL;
	Q_ASSERT(index < m_pArchive->m_Files.count());
	m_Index = index;
	//TRACE(L"Archive Item '%S'", QS2CS(m_pArchive->FileProperty(m_Index, "Path").toString()));
	if(CArchiveIO* pFile = m_Files.take(m_Index))
	{
		Q_ASSERT(askExtractMode == NArchive::NExtract::NAskMode::kExtract);
		if (!pFile->Open(QIODevice::WriteOnly))
			return S_FALSE;
		CMyComPtr<IOutStream> outStreamLoc = pFile;
		*outStream = outStreamLoc.Detach();
	}
	return S_OK;
}

STDMETHODIMP CArchiveExtractor::SetOperationResult (Int32 operationResult)
{
	switch(operationResult)
	{
		case NArchive::NExtract::NOperationResult::kOK:					
			//TRACE(L"... Completed");					
			break;
		case NArchive::NExtract::NOperationResult::kUnsupportedMethod:	
			//TRACE(L"... Error (Un Supported Method)");	
			m_pArchive->LogError(QString("File Extraction Fails (Un Supported Method): %1").arg(m_pArchive->FileProperty(m_Index, "Path").toString()));
			m_pArchive->FileProperty(m_Index, "Error", "Un Supported Method");
			break;
		case NArchive::NExtract::NOperationResult::kCRCError:			
			//TRACE(L"... Error (CRC)");					
			m_pArchive->LogError(QString("File Extraction Fails (CRC Error): %1").arg(m_pArchive->FileProperty(m_Index, "Path").toString()));
			m_pArchive->FileProperty(m_Index, "Error", "CRC Error");
			break;
		case NArchive::NExtract::NOperationResult::kDataError:			
			//TRACE(L"... Error (Data)");					
			m_pArchive->LogError(QString("File Extraction Fails (Data Error): %1").arg(m_pArchive->FileProperty(m_Index, "Path").toString()));
			m_pArchive->FileProperty(m_Index, "Error", "Data Error");
			break;
		default: Q_ASSERT(0);
	}
	return S_OK;
}

STDMETHODIMP CArchiveExtractor::CryptoGetTextPassword(BSTR *password)
{
	QString Password = m_pArchive->GetPassword(true);
	if(Password.isEmpty())
		return E_ABORT;
	return StringToBstr(Password.toStdWString().c_str(), password);
}

#endif