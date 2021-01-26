#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_NewBoxWindow.h"
#include "SbiePlusAPI.h"

class CNewBoxWindow : public QDialog
{
	Q_OBJECT

public:
	CNewBoxWindow(QWidget *parent = Q_NULLPTR);
	~CNewBoxWindow();

	QString m_Name;

private slots:
	void CreateBox();

	void OnPreset();

protected:
	enum ETemplates
	{
		eHardened = 0,
		eDefault,
		eLegacy,
		eLenient,
		eOpen
	};

private:
	Ui::NewBoxWindow ui;
};
