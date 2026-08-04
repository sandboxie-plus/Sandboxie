#include "stdafx.h"
#include "RenameSandboxDialog.h"
#include "../MiscHelpers/Common/Settings.h"
#include <QLayout>

CRenameSandboxDialog::CRenameSandboxDialog(const QString& boxName, const QString& alias, bool aliasDisabled, bool hasAliasSetting, QWidget* parent)
	: QDialog(parent)
{
	Q_UNUSED(hasAliasSetting);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie-Plus - Rename Sandbox"));

	ui.txtBoxName->setText(boxName);
	ui.txtBoxAlias->setText(alias);
	ui.chkAliasDisabled->setChecked(aliasDisabled);

	ui.chkAliasDisabled->setToolTip(tr("When enabled, alias display is disabled for this sandbox."));
	ui.chkHideAlias->setToolTip(tr("Hide alias input in this dialog. This preference is remembered."));

	const bool hideAlias = theConf->GetBool("Options/HideAliasInput", true);
	ui.chkHideAlias->setChecked(hideAlias);

	connect(ui.chkHideAlias, SIGNAL(toggled(bool)), this, SLOT(OnHideAliasToggled(bool)));
	connect(ui.txtBoxAlias, SIGNAL(textChanged(const QString&)), this, SLOT(OnAliasTextChanged(const QString&)));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	UpdateAliasDisabledState();
	OnHideAliasToggled(hideAlias);
	UpdateFixedHeight();
}

CRenameSandboxDialog::~CRenameSandboxDialog()
{
	theConf->SetValue("Options/HideAliasInput", IsAliasHidden());
}

void CRenameSandboxDialog::OnHideAliasToggled(bool checked)
{
	ui.lblAliasPrompt->setVisible(!checked);
	ui.txtBoxAlias->setVisible(!checked);
	ui.gridLayout->invalidate();
	layout()->activate();
	UpdateFixedHeight();
}

void CRenameSandboxDialog::OnAliasTextChanged(const QString& text)
{
	Q_UNUSED(text);
	UpdateAliasDisabledState();
}

void CRenameSandboxDialog::UpdateAliasDisabledState()
{
	const bool hasAliasText = !ui.txtBoxAlias->text().trimmed().isEmpty();
	ui.chkAliasDisabled->setEnabled(hasAliasText);
	if (!hasAliasText)
		ui.chkAliasDisabled->setChecked(false);
}

void CRenameSandboxDialog::UpdateFixedHeight()
{
	const int h = sizeHint().height();
	if (height() != h)
		resize(width(), h);
	setMinimumHeight(h);
	setMaximumHeight(h);
}
