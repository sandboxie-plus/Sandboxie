#pragma once

#include "../mischelpers_global.h"

// *Note* no archiver specific includes here

#define USE_7Z

#ifdef USE_7Z

#define ERR_7Z_OK					0
#define ERR_7Z_NO_INTERFACE			1
#define ERR_7Z_ALREADY_OPEN			2
#define ERR_7Z_FILE_NOT_EXIST		3
#define ERR_7Z_OPEN_FAILED			4
#define ERR_7Z_PASSWORD_REQUIRED	5
#define ERR_7Z_UNSUPPORTED_FORMAT	6

class MISCHELPERS_EXPORT CArchive
{
public:
	CArchive(const QString &ArchivePath, QIODevice* pDevice = NULL);
	static void					Init();
	static bool					IsInit();
	~CArchive();

	QString						GetPassword(bool bSet = false)			{if(bSet) m_PasswordUsed = true; return m_Password;}
	void						SetPassword(const QString& Password)	{m_Password = Password; m_PasswordUsed = false;}
	bool						HasUsedPassword()						{return m_PasswordUsed;}
	bool						SetPartSize(quint64 PartSize)			{if(m_Archive) return false; m_PartSize = PartSize; return true;}

	int							Open();
	bool						Extract(QString Path = "");
	bool						Extract(QMap<int, QIODevice*> *FileList, bool bDelete = true);
	bool						Close();

	bool						Update(QMap<int, QIODevice*> *FileList, bool bDelete = true, int Level = 0);

	int							AddFile(QString Path);
	int							FileCount()								{return m_Files.count();}
	int							FindByPath(QString Path);
	int							FindByIndex(int Index);
	void						RemoveFile(int ArcIndex);

	QVariant					FileProperty(int ArcIndex, QString Name);
	void						FileProperty(int ArcIndex, QString Name, QVariant Value);

	quint64						GetPartSize()							{return m_PartSize;}
	const QString&				GetArchivePath()						{return m_ArchivePath;}
	double						GetProgress()							{return m_Progress.GetValue();}

	void						SetPartList(const QStringList& Parts)	{m_AuxParts = Parts;}

	static QString				PrepareExtraction(QString FileName, QString Path);

protected:
	int							GetIndex(int ArcIndex);

	QString						GetNextPart(QString FileName);

	virtual void				LogError(const QString& Error);

	friend class CArchiveOpener;
	friend class CArchiveExtractor;
	friend class CArchiveUpdater;

	QString						m_ArchivePath;
	QIODevice*					m_pDevice;
	QString						m_Password;
	bool						m_PasswordUsed;
	quint64						m_PartSize;

	QStringList					m_AuxParts;

	struct SArchive;
	SArchive*					m_Archive;

	struct SFile
	{
		SFile(int Index = -1)
		{
			ArcIndex = Index;
			//NewData = false;
			//NewInfo = false;
		}
		int			ArcIndex;
		QVariantMap	Properties;
		//bool		NewData;
		//bool		NewInfo;
	};
	QVector<SFile>				m_Files;

	struct SProgress
	{
		SProgress(){
			uTotal = 0;
			uCompleted = 0;
		}
		void			SetTotal(quint64 Total) {uTotal = Total;}
		void			SetCompleted(quint64 Completed) {uCompleted = Completed;}
		double			GetValue()
		{
			double Total = uTotal;
			double Completed = uCompleted;
			return (Total > 0) ? Completed/Total : 0;
		}
		quint64			uTotal;
		quint64			uCompleted;
	}							m_Progress;
};

#endif

struct MISCHELPERS_EXPORT SArcInfo
{
	SArcInfo() 
	{
		PartNumber = -1;	// Single Part 
		FormatIndex = 0;	// Not an archive (only a split file) or unsupported format
		FixRar = false;		// indicates if rar file name needs fixing
	}
	QString		FileName;		// Filename after extension removal
	QString		ArchiveExt;		// Archive Extension
	int			FormatIndex;	// Archive format Index
	int			PartNumber;		// Part Number, -1 means not a multipart , 1 means first part
	bool		FixRar;
};

SArcInfo MISCHELPERS_EXPORT GetArcInfo(const QString &FileName);

class CArchiveInterface;
extern CArchiveInterface theArc;
