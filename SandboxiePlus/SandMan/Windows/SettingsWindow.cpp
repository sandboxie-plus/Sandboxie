#include "stdafx.h"
#include "SettingsWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "Helpers/WinAdmin.h"
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"
#include "../QSbieAPI/SbieUtils.h"


CSettingsWindow::CSettingsWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QWidget* centralWidget = new QWidget();
	ui.setupUi(centralWidget);
	this->setCentralWidget(centralWidget);
	this->setWindowTitle(tr("Sandboxie Plus - Settings"));

	ui.uiLang->addItem("International English", "");
	QDir langDir(QApplication::applicationDirPath() + "/translations/");
	foreach(const QString& langFile, langDir.entryList(QStringList("sandman_*.qm"), QDir::Files))
	{
		QString Code = langFile.mid(8, langFile.length() - 8 - 3);
		QLocale Locale(Code);
		QString Lang = Locale.nativeLanguageName();
		ui.uiLang->addItem(Lang, Code);
	}
	ui.uiLang->setCurrentIndex(ui.uiLang->findData(theConf->GetString("Options/Language")));

	ui.chkAutoStart->setChecked(IsAutorunEnabled());

	switch (theConf->GetInt("Options/CheckForUpdates", 2)) {
	case 0: ui.chkAutoUpdate->setCheckState(Qt::Unchecked); break;
	case 1: ui.chkAutoUpdate->setCheckState(Qt::Checked); break;
	case 2: ui.chkAutoUpdate->setCheckState(Qt::PartiallyChecked); break;
	}


	ui.chkShellMenu->setCheckState((Qt::CheckState)CSbieUtils::IsContextMenu());

	ui.chkDarkTheme->setChecked(theConf->GetBool("Options/DarkTheme", false));

	ui.chkNotifications->setChecked(theConf->GetBool("Options/ShowNotifications", true));

	switch (theConf->GetInt("Options/OpenUrlsSandboxed", 2)) {
	case 0: ui.chkSandboxUrls->setCheckState(Qt::Unchecked); break;
	case 1: ui.chkSandboxUrls->setCheckState(Qt::Checked); break;
	case 2: ui.chkSandboxUrls->setCheckState(Qt::PartiallyChecked); break;
	}

	ui.chkWatchConfig->setChecked(theConf->GetBool("Options/WatchIni", true));

	ui.onClose->addItem(tr("Close to Tray"), "ToTray");
	ui.onClose->addItem(tr("Prompt before Close"), "Prompt");
	ui.onClose->addItem(tr("Close"), "Close");
	ui.onClose->setCurrentIndex(ui.onClose->findData(theConf->GetString("Options/OnClose", "ToTray")));

	ui.chkShowTray->setChecked(theConf->GetBool("Options/ShowSysTray", true));

	connect(ui.chkShowTray, SIGNAL(stateChanged(int)), this, SLOT(OnChange()));
	//connect(ui.chkUseCycles, SIGNAL(stateChanged(int)), this, SLOT(OnChange()));

	if (theAPI->IsConnected())
	{
		QString FileRootPath_Default = "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%";
		QString KeyRootPath_Default  = "\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%";
		QString IpcRootPath_Default  = "\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%";

		ui.fileRoot->setText(theAPI->GetGlobalSettings()->GetText("FileRootPath", FileRootPath_Default));
		ui.chkSeparateUserFolders->setChecked(theAPI->GetGlobalSettings()->GetBool("SeparateUserFolders", true));
		ui.regRoot->setText(theAPI->GetGlobalSettings()->GetText("KeyRootPath", KeyRootPath_Default));
		ui.ipcRoot->setText(theAPI->GetGlobalSettings()->GetText("IpcRootPath", IpcRootPath_Default));


		ui.chkAdminOnly->setChecked(theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false));
		ui.chkPassRequired->setChecked(!theAPI->GetGlobalSettings()->GetText("EditPassword", "").isEmpty());
		connect(ui.chkPassRequired, SIGNAL(stateChanged(int)), this, SLOT(OnChange()));
		connect(ui.btnSetPassword, SIGNAL(pressed()), this, SLOT(OnSetPassword()));
		ui.chkAdminOnlyFP->setChecked(theAPI->GetGlobalSettings()->GetBool("ForceDisableAdminOnly", false));
		ui.chkClearPass->setChecked(theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false));

		ui.chkStartBlock->setChecked(theAPI->GetGlobalSettings()->GetBool("StartRunAlertDenied", false));
		connect(ui.chkStartBlock, SIGNAL(stateChanged(int)), this, SLOT(OnWarnChanged()));
		ui.chkStartBlockMsg->setChecked(theAPI->GetGlobalSettings()->GetBool("NotifyStartRunAccessDenied", true));
		connect(ui.chkStartBlockMsg, SIGNAL(stateChanged(int)), this, SLOT(OnWarnChanged()));
		connect(ui.btnAddWarnProg, SIGNAL(pressed()), this, SLOT(OnAddWarnProg()));
		connect(ui.btnAddWarnFolder, SIGNAL(pressed()), this, SLOT(OnAddWarnFolder()));
		connect(ui.btnDelWarnProg, SIGNAL(pressed()), this, SLOT(OnDelWarnProg()));

		foreach(const QString& Value, theAPI->GetGlobalSettings()->GetTextList("AlertProcess", false))
			AddWarnEntry(Value, 1);
		
		foreach(const QString& Value, theAPI->GetGlobalSettings()->GetTextList("AlertFolder", false))
			AddWarnEntry(Value, 2);
	}
	else
	{
		ui.fileRoot->setEnabled(false);
		ui.chkSeparateUserFolders->setEnabled(false);
		ui.regRoot->setEnabled(false);
		ui.ipcRoot->setEnabled(false);
		ui.chkAdminOnly->setEnabled(false);
		ui.chkPassRequired->setEnabled(false);
		ui.chkAdminOnlyFP->setEnabled(false);
		ui.chkClearPass->setEnabled(false);
		ui.btnSetPassword->setEnabled(false);
		ui.treeWarnProgs->setEnabled(false);
		ui.btnAddWarnProg->setEnabled(false);
		ui.btnDelWarnProg->setEnabled(false);
		ui.treeCompat->setEnabled(false);
		ui.btnAddCompat->setEnabled(false);
		ui.btnDelCompat->setEnabled(false);
	}
	m_WarnProgsChanged = false;

	connect(ui.btnAddCompat, SIGNAL(pressed()), this, SLOT(OnAddCompat()));
	connect(ui.btnDelCompat, SIGNAL(pressed()), this, SLOT(OnDelCompat()));

	m_CompatLoaded = 0;
	m_CompatChanged = false;

	ui.chkNoCompat->setChecked(!theConf->GetBool("Options/AutoRunSoftCompat", true));

	connect(ui.treeCompat, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));

	connect(ui.tabs, SIGNAL(currentChanged(int)), this, SLOT(OnTab()));

	connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(pressed()), this, SLOT(accept()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(pressed()), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	restoreGeometry(theConf->GetBlob("SettingsWindow/Window_Geometry"));

	OnChange();
}

CSettingsWindow::~CSettingsWindow()
{
	theConf->SetBlob("SettingsWindow/Window_Geometry",saveGeometry());
}

void CSettingsWindow::showCompat()
{
	m_CompatLoaded = 2;
	ui.tabs->setCurrentWidget(ui.tabCompat);
	show();
}

void CSettingsWindow::closeEvent(QCloseEvent *e)
{
	this->deleteLater();
}

void CSettingsWindow::apply()
{
	theConf->SetValue("Options/Language", ui.uiLang->currentData());

	theConf->SetValue("Options/DarkTheme", ui.chkDarkTheme->isChecked());

	AutorunEnable(ui.chkAutoStart->isChecked());

	switch (ui.chkAutoUpdate->checkState()) {
	case Qt::Unchecked: theConf->SetValue("Options/CheckForUpdates", 0); break;
	case Qt::PartiallyChecked: theConf->SetValue("Options/CheckForUpdates", 2); break;
	case Qt::Checked: theConf->SetValue("Options/CheckForUpdates", 1); break;
	}

	if (ui.chkShellMenu->checkState() != CSbieUtils::IsContextMenu())
	{
		if (ui.chkShellMenu->isChecked())
			CSbieUtils::AddContextMenu(QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe");
		else
			CSbieUtils::RemoveContextMenu();
	}

	theConf->SetValue("Options/ShowNotifications", ui.chkNotifications->isChecked());

	switch (ui.chkSandboxUrls->checkState()) {
	case Qt::Unchecked: theConf->SetValue("Options/OpenUrlsSandboxed", 0); break;
	case Qt::PartiallyChecked: theConf->SetValue("Options/OpenUrlsSandboxed", 2); break;
	case Qt::Checked: theConf->SetValue("Options/OpenUrlsSandboxed", 1); break;
	}

	theConf->SetValue("Options/WatchIni", ui.chkWatchConfig->isChecked());

	theConf->SetValue("Options/OnClose", ui.onClose->currentData());

	theConf->SetValue("Options/ShowSysTray", ui.chkShowTray->isChecked());


	if (theAPI->IsConnected())
	{
		if (ui.fileRoot->text().isEmpty())
			ui.fileRoot->setText("\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
		theAPI->GetGlobalSettings()->SetText("FileRootPath", ui.fileRoot->text());
		theAPI->GetGlobalSettings()->SetBool("SeparateUserFolders", ui.chkSeparateUserFolders->isChecked());

		if (ui.regRoot->text().isEmpty())
			ui.regRoot->setText("\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%");
		theAPI->GetGlobalSettings()->SetText("KeyRootPath", ui.regRoot->text());

		if (ui.ipcRoot->text().isEmpty())
			ui.ipcRoot->setText("\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%");
		theAPI->GetGlobalSettings()->SetText("IpcRootPath", ui.ipcRoot->text());


		theAPI->GetGlobalSettings()->SetBool("EditAdminOnly", ui.chkAdminOnly->isChecked());

		bool isPassSet = !theAPI->GetGlobalSettings()->GetText("EditPassword", "").isEmpty();
		if (ui.chkPassRequired->isChecked())
		{
			if (!isPassSet && m_NewPassword.isEmpty())
				OnSetPassword(); // request password entry if it wasn't entered already
			if (!m_NewPassword.isEmpty()) {
				theAPI->LockConfig(m_NewPassword); // set new/changed password
				m_NewPassword.clear();
			}
		}
		else if (isPassSet)
			theAPI->LockConfig(QString()); // clear password

		theAPI->GetGlobalSettings()->SetBool("ForceDisableAdminOnly", ui.chkAdminOnlyFP->isChecked());
		theAPI->GetGlobalSettings()->SetBool("ForgetPassword", ui.chkClearPass->isChecked());

		if (m_WarnProgsChanged)
		{
			theAPI->GetGlobalSettings()->SetBool("StartRunAlertDenied", ui.chkStartBlock->isChecked());
			theAPI->GetGlobalSettings()->SetBool("NotifyStartRunAccessDenied", ui.chkStartBlockMsg->isChecked());

			QStringList AlertProcess;
			QStringList AlertFolder;
			for (int i = 0; i < ui.treeWarnProgs->topLevelItemCount(); i++)
			{
				QTreeWidgetItem* pItem = ui.treeWarnProgs->topLevelItem(i);
				int Type = pItem->data(0, Qt::UserRole).toInt();
				switch (Type)
				{
				case 1:	AlertProcess.append(pItem->data(1, Qt::UserRole).toString()); break;
				case 2: AlertFolder.append(pItem->data(1, Qt::UserRole).toString()); break;
				}
			}

			theAPI->GetGlobalSettings()->UpdateTextList("AlertProcess", AlertProcess, false);
			theAPI->GetGlobalSettings()->UpdateTextList("AlertFolder", AlertFolder, false);
			m_WarnProgsChanged = false;
		}

		if (m_CompatChanged)
		{
			QStringList Used;
			QStringList Rejected;
			for (int i = 0; i < ui.treeCompat->topLevelItemCount(); i++) {
				QTreeWidgetItem* pItem = ui.treeCompat->topLevelItem(i);
				if (pItem->checkState(0) == Qt::Checked)
					Used.append(pItem->data(0, Qt::UserRole).toString());
				else
					Rejected.append(pItem->data(0, Qt::UserRole).toString());
			}

			theAPI->GetGlobalSettings()->UpdateTextList("Template", Used, false);
			theAPI->GetGlobalSettings()->UpdateTextList("TemplateReject", Rejected, false);
			m_CompatChanged = false;
		}
	}

	theConf->SetValue("Options/AutoRunSoftCompat", !ui.chkNoCompat->isChecked());

	emit OptionsChanged();
}

void CSettingsWindow::accept()
{
	apply();

	this->close();
}

void CSettingsWindow::reject()
{
	this->close();
}

void CSettingsWindow::OnChange()
{
	//ui.chkLinuxStyle->setEnabled(!ui.chkUseCycles->isChecked());

	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.onClose->model());
	QStandardItem *item = model->item(0);
	item->setFlags((!ui.chkShowTray->isChecked()) ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);

	ui.btnSetPassword->setEnabled(ui.chkPassRequired->isChecked());
}

void CSettingsWindow::OnTab()
{
	if (ui.tabs->currentWidget() == ui.tabCompat && m_CompatLoaded != 1 && theAPI->IsConnected())
	{
		if(m_CompatLoaded == 0)
			theGUI->GetCompat()->RunCheck();

		ui.treeCompat->clear();

		QMap<QString, int> Templates = theGUI->GetCompat()->GetTemplates();
		for (QMap<QString, int>::iterator I = Templates.begin(); I != Templates.end(); ++I)
		{
			if (I.value() == CSbieTemplates::eNone)
				continue;

			QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + I.key(), theAPI));

			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setText(0, pTemplate->GetText("Tmpl.Title"));
			pItem->setData(0, Qt::UserRole, I.key());
			pItem->setCheckState(0, (I.value() & CSbieTemplates::eEnabled) ? Qt::Checked : Qt::Unchecked);
			ui.treeCompat->addTopLevelItem(pItem);
		}

		m_CompatLoaded = 1;
		m_CompatChanged = false;
	}
}

void CSettingsWindow::OnSetPassword()
{
retry:
	QString Value1 = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the new configuration password."), QLineEdit::Password);
	if (Value1.isEmpty())
		return;

	QString Value2 = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please re-enter the new configuration password."), QLineEdit::Password);
	if (Value2.isEmpty())
		return;

	if (Value1 != Value2) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("Passwords did not match, please retry."));
		goto retry;
	}

	m_NewPassword = Value1;
}

void CSettingsWindow::AddWarnEntry(const QString& Name, int type)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, (type == 1 ? tr("Process") : tr("Folder")));
	pItem->setData(0, Qt::UserRole, type);

	pItem->setData(1, Qt::UserRole, Name);
	pItem->setText(1, Name);
	ui.treeWarnProgs->addTopLevelItem(pItem);
}

void CSettingsWindow::OnAddWarnProg()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a program file name"));
	if (Value.isEmpty())
		return;
	AddWarnEntry(Value, 1);
	m_WarnProgsChanged = true;
}

void CSettingsWindow::OnAddWarnFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddWarnEntry(Value, 2);
	m_WarnProgsChanged = true;
}

void CSettingsWindow::OnDelWarnProg()
{
	QTreeWidgetItem* pItem = ui.treeWarnProgs->currentItem();
	if (!pItem)
		return;

	delete pItem;
	m_WarnProgsChanged = true;
}

void CSettingsWindow::OnTemplateClicked(QTreeWidgetItem* pItem, int Column)
{
	// todo: check if really changed
	m_CompatChanged = true;
}

void CSettingsWindow::OnAddCompat()
{
	QTreeWidgetItem* pItem = ui.treeCompat->currentItem();
	if (!pItem)
		return;

	pItem->setCheckState(0, Qt::Checked);
	m_CompatChanged = true;
}

void CSettingsWindow::OnDelCompat()
{
	QTreeWidgetItem* pItem = ui.treeCompat->currentItem();
	if (!pItem)
		return;

	pItem->setCheckState(0, Qt::Unchecked);
	m_CompatChanged = true;
}
