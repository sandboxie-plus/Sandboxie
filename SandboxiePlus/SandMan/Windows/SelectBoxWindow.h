#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SelectBoxWindow.h"
#include "SbiePlusAPI.h"

class CSelectBoxWindow : public QDialog
{
	Q_OBJECT

public:
	CSelectBoxWindow(const QStringList& Commands, const QString& BoxName, QWidget *parent = Q_NULLPTR);
	~CSelectBoxWindow();

private slots:
	void OnBoxDblClick(QTreeWidgetItem*);
	void OnBoxType();
	void OnRun();

protected:
	void closeEvent(QCloseEvent* e);

	QStringList		m_Commands;

private:
	Ui::SelectBoxWindow ui;
};
