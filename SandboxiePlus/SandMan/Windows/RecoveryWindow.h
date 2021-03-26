#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RecoveryWindow.h"
#include "SbiePlusAPI.h"
class CSimpleTreeModel;

class CRecoveryCounter : public QThread
{
	Q_OBJECT
public:
	CRecoveryCounter(const QString& BoxPath, QWidget* parent = Q_NULLPTR) : QThread(parent) {
		m_BoxPath = BoxPath;
		m_run = true;
		start(QThread::LowPriority);
	}
	~CRecoveryCounter() {
		m_run = false;
		wait(2000);
		terminate();
	}

signals:
	void		Count(quint32 fileCount, quint32 folderCount, quint64 totalSize);

protected:
	void		run();

	QString		m_BoxPath;
	bool		m_run;
};

class CRecoveryWindow : public QDialog
{
	Q_OBJECT

public:
	CRecoveryWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CRecoveryWindow();

	virtual void accept() {}
	virtual void reject() { this->close(); }

public slots:
	int			exec();

	int			FindFiles();

private slots:
	void		OnAddFolder();
	void		OnRecover()		{ RecoverFiles(false); }
	void		OnRecoverTo()	{ RecoverFiles(true); }
	void		OnDeleteAll();

	void		OnCount(quint32 fileCount, quint32 folderCount, quint64 totalSize);

protected:
	void		closeEvent(QCloseEvent *e);

	int			FindFiles(const QString& Folder);
	int			FindBoxFiles(const QString& Folder);
	int			FindFiles(const QString& RecParent, const QString& BoxedFolder, const QString& RealFolder);

	void		RecoverFiles(bool bBrowse);

	CSandBoxPtr m_pBox;

	QMap<QVariant, QVariantMap> m_FileMap;

	QStringList m_RecoveryFolders;

	CRecoveryCounter* m_pCounter;

private:
	Ui::RecoveryWindow ui;
	CSimpleTreeModel* m_pFileModel;
};
