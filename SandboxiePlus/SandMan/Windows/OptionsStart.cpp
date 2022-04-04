#include "stdafx.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include "SettingsWindow.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/ComboInputDialog.h"
#include "../MiscHelpers/Common/SettingsWidgets.h"
#include "Helpers/WinAdmin.h"

void COptionsWindow::LoadStart()
{
	ui.chkStartBlockMsg->setEnabled(!ui.radStartAll->isChecked());
	ui.chkStartBlockMsg->setChecked(m_pBox->GetBool("NotifyStartRunAccessDenied", true));
	
	m_StartChanged = false;
}

void COptionsWindow::SaveStart()
{
	WriteAdvancedCheck(ui.chkStartBlockMsg, "NotifyStartRunAccessDenied", "", "n");

	m_StartChanged = false;
}

void COptionsWindow::OnRestrictStart()
{
	// only selected
	bool Enable = ui.radStartSelected->isChecked();
	if (Enable)
		SetAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");
	else
		DelAccessEntry(eIPC, "!<StartRunAccess>", eClosed, "*");

	// all except selected
	Enable = ui.radStartExcept->isChecked();
	if (Enable)
		SetAccessEntry(eIPC, "<StartRunAccess>", eClosed, "*");
	else
		DelAccessEntry(eIPC, "<StartRunAccess>", eClosed, "*");

	ui.chkStartBlockMsg->setEnabled(!ui.radStartAll->isChecked());
	//m_StartChanged = true;
	//OnOptChanged();
}

void COptionsWindow::OnAddStartProg()
{
	AddProgToGroup(ui.treeStart, "<StartRunAccess>");
	//m_StartChanged = true;
	//OnOptChanged();
}

void COptionsWindow::OnDelStartProg()
{
	DelProgFromGroup(ui.treeStart, "<StartRunAccess>");
	DelProgFromGroup(ui.treeStart, "<StartRunAccessDisabled>");
	//m_StartChanged = true;
	//OnOptChanged();
}

void COptionsWindow::OnStartChanged(QTreeWidgetItem* pItem, int Index) 
{ 
	if (pItem->checkState(0) == Qt::Checked) {
		if(DelProgramFromGroup(pItem->data(0, Qt::UserRole).toString(), "<StartRunAccessDisabled>"))
			AddProgramToGroup(pItem->data(0, Qt::UserRole).toString(), "<StartRunAccess>");
	}
	else {
		if(DelProgramFromGroup(pItem->data(0, Qt::UserRole).toString(), "<StartRunAccess>"))
			AddProgramToGroup(pItem->data(0, Qt::UserRole).toString(), "<StartRunAccessDisabled>");
	}
	//m_StartChanged = true;  
	//OnOptChanged(); 
}