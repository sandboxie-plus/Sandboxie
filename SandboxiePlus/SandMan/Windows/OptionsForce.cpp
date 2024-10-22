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

	foreach(const QString& Value, m_pBox->GetTextList("ForceChildren", m_Template))
		AddForcedEntry(Value, (int)eParent);

	foreach(const QString& Value, m_pBox->GetTextList("ForceChildrenDisabled", m_Template))
		AddForcedEntry(Value, (int)eParent, true);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolder", m_Template))
		AddForcedEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("ForceFolderDisabled", m_Template))
		AddForcedEntry(Value, (int)ePath, true);

	ui.chkDisableForced->setChecked(m_pBox->GetBool("DisableForceRules", false));

	ui.treeBreakout->clear();

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcess", m_Template))
		AddBreakoutEntry(Value, (int)eProcess);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutProcessDisabled", m_Template))
		AddBreakoutEntry(Value, (int)eProcess, true);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolder", m_Template))
		AddBreakoutEntry(Value, (int)ePath);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutFolderDisabled", m_Template))
		AddBreakoutEntry(Value, (int)ePath, true);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutDocument", m_Template))
		AddBreakoutEntry(Value, (int)eText);

	foreach(const QString& Value, m_pBox->GetTextList("BreakoutDocumentDisabled", m_Template))
		AddBreakoutEntry(Value, (int)eText, true);

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

			foreach(const QString& Value, m_pBox->GetTextListTmpl("ForceChildren", Template))
				AddForcedEntry(Value, (int)eParent, false, Template);

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

			foreach(const QString& Value, m_pBox->GetTextListTmpl("BreakoutDocument", Template))
				AddBreakoutEntry(Value, (int)eText, false, Template);
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
	QString Type;
	switch (type)
	{
	case eProcess: Type = tr("Process"); break;
	case ePath: Type = tr("Folder"); break;
	case eParent: Type = tr("Children"); break;
	}
	pItem->setText(0, Type + (Template.isEmpty() ? "" : (" (" + Template + ")")));
	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	SetProgramItem(Name, pItem, 1);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeForced->addTopLevelItem(pItem);
}

void COptionsWindow::AddBreakoutEntry(const QString& Name, int type, bool disabled, const QString& Template)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setCheckState(0, disabled ? Qt::Unchecked : Qt::Checked);
	QString Type;
	switch (type)
	{
	case eProcess: Type = tr("Process"); break;
	case ePath: Type = tr("Folder"); break;
	case eText: Type = tr("Document"); break;
	}
	pItem->setText(0, Type + (Template.isEmpty() ? "" : (" (" + Template + ")")));

	pItem->setData(0, Qt::UserRole, Template.isEmpty() ? type : (int)eTemplate);
	SetProgramItem(Name, pItem, 1, QString(), type == eProcess);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeBreakout->addTopLevelItem(pItem);
}

void COptionsWindow::SaveForced()
{
	QStringList ForceProcess;
	QStringList ForceProcessDisabled;
	QStringList ForceChildren;
	QStringList ForceChildrenDisabled;
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
			case eParent:	ForceChildren.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath:		ForceFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	ForceProcessDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case eParent:	ForceChildrenDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath:		ForceFolderDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
	}

	WriteTextList("ForceProcess", ForceProcess);
	WriteTextList("ForceProcessDisabled", ForceProcessDisabled);
	WriteTextList("ForceChildren", ForceChildren);
	WriteTextList("ForceChildrenDisabled", ForceChildrenDisabled);
	WriteTextList("ForceFolder", ForceFolder);
	WriteTextList("ForceFolderDisabled", ForceFolderDisabled);

	WriteAdvancedCheck(ui.chkDisableForced, "DisableForceRules", "y", "");


	QStringList BreakoutProcess;
	QStringList BreakoutProcessDisabled;
	QStringList BreakoutFolder;
	QStringList BreakoutFolderDisabled;
	QStringList BreakoutDocument;
	QStringList BreakoutDocumentDisabled;

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
			case eText: BreakoutDocument.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
		else {
			switch (Type) {
			case eProcess:	BreakoutProcessDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case ePath: BreakoutFolderDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			case eText: BreakoutDocumentDisabled.append(pItem->data(1, Qt::UserRole).toString()); break;
			}
		}
	}

	WriteTextList("BreakoutProcess", BreakoutProcess);
	WriteTextList("BreakoutProcessDisabled", BreakoutProcessDisabled);
	WriteTextList("BreakoutFolder", BreakoutFolder);
	WriteTextList("BreakoutFolderDisabled", BreakoutFolderDisabled);
	WriteTextList("BreakoutDocument", BreakoutDocument);
	WriteTextList("BreakoutDocumentDisabled", BreakoutDocumentDisabled);

	m_ForcedChanged = false;
}

void COptionsWindow::OnForceProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eProcess))
		return;
	AddForcedEntry(Value, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutProg()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Value, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnForceBrowseProg()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eProcess))
		return;
	AddForcedEntry(Split2(Value, "/", true).second, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutBrowse()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Split2(Value, "/", true).second, (int)eProcess);
	OnForcedChanged();
}

void COptionsWindow::OnForceChild()
{
	QString Value = SelectProgram();
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eParent))
		return;
	AddForcedEntry(Value, (int)eParent);
	OnForcedChanged();
}

void COptionsWindow::OnForceBrowseChild()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Executable File"), "", tr("Executable Files (*.exe)"));
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)eParent))
		return;
	AddForcedEntry(Split2(Value, "/", true).second, (int)eParent);
	OnForcedChanged();
}

void COptionsWindow::OnForceDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	if (!CheckForcedItem(Value, (int)ePath))
		return;
	AddForcedEntry(Value, (int)ePath);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutDir()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddBreakoutEntry(Value, (int)ePath);
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutDoc()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Document Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	QString Ext = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter Document File Extension."));
	if (Ext.isEmpty())
		return;
	
	if (Ext.left(1) == ".")
		Ext.prepend("*");
	else if (Ext.left(1) != "*")
		Ext.prepend("*.");

	if (Ext.right(1) == "*") {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("For security reasons it is not permitted to create entirely wildcard BreakoutDocument presets."));
		return;
	}
	QStringList BannedExt = QString(// from: https://learn.microsoft.com/en-us/troubleshoot/developer/browsers/security-privacy/information-about-the-unsafe-file-list
		"*.ade;*.adp;*.app;*.asp;*.bas;*.bat;*.cer;*.chm;*.cmd;*.cnt;*.com;*.cpl;*.crt;*.csh;*.der;*.exe;*.fxp;*.gadget;*.grp;*.hlp;*.hpj;*.hta;"
		"*.img;*.inf;*.ins;*.iso;*.isp;*.its;*.js;*.jse;*.ksh;*.lnk;*.mad;*.maf;*.mag;*.mam;*.maq;*.mar;*.mas;*.mat;*.mau;*.mav;*.maw;*.mcf;*.mda;"
		"*.mdb;*.mde;*.mdt;*.mdw;*.mdz;*.msc;*.msh;*.msh1;*.msh1xml;*.msh2;*.msh2xml;*.mshxml;*.msi;*.msp;*.mst;*.msu;*.ops;*.pcd;*.pif;*.pl;*.plg;"
		"*.prf;*.prg;*.printerexport;*.ps1;*.ps1xml;*.ps2;*.ps2xml;*.psc1;*.psc2;*.psd1;*.psm1;*.pst;*.reg;*.scf;*.scr;*.sct;*.shb;*.shs;*.theme;"
		"*.tmp;*.url;*.vb;*.vbe;*.vbp;*.vbs;*.vhd;*.vhdx;*.vsmacros;*.vsw;*.webpnp;*.ws;*.wsc;*.wsf;*.wsh;*.xnk").split(";");
	if (BannedExt.contains(Ext.toLower())) {
		QMessageBox::warning(this, "Sansboxie-Plus", tr("For security reasons the specified extension %1 should not be broken out.").arg(Ext));
		// bypass security by holding down Ctr+Alt
		if ((QGuiApplication::queryKeyboardModifiers() & (Qt::AltModifier | Qt::ControlModifier)) != (Qt::AltModifier | Qt::ControlModifier))
			return;
	}

	Value += "\\" + Ext;

	AddBreakoutEntry(Value, (int)eText);
	OnForcedChanged();
}

void COptionsWindow::OnDelForce()
{
	DeleteAccessEntry(ui.treeForced->currentItem());
	OnForcedChanged();
}

void COptionsWindow::OnDelBreakout()
{
	DeleteAccessEntry(ui.treeBreakout->currentItem());
	OnForcedChanged();
}

void COptionsWindow::OnForcedChanged()
{
	m_ForcedChanged = true; 
	OnOptChanged();
}

bool COptionsWindow::CheckForcedItem(const QString& Value, int type)
{
	bool bDangerous = false;

	QString winPath = QString::fromUtf8(qgetenv("SystemRoot"));

	if (type == eProcess || type == eParent)
	{
		if (Value.compare("explorer.exe", Qt::CaseInsensitive) == 0 || Value.compare(winPath + "\\explorer.exe", Qt::CaseInsensitive) == 0)
			bDangerous = true;
		else if (Value.compare("taskmgr.exe", Qt::CaseInsensitive) == 0 || Value.compare(winPath + "\\system32\\taskmgr.exe", Qt::CaseInsensitive) == 0)
			bDangerous = true;
		else if (Value.contains("sbiesvc.exe", Qt::CaseInsensitive))
			bDangerous = true;
		else if (Value.contains("sandman.exe", Qt::CaseInsensitive))
			bDangerous = true;
	}
	else if (type == ePath)
	{
		if (Value.compare(winPath.left(3), Qt::CaseInsensitive) == 0)
			bDangerous = true; // SystemDrive (C:\)
		else if (Value.compare(winPath, Qt::CaseInsensitive) == 0)
			bDangerous = true; // SystemRoot (C:\Windows)
		else if (Value.left(winPath.length() + 1).compare(winPath + "\\", Qt::CaseInsensitive) == 0)
			bDangerous = true; // sub path of C:\Windows
	}

	if (bDangerous && QMessageBox::warning(this, "Sandboxie-Plus", tr("Forcing the specified entry will most likely break Windows, are you sure you want to proceed?")
		, QDialogButtonBox::Yes, QDialogButtonBox::No) != QDialogButtonBox::Yes)
		return false;
	return true;
}

void COptionsWindow::OnForcedChanged(QTreeWidgetItem *pItem, int) 
{
	QString Value = pItem->data(1, Qt::UserRole).toString();
	if (pItem->checkState(0) == Qt::Checked && !CheckForcedItem(Value, pItem->data(0, Qt::UserRole).toInt()))
		pItem->setCheckState(0, Qt::Unchecked);
	//qDebug() << Test;
	OnForcedChanged();
}

void COptionsWindow::OnBreakoutChanged(QTreeWidgetItem *pItem, int) 
{
	OnForcedChanged();
}
