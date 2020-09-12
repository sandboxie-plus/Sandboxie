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
	foreach(const QString& langFile, langDir.entryList(QStringList("taskexplorer_*.qm"), QDir::Files))
	{
		QString Code = langFile.mid(13, langFile.length() - 13 - 3);
		QLocale Locale(Code);
		QString Lang = Locale.nativeLanguageName();
		ui.uiLang->addItem(Lang, Code);
	}
	ui.uiLang->setCurrentIndex(ui.uiLang->findData(theConf->GetString("Options/Language")));

	ui.chkAutoStart->setChecked(IsAutorunEnabled());

	ui.chkShellMenu->setCheckState((Qt::CheckState)CSbieUtils::IsContextMenu());

	ui.chkDarkTheme->setChecked(theConf->GetBool("Options/DarkTheme", false));

	ui.chkNotifications->setChecked(theConf->GetBool("Options/ShowNotifications", true));

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
		ui.fileRoot->setText(theAPI->GetGlobalSettings()->GetText("FileRootPath"));
		ui.chkSeparateUserFolders->setChecked(theAPI->GetGlobalSettings()->GetBool("SeparateUserFolders", true));
		ui.regRoot->setText(theAPI->GetGlobalSettings()->GetText("KeyRootPath"));
		ui.ipcRoot->setText(theAPI->GetGlobalSettings()->GetText("IpcRootPath"));

		ui.chkAdminOnly->setChecked(theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false));
		ui.chkPassRequired->setChecked(!theAPI->GetGlobalSettings()->GetText("EditPassword", "").isEmpty());
		connect(ui.chkPassRequired, SIGNAL(stateChanged(int)), this, SLOT(OnChange()));
		connect(ui.btnSetPassword, SIGNAL(pressed()), this, SLOT(OnSetPassword()));
		ui.chkAdminOnlyFP->setChecked(theAPI->GetGlobalSettings()->GetBool("ForceDisableAdminOnly", false));
		ui.chkClearPass->setChecked(theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false));

		connect(ui.btnAddWarnProg, SIGNAL(pressed()), this, SLOT(OnAddWarnProg()));
		connect(ui.btnDelWarnProg, SIGNAL(pressed()), this, SLOT(OnDelWarnProg()));

		QStringList WarnProgs = theAPI->GetGlobalSettings()->GetTextList("AlertProcess");
		foreach(const QString& Value, WarnProgs) {
			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setText(0, Value);
			ui.treeWarnProgs->addTopLevelItem(pItem);
		}
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

	if (ui.chkShellMenu->checkState() != CSbieUtils::IsContextMenu())
	{
		if (ui.chkShellMenu->isChecked())
			CSbieUtils::AddContextMenu(QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe");
		else
			CSbieUtils::RemoveContextMenu();
	}

	theConf->SetValue("Options/ShowNotifications", ui.chkNotifications->isChecked());

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
				OnSetPassword(); // request password entry if it wasn't already
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
			QStringList WarnProgs;
			for (int i = 0; i < ui.treeWarnProgs->topLevelItemCount(); i++) {
				QTreeWidgetItem* pItem = ui.treeWarnProgs->topLevelItem(i);
				WarnProgs.append(pItem->text(0));
			}

			theAPI->GetGlobalSettings()->UpdateTextList("AlertProcess", WarnProgs);
			m_WarnProgsChanged = false;
		}

		if (m_CompatChanged)
		{
			QStringList Used;
			QStringList Rejected;
			for (int i = 0; i < ui.treeCompat->topLevelItemCount(); i++) {
				QTreeWidgetItem* pItem = ui.treeCompat->topLevelItem(i);
				if (pItem->checkState(0) == Qt::Checked)
					Used.append(pItem->text(0));
				else
					Rejected.append(pItem->text(0));
			}

			theAPI->GetGlobalSettings()->UpdateTextList("Template", Used);
			theAPI->GetGlobalSettings()->UpdateTextList("TemplateReject", Rejected);
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

			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setText(0, I.key());
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

	QString Value2 = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please re enter the new configuration password."), QLineEdit::Password);
	if (Value2.isEmpty())
		return;

	if (Value1 != Value2) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("Passwords did not match, please retry."));
		goto retry;
	}

	m_NewPassword = Value1;
}

void CSettingsWindow::OnAddWarnProg()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please a program file name"));
	if (Value.isEmpty())
		return;

	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Value);
	ui.treeWarnProgs->addTopLevelItem(pItem);

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