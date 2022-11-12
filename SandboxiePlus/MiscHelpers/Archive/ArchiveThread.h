#pragma once

#include "../mischelpers_global.h"

#include "../Common/MT/ThreadLock.h"
#include "Archive.h"

class CArchiveWorker: public QObject
{
	Q_OBJECT

public:

	virtual double			GetProgress() = 0;
	virtual const QString&	GetArchivePath() = 0;
	virtual quint64			GetPartSize() = 0;
	virtual QString			GetPassword() = 0;
	virtual bool			HasUsedPassword() = 0;

	virtual QMap<QString, quint64> GetFileList() = 0;

public slots:
	virtual void			OnExtractAllFiles() = 0;
	virtual void			OnInsertAllFiles() = 0;
	virtual void			OnExtractFiles(QStringList FileList) = 0;
	virtual void			OnInsertFiles(QStringList FileList) = 0;

protected:
	friend class CArchiveThread;

	virtual void			LogError(const QString &Error)		{m_Errors.append(Error);}

	QMutex					m_Mutex;
	CThreadLock				m_Lock;
	QString					m_WorkingPath;

	QStringList				m_Errors;
};

class MISCHELPERS_EXPORT CArchiveThread: public QThread
{
	Q_OBJECT

public:
	CArchiveThread(const QString &ArchivePath, const QString &WorkingPath, const QString &Password = QString(), quint64 PartSize = -1, const QStringList& Parts = QStringList());
	CArchiveThread(const QString &WinRarPath, const QString &ArchivePath, const QString &WorkingPath, const QString &Password = QString(), quint64 PartSize = -1, const QString &CommentPath = "");
	void Terminate();

	static QStringList		ListParts(const QString &ArchivePath, bool NamesOnly = false);
	QMap<QString, quint64>	GetFileList()						{return m_Worker->GetFileList();}

	void					ExtractAllFiles()						{m_Worker->m_Lock.Reset(); emit AsyncExtractAllFiles();		m_Worker->m_Lock.Lock();}
	void					InsertAllFiles()						{m_Worker->m_Lock.Reset(); emit AsyncInsertAllFiles();		m_Worker->m_Lock.Lock();}
	void					ExtractFiles(QStringList FileList)		{m_Worker->m_Lock.Reset(); emit AsyncExtractFiles(FileList);m_Worker->m_Lock.Lock();}		// paths in archive
	void					InsertFiles(QStringList FileList)		{m_Worker->m_Lock.Reset(); emit AsyncInsertFiles(FileList);	m_Worker->m_Lock.Lock();}		// paths relative to the WorkingPath

	bool					IsBusy()								{if(!m_Worker->m_Mutex.tryLock()) return true; m_Worker->m_Mutex.unlock(); return false;}
	double					GetProgress()							{return m_Worker->GetProgress();}
	QStringList				GetArchiveParts(bool NamesOnly = false)	{return ListParts(m_Worker->GetArchivePath(), NamesOnly);}
	quint64					GetPartSize()							{return m_Worker->GetPartSize();}
	QStringList				GetErrors();
	bool					HasUsedPassword()						{return m_Worker->HasUsedPassword();}
	QString					GetPassword()							{return m_Worker->GetPassword();}

	void run()
	{
		connect(this, SIGNAL(AsyncExtractAllFiles()), m_Worker, SLOT(OnExtractAllFiles()));
		connect(this, SIGNAL(AsyncInsertAllFiles()), m_Worker, SLOT(OnInsertAllFiles()));
		connect(this, SIGNAL(AsyncExtractFiles(QStringList)), m_Worker, SLOT(OnExtractFiles(QStringList)));
		connect(this, SIGNAL(AsyncInsertFiles(QStringList)), m_Worker, SLOT(OnInsertFiles(QStringList)));
		m_Worker->m_Lock.Release();
		exec();
	}

	void stop()
	{
		quit();
		wait();
	}

signals:
	void					AsyncExtractAllFiles();
	void					AsyncInsertAllFiles();
	void					AsyncExtractFiles(QStringList FileList);
	void					AsyncInsertFiles(QStringList FileList);

protected:
	CArchiveWorker*			m_Worker;
};

#ifdef USE_7Z

class MISCHELPERS_EXPORT C7zWorker: public CArchiveWorker, protected CArchive 
{
	Q_OBJECT

public:
	C7zWorker(const QString &ArchivePath, const QString &WorkingPath, const QString &Password = QString(), quint64 PartSize = -1, const QStringList& Parts = QStringList());
	~C7zWorker();

	virtual double			GetProgress()							{return CArchive::GetProgress();}
	virtual const QString&	GetArchivePath()						{return CArchive::GetArchivePath();}
	quint64					GetPartSize()							{return CArchive::GetPartSize();}
	virtual bool			HasUsedPassword()						{return CArchive::HasUsedPassword();}
	virtual QString			GetPassword()							{return CArchive::GetPassword();}

	virtual QMap<QString, quint64> GetFileList();

public slots:
	virtual void			OnExtractAllFiles();
	virtual void			OnInsertAllFiles();
	virtual void			OnExtractFiles(QStringList FileList);
	virtual void			OnInsertFiles(QStringList FileList);

};

#endif

class MISCHELPERS_EXPORT CRarWorker: public CArchiveWorker
{
	Q_OBJECT

public:
	CRarWorker(const QString &WinRarPath, const QString &ArchivePath, const QString &WorkingPath, const QString &Password = QString(), quint64 PartSize = -1, const QString &CommentPath = "");

	virtual double			GetProgress()							{return 0;}
	virtual const QString&	GetArchivePath()						{return m_ArchivePath;}
	quint64					GetPartSize()							{return m_PartSize;}
	virtual bool			HasUsedPassword()						{ASSERT(0); return false;} // rar should notbe used for unpacking
	virtual QString			GetPassword()							{return m_Password;}

	virtual QMap<QString, quint64> GetFileList();

public slots:
	virtual void			OnExtractAllFiles();
	virtual void			OnInsertAllFiles();
	virtual void			OnExtractFiles(QStringList FileList);
	virtual void			OnInsertFiles(QStringList FileList);

protected:
	QString					m_ArchivePath;
	QString					m_Password;
	quint64					m_PartSize;

	QString					m_WinRarPath;
	QString					m_CommentPath;
};
