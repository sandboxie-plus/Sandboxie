#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SettingsWindow.h"

class CSettingsWindow : public QDialog
{
	Q_OBJECT

public:
	CSettingsWindow(QWidget *parent = Q_NULLPTR);
	~CSettingsWindow();

	virtual void accept() {}
	virtual void reject();

signals:
	void OptionsChanged();
	void Closed();

public slots:
	void ok();
	void apply();

	void showCompat();

private slots:
	void OnChange();

	void OnTab();

	void OnBrowse();

	void OnSetPassword();

	void OnWarnChanged() { m_WarnProgsChanged = true; }
	void OnAddWarnProg();
	void OnAddWarnFolder();
	void OnDelWarnProg();

	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnAddCompat();
	void OnDelCompat();

protected:
	void closeEvent(QCloseEvent *e);

	void	AddWarnEntry(const QString& Name, int type);

	int 	m_CompatLoaded;
	QString m_NewPassword;
	bool	m_WarnProgsChanged;
	bool	m_CompatChanged;
private:
	Ui::SettingsWindow ui;
};
