#include "stdafx.h"
#include "ArchiveThread.h"
#include "SplitFile.h"
#include "../Common/Common.h"
#include "../Common/OtherFunctions.h"

int _QStringList_pType = qRegisterMetaType<QStringList>("QStringList");

CArchiveThread::CArchiveThread(const QString &ArchivePath, const QString &WorkingPath, const QString &Password, quint64 PartSize, const QStringList& Parts)
{
#ifdef USE_7Z
	m_Worker = new C7zWorker(ArchivePath, WorkingPath, Password, PartSize, Parts);
	m_Worker->moveToThread(this);
	start();
	m_Worker->m_Lock.Lock();
#else
	ASSERT(0);
#endif
}

CArchiveThread::CArchiveThread(const QString &WinRarPath, const QString &ArchivePath, const QString &WorkingPath, const QString &Password, quint64 PartSize, const QString &CommentPath)
{
	m_Worker = new CRarWorker(WinRarPath, ArchivePath, WorkingPath, Password, PartSize, CommentPath);
	m_Worker->moveToThread(this);
	start();
	m_Worker->m_Lock.Lock();
}

void CArchiveThread::Terminate()
{
	stop();
	delete m_Worker;
	delete this;
}

QStringList CArchiveThread::ListParts(const QString &ArchivePath, bool NamesOnly)
{
	QStringList Parts;

	StrPair PathName = Split2(ArchivePath, "/", true);
	StrPair NameEx = Split2(PathName.second, ".", true);
	QString PatternStr;
	if(NameEx.second == "rar")
		PatternStr = QRegularExpression::escape(NameEx.first) + "(\\.part[0-9]*)?\\.rar";
	else
		PatternStr = QRegularExpression::escape(NameEx.first + "." + NameEx.second) + "(\\.[0-9]*)?";
	QRegularExpression Pattern = QRegularExpression(QRegularExpression::anchoredPattern(PatternStr), QRegularExpression::CaseInsensitiveOption);

	QDir Dir(PathName.first);
	foreach (const QString& File, Dir.entryList())
	{
		if (File.compare(".") == 0 || File.compare("..") == 0)
			continue;
		QRegularExpressionMatch Result = Pattern.match(File);
		if(Result.hasMatch())
		{
			if(NamesOnly)
				Parts.append(File);
			else
				Parts.append(PathName.first + "/" + File);
		}
	}
	return Parts;
}

QStringList CArchiveThread::GetErrors()
{
	QStringList Errors;
	if(IsBusy())
		return Errors;
	Errors = m_Worker->m_Errors;
	if(!Errors.isEmpty() && HasUsedPassword())
		Errors.append("Wrong Password ?");
	m_Worker->m_Errors.clear();
	return Errors;
}

//////////////////////////////////////////////////////////////////////////
// Buildin 7z Library worker
//

#ifdef USE_7Z

C7zWorker::C7zWorker(const QString &ArchivePath, const QString &WorkingPath, const QString &Password, quint64 PartSize, const QStringList& Parts)
: CArchive(ArchivePath)
{
	SetPassword(Password);
	SetPartSize(PartSize);
	m_WorkingPath = WorkingPath;
	SetPartList(Parts);

	if(QFile::exists(ArchivePath))
	{
		if(Open() != ERR_7Z_OK)
			m_Errors.append("Open Failed");
	}
}

C7zWorker::~C7zWorker()
{
	Close();
}

QMap<QString, quint64> C7zWorker::GetFileList()
{
	QMap<QString, quint64> FileList;
	if(!m_Mutex.tryLock(100))
	{
		//LogLine(LOG_DEBUG | LOG_ERROR, tr("attempted to get the file list of an already busy archive thread"));
		return FileList;
	}
	
	if(m_Archive)
	{
		foreach(const SFile& File, m_Files)	
		{
			if(File.Properties["IsDir"].toBool())
				continue;
			FileList.insert(File.Properties["Path"].toString(), File.Properties["Size"].toULongLong());
		}
	}
	//else
		//LogLine(LOG_DEBUG | LOG_ERROR, tr("Archive %1, is not open for extraction").arg(m_ArchivePath));

	m_Mutex.unlock();
	return FileList;
}

void C7zWorker::OnExtractAllFiles()
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	SArcInfo Info = GetArcInfo(m_ArchivePath);
	if(Info.FormatIndex == 0) // not an archive just a split file
	{
		if(!QDir().exists(m_WorkingPath))
			QDir().mkpath(m_WorkingPath);

		if(Info.PartNumber == -1) // single aprt
		{
			if(!QFile::rename(m_ArchivePath,m_WorkingPath + Info.FileName))
				m_Errors.append("Failed to move file");
		}
		else // multi part, to be merged
		{
			int Pos = m_ArchivePath.indexOf(QRegularExpression("\\.[0-9]+$"));
			if(Pos != -1)
				m_ArchivePath.truncate(Pos);

			CSplitFile SplitFile(m_ArchivePath);
			if(!SplitFile.open(QIODevice::ReadOnly))
				m_Errors.append("Failed to open temp file");
			else
			{
				QFile File(m_WorkingPath + Info.FileName);
				if(!File.open(QIODevice::WriteOnly))
					m_Errors.append("Failed to open file");
				else
				{
					const qint64 Size = 16*1024;
					char Buffer[Size];
					while(!SplitFile.atEnd())
					{
						qint64 Temp = SplitFile.read(Buffer, Size);
						File.write(Buffer, Temp);
					}
				}
			}
		}
	}
	else

	if(!Extract(m_WorkingPath))
		m_Errors.append("Extraction Failed");
}

void C7zWorker::OnInsertAllFiles()
{
	OnInsertFiles(ListDir(m_WorkingPath));
}

void C7zWorker::OnExtractFiles(QStringList FileList)
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	if(!QDir().exists(m_WorkingPath))
		QDir().mkpath(m_WorkingPath);

	QMap<int, QIODevice*> Files;
	foreach(const QString& Name, FileList)
	{
		int ArcIndex = FindByPath(Name);
		if(ArcIndex != -1)
			Files.insert(ArcIndex, new QFile(PrepareExtraction(Name, m_WorkingPath)));
		//else
			//LogLine(LOG_DEBUG, tr("file %1 couldn't be find in archive %2").arg(Name, m_ArchivePath)); // should not happen
	}

	if(!Extract(&Files))
		m_Errors.append("Extraction Failed");
}

void C7zWorker::OnInsertFiles(QStringList FileList)
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	QMap<int, QIODevice*> Files;
	foreach(const QString& File, FileList)
	{
		StrPair RootName = Split2(File, "\\", true);
		if(RootName.second.isEmpty())
		{
			RootName.second = RootName.first;
			RootName.first = "";
		}
		int ArcIndex = AddFile(RootName.second);
		if(ArcIndex != -1)
		{
			QString FileName = (RootName.first.isEmpty() ?  m_WorkingPath  : RootName.first) + RootName.second;
			Files.insert(ArcIndex, new QFile(FileName));
		}
		//else
			//LogLine(LOG_DEBUG, tr("file %1 is already present in archive %2").arg(RootName.second, m_ArchivePath)); // should not happen
	}

	if(!Update(&Files))
		m_Errors.append("Insertion Failed");
}

#endif

//////////////////////////////////////////////////////////////////////////
// Extern WinRar worker
//

CRarWorker::CRarWorker(const QString &WinRarPath, const QString &ArchivePath, const QString &WorkingPath, const QString &Password, quint64 PartSize, const QString &CommentPath)
{
	m_WinRarPath = WinRarPath;
	if(CommentPath.contains(" "))
		m_CommentPath = "\"" + CommentPath + "\"";
	else
		m_CommentPath = CommentPath;

	m_ArchivePath = ArchivePath;
	m_WorkingPath = WorkingPath;
	CreateDir(m_WorkingPath);
	m_Password = Password;
	m_PartSize = PartSize;

	if(!QFile::exists(WinRarPath + "rar.exe"))
		m_Errors.append("WinRar is not present or the path is wrong");
}

QMap<QString, quint64> CRarWorker::GetFileList()
{
	QMap<QString, quint64> FileList;
	if(!m_Mutex.tryLock(100))
	{
		//LogLine(LOG_DEBUG | LOG_ERROR, tr("attempted to get the file list of an already busy archive thread"));
		return FileList;
	}
	
	QStringList Arguments;
	Arguments.append("v");
	if(!m_Password.isEmpty())
		Arguments.append("-p" + m_Password);
	Arguments.append(m_ArchivePath);

	QProcess* pWinRar = new QProcess(this);
	pWinRar->start(m_WinRarPath + "unrar.exe", Arguments);
	/*for(;;)
	{
		bool bDone = pWinRar->waitForFinished(1000);
		QStringList Test;
		while(pWinRar->canReadLine())
		{
			QString Line = pWinRar->readLine();
			Test.append(Line);
		}
		if(bDone)
			break;
	}*/
	pWinRar->waitForFinished(-1);
	QStringList Test;
	int Index = -1;
	QString Name;
	while(pWinRar->canReadLine())
	{
		QString Line = pWinRar->readLine().trimmed();
		if(Line == "-------------------------------------------------------------------------------")
		{
			if(Index == -1)
			{
				Index = 0;
				continue;
			}
			break;
		} 
		else if(Index == -1)
			continue;
		Index++;

		if(Index % 2 == 1)
			Name = Line.replace("\\","/");
		else
			FileList.insert(Name,Split2(Line," ").first.toULongLong());
	}
	delete pWinRar;

	m_Mutex.unlock();
	return FileList;
}

void CRarWorker::OnExtractAllFiles()
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	QStringList Arguments;
	Arguments.append("x");
	if(!m_Password.isEmpty())
		Arguments.append("-p" + m_Password);
	Arguments.append(m_ArchivePath);
	Arguments.append(m_WorkingPath);

	QProcess* pWinRar = new QProcess(this);
	pWinRar->start(m_WinRarPath + "unrar.exe", Arguments);
	pWinRar->waitForFinished(-1);
	if(int ExitCode = pWinRar->exitCode())
		m_Errors.append(QString("WinRar returrned error code %1").arg(ExitCode));
	delete pWinRar;
}

void CRarWorker::OnInsertAllFiles()
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	QStringList Arguments;
	Arguments.append("a");
	Arguments.append("-m0");
	if(!m_Password.isEmpty())
		Arguments.append("-hp" + m_Password);
	if(m_PartSize != -1)
		Arguments.append("-v" + QString::number(m_PartSize) + "b");
	if(!m_CommentPath.isEmpty())
		Arguments.append("-z" + m_CommentPath);
	Arguments.append(m_ArchivePath);
	Arguments.append(m_WorkingPath);

	QProcess* pWinRar = new QProcess(this);
	pWinRar->start(m_WinRarPath + "rar.exe", Arguments);
	pWinRar->waitForFinished(-1);
	if(int ExitCode = pWinRar->exitCode())
		m_Errors.append(QString("WinRar returrned error code %1").arg(ExitCode));
	delete pWinRar;
}

void CRarWorker::OnExtractFiles(QStringList FileList)
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	QStringList Arguments;
	Arguments.append("x");
	if(!m_Password.isEmpty())
		Arguments.append("-p" + m_Password);
	Arguments.append(m_ArchivePath);
	foreach(const QString& File, FileList)
		Arguments.append(File);
	Arguments.append(m_WorkingPath);

	QProcess* pWinRar = new QProcess(this);
	pWinRar->start(m_WinRarPath + "unrar.exe", Arguments);
	pWinRar->waitForFinished(-1);
	if(int ExitCode = pWinRar->exitCode())
		m_Errors.append(QString("WinRar returrned error code %1").arg(ExitCode));
	delete pWinRar;
}

void CRarWorker::OnInsertFiles(QStringList FileList)
{
	QMutexLocker Locker(&m_Mutex);
	m_Lock.Release();

	QStringList Arguments;
	Arguments.append("a");
	Arguments.append("-m0");
	Arguments.append("-ep");
	Arguments.append("-ap" + Split2(Split2(m_ArchivePath, "/", true).second, ".", true).first.replace(" ", "_"));
	if(!m_Password.isEmpty())
		Arguments.append("-hp" + m_Password);
	if(m_PartSize != -1)
		Arguments.append("-v" + QString::number(m_PartSize) + "b");
	if(!m_CommentPath.isEmpty())
		Arguments.append("-z" + m_CommentPath);
	Arguments.append(m_ArchivePath);
	foreach(const QString& File, FileList)
	{
		StrPair RootName = Split2(File, "\\", true);
		if(RootName.second.isEmpty())
		{
			RootName.second = RootName.first;
			RootName.first = "";
		}
		QString FileName = (RootName.first.isEmpty() ? m_WorkingPath : RootName.first) + RootName.second;
		Arguments.append(FileName);
	}

	QProcess* pWinRar = new QProcess(this);
	pWinRar->start(m_WinRarPath + "rar.exe", Arguments);
	pWinRar->waitForFinished(-1);
	if(int ExitCode = pWinRar->exitCode())
		m_Errors.append(QString("WinRar returrned error code %1").arg(ExitCode));
	delete pWinRar;
}
