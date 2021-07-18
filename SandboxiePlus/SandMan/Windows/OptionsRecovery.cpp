#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"


void COptionsWindow::LoadRecoveryList()
{
	ui.treeRecovery->clear();

	foreach(const QString& Value, m_pBox->GetTextList("RecoverFolder", m_Template))
		AddRecoveryEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("AutoRecoverIgnore", m_Template))
		AddRecoveryEntry(Value, 2);

	LoadRecoveryListTmpl();

	ui.chkAutoRecovery->setChecked(m_pBox->GetBool("AutoRecover", false));

	m_RecoveryChanged = false;
}

void COptionsWindow::LoadRecoveryListTmpl(bool bUpdate)
{	
	if (ui.chkShowRecoveryTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("RecoverFolder", Template))
				AddRecoveryEntry(Value, 1, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("AutoRecoverIgnore", Template))
				AddRecoveryEntry(Value, 2, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeRecovery->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeRecovery->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddRecoveryEntry(const QString& Name, int type, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Folder") : tr("Exclusion")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	pItem->setText(1, Name);
	pItem->setData(1, Qt::UserRole, Name);
	ui.treeRecovery->addTopLevelItem(pItem);
}

void COptionsWindow::SaveRecoveryList()
{
	QStringList RecoverFolder;
	QStringList AutoRecoverIgnore;
	for (int i = 0; i < ui.treeRecovery->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeRecovery->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	RecoverFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: AutoRecoverIgnore.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	WriteTextList("RecoverFolder", RecoverFolder);
	WriteTextList("AutoRecoverIgnore", AutoRecoverIgnore);

	WriteAdvancedCheck(ui.chkAutoRecovery, "AutoRecover", "y", "");

	m_RecoveryChanged = false;
}

void COptionsWindow::OnAddRecFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 1);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnAddRecIgnore()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 2);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnAddRecIgnoreExt()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a file extension to be excluded"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value, 2);
	m_RecoveryChanged = true;
}

void COptionsWindow::OnDelRecEntry()
{
	QTreeWidgetItem* pItem = ui.treeRecovery->currentItem();
	if (!pItem)
		return;

	if (pItem->data(0, Qt::UserRole).toInt() == -1) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be removed."));
		return;
	}

	delete pItem;
	m_RecoveryChanged = true;
}
