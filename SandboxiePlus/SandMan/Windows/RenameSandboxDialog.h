#pragma once

#include <QtWidgets/QDialog>
#include "ui_RenameSandboxDialog.h"

class CRenameSandboxDialog : public QDialog
{
	Q_OBJECT

public:
	CRenameSandboxDialog(const QString& boxName, const QString& alias, bool aliasDisabled, bool hasAliasSetting, QWidget* parent = Q_NULLPTR);
	~CRenameSandboxDialog();

	QString GetBoxName() const { return ui.txtBoxName->text().trimmed(); }
	QString GetAlias() const { return ui.txtBoxAlias->text().trimmed(); }
	bool IsAliasDisabled() const { return ui.chkAliasDisabled->isChecked(); }
	bool IsAliasHidden() const { return ui.chkHideAlias->isChecked(); }

private slots:
	void OnHideAliasToggled(bool checked);
	void OnAliasTextChanged(const QString& text);

private:
	void UpdateFixedHeight();

	void UpdateAliasDisabledState();

	Ui::RenameSandboxDialog ui;
};
