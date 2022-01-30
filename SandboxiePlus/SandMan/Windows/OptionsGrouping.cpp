#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"


void COptionsWindow::LoadGroups()
{
	ui.treeGroups->clear();

	QStringList ProcessGroups = m_pBox->GetTextList("ProcessGroup", m_Template);
	foreach(const QString& Group, ProcessGroups)
	{
		QStringList Entries = Group.split(",");
		QString GroupName = Entries.takeFirst();

		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setData(0, Qt::UserRole, GroupName);
		if (GroupName.length() > 2)
			GroupName = GroupName.mid(1, GroupName.length() - 2);
		pItem->setText(0, GroupName);
		for (int i = 0; i < Entries.count(); i++) 
		{
			if (Entries[i].isEmpty())
				continue;
			QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
			SetProgramItem(Entries[i], pSubItem, 0);
			pSubItem->setFlags(pSubItem->flags() | Qt::ItemIsEditable);
			pItem->addChild(pSubItem);
		}
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
		ui.treeGroups->addTopLevelItem(pItem);
	}
	
	LoadGroupsTmpl();

	ui.treeGroups->expandAll();

	m_GroupsChanged = false;
}

void COptionsWindow::LoadGroupsTmpl(bool bUpdate)
{
	if (ui.chkShowGroupTmpl->isChecked())
	{
		foreach(const QString& Template, m_pBox->GetTemplates())
		{
			foreach(const QString& Group, m_pBox->GetTextListTmpl("ProcessGroup", Template))
			{
				QStringList Entries = Group.split(",");
				QString GroupName = Entries.takeFirst();
				

				QTreeWidgetItem* pItem = new QTreeWidgetItem();
				if (GroupName.length() > 2)
					GroupName = GroupName.mid(1, GroupName.length() - 2);
				pItem->setText(0, GroupName + " (" + Template + ")");
				for (int i = 0; i < Entries.count(); i++) 
				{
					if (Entries[i].isEmpty())
						continue;
					QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
					SetProgramItem(Entries[i], pSubItem, 0);
					pItem->addChild(pSubItem);
				}
				ui.treeGroups->addTopLevelItem(pItem);
			}
		}

		if (bUpdate)
			ui.treeGroups->expandAll();
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeGroups->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
			QString GroupName = pItem->data(0, Qt::UserRole).toString();
			if (GroupName.isEmpty()) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

QStringList COptionsWindow::GetCurrentGroups()
{
	QStringList Groups;

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		QString GroupName = pItem->data(0, Qt::UserRole).toString();
		Groups.append(GroupName);
	}

	foreach(const QString& Template, m_pBox->GetTemplates())
	{
		foreach(const QString& Group, m_pBox->GetTextListTmpl("ProcessGroup", Template))
		{
			QStringList Entries = Group.split(",");
			QString GroupName = Entries.takeFirst();
			if (!Groups.contains(GroupName))
				Groups.append(GroupName);
		}
	}

	return Groups;
}

void COptionsWindow::SaveGroups()
{
	QStringList ProcessGroups;
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		QString GroupName = pItem->data(0, Qt::UserRole).toString();
		if (GroupName.isEmpty())
			continue; // this is a template entry, dont save
		QStringList Programs;
		for (int j = 0; j < pItem->childCount(); j++)
			Programs.append(pItem->child(j)->data(0, Qt::UserRole).toString());
		QString Group = GroupName + "," + Programs.join(",");
		ProcessGroups.append(Group);
	}

	WriteTextList("ProcessGroup", ProcessGroups);

	m_GroupsChanged = false;
}

void COptionsWindow::OnAddGroup()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a name for the new group"), QLineEdit::Normal, "NewGroup");
	if (Value.isEmpty())
		return;
	
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		if (pItem->text(0).compare(Value, Qt::CaseInsensitive) == 0)
			return;
	}

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Value);
	pItem->setData(0, Qt::UserRole, "<" + Value + ">");
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeGroups->addTopLevelItem(pItem);

	m_GroupsChanged = true;
	OnOptChanged();
}

void COptionsWindow::AddProgToGroup(QTreeWidget* pTree, const QString& Groupe, bool disabled)
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	SetProgramItem(Value, pItem, 0);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	pTree->addTopLevelItem(pItem);

	AddProgramToGroup(Value, Groupe);
}

void COptionsWindow::DelProgFromGroup(QTreeWidget* pTree, const QString& Groupe)
{
	QTreeWidgetItem* pItem = pTree->currentItem();
	if (!pItem)
		return;

	QString Value = pItem->data(0, Qt::UserRole).toString();

	delete pItem;

	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pGroupItem = ui.treeGroups->topLevelItem(i);
		if (pGroupItem->data(0, Qt::UserRole).toString().compare(Groupe, Qt::CaseInsensitive) == 0)
		{
			for (int j = 0; j < pGroupItem->childCount(); j++)
			{
				QTreeWidgetItem* pProgItem = pGroupItem->child(j);
				if (pProgItem->data(0, Qt::UserRole).toString().compare(Value, Qt::CaseInsensitive) == 0)
				{
					delete pProgItem;
					m_GroupsChanged = true;
					OnOptChanged();
					break;
				}
			}
			break;
		}
	}
}


void COptionsWindow::AddProgramToGroup(const QString& Program, const QString& Group)
{
	QTreeWidgetItem* pItem = FindGroupByName(Group, true);

	QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
	SetProgramItem(Program, pSubItem, 0);
	pSubItem->setFlags(pSubItem->flags() | Qt::ItemIsEditable);
	pItem->addChild(pSubItem);

	m_GroupsChanged = true;
	OnOptChanged();
}

bool COptionsWindow::DelProgramFromGroup(const QString& Program, const QString& Group)
{
	QTreeWidgetItem* pItem = FindGroupByName(Group, true);

	bool bFound = false;
	for (int j = 0; j < pItem->childCount(); j++){
		QTreeWidgetItem* pProgItem = pItem->child(j);
		if (pProgItem->data(0, Qt::UserRole).toString().compare(Program, Qt::CaseInsensitive) == 0)  {
			delete pProgItem;
			bFound = true;
			break;
		}
	}

	m_GroupsChanged = true;
	OnOptChanged();

	return bFound;
}

QTreeWidgetItem* COptionsWindow::FindGroupByName(const QString& Group, bool bAdd)
{
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) {
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toString().compare(Group, Qt::CaseInsensitive) == 0)
			return pItem;
	}

	if (bAdd) 
	{
		QString GroupName = Group;
		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setData(0, Qt::UserRole, GroupName);
		if (GroupName.length() > 2)
			GroupName = GroupName.mid(1, GroupName.length() - 2);
		pItem->setText(0, GroupName);
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
		ui.treeGroups->addTopLevelItem(pItem);
		return pItem;
	}

	return NULL;
}

void COptionsWindow::OnAddProg()
{
	QTreeWidgetItem* pItem = ui.treeGroups->currentItem();
	while (pItem && pItem->parent())
		pItem = pItem->parent();

	if (!pItem)
	{
		QMessageBox::warning(this, "SandboxiePlus", tr("Please select group first."));
		return;
	}

	if (pItem->data(0, Qt::UserRole).toString().isEmpty()) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;

	QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
	SetProgramItem(Value, pSubItem, 0);
	pSubItem->setFlags(pSubItem->flags() | Qt::ItemIsEditable);
	pItem->addChild(pSubItem);

	m_GroupsChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelProg()
{
	QTreeWidgetItem* pItem = ui.treeGroups->currentItem();
	if (!pItem)
		return;

	if ((pItem->parent() ? pItem->parent() : pItem)->data(0, Qt::UserRole).toString().isEmpty()) {
		QMessageBox::warning(this, "SandboxiePlus", tr("Template values can not be edited."));
		return;
	}

	delete pItem;

	m_GroupsChanged = true;
	OnOptChanged();
}

void COptionsWindow::CopyGroupToList(const QString& Groupe, QTreeWidget* pTree, bool disabled)
{
	for (int i = 0; i < ui.treeGroups->topLevelItemCount(); i++) 
	{
		QTreeWidgetItem* pItem = ui.treeGroups->topLevelItem(i);
		if (pItem->data(0, Qt::UserRole).toString().compare(Groupe, Qt::CaseInsensitive) == 0)
		{
			for (int j = 0; j < pItem->childCount(); j++)
			{
				QString Value = pItem->child(j)->data(0, Qt::UserRole).toString();

				QTreeWidgetItem* pSubItem = new QTreeWidgetItem();
				pSubItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
				SetProgramItem(Value, pSubItem, 0);
				pSubItem->setFlags(pSubItem->flags() | Qt::ItemIsEditable);
				pTree->addTopLevelItem(pSubItem);
			}
			break;
		}
	}
}
