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

	virtual SB_STATUS SetText(const QString& Setting, const QString& Value);
	virtual SB_STATUS SetNum(const QString& Setting, int Value);
	virtual SB_STATUS SetNum64(const QString& Setting, __int64 Value);
	virtual SB_STATUS SetBool(const QString& Setting, bool Value);

	virtual QString GetText(const QString& Setting, const QString& Default = QString()) const;
	virtual int GetNum(const QString& Setting, int Default = 0) const;
	virtual __int64 GetNum64(const QString& Setting, __int64 Default = 0) const;
	virtual bool GetBool(const QString& Setting, bool Default = false) const;

	virtual QStringList GetTextList(const QString &Setting, bool withTemplates, bool expand = false) const;
	virtual SB_STATUS UpdateTextList(const QString &Setting, const QStringList& List, bool withTemplates);
	virtual QStringList GetTemplates() const;
	virtual QStringList GetTextListTmpl(const QString &Setting, const QString& Template) const;

	virtual SB_STATUS InsertText(const QString& Setting, const QString& Value);
	virtual SB_STATUS AppendText(const QString& Setting, const QString& Value);

	virtual SB_STATUS DelValue(const QString& Setting, const QString& Value = QString());

	virtual QList<QPair<QString, QString>> GetIniSection(qint32* pStatus = NULL, bool withTemplates = false) const;

	virtual SB_STATUS RenameSection(const QString& NewName, bool deleteOld = true);
	virtual SB_STATUS RemoveSection();

	CSbieAPI*			GetAPI() { return m_pAPI; }

protected:

	QString				m_Name;
	class CSbieAPI*		m_pAPI;
};