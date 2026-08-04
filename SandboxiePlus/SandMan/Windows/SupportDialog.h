#pragma once

#include <QtWidgets/QMainWindow>
//#include "ui_SupportDialog.h"
#include "SbiePlusAPI.h"

class CSupportDialog : public QDialog
{
	Q_OBJECT

public:
	CSupportDialog(const QString& Message, bool NoGo = false, int Wait = 3, QWidget *parent = Q_NULLPTR);
	~CSupportDialog();

	static bool IsBusinessUse();

	static bool CheckSupport(bool bOnRun = false);

	static bool ShowDialog(bool NoGo = false , int Wait = 3);

	static bool m_ReminderShown;

	static QDateTime GetSbieInstallationDate();

private slots:
	void OnButton();

protected:
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;

	int					m_CountDown;

private:
	void				UpdateButtons();

	//Ui::SupportDialog ui;
	QPushButton*		m_Buttons[3];
};
