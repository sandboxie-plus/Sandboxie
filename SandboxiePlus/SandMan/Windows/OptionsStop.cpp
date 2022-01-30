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
		AddStopEntry(Value, 1);

	foreach(const QString& Value, m_pBox->GetTextList("LingerProcessDisabled", m_Template))
		AddStopEntry(Value, 1, true);

	foreach(const QString& Value, m_pBox->GetTextList("LeaderProcess", m_Template))
		AddStopEntry(Value, 2);

	foreach(const QString& Value, m_pBox->GetTextList("LeaderProcessDisabled", m_Template))
		AddStopEntry(Value, 2, true);

	LoadStopTmpl();

	m_StopChanged = false;
}

void COptionsWindow::LoadStopTmpl(bool bUpdate)
{
	if (ui.chkShowStopTmpl->isChecked())
	{
		foreach(const QString & Template, m_pBox->GetTemplates())
		{
			foreach(const QString & Value, m_pBox->GetTextListTmpl("LingerProcess", Template))
				AddStopEntry(Value, 1, false, Template);

			foreach(const QString & Value, m_pBox->GetTextListTmpl("LeaderProcess", Template))
				AddStopEntry(Value, 2, false, Template);
		}
	}
	else if (bUpdate)
	{
		for (int i = 0; i < ui.treeStop->topLevelItemCount(); )
		{
			QTreeWidgetItem* pItem = ui.treeStop->topLevelItem(i);
			int Type = pItem->data(0, Qt::UserRole).toInt();
			if (Type == -1) {
				delete pItem;
				continue; // entry from template
			}
			i++;
		}
	}
}

void COptionsWindow::AddStopEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	pItem->setText(0, (type == 1 ? tr("Lingerer") : tr("Leader")) + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : -1);
	SetProgramItem(Name, pItem, 1);
	if(Template.isEmpty())
		pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeStop->addTopLevelItem(pItem);
}

void COptionsWindow::SaveStop()
{
	QStringList LingerProcess;
	QStringList LeaderProcess;
	for (int i = 0; i < ui.treeStop->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* pItem = ui.treeStop->topLevelItem(i);
		int Type = pItem->data(0, Qt::UserRole).toInt();
		if (Type == -1)
			continue; // entry from template
		switch (Type)
		{
		case 1:	LingerProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		case 2: LeaderProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
		}
	}

	WriteTextList("LingerProcess", LingerProcess);
	WriteTextList("LeaderProcess", LeaderProcess);

	m_StopChanged = false;
}

void COptionsWindow::OnAddLingering()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddStopEntry(Value, 1);
	m_StopChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnAddLeader()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddStopEntry(Value, 2);
	m_StopChanged = true;
	OnOptChanged();
}

void COptionsWindow::OnDelStopProg()
{
	DeleteAccessEntry(ui.treeStop->currentItem());
	m_StopChanged = true;
	OnOptChanged();
}
