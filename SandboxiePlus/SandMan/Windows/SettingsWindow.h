#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SettingsWindow.h"

class CSettingsWindow : public QMainWindow
{
	Q_OBJECT

public:
	CSettingsWindow(QWidget *parent = Q_NULLPTR);
	~CSettingsWindow();

signals:
	void OptionsChanged();

public slots:
	void apply();
	void accept();
	void reject();

private slots:
	void OnChange();
	void OnSetPassword();

protected:
	void closeEvent(QCloseEvent *e);

	QString m_NewPassword;

private:
	Ui::SettingsWindow ui;
};
