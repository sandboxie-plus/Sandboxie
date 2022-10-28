#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"


void COptionsWindow::LoadForced()
{
	ui.treeForced->clear();

	foreach(const QString& Value, m_pBox->GetTextList("ForceProcess", m_Template))
		AddForcedEntry(Value, (int)eProcess);

	foreach(const QString& Value, m_pBox->GetTextList("ForceProcessDisabled", m_Template))
		AddForcedEntry(Value, (int)eProcess, true);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolder", m_Template))
		AddForcedEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolderDisabled", m_Template))
		AddForcedEntry(Value, (int)ePath, true);


	ui.treeBreakout->clear();

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcess", m_Template))
		AddBreakoutEntry(Value, (int)eProcess);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcessDisabled", m_Template))
		AddBreakoutEntry(Value, (int)eProcess, true);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolder", m_Template))
		AddBreakoutEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolderDisabled", m_Template))
		AddBreakoutEntry(Value, (int)ePath, true);


	LoadForcedTmpl();
	LoadBreakoutTmpl();

	m_ForcedChanged = false;
}

void COptionsWindow::LoadForcedTmpl(bool bUpdate)
{
	if (ui.chkShowForceTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceProcess", Template))
				AddForcedEntry(Value, (int)eProcess, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceFolder", Template))
				AddForcedEntry(Value, (int)ePath, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeForced->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == (int)eTemplate) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::LoadBreakoutTmpl(bool bUpdate)
{
	if (ui.chkShowBreakoutTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutProcess", Template))
				AddBreakoutEntry(Value, (int)eProcess, false, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutFolder", Template))
				AddBreakoutEntry(Value, (int)ePath, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeBreakout->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeBreakout->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == (int)eTemplate) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddForcedEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	pItem->setText(0, (type == (int)eProcess ? tr("Process") : tr("Folder")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	SetProgramItem(Name, pItem, (int)eProcess);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeForced->addTopLevelItem(pItem);
}

void COptionsWindow::AddBreakoutEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	pItem->setText(0, (type == (int)eProcess ? tr("Process") : tr("Folder")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	SetProgramItem(Name, pItem, (int)eProcess);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeBreakout->addTopLevelItem(pItem);
}

void COptionsWindow::SaveForced()
{
	QStringList ForceProcess;
	QStringList ForceProcessDisabled;
	QStringList ForceFolder;
	QStringList ForceFolderDisabled;

	for (int i = 0; i < ui.treeForced->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == (int)eTemplate)
			continue; // entry from template

		if (pItem->checkState(0) == Qt::Checked) {
			switch (Type) {
			case eProcess:	ForceProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath: ForceFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	ForceProcessDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath: ForceFolderDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
	}

	WriteTextList("ForceProcess", ForceProcess);
	WriteTextList("ForceProcessDisabled", ForceProcessDisabled);
	WriteTextList("ForceFolder", ForceFolder);
	WriteTextList("ForceFolderDisabled", ForceFolderDisabled);



	QStringList BreakoutProcess;
	QStringList BreakoutProcessDisabled;
	QStringList BreakoutFolder;
	QStringList BreakoutFolderDisabled;

	for (int i = 0; i < ui.treeBreakout->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeBreakout->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == (int)eTemplate)
			continue; // entry from template

		if (pItem->checkState(0) == Qt::Checked) {
			switch (Type) {
			case eProcess:	BreakoutProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath: BreakoutFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	BreakoutProcessDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath: BreakoutFolderDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
	}

	WriteTextList("BreakoutProcess", BreakoutProcess);
	WriteTextList("BreakoutProcessDisabled", BreakoutProcessDisabled);
	WriteTextList("BreakoutFolder", BreakoutFolder);
	WriteTextList("BreakoutFolderDisabled", BreakoutFolderDisabled);


	m_ForcedChanged = false;
}

void COptionsWindow::OnForceProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, (int)eProcess);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnBreakoutProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Value, (int)eProcess);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnForceBrowse()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	AddForcedEntry(Split2(Value, "/", true).second, (int)eProcess);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnBreakoutBrowse()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Split2(Value, "/", true).second, (int)eProcess);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnForceDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, (int)ePath);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnBreakoutDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Value, (int)ePath);
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelForce()
{
	DeleteAccessEntry(ui.treeForced->currentItem());
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelBreakout()
{
	DeleteAccessEntry(ui.treeBreakout->currentItem());
	m_ForcedChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnForcedChanged(QTreeWidgetItem* pItem, int Index) 
{
	//QString Test = pItem->data(1, Qt::UserRole).toString();
	//qDebug() << Test;
	m_ForcedChanged = true; 
	OnOptChanged();
}
