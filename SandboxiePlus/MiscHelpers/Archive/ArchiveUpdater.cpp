#include "stdafx.h"
#include "ArchiveUpdater.h"
#include "Archive.h"

#ifdef USE_7Z

CArchiveUpdater::CArchiveUpdater(CArchive* pArchive, const QMap<int,CArchiveIO*>& Files)
{
	m_pArchive = pArchive;
	m_Files = Files;
}

CArchiveUpdater::~CArchiveUpdater()
{
	foreach(CArchiveIO* pFile, m_Files)
		delete pFile;
}

STDMETHODIMP CArchiveUpdater::SetTotal(UInt64 size)
{
	m_pArchive->m_Progress.SetTotal(size);
	return S_OK;
}

STDMETHODIMP CArchiveUpdater::SetCompleted(const UInt64 *completeValue)
{
	ASSERT(completeValue);
	m_pArchive->m_Progress.SetCompleted(*completeValue);
	return S_OK;
}

STDMETHODIMP CArchiveUpdater::GetUpdateItemInfo(UInt32 index, Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
	if (newData != NULL)
		*newData = true;
	if (newProperties != NULL)
		*newProperties = true;
	if (indexInArchive != NULL)
		*indexInArchive = (UInt32)-1;
	return S_OK;
}

STDMETHODIMP CArchiveUpdater::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
	NWindows::NCOM::CPropVariant prop;
	if (propID == kpidIsAnti)
		prop = false;
	else 
	{
		Q_ASSERT(index < m_pArchive->m_Files.count());
		int ArcIndex = m_pArchive->m_Files[index].ArcIndex;
		switch(propID)
		{
			case kpidPath:		prop = m_pArchive->FileProperty(ArcIndex, "Path").toString().toStdWString().c_str();	break;
			case kpidSize:		prop = m_pArchive->FileProperty(ArcIndex, "Size").toULongLong();						break;
			case kpidAttrib:	prop = m_pArchive->FileProperty(ArcIndex, "Attrib").toUInt();							break;
			case kpidIsDir:		prop = m_pArchive->FileProperty(ArcIndex, "IsDir").toBool();							break;
			case kpidCTime:	{quint64 date = m_pArchive->FileProperty(ArcIndex, "CTime").toULongLong(); // ToDo
								prop = *reinterpret_cast<FILETIME*>(&date);}											break;
			case kpidATime:	{quint64 date = m_pArchive->FileProperty(ArcIndex, "ATime").toULongLong(); // ToDo
								prop = *reinterpret_cast<FILETIME*>(&date);}											break;
			case kpidMTime:	{quint64 date = m_pArchive->FileProperty(ArcIndex, "MTime").toULongLong(); // ToDo
								prop = *reinterpret_cast<FILETIME*>(&date);}											break;
			//default: Q_ASSERT(0); // ToDo
		}
	}
	prop.Detach(value);
	return S_OK;
}

STDMETHODIMP CArchiveUpdater::GetStream(UInt32 index, ISequentialInStream **inStream)
{
	*inStream = NULL;
	Q_ASSERT(index < m_pArchive->m_Files.count());
	int ArcIndex = m_pArchive->m_Files[index].ArcIndex;
	//TRACE(L"Archive Item '%S'", QS2CS(m_pArchive->FileProperty(ArcIndex, "Path").toString()));
	CArchiveIO* pFile = m_Files.take(ArcIndex);
	if(pFile == NULL)
	{
		QTemporaryFile* pTemp = new QTemporaryFile;
		QMap<int, QIODevice*> Files;
		Files.insert(ArcIndex, pTemp);
		if(!m_pArchive->Extract(&Files,false))
		{
			m_pArchive->LogError(QString("Failed to extract old %1 file form source archive").arg(m_pArchive->FileProperty(ArcIndex, "Path").toString()));
			TRACE(L"Failed to extract old file form source archive");
			delete pTemp;
			return E_ABORT;
		}
		pFile = new CArchiveIO(pTemp, QIODevice::ReadOnly);
	}
	if (!pFile->Open(QIODevice::ReadOnly))
		return S_FALSE;
	CMyComPtr<ISequentialInStream> inStreamLoc = pFile;
	*inStream = inStreamLoc.Detach();
	return S_OK;
}

STDMETHODIMP CArchiveUpdater::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
	QString Password = m_pArchive->GetPassword(true);
	*passwordIsDefined = Password.isEmpty() ? false : true;
	return StringToBstr(Password.toStdWString().c_str(), password);
}

#endif