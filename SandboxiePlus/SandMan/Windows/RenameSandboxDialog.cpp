#include "stdafx.h"
#include "RenameSandboxDialog.h"
#include "../MiscHelpers/Common/Settings.h"
#include <QLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include "../SbiePlusAPI.h"

CRenameSandboxDialog::CRenameSandboxDialog(const QString& boxName, const QString& alias, bool aliasDisabled, bool hasAliasSetting,
	QWidget* parent, int index, int count, bool pendingDrafts)
	: QDialog(parent), m_BatchMode(count > 1), m_HasPendingDrafts(pendingDrafts),
	m_SavedName(boxName), m_SavedAlias(alias.trimmed()), m_SavedAliasDisabled(aliasDisabled && !alias.trimmed().isEmpty()),
	m_ManualName(boxName)
{
	Q_UNUSED(hasAliasSetting);

	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	flags &= ~Qt::WindowContextHelpButtonHint;
	setWindowFlags(flags);

	ui.setupUi(this);
	this->setWindowTitle(m_BatchMode ? tr("Sandboxie-Plus - Rename Sandboxes (%1/%2)").arg(index + 1).arg(count)
		: tr("Sandboxie-Plus - Rename Sandbox"));

	ui.txtBoxName->setText(boxName);
	ui.txtBoxAlias->setText(alias);
	ui.chkAliasDisabled->setChecked(aliasDisabled);

	ui.chkAliasDisabled->setToolTip(tr("When enabled, alias display is disabled for this sandbox."));
	ui.chkHideAlias->setToolTip(tr("Hide alias input in this dialog. This preference is remembered."));

	const bool hideAlias = theConf->GetBool("Options/HideAliasInput", true);
	ui.chkHideAlias->setChecked(hideAlias);
	ui.widgetNavigation->setVisible(m_BatchMode);
	ui.btnPrevious->setEnabled(index > 0);
	ui.btnNext->setEnabled(index + 1 < count);
	ui.lblPosition->setText(QString("%1 / %2").arg(index + 1).arg(count));
	if (m_BatchMode) {
		ui.buttonBox->button(QDialogButtonBox::Ok)->setToolTip(
			tr("Save this sandbox and advance. On the last sandbox, also save all remaining drafts."));
		ui.buttonBox->button(QDialogButtonBox::Cancel)->setToolTip(
			tr("Discard unsaved drafts. Changes already saved with OK are kept."));
	}

	connect(ui.txtBoxName, &QLineEdit::textChanged, this, &CRenameSandboxDialog::OnBoxNameChanged);
	connect(ui.chkGuidName, &QCheckBox::toggled, this, &CRenameSandboxDialog::OnGuidNameToggled);
	connect(ui.chkHideAlias, SIGNAL(toggled(bool)), this, SLOT(OnHideAliasToggled(bool)));
	connect(ui.txtBoxAlias, SIGNAL(textChanged(const QString&)), this, SLOT(OnAliasTextChanged(const QString&)));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui.btnPrevious, &QPushButton::clicked, this, [this]() { done(ePrevious); });
	connect(ui.btnNext, &QPushButton::clicked, this, [this]() { done(eNext); });

	OnBoxNameChanged(boxName);
	UpdateAliasDisabledState();
	OnHideAliasToggled(hideAlias);
	UpdateFixedHeight();
}

CRenameSandboxDialog::~CRenameSandboxDialog()
{
	theConf->SetValue("Options/HideAliasInput", IsAliasHidden());
}

void CRenameSandboxDialog::SetDraft(const QString& boxName, const QString& alias, bool aliasDisabled,
	bool useGuidName, const QString& manualName)
{
	const QSignalBlocker BlockGuid(ui.chkGuidName);
	m_ManualName = useGuidName ? manualName : boxName;
	ui.chkGuidName->setChecked(useGuidName);
	ui.txtBoxName->setReadOnly(useGuidName);
	ui.txtBoxName->setText(boxName);
	ui.txtBoxAlias->setText(alias);
	ui.chkAliasDisabled->setChecked(aliasDisabled);
	UpdateAliasDisabledState();
}

bool CRenameSandboxDialog::ConfirmDiscard()
{
	if (!m_HasPendingDrafts
		&& GetBoxName().replace(" ", "_").compare(QString(m_SavedName).replace(" ", "_"), Qt::CaseInsensitive) == 0
		&& GetAlias() == m_SavedAlias && IsAliasDisabled() == m_SavedAliasDisabled)
		return true;
	return QMessageBox::question(this, tr("Discard unsaved renames?"),
		tr("Discard all unsaved sandbox names and aliases?\nChanges already saved with OK will be kept."),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void CRenameSandboxDialog::reject()
{
	if (ConfirmDiscard())
		QDialog::reject();
}

void CRenameSandboxDialog::closeEvent(QCloseEvent* event)
{
	if (!ConfirmDiscard()) {
		event->ignore();
		return;
	}
	QDialog::reject();
	event->accept();
}

void CRenameSandboxDialog::OnBoxNameChanged(const QString& name)
{
	ui.chkGuidName->setText(CSbiePlusAPI::IsGuidBoxName(name)
		? tr("Generate a new GUID") : tr("Use a random GUID as sandbox name"));
}

void CRenameSandboxDialog::OnGuidNameToggled(bool checked)
{
	if (checked) {
		m_ManualName = GetBoxName();
		if (!CSbiePlusAPI::IsGuidBoxName(m_ManualName)
			&& (GetAlias().isEmpty() || QMessageBox::question(this, tr("Replace alias?"),
				tr("The alias field is not empty. Replace it with the current sandbox name?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes))
			ui.txtBoxAlias->setText(m_ManualName);
		ui.txtBoxName->setText(theAPI->MkNewGuidName());
	} else
		ui.txtBoxName->setText(m_ManualName);
	ui.txtBoxName->setReadOnly(checked);
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
