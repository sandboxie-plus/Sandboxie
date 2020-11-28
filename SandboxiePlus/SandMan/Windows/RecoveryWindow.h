#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RecoveryWindow.h"
#include "SbiePlusAPI.h"
class CSimpleTreeModel;

class CRecoveryWindow : public QDialog
{
	Q_OBJECT

public:
	CRecoveryWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CRecoveryWindow();

public slots:
	int			exec();

	int			FindFiles();

private slots:
	void		OnAddFolder();
	void		OnRecover()		{ RecoverFiles(false); }
	void		OnRecoverTo()	{ RecoverFiles(true); }
	void		OnDeleteAll();

protected:
	void closeEvent(QCloseEvent *e);

	int			FindFiles(const QString& Folder);

	void		RecoverFiles(bool bBrowse);

	CSandBoxPtr m_pBox;

	QMap<QVariant, QVariantMap> m_FileMap;

	QMap<QString, QString> m_RecoveryFolders;

private:
	Ui::RecoveryWindow ui;
	CSimpleTreeModel* m_pFileModel;
};
