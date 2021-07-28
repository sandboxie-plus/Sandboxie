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
		AddForcedEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolder", m_Template))
		AddForcedEntry(Value, 2);

	LoadForcedTmpl();

	m_ForcedChanged = false;
}

void COptionsWindow::LoadForcedTmpl(bool bUpdate)
{
	if (ui.chkShowForceTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceProcess", Template))
				AddForcedEntry(Value, 1, Template);

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceFolder", Template))
				AddForcedEntry(Value, 2, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeForced->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddForcedEntry(const QString& Name, int type, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Process") : tr("Folder")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	SetProgramItem(Name, pItem, 1);
	ui.treeForced->addTopLevelItem(pItem);
}

void COptionsWindow::SaveForced()
{
	QStringList ForceProcess;
	QStringList ForceFolder;
	for (int i = 0; i < ui.treeForced->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeForced->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	ForceProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: ForceFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	WriteTextList("ForceProcess", ForceProcess);
	WriteTextList("ForceFolder", ForceFolder);

	m_ForcedChanged = false;
}

void COptionsWindow::OnForceProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, 1);
	m_ForcedChanged = true;
}

void COptionsWindow::OnForceDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddForcedEntry(Value, 2);
	m_ForcedChanged = true;
}

void COptionsWindow::OnDelForce()
{
	DeleteAccessEntry(ui.treeForced->currentItem());
	m_ForcedChanged = true;
}
