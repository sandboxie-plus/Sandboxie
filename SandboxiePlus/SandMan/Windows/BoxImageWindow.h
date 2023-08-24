#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BoxImageWindow.h"
#include "SbiePlusAPI.h"

class CBoxImageWindow : public QDialog
{
	Q_OBJECT

public:
	enum EAction {
		eNew,
		eMount,
		eChange,
		eExport,
		eImport
	};

	CBoxImageWindow(EAction Action, QWidget *parent = Q_NULLPTR);
	~CBoxImageWindow();

	QString		GetPassword() const { return m_Password; }
	QString		GetNewPassword() const { return m_NewPassword; }
	void		SetImageSize(quint64 uSize) const { return ui.txtImageSize->setText(QString::number(uSize / 1024)); }
	quint64		GetImageSize() const { return ui.txtImageSize->text().toULongLong() * 1024; }
	bool		UseProtection() const { return ui.chkProtect->isChecked(); }

private slots:
	void		OnShowPassword();
	void		OnImageSize();
	void		CheckPassword();

private:
	Ui::BoxImageWindow ui;

	EAction m_Action;
	QString m_Password;
	QString m_NewPassword;
};
