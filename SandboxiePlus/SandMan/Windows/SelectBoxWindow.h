#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SelectBoxWindow.h"
#include "SbiePlusAPI.h"

//////////////////////////////////////////////////////////////////////////////////////
// CBoxPicker
//

class CBoxPicker : public QWidget 
{
	Q_OBJECT

public:
	CBoxPicker(QString DefaultBox = "", QWidget *parent = Q_NULLPTR);

	void LoadBoxed(const QString& Filter = QString(), const QString& SelectBox = QString());

	QString GetBoxName() const;

	static QTreeWidgetItem* GetBoxParent(const QMap<QString, QStringList>& Groups, QMap<QString, QTreeWidgetItem*>& GroupItems, QTreeWidget* treeBoxes, const QString& Name, int Depth = 0);
	static double GetBoxOrder(const QMap<QString, QStringList>& Groups, const QString& Name, double value = 0.0, int Depth = 0);

signals:
	void BoxDblClick();

private slots:
	void SetFilter(const QString& Exp, int iOptions, int Column);

private:
	QTreeWidget *m_pTreeBoxes;
};

//////////////////////////////////////////////////////////////////////////////////////
// CSelectBoxWindow
//

class CSelectBoxWindow : public QDialog
{
	Q_OBJECT

public:
	CSelectBoxWindow(const QStringList& Commands, const QString& BoxName, const QString& WrkDir = QString(), QWidget *parent = Q_NULLPTR);
	~CSelectBoxWindow();

private slots:
	void OnBoxType();
	void OnRun();

protected:
	void closeEvent(QCloseEvent* e);

	QStringList		m_Commands;
	QString			m_WrkDir;

private:
	Ui::SelectBoxWindow ui;
	CBoxPicker*		m_pBoxPicker;
};
