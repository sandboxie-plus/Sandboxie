#pragma once
#include <QObject>

#include "../qsbieapi_global.h"

#include "../SbieStatus.h"

class QSBIEAPI_EXPORT CSbieTemplates : public QObject
{
	Q_OBJECT
public:
	CSbieTemplates(class CSbieAPI* pAPI, QObject* paretn = 0);

	bool RunCheck();

	enum EStates
	{
		eNone = 0x00,
		eEnabled = 0x01,
		eRequired = 0x02,
		eDisabled = 0x04,
		eConfigured = eEnabled | eDisabled
	};

	QMap<QString, int> GetTemplates() { return m_Templates; }

protected:
	void CollectObjects();
	void CollectClasses();
	void CollectServices();
	void CollectProducts();
	void CollectTemplates();

	QStringList GetTemplateNames(const QString& forClass);

	bool CheckTemplate(const QString& Name);
	bool CheckRegistryKey(const QString& Value);
	bool CheckFile(const QString& Value);

	void InitExpandPaths(bool WithUser);
	QString ExpandPath(QString path);

	list<wstring> m_Objects;
	list<wstring> m_Classes;
	list<wstring> m_Services;
	list<wstring> m_Products;

	QMap<QString, int> m_Templates;

	QMap<QString, QString> m_Expands;

	class CSbieAPI* m_pAPI;
};