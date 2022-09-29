#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"


void COptionsWindow::LoadStop()
{
	ui.treeStop->clear();
	foreach(const QString& Value, m_pBox->GetTextList("LingerProcess", m_Template))
		AddStopEntry(Value);
	foreach(const QString& Value, m_pBox->GetTextList("LingerProcessDisabled", m_Template))
		AddStopEntry(Value, true);
	LoadStopTmpl();

	ui.treeLeader->clear();
	foreach(const QString& Value, m_pBox->GetTextList("LeaderProcess", m_Template))
		AddLeaderEntry(Value);
	foreach(const QString& Value, m_pBox->GetTextList("LeaderProcessDisabled", m_Template))
		AddLeaderEntry(Value, true);
	LoadLeaderTmpl();

	m_StopChanged = false;
}

void COptionsWindow::LoadStopTmpl(bool bUpdate)
{
	if (ui.chkShowStopTmpl->isChecked())
	{
		foreach(const QString & Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("LingerProcess", Template))
				AddStopEntry(Value, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeStop->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeStop->topLevelItem(i);
			int Type = pItem->data(1, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::LoadLeaderTmpl(bool bUpdate)
{
	if (ui.chkShowLeaderTmpl->isChecked())
	{
		foreach(const QString & Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("LeaderProcess", Template))
				AddLeaderEntry(Value, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeLeader->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeLeader->topLevelItem(i);
			int Type = pItem->data(1, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddStopEntry(const QString& Name, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	pItem->setData(1, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	SetProgramItem(Name, pItem, 0, (Template.isEmpty() ? "" : " (" + Template + ")"));
	if(Template.isEmpty())
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeStop->addTopLevelItem(pItem);
}

void COptionsWindow::AddLeaderEntry(const QString& Name, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	pItem->setData(1, Qt::UserRole, Template.isEmpty() ? 0 : -1);
	SetProgramItem(Name, pItem, 0, (Template.isEmpty() ? "" : " (" + Template + ")"));
	if(Template.isEmpty())
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeLeader->addTopLevelItem(pItem);
}

void COptionsWindow::SaveStop()
{
	QStringList LingerProcess;
	QStringList LingerProcessDisabled;
	for (int i = 0; i < ui.treeStop->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeStop->topLevelItem(i);
		int Type = pItem->data(1, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		if (pItem->checkState(0) == Qt::Checked)
			LingerProcess.append(pItem->data(0, Qt::UserRole).toString());
		else
			LingerProcessDisabled.append(pItem->data(0, Qt::UserRole).toString());
	}
	WriteTextList("LingerProcess", LingerProcess);
	WriteTextList("LingerProcessDisabled", LingerProcessDisabled);

	QStringList LeaderProcess;
	QStringList LeaderProcessDisabled;
	for (int i = 0; i < ui.treeLeader->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeLeader->topLevelItem(i);
		int Type = pItem->data(1, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		if (pItem->checkState(0) == Qt::Checked)
			LeaderProcess.append(pItem->data(0, Qt::UserRole).toString());
		else
			LeaderProcessDisabled.append(pItem->data(0, Qt::UserRole).toString());
	}
	WriteTextList("LeaderProcess", LeaderProcess);
	WriteTextList("LeaderProcessDisabled", LeaderProcessDisabled);

	m_StopChanged = false;
}

void COptionsWindow::OnAddLingering()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddStopEntry(Value);
	m_StopChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddLeader()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddLeaderEntry(Value);
	m_StopChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelStopProg()
{
	DeleteAccessEntry(ui.treeStop->currentItem(), 1);
	m_StopChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelLeader()
{
	DeleteAccessEntry(ui.treeLeader->currentItem(), 1);
	m_StopChanged = true;
	OnOptChanged();
}
