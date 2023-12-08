#pragma once
#include <QObject>

#include "../qsbieapi_global.h"

#include "../SbieStatus.h"

class QSBIEAPI_EXPORT CSbieTemplates : public QObject
{
	Q_OBJECT
public:
	CSbieTemplates(class CSbieAPI* pAPI, QObject* paretn = 0);

	virtual void RunCheck();
	virtual void SetCheckResult(const QStringList& Result);
	virtual bool GetCheckState();

	enum EStates
	{
		eNone = 0x00,
		eEnabled = 0x01,
		eRequired = 0x02,
		eDisabled = 0x04,
		eConfigured = eEnabled | eDisabled
	};

	virtual void Reset();

	QStringList GetObjects();
	QStringList GetClasses();
	QStringList GetServices();
	QStringList GetProducts();
	QMap<QString, int> GetTemplates() { return m_Templates; }


	QString ExpandPath(QString path);

	bool CheckRegistryKey(const QString& Value);
	bool CheckFile(const QString& Value);
	bool CheckClasses(const QString& Value);
    bool CheckServices(const QString& Value);
    bool CheckProducts(const QString& Value);
    bool CheckObjects(const QString& Value);

protected:
	void CollectObjects();
	void CollectClasses();
	void CollectServices();
	void CollectProducts();
	void CollectTemplates();

	QStringList GetTemplateNames(const QString& forClass);

	bool CheckTemplate(const QString& Name);

	void InitExpandPaths(bool WithUser);

	QStringList m_Objects;
	QStringList m_Classes;
	QStringList m_Services;
	QStringList m_Products;

	QMap<QString, int> m_Templates;

	QMap<QString, QString> m_Expands;

	class CSbieAPI* m_pAPI;
};