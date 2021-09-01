#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FileBrowserWindow.h"
#include "SbiePlusAPI.h"
#include <QFileSystemModel>

class CFileBrowserWindow : public QDialog
{
	Q_OBJECT

public:
	CFileBrowserWindow(const CSandBoxPtr& pBox, QWidget *parent = Q_NULLPTR);
	~CFileBrowserWindow();

	virtual void accept() {}
	virtual void reject() { this->close(); }

signals:
	void Closed();

private slots:
	void				OnFileMenu(const QPoint &);
	void				OnFileDblClick(const QModelIndex &);

protected:
	void closeEvent(QCloseEvent *e);

	CSandBoxPtr m_pBox;

private:
	Ui::FileBrowserWindow ui;

	QFileSystemModel* m_pFileModel;
};
