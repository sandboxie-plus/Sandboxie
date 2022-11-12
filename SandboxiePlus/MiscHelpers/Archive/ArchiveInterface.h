#pragma once

#include "ArchiveHelper.h"

#ifdef USE_7Z


/*struct CCdrInfo
{
	UInt32 Index;
	CLSID Encoder;
	CLSID Decoder;
	bool EncoderIsAssigned;
	bool DecoderIsAssigned;
};*/

struct CArcInfoEx
{
	CArcInfoEx(): UpdateEnabled(false), KeepName(false) {}

	CLSID ClassID;
	QString Name;

	bool UpdateEnabled;
	bool KeepName;

	struct CArcExtInfo
	{
		CArcExtInfo(const QString ext = "", const QString addExt = ""): Ext(ext), AddExt(addExt) {}
		QString Ext;
		QString AddExt;
	};
	QList<CArcExtInfo> Exts;

#ifdef _SFX
	QByteArray StartSignature;
	// CByteBuffer FinishSignature;
#endif

	QString GetMainExt() const
	{
		if (Exts.isEmpty())
			return "";
		return Exts[0].Ext;
	}
	int FindExtension(const QString &ext) const
	{
		for (int i = 0; i < Exts.count(); i++)
			if (ext.compare(Exts[i].Ext, Qt::CaseInsensitive) == 0)
				return i;
		return -1;
	}
	QString GetAllExtensions() const
	{
		QString s;
		for (int i = 0; i < Exts.count(); i++)
		{
			if (i > 0)
				s += ' ';
			s += Exts[i].Ext;
		}
		return s;
	}
	void AddExts(const QString &ext, const QString &addExt)
	{
		QStringList exts = ext.split(" ");
		exts.removeAll("");
		QStringList addExts  = addExt.split(" ");
		addExts.removeAll("");
		for (int i = 0; i < exts.count(); i++)
		{
			CArcExtInfo extInfo;
			extInfo.Ext = exts[i];
			if (i < addExts.count())
			{
				extInfo.AddExt = addExts[i];
				if (extInfo.AddExt == "*")
					extInfo.AddExt.clear();
			}
			Exts.append(extInfo);
		}
	}
};


class CArchiveInterface
{
public:
	CArchiveInterface();
	bool					Init();

	bool					IsOperational()		{return m_Operational;}

	int						FindByExt(const QString &Ext) const;
	//QList<int>				FindByExt(const QString &Ext);
	QString					GetArchiveName(int formatIndex);

	int						GetFormatCount()	{return m_Formats.count();}
	bool					CreateInArchive(int formatIndex, CMyComPtr<IInArchive> &archive) const;
	bool					CreateOutArchive(int formatIndex, CMyComPtr<IOutArchive> &archive) const;

protected:
	bool					CreateArchiveHandler(const CArcInfoEx &ai, void **archive, bool outHandler) const;

	QLibrary				m_7z;
	//QList<CCdrInfo>			m_Coders;
	QVector<CArcInfoEx>		m_Formats;
	CreateObjectFunc		createObjectFunc;

	bool					m_Operational;
	mutable QMutex			m_Mutex;
};

#endif
