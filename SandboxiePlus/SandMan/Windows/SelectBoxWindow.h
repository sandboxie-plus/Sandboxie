#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SelectBoxWindow.h"
#include "SbiePlusAPI.h"

class CSelectBoxWindow : public QDialog
{
	Q_OBJECT

public:
	CSelectBoxWindow(const QStringList& Commands, const QString& BoxName, const QString& WrkDir = QString(), QWidget *parent = Q_NULLPTR);
	~CSelectBoxWindow();

private slots:
	void OnBoxDblClick(QTreeWidgetItem*);
	void OnBoxType();
	void OnRun();

protected:
	void closeEvent(QCloseEvent* e);

	QStringList		m_Commands;
	QString			m_WrkDir;

private:
	Ui::SelectBoxWindow ui;
};
