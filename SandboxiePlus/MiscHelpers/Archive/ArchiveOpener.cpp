#include "stdafx.h"
#include "ArchiveOpener.h"
#include "Archive.h"

#ifdef USE_7Z

CArchiveOpener::CArchiveOpener(CArchive* pArchive)
{
	m_pArchive = pArchive;
}

CArchiveOpener::~CArchiveOpener()
{
	foreach(CArchiveIO* pFile, m_Files)
		delete pFile;
}

STDMETHODIMP CArchiveOpener::GetProperty(PROPID propID, PROPVARIANT *value)
{
	NWindows::NCOM::CPropVariant prop;
	switch(propID)
	{
		case kpidName:
		{
			QString Name = m_pArchive->m_ArchivePath;
			int Pos = m_pArchive->m_ArchivePath.lastIndexOf("/");
			if(Pos != -1)
				Name.remove(0,Pos+1);

			prop = Name.toStdWString().c_str(); break;
		}
		default: Q_ASSERT(0);
	}
	prop.Detach(value);
	return S_OK;
}

STDMETHODIMP CArchiveOpener::GetStream(const wchar_t *name, IInStream **inStream)
{
	m_pArchive->SetPartSize(0); // tell that this is multipart
	QString Path = m_pArchive->GetNextPart(QString::fromStdWString(name));
	if(QFile::exists(Path))
	{
		CArchiveIO* pFile = new CArchiveIO(new QFile(Path), QIODevice::ReadOnly);
		m_Files.append(pFile);
		CMyComPtr<IInStream> inStreamLoc(pFile);
		*inStream = inStreamLoc.Detach();
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CArchiveOpener::CryptoGetTextPassword(BSTR *password)
{
	QString Password = m_pArchive->GetPassword(true);
	if(Password.isEmpty())
		return E_ABORT;
	return StringToBstr(Password.toStdWString().c_str(), password);
}

#endif