#pragma once

#include <QtWidgets/QDialog>
#include "ui_RenameSandboxDialog.h"

class CRenameSandboxDialog : public QDialog
{
	Q_OBJECT

public:
	enum EResult { ePrevious = 2, eNext };

	CRenameSandboxDialog(const QString& boxName, const QString& alias, bool aliasDisabled, bool hasAliasSetting,
		QWidget* parent = Q_NULLPTR, int index = 0, int count = 1, bool pendingDrafts = false);
	~CRenameSandboxDialog();

	QString GetBoxName() const { return ui.txtBoxName->text().trimmed(); }
	QString GetAlias() const { return ui.txtBoxAlias->text().trimmed(); }
	bool IsAliasDisabled() const { return ui.chkAliasDisabled->isChecked(); }
	bool IsAliasHidden() const { return ui.chkHideAlias->isChecked(); }
	bool IsGuidName() const { return ui.chkGuidName->isChecked(); }
	QString GetManualName() const { return IsGuidName() ? m_ManualName : GetBoxName(); }
	void SetDraft(const QString& boxName, const QString& alias, bool aliasDisabled,
		bool useGuidName = false, const QString& manualName = QString());

public slots:
	void reject() override;

protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void OnBoxNameChanged(const QString& name);
	void OnGuidNameToggled(bool checked);
	void OnHideAliasToggled(bool checked);
	void OnAliasTextChanged(const QString& text);

private:
	void UpdateFixedHeight();
	void UpdateAliasDisabledState();
	bool ConfirmDiscard();

	bool m_BatchMode;
	bool m_HasPendingDrafts;
	QString m_SavedName;
	QString m_SavedAlias;
	bool m_SavedAliasDisabled;
	QString m_ManualName;

	Ui::RenameSandboxDialog ui;
};
