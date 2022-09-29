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
		AddRecoveryEntry(Value);
	LoadRecoveryListTmpl();

	ui.chkAutoRecovery->setChecked(m_pBox->GetBool("AutoRecover", false));
	ui.treeRecIgnore->clear();
	foreach(const QString& Value, m_pBox->GetTextList("AutoRecoverIgnore", m_Template))
		AddRecIgnoreEntry(Value);
	LoadRecIgnoreListTmpl();

	m_RecoveryChanged = false;
}

void COptionsWindow::LoadRecoveryListTmpl(bool bUpdate)
{	
	if (ui.chkShowRecoveryTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("RecoverFolder", Template))
				AddRecoveryEntry(Value, Template);
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

void COptionsWindow::LoadRecIgnoreListTmpl(bool bUpdate)
{	
	if (ui.chkShowRecIgnoreTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("AutoRecoverIgnore", Template))
				AddRecIgnoreEntry(Value, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeRecIgnore->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeRecIgnore->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddRecoveryEntry(const QString& Name, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	ui.treeRecovery->addTopLevelItem(pItem);
}

void COptionsWindow::AddRecIgnoreEntry(const QString& Name, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name + (Template.isEmpty() ? "" : " (" + Template + ")"));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	ui.treeRecIgnore->addTopLevelItem(pItem);
}

void COptionsWindow::SaveRecoveryList()
{
	QStringList RecoverFolder;
	for (int i = 0; i < ui.treeRecovery->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeRecovery->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		RecoverFolder.append(pItem->data(1, Qt::UserRole).toString()); 
	}
	WriteTextList("RecoverFolder", RecoverFolder);

	WriteAdvancedCheck(ui.chkAutoRecovery, "AutoRecover", "y", "");

	QStringList AutoRecoverIgnore;
	for (int i = 0; i < ui.treeRecIgnore->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeRecIgnore->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		AutoRecoverIgnore.append(pItem->data(1, Qt::UserRole).toString()); 
	}
	WriteTextList("AutoRecoverIgnore", AutoRecoverIgnore);

	m_RecoveryChanged = false;
}

void COptionsWindow::OnAddRecFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecoveryEntry(Value);
	m_RecoveryChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddRecIgnore()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	AddRecIgnoreEntry(Value);
	m_RecoveryChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddRecIgnoreExt()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a file extension to be excluded"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	AddRecIgnoreEntry(Value);
	m_RecoveryChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelRecEntry()
{
	DeleteAccessEntry(ui.treeRecovery->currentItem());
	m_RecoveryChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelRecIgnoreEntry()
{
	DeleteAccessEntry(ui.treeRecIgnore->currentItem());
	m_RecoveryChanged = true;
	OnOptChanged();
}
