#pragma once
#include <QObject>

#include "../qsbieapi_global.h"

#include "../SbieStatus.h"

class QSBIEAPI_EXPORT CSbieIni: public QObject
{
	Q_OBJECT
public:
	CSbieIni(const QString& Section, class CSbieAPI* pAPI, QObject* parent = 0);
	virtual ~CSbieIni();

	virtual QString		GetName() const { return m_Name; }
	virtual void		SetRefreshOnChange(bool bSet) { m_RefreshOnChange = bSet; }

	virtual SB_STATUS	SetText(const QString& Setting, const QString& Value);
	virtual SB_STATUS	SetNum(const QString& Setting, int Value);
	virtual SB_STATUS	SetNum64(const QString& Setting, __int64 Value);
	virtual SB_STATUS	SetBool(const QString& Setting, bool Value);
	virtual SB_STATUS	SetBoolSafe(const QString& Setting, bool Value);

	virtual QString		GetText(const QString& Setting, const QString& Default = QString(), bool bWithGlobal = false, bool bNoExpand = true, bool withTemplates = false, bool getProperty = false) const;
	virtual int			GetNum(const QString& Setting, int Default = 0, bool bWithGlobal = false, bool withTemplates = false, bool getProperty = false) const;
	virtual __int64		GetNum64(const QString& Setting, __int64 Default = 0, bool bWithGlobal = false, bool withTemplates = false, bool getProperty = false) const;
	virtual bool		GetBool(const QString& Setting, bool Default = false, bool bWithGlobal = false, bool withTemplates = false, bool getProperty = false) const;

	virtual QStringList GetTextList(const QString &Setting, bool withTemplates, bool bExpand = false, bool bWithGlobal = false) const;
	virtual SB_STATUS	UpdateTextList(const QString &Setting, const QStringList& List, bool withTemplates);
	virtual QStringList GetTemplates() const;
	virtual QStringList GetTextListTmpl(const QString &Setting, const QString& Template) const;

	virtual SB_STATUS	InsertText(const QString& Setting, const QString& Value);
	virtual SB_STATUS	AppendText(const QString& Setting, const QString& Value);

	virtual void		SetTextMap(const QString& Setting, const QMap<QString, QStringList> Map);
	virtual QMap<QString, QStringList> GetTextMap(const QString& Setting);

	virtual SB_STATUS	DelValue(const QString& Setting, const QString& Value = QString());

	struct SbieIniValue
	{
		QString Name;
		quint32 Type = 0;
		QString Value;
	};
	virtual QList<SbieIniValue> GetIniSection(qint32* pStatus = NULL, bool withTemplates = false, bool withGlobals = false, bool noExpand = true) const;

	enum ESetMode
	{
		eIniUpdate = 0,
		eIniAppend,
		eIniInsert,
		eIniDelete
	};

	virtual SB_STATUS	RenameSection(const QString& NewName, bool deleteOld = true);
	virtual SB_STATUS	RemoveSection();
	virtual void		CommitIniChanges();
	virtual SB_STATUS	SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode = eIniUpdate, bool bRefresh = true);
	virtual QString		SbieIniGet(const QString& Section, const QString& Setting, quint32 Index, qint32* ErrCode = NULL, quint32* pType = NULL) const;
	virtual QString		SbieIniGetEx(const QString& Section, const QString& Setting) const;

	CSbieAPI*			GetAPI() { return m_pAPI; }

protected:

	QString				m_Name;
	class CSbieAPI*		m_pAPI;
	bool				m_RefreshOnChange;
	bool				m_IsVirtual;
};