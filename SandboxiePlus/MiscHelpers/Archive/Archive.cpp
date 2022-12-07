#include "stdafx.h"
#include "ArchiveHelper.h"
#include "Archive.h"
#include "ArchiveInterface.h"
#include "ArchiveOpener.h"
#include "ArchiveExtractor.h"
#include "ArchiveUpdater.h"

#include "SplitFile.h"

#ifdef USE_7Z

CArchiveInterface theArc;

struct CArchive::SArchive
{
	SArchive(CMyComPtr<IInArchive> &InArchive) {In = InArchive;}
	~SArchive() {In->Close();}
	CMyComPtr<IInArchive>	In;
};

CArchive::CArchive(const QString &ArchivePath, QIODevice* pDevice)
{
	Init();

	m_pDevice = pDevice;
	m_ArchivePath = ArchivePath;
	m_Archive = NULL;
	m_PartSize = -1;
	m_PasswordUsed = false;
}

void CArchive::Init()
{
	if(!theArc.IsOperational())
		theArc.Init();
}

bool CArchive::IsInit() 
{
	return theArc.IsOperational();
}

CArchive::~CArchive()
{
	Close();
}

int CArchive::Open()
{
	if(!theArc.IsOperational())
	{
		LogError("Couldn't open interface");
		return 0; // failed
	}

	if(m_Archive)
	{
		LogError("archive is already open");
		return 0; // fails
	}

	if(!m_pDevice && !QFile::exists(m_ArchivePath))
	{
		LogError("archive does not exist");
		return 0; // failed
	}

	SArcInfo Info = GetArcInfo(m_ArchivePath);
	if(Info.FormatIndex == 0)
		return -1; // unsupported format

	for (int i = 0; i <= theArc.GetFormatCount(); i++)
	{	
		CMyComPtr<IInArchive> InArchive;
		if(!theArc.CreateInArchive(i ? i : Info.FormatIndex, InArchive)) // First try the most likely one and than try everyone else
			break;
		if (!InArchive)
			continue;

		// Open Archive
		const UInt64 kMaxCheckStartPosition = 1 << 22; // 4MB
		CMyComPtr<IArchiveOpenCallback> callback(new CArchiveOpener(this));
		HRESULT Ret = InArchive->Open(new CArchiveIO(m_pDevice ? m_pDevice : new QFile(m_ArchivePath), QIODevice::ReadOnly, m_pDevice == NULL), &kMaxCheckStartPosition, callback);
		if(Ret != S_OK)
		{
			InArchive->Close();
			if(Ret == S_FALSE)
				continue; // not supported
			if(Ret == E_ABORT)
				LogError(QString("Password required for archive"));
			break; // error
		}

		//TRACE(L"%S Archive %S Opened for extraction", QS2CS(theArc.GetArchiveName(i ? i : Info.FormatIndex)), QS2CS(m_ArchivePath));
		m_Archive = new SArchive(InArchive);
		break;
	}

	if(m_Archive == NULL)
	{
		LogError("Failed to open archive");
		return 0; // failed
	}

	// list archive content
	UInt32 numItems = 0;
	m_Archive->In->GetNumberOfItems(&numItems);
	for (UInt32 i = 0; i < numItems; i++)
	{
		NWindows::NCOM::CPropVariant prop;
		SFile File(i);
		UInt32 numProps = 0;
		m_Archive->In->GetNumberOfProperties(&numProps);
		for(UInt32 j=0; j < numProps; j++)
		{
			m_Archive->In->GetProperty(i, j, &prop);

			QVariant Property;
			switch (prop.vt)
			{
				case VT_BSTR:		Property = QString::fromStdWString(prop.bstrVal);		break;
				case VT_UI1:		Property = prop.bVal;									break;
				case VT_UI2:		Property = prop.uiVal;									break;
				case VT_UI4:		Property = (qint32)prop.ulVal;							break;
				case VT_UI8:		Property = (qint64)prop.uhVal.QuadPart;					break;
				case VT_BOOL:		Property =	VARIANT_BOOLToBool(prop.boolVal);			break;
				case VT_FILETIME:	Property = *reinterpret_cast<qint64*>(&prop.filetime);	break;	// ToDo
				default: 
					//TRACE(L"Unhandled archive property %S (%d)", QS2CS(GetPropertyName(j)), prop.vt);
				case VT_EMPTY:
					continue;
			}

			File.Properties.insert(GetPropertyName(j), Property);
			//TRACE(L" >> File %S: %S=%S", QS2CS(File.Properties["Path"].toString()), QS2CS(GetPropertyName(j)), QS2CS(Property.toString()));
		}
		m_Files.append(File);
	}

	return 1; // success
}

bool CArchive::Extract(QString Path)
{
	if(!m_Archive)
	{
		LogError("archive is not open");
		return false;
	}

	if(Path.isEmpty())
		Path = m_ArchivePath.left(m_ArchivePath.lastIndexOf(".")) + "/";
	else if(Path.right(1) != "/")
		Path.append("/");

	QMap<int, QIODevice*> Files;
	foreach(const SFile& File, m_Files)
	{
		if(File.Properties["IsDir"].toBool())
			continue;

		Files.insert(File.ArcIndex, new QFile(PrepareExtraction(File.Properties["Path"].toString(), Path)));
	}

	return Extract(&Files);
}

bool CArchive::Extract(QMap<int, QIODevice*> *FileList, bool bDelete)
{
	if(!m_Archive)
	{
		LogError("archive is not open");
		return false;
	}

	QMap<int, CArchiveIO*> Files;
	foreach(int ArcIndex, FileList->keys())
	{
		FileProperty(ArcIndex, "Error", QVariant());
		Files.insert(ArcIndex, new CArchiveIO(FileList->value(ArcIndex), QIODevice::NotOpen, bDelete));
	}
	
	CMyComPtr<IArchiveExtractCallback> callback(new CArchiveExtractor(this, Files));
	if(m_Archive->In->Extract(NULL, (UInt32)(Int32)(-1), false, callback) != S_OK)
	{
		LogError(QString("Error(s) While extracting from archive"));
		return false;
	}

	foreach(int ArcIndex, FileList->keys())
	{
		QVariant Error = FileProperty(ArcIndex, "Error");
		if(Error.isValid())
			return false;
	}
	return true;
}

bool CArchive::Close()
{
	m_Files.clear();
	if(m_Archive)
	{
		delete m_Archive;
		m_Archive = NULL;
		return true;
	}
	return false;
}

bool CArchive::Update(QMap<int, QIODevice*> *FileList, bool bDelete, int Level)
{
	if(!theArc.IsOperational())
	{
		LogError("Couldn't open interface");
		return false; 
	}

	SArcInfo Info = GetArcInfo(m_ArchivePath);

	CMyComPtr<IOutArchive> OutArchive;
	if(!theArc.CreateOutArchive(Info.FormatIndex, OutArchive) || !OutArchive)
	{
		LogError("Archive can not be updated");
		return false;
	}

	CMyComPtr<ISetProperties> setProperties;
	OutArchive->QueryInterface(IID_ISetProperties, (void **)&setProperties);
	if (!setProperties)
	{
		TRACE(L"ISetProperties unsupported");
		Q_ASSERT(0);
	}
	else
	{
		/* http://www.dotnetperls.com/7-zip-examples
			Switch -mx0: Don't compress at all. This is called "copy mode."
			Switch -mx1: Low compression. This is called "fastest" mode.
			Switch -mx3: Fast compression mode. Will set various parameters automatically.
			Switch -mx5: Same as above, but "normal."
			Switch -mx7: This means "maximum" compression.
			Switch -mx9: This means "ultra" compression. You probably want to use this.

			Switch -ms=on: Enable solid mode.	This is the default so you won't often need this.
			Switch -ms=off: Disable solid mode.	This is useful when you need to update individual files. Will reduce compression ratios normally.
		*/
		const wchar_t *names[] =
		{
			L"s",
			L"x",
			L"he"
		};
		const int kNumProps = sizeof(names) / sizeof(names[0]);
		NWindows::NCOM::CPropVariant values[kNumProps] =
		{
			false,			// solid mode OFF
			(UInt32)Level,	// compression level = 9 - ultra
			true			// file name encryption (7z only)
		};

		if(setProperties->SetProperties(names, values, kNumProps) != S_OK)
		{
			TRACE(L"ISetProperties failed");
			Q_ASSERT(0);
		}
	}

	QMap<int, CArchiveIO*> Files;
	foreach(int ArcIndex, FileList->keys())
	{
		Files.insert(ArcIndex, new CArchiveIO(FileList->value(ArcIndex), QIODevice::NotOpen, bDelete));
		FileProperty(ArcIndex, "Size", FileList->value(ArcIndex)->size());
		FileProperty(ArcIndex, "Attrib", 32);
	}

	//TRACE(L"%S Archive %S Opened for update", QS2CS(theArc.GetArchiveName(Info.FormatIndex)), QS2CS(m_ArchivePath));

	QIODevice* pFile = NULL;
	bool bUpdate = false;
	if(!m_pDevice)
	{
		if(m_PartSize != -1)
		{
			if(m_Archive)
			{
				LogError("can not update multipart archive");
				return false;
			}
			pFile = new CSplitFile(m_ArchivePath, m_PartSize);
		}
		else if(m_Archive)
		{
			bUpdate = true;
			pFile = new QFile(m_ArchivePath + ".tmp");
		}
		else
			pFile = new QFile(m_ArchivePath);
	}
	else if(m_pDevice->isOpen())
	{
		bUpdate = true;
		m_pDevice->close();
	}

    CMyComPtr<IArchiveUpdateCallback2> callback(new CArchiveUpdater(this, Files));
	if(OutArchive->UpdateItems(new CArchiveIO(m_pDevice ? m_pDevice : pFile, QIODevice::WriteOnly, m_pDevice == NULL), FileCount(), callback) != S_OK)
	{
		LogError("Error(s) while updating Archive");
		return false;
	}

	Close(); // close even if it wasn't open to clear the file list
	if(bUpdate)
	{
		if(!m_pDevice)
		{
			QFile::remove(m_ArchivePath);
			QFile::rename(m_ArchivePath + ".tmp", m_ArchivePath);
		}
		return Open();
	}
	return true;
}

int CArchive::AddFile(QString Path)
{
	if(FindByPath(Path) != -1)
		return -1;

	SFile File(m_Files.isEmpty() ? 0 : m_Files.last().ArcIndex+1);
	//File.NewData = true;
	m_Files.append(File);
	FileProperty(File.ArcIndex,"Path",Path);
	return File.ArcIndex;
}

int CArchive::FindByPath(QString Path)
{
	if(Path.left(1) == "/")
		Path.remove(0,1);
	foreach(const SFile& File, m_Files)
	{
		if(Path.compare(File.Properties["Path"].toString().replace("\\","/")) == 0)
			return File.ArcIndex;
	}
	return -1;
}

int CArchive::FindByIndex(int Index)
{
	if(Index > m_Files.count())
		return -1;
	return m_Files[Index].ArcIndex;
}

int CArchive::GetIndex(int ArcIndex)
{
	for(int Index = 0; Index < m_Files.count(); Index++)
	{
		const SFile& File = m_Files[Index];
		if(File.ArcIndex == ArcIndex)
			return Index;
	}
	return -1;
}

void CArchive::RemoveFile(int ArcIndex)
{
	int Index = GetIndex(ArcIndex);
	if(Index != -1)
		m_Files.remove(Index);
}

QString CArchive::PrepareExtraction(QString FileName, QString Path)
{
	// Cleanup
	FileName.replace("\\","/");
	FileName.remove(QRegularExpression("[:*?<>|\"]"));
	if(FileName.left(1) == "/")
		FileName.remove(0,1);

	// Create Sub Paths if needed
	QString SubPath = Path;
	int Pos = FileName.lastIndexOf("/");
	if(Pos != -1)
		SubPath += FileName.left(Pos);
	if(!QDir().exists(SubPath))
		QDir().mkpath(SubPath);

	return Path + FileName;
}

QString CArchive::GetNextPart(QString FileName)
{
	if(!m_AuxParts.isEmpty())
	{
		SArcInfo ArcInfo = GetArcInfo(FileName);
		foreach(const QString& Part, m_AuxParts)
		{
			if(GetArcInfo(Part).PartNumber == ArcInfo.PartNumber)
			{
				FileName = Part;
				break;
			}
		}
	}

	int Pos = m_ArchivePath.lastIndexOf("/");
	if(Pos != -1)
		FileName.prepend(m_ArchivePath.left(Pos+1));
	return FileName;
}

QVariant CArchive::FileProperty(int ArcIndex, QString Name)
{
	int Index = GetIndex(ArcIndex);
	if(Index != -1)
		return m_Files[Index].Properties.value(Name);
	return QVariant();
}

void CArchive::FileProperty(int ArcIndex, QString Name, QVariant Value)
{
	int Index = GetIndex(ArcIndex);
	if(Index != -1)
	{
		m_Files[Index].Properties.insert(Name, Value);
		//m_Files[Index].NewInfo = true;
	}
}

#endif

SArcInfo GetArcInfo(const QString &FileName)
{
	SArcInfo ArcInfo;
	ArcInfo.FileName = FileName.trimmed();

	int Pos = ArcInfo.FileName.lastIndexOf("/");
	if(Pos != -1)
		ArcInfo.FileName.remove(0,Pos+1);

	Pos = ArcInfo.FileName.lastIndexOf(".");
	if(Pos != -1)
	{
		ArcInfo.ArchiveExt = ArcInfo.FileName.mid(Pos+1);
		ArcInfo.FileName.remove(Pos, ArcInfo.FileName.length()-Pos);
	}

	// RAR special case
	if(ArcInfo.ArchiveExt.indexOf(QRegularExpression("(rar|rev|r[0-9]{2,})", QRegularExpression::CaseInsensitiveOption)) == 0)
	{
		if(ArcInfo.ArchiveExt.compare("rar", Qt::CaseInsensitive) == 0 || ArcInfo.ArchiveExt.compare("rev", Qt::CaseInsensitive) == 0) // is this a new naming scheme
		{
			ArcInfo.PartNumber = 1; // rar is always to be threaded like multipart

			Pos = ArcInfo.FileName.lastIndexOf("part", -1, Qt::CaseInsensitive);
			if(Pos != -1)
			{
				int Pos1 = Pos+4;
				int Pos2 = ArcInfo.FileName.indexOf(QRegularExpression("[^0-9]"), Pos1);
				if(Pos2 == -1)
					Pos2 = ArcInfo.FileName.length();

				ArcInfo.FixRar = (ArcInfo.FileName.lastIndexOf(QRegularExpression("\\.part[0-9]+$", QRegularExpression::CaseInsensitiveOption), -1) + 1 != Pos);

				ArcInfo.PartNumber = ArcInfo.FileName.mid(Pos1, Pos2-Pos1).toInt();
				ArcInfo.FileName.remove(Pos, Pos2-Pos);	
			}
		}
		else // no its the old naming scheme
		{
			Pos = ArcInfo.ArchiveExt.indexOf(QRegularExpression("[0-9]", QRegularExpression::CaseInsensitiveOption));
			ArcInfo.PartNumber = ArcInfo.ArchiveExt.mid(Pos).toInt()+2; // .rar is 1 .r00 is 2, etc....
		}
		ArcInfo.ArchiveExt = "rar";
	}

	if(ArcInfo.ArchiveExt.indexOf(QRegularExpression("(part|)[0-9]{3,}")) == 0)
	{
		ArcInfo.PartNumber = ArcInfo.ArchiveExt.toInt();
		ArcInfo.ArchiveExt.clear();

		Pos = ArcInfo.FileName.lastIndexOf(".");
		if(Pos != -1)
		{
			ArcInfo.ArchiveExt = ArcInfo.FileName.mid(Pos+1);
			ArcInfo.FileName.remove(Pos, ArcInfo.FileName.length()-Pos);
		}
	}

#ifdef USE_7Z
	if(ArcInfo.ArchiveExt.indexOf(QRegularExpression("(rar|zip|7z)", QRegularExpression::CaseInsensitiveOption)) != -1)
		ArcInfo.FormatIndex = theArc.FindByExt(ArcInfo.ArchiveExt);
#endif

	ArcInfo.FileName += "." + ArcInfo.ArchiveExt;
	if(ArcInfo.FormatIndex == 0) // not a known archive
		ArcInfo.ArchiveExt.clear();
	return ArcInfo;
}
