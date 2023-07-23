#pragma once
#include <QObject>

class CScriptManager: public QObject
{
	Q_OBJECT
public:
	CScriptManager(QObject* parent);

	Q_INVOKABLE QString GetScript(const QString& Name);

	void LoadIssues();
	void LoadIssues(const QString& IssueDir);

	QMap<QString, QList<QVariantMap>> GetIssues() { return m_GroupedIssues; }

	QString Tr(const QString& Text) { return m_Translation.value(Text, Text).toString(); }

	static QString GetIssueDir(class C7zFileEngineHandler& IssueFS, QDateTime* pDate = NULL);

signals:
	void IssuesUpdated();

private slots:
    void OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);
    void OnDownload(const QString& Path, const QVariantMap& Params);

private:
	QMap<QString,QList<QVariantMap>> m_GroupedIssues;

	QDateTime m_IssueDate;

	QVariantMap m_Translation;
};

