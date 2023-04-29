#include "stdafx.h"
#include "SettingsWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "Helpers/WinAdmin.h"
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"
#include "../QSbieAPI/SbieUtils.h"
#include "OptionsWindow.h"
#include "../OnlineUpdater.h"
#include "../MiscHelpers/Archive/ArchiveFS.h"
#include <QJsonDocument>
#include "../Wizards/TemplateWizard.h"


#include <windows.h>
#include <shellapi.h>

QSize CustomTabStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const {
	QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
	if (type == QStyle::CT_TabBarTab && widget->property("isSidebar").toBool()) {
		s.transpose();
		if(theGUI->m_FusionTheme)
			s.setHeight(s.height() * 13 / 10);
		else
			s.setHeight(s.height() * 15 / 10);
		s.setWidth(s.width() * 11 / 10); // for the the icon
	}
	return s;
}

void CustomTabStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
	if (element == CE_TabBarTabLabel && widget->property("isSidebar").toBool()) {
		if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
			QStyleOptionTab opt(*tab);
			opt.shape = QTabBar::RoundedNorth;
			//opt.iconSize = QSize(32, 32);
			opt.iconSize = QSize(24, 24);
			QProxyStyle::drawControl(element, &opt, painter, widget);
			return;
		}
	}
	QProxyStyle::drawControl(element, option, painter, widget);
}


void FixTriStateBoxPallete(QWidget* pWidget)
{
	if (QApplication::style()->objectName() == "windows") {

		//
		// the built in "windows" theme of Qt does not properly rendered PartiallyChecked
		// checkboxes, to remedi this issue we connect to the stateChanged slot
		// and change the pattern to improve the rendering.
		//

		foreach(QCheckBox* pCheck, pWidget->findChildren<QCheckBox*>()) {
			QObject::connect(pCheck, &QCheckBox::stateChanged, [pCheck](int state) {
				if (pCheck->isTristate()) {
					QPalette palette = QApplication::palette();
					if (state == Qt::PartiallyChecked)
						palette.setColor(QPalette::Base, Qt::darkGray);
					pCheck->setPalette(palette);
				}
			});
		}
	}
}


void AddIconToLabel(QLabel* pLabel, const QPixmap& Pixmap)
{
	QWidget* pParent = pLabel->parentWidget();
	QWidget* pWidget = new QWidget(pParent);
	pParent->layout()->replaceWidget(pLabel, pWidget);
	QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	pLayout->setSpacing(1);
	QLabel* pIcon = new QLabel();
	pIcon = new QLabel();
	pIcon->setPixmap(Pixmap);
	pLayout->addWidget(pIcon);
	pLayout->addWidget(pLabel);
}


int CSettingsWindow__Chk2Int(Qt::CheckState state)
{
	switch (state) {
	case Qt::Unchecked: return 0;
	case Qt::Checked: return 1;
	default:
	case Qt::PartiallyChecked: return 2;
	}
}

Qt::CheckState CSettingsWindow__Int2Chk(int state)
{
	switch (state) {
	case 0: return Qt::Unchecked;
	case 1: return Qt::Checked;
	default:
	case 2: return Qt::PartiallyChecked;
	}
}

quint32 g_FeatureFlags = 0;

QByteArray g_Certificate;
SCertInfo g_CertInfo = { 0 };

CSettingsWindow::CSettingsWindow(QWidget* parent)
	: CConfigDialog(parent)
{
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	//flags &= ~Qt::WindowMinMaxButtonsHint;
	flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	bool bAlwaysOnTop = theConf->GetBool("Options/AlwaysOnTop", false);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	ui.setupUi(this);
	this->setWindowTitle(tr("Sandboxie Plus - Global Settings"));

	if (theConf->GetBool("Options/AltRowColors", false)) {
		foreach(QTreeWidget* pTree, this->findChildren<QTreeWidget*>()) 
			pTree->setAlternatingRowColors(true);
	}

	FixTriStateBoxPallete(this);

	ui.tabs->setTabPosition(QTabWidget::West);
	ui.tabs->tabBar()->setStyle(new CustomTabStyle(ui.tabs->tabBar()->style()));
	ui.tabs->tabBar()->setProperty("isSidebar", true);

	ui.tabs->setCurrentIndex(0);
	ui.tabs->setTabIcon(0, CSandMan::GetIcon("Config"));
	ui.tabs->setTabIcon(1, CSandMan::GetIcon("Shell"));
	ui.tabs->setTabIcon(2, CSandMan::GetIcon("Design"));
	ui.tabs->setTabIcon(3, CSandMan::GetIcon("Support"));
	ui.tabs->setTabIcon(4, CSandMan::GetIcon("Advanced"));
	ui.tabs->setTabIcon(5, CSandMan::GetIcon("Control"));
	ui.tabs->setTabIcon(6, CSandMan::GetIcon("Compatibility"));
	ui.tabs->setTabIcon(7, CSandMan::GetIcon("Editor"));

	ui.tabsGeneral->setCurrentIndex(0);
	ui.tabsGeneral->setTabIcon(0, CSandMan::GetIcon("Presets"));
	ui.tabsGeneral->setTabIcon(1, CSandMan::GetIcon("Notification"));

	ui.tabsShell->setCurrentIndex(0);
	ui.tabsShell->setTabIcon(0, CSandMan::GetIcon("Windows"));
	ui.tabsShell->setTabIcon(1, CSandMan::GetIcon("Run"));

	ui.tabsGUI->setCurrentIndex(0);
	ui.tabsGUI->setTabIcon(0, CSandMan::GetIcon("Interface"));
	ui.tabsGUI->setTabIcon(1, CSandMan::GetIcon("Monitor"));

	ui.tabsAdvanced->setCurrentIndex(0);
	ui.tabsAdvanced->setTabIcon(0, CSandMan::GetIcon("Options"));
	ui.tabsAdvanced->setTabIcon(1, CSandMan::GetIcon("EditIni"));

	ui.tabsControl->setCurrentIndex(0);
	ui.tabsControl->setTabIcon(0, CSandMan::GetIcon("Alarm"));
	//ui.tabsControl->setTabIcon(1, CSandMan::GetIcon("USB"));

	ui.tabsTemplates->setCurrentIndex(0);
	ui.tabsTemplates->setTabIcon(0, CSandMan::GetIcon("Program"));
	ui.tabsTemplates->setTabIcon(1, CSandMan::GetIcon("Template"));

	int size = 16.0;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	size *= (QApplication::desktop()->logicalDpiX() / 96.0); // todo Qt6
#endif
	AddIconToLabel(ui.lblGeneral, CSandMan::GetIcon("Options").pixmap(size,size));
	AddIconToLabel(ui.lblRecovery, CSandMan::GetIcon("Recover").pixmap(size,size));
	AddIconToLabel(ui.lblNotify, CSandMan::GetIcon("Notification").pixmap(size,size));
	AddIconToLabel(ui.lblMessages, CSandMan::GetIcon("Email").pixmap(size,size));
	//AddIconToLabel(ui.lblMessages, CSandMan::GetIcon("Filter").pixmap(size,size));

	AddIconToLabel(ui.lblStartUp, CSandMan::GetIcon("Start").pixmap(size,size));
	AddIconToLabel(ui.lblRunBoxed, CSandMan::GetIcon("Run").pixmap(size,size));
	AddIconToLabel(ui.lblStartMenu, CSandMan::GetIcon("StartMenu").pixmap(size,size));
	AddIconToLabel(ui.lblSysTray, CSandMan::GetIcon("Maintenance").pixmap(size,size));

	AddIconToLabel(ui.lblInterface, CSandMan::GetIcon("GUI").pixmap(size,size));

	AddIconToLabel(ui.lblDisplay, CSandMan::GetIcon("Advanced").pixmap(size,size));

	AddIconToLabel(ui.lblBoxRoot, CSandMan::GetIcon("Sandbox").pixmap(size,size));
	AddIconToLabel(ui.lblBoxFeatures, CSandMan::GetIcon("Miscellaneous").pixmap(size,size));

	AddIconToLabel(ui.lblProtection, CSandMan::GetIcon("Lock").pixmap(size,size));

	AddIconToLabel(ui.lblUpdates, CSandMan::GetIcon("Update").pixmap(size,size));

	int iViewMode = theConf->GetInt("Options/ViewMode", 1);

	/*if (iViewMode == 0)
	{
		if (iOptionLayout == 1)
			ui.tabs->removeTab(6); // ini edit
		else 
			ui.tabs->removeTab(7); // ini edit
	}*/

	ui.tabs->setCurrentIndex(0);

	{
		ui.uiLang->addItem(tr("Auto Detection"), "");
		ui.uiLang->addItem(tr("No Translation"), "native");

		C7zFileEngineHandler LangFS(QApplication::applicationDirPath() + "/translations.7z", "lang", this);

		QString langDir;
		if (LangFS.IsOpen())
			langDir = LangFS.Prefix() + "/";
		else
			langDir = QApplication::applicationDirPath() + "/translations/";

		foreach(const QString & langFile, QDir(langDir).entryList(QStringList("sandman_*.qm"), QDir::Files))
		{
			QString Code = langFile.mid(8, langFile.length() - 8 - 3);
			QLocale Locale(Code);
			QString Lang = Locale.nativeLanguageName();
			ui.uiLang->addItem(Lang, Code);
		}
		ui.uiLang->setCurrentIndex(ui.uiLang->findData(theConf->GetString("Options/UiLanguage")));
	}

	ui.cmbIntegrateMenu->addItem(tr("Don't integrate links"));
	ui.cmbIntegrateMenu->addItem(tr("As sub group"));
	ui.cmbIntegrateMenu->addItem(tr("Fully integrate"));

	ui.cmbSysTray->addItem(tr("Don't show any icon"));
	ui.cmbSysTray->addItem(tr("Show Plus icon"));
	ui.cmbSysTray->addItem(tr("Show Classic icon"));

	ui.cmbTrayBoxes->addItem(tr("All Boxes"));
	ui.cmbTrayBoxes->addItem(tr("Active + Pinned"));
	ui.cmbTrayBoxes->addItem(tr("Pinned Only"));

	ui.cmbDPI->addItem(tr("None"), 0);
	ui.cmbDPI->addItem(tr("Native"), 1);
	ui.cmbDPI->addItem(tr("Qt"), 2);

	ui.cmbUpdate->addItem(tr("Ignore"), "ignore");
	ui.cmbUpdate->addItem(tr("Notify"), "notify");
	ui.cmbUpdate->addItem(tr("Download & Notify"), "download");
	ui.cmbUpdate->addItem(tr("Download & Install"), "install");
	
	//ui.cmbRelease->addItem(tr("Ignore"), "ignore");
	ui.cmbRelease->addItem(tr("Notify"), "notify");
	ui.cmbRelease->addItem(tr("Download & Notify"), "download");
	ui.cmbRelease->addItem(tr("Download & Install"), "install");

	int FontScales[] = { 75,100,125,150,175,200,225,250,275,300,350,400, 0 };
	for (int* pFontScales = FontScales; *pFontScales != 0; pFontScales++)
		ui.cmbFontScale->addItem(tr("%1").arg(*pFontScales), *pFontScales);

	QSettings CurrentVersion("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", QSettings::NativeFormat);
	if (CurrentVersion.value("CurrentBuild").toInt() >= 22000) { // Windows 11
		QCheckBox* SecretCheckBox = new CSecretCheckBox(ui.chkShellMenu->text());
		((QGridLayout*)((QWidget*)ui.chkShellMenu->parent())->layout())->replaceWidget(ui.chkShellMenu, SecretCheckBox);
		ui.chkShellMenu->deleteLater();
		ui.chkShellMenu = SecretCheckBox;
	}

	m_HoldChange = false;

	LoadSettings();


	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

	connect(theGUI, SIGNAL(DrivesChanged()), this, SLOT(UpdateDrives()));

	// General Config
	connect(ui.uiLang, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeGUI()));

	connect(ui.chkSandboxUrls, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkMonitorSize, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkPanic, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.keyPanic, SIGNAL(keySequenceChanged(const QKeySequence &)), this, SLOT(OnOptChanged()));
	connect(ui.chkAsyncBoxOps, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));

	connect(ui.chkSilentMode, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkCopyProgress, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkNoMessages, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));

	connect(ui.btnAddMessage, SIGNAL(clicked(bool)), this, SLOT(OnAddMessage()));
	connect(ui.btnDelMessage, SIGNAL(clicked(bool)), this, SLOT(OnDelMessage()));
	connect(ui.treeMessages, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnMessageChanged()));
	m_MessagesChanged = false;

	connect(ui.chkNotifyRecovery, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkShowRecovery, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkRecoveryTop, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	//

	// Shell Integration
	connect(ui.chkAutoStart, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkSvcStart, SIGNAL(stateChanged(int)), this, SLOT(OnGeneralChanged()));
	
	connect(ui.chkShellMenu, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkAlwaysDefault, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkShellMenu2, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	
	connect(ui.chkScanMenu, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.cmbIntegrateMenu, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeGUI()));
	
	connect(ui.cmbSysTray, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.cmbTrayBoxes, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkCompactTray, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkBoxOpsNotify, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	//

	// Interface Config
	connect(ui.cmbDPI, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkDarkTheme, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkFusionTheme, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkAltRows, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkBackground, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkLargeIcons, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkNoIcons, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkOptTree, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkNewLayout, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.chkColorIcons, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.chkOverlayIcons, SIGNAL(stateChanged(int)), this, SLOT(OnChangeGUI()));

	connect(ui.cmbFontScale, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeGUI()));
	connect(ui.cmbFontScale, SIGNAL(currentTextChanged(const QString&)), this, SLOT(OnChangeGUI()));
	m_bRebuildUI = false;
	//

	// Run tab
	connect(ui.btnAddCmd, SIGNAL(clicked(bool)), this, SLOT(OnAddCommand()));
	QMenu* pRunBtnMenu = new QMenu(ui.btnAddCmd);
	pRunBtnMenu->addAction(tr("Browse for Program"), this, SLOT(OnBrowsePath()));
	ui.btnAddCmd->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddCmd->setMenu(pRunBtnMenu);
	connect(ui.btnCmdUp, SIGNAL(clicked(bool)), this, SLOT(OnCommandUp()));
	connect(ui.btnCmdDown, SIGNAL(clicked(bool)), this, SLOT(OnCommandDown()));
	connect(ui.btnDelCmd, SIGNAL(clicked(bool)), this, SLOT(OnDelCommand()));
	connect(ui.treeRun, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(OnRunChanged()));
	m_RunChanged = false;
	//

	// Advanced Config
	connect(ui.cmbDefault, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkAutoRoot, SIGNAL(stateChanged(int)), this, SLOT(OnRootChanged())); // not sbie ini
	connect(ui.fileRoot, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.regRoot, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));
	connect(ui.ipcRoot, SIGNAL(textChanged(const QString&)), this, SLOT(OnGeneralChanged()));

	connect(ui.chkWFP, SIGNAL(stateChanged(int)), this, SLOT(OnFeaturesChanged()));
	connect(ui.chkObjCb, SIGNAL(stateChanged(int)), this, SLOT(OnFeaturesChanged()));
	if (CurrentVersion.value("CurrentBuild").toInt() < 14393) // Windows 10 RS1 and later
		ui.chkWin32k->setEnabled(false);
	//connect(ui.chkWin32k, SIGNAL(stateChanged(int)), this, SLOT(OnFeaturesChanged()));
	m_FeaturesChanged = false;
	connect(ui.chkWin32k, SIGNAL(stateChanged(int)), this, SLOT(OnGeneralChanged()));
	connect(ui.chkSbieLogon, SIGNAL(stateChanged(int)), this, SLOT(OnGeneralChanged()));
	m_GeneralChanged = false;

	connect(ui.chkWatchConfig, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged())); // not sbie ini

	connect(ui.chkAdminOnly, SIGNAL(stateChanged(int)), this, SLOT(OnProtectionChange()));
	connect(ui.chkPassRequired, SIGNAL(stateChanged(int)), this, SLOT(OnProtectionChange()));
	connect(ui.btnSetPassword, SIGNAL(clicked(bool)), this, SLOT(OnSetPassword()));
	connect(ui.chkAdminOnlyFP, SIGNAL(stateChanged(int)), this, SLOT(OnProtectionChange()));
	connect(ui.chkClearPass, SIGNAL(stateChanged(int)), this, SLOT(OnProtectionChange()));
	m_ProtectionChanged = false;
	//
	
	// Program Control
	connect(ui.chkStartBlock, SIGNAL(stateChanged(int)), this, SLOT(OnWarnChanged()));

	connect(ui.chkStartBlockMsg, SIGNAL(stateChanged(int)), this, SLOT(OnWarnChanged()));
	connect(ui.chkNotForcedMsg, SIGNAL(stateChanged(int)), this, SLOT(OnWarnChanged()));
	connect(ui.btnAddWarnProg, SIGNAL(clicked(bool)), this, SLOT(OnAddWarnProg()));
	connect(ui.btnAddWarnFolder, SIGNAL(clicked(bool)), this, SLOT(OnAddWarnFolder()));
	connect(ui.btnDelWarnProg, SIGNAL(clicked(bool)), this, SLOT(OnDelWarnProg()));

	connect(ui.btnBrowse, SIGNAL(clicked(bool)), this, SLOT(OnBrowse()));
	m_WarnProgsChanged = false;
	//

	// Templates
	connect(ui.btnAddCompat, SIGNAL(clicked(bool)), this, SLOT(OnAddCompat()));
	connect(ui.btnDelCompat, SIGNAL(clicked(bool)), this, SLOT(OnDelCompat()));
	m_CompatLoaded = 0;
	m_CompatChanged = false;
	ui.chkNoCompat->setChecked(!theConf->GetBool("Options/AutoRunSoftCompat", true));

	connect(ui.treeCompat, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeCompat, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateDoubleClicked(QTreeWidgetItem*, int)));

	connect(ui.txtTemplates, SIGNAL(textChanged(const QString&)), this, SLOT(OnFilterTemplates()));
	//connect(ui.treeTemplates, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnTemplateClicked(QTreeWidgetItem*, int)));
	connect(ui.treeTemplates, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnTemplateDoubleClicked(QTreeWidgetItem*, int)));
	connect(ui.btnAddTemplate, SIGNAL(clicked(bool)), this, SLOT(OnAddTemplates()));
	QMenu* pTmplBtnMenu = new QMenu(ui.btnAddTemplate);
	for(int i = 1; i < CTemplateWizard::TmplMax; i++)
		pTmplBtnMenu->addAction(tr("Add %1 Template").arg(CTemplateWizard::GetTemplateLabel((CTemplateWizard::ETemplateType)i)), this, SLOT(OnTemplateWizard()))->setData(i);
	ui.btnAddTemplate->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnAddTemplate->setMenu(pTmplBtnMenu);
	connect(ui.btnDelTemplate, SIGNAL(clicked(bool)), this, SLOT(OnDelTemplates()));
	//

	// Support
	connect(ui.lblSupport, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
	connect(ui.lblSupportCert, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
	connect(ui.lblCertExp, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));
	//connect(ui.lblInsiderInfo, SIGNAL(linkActivated(const QString&)), theGUI, SLOT(OpenUrl(const QString&)));

	m_CertChanged = false;
	connect(ui.txtCertificate, SIGNAL(textChanged()), this, SLOT(CertChanged()));
	connect(theGUI, SIGNAL(CertUpdated()), this, SLOT(UpdateCert()));

	ui.txtCertificate->setPlaceholderText(
		"NAME: User Name\n"
		"LEVEL: ULTIMATE\n"
		"DATE: dd.mm.yyyy\n"
		"UPDATEKEY: 00000000000000000000000000000000\n"
		"SIGNATURE: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
	);

	connect(ui.lblCurrent, SIGNAL(linkActivated(const QString&)), this, SLOT(OnUpdate(const QString&)));
	connect(ui.lblStable, SIGNAL(linkActivated(const QString&)), this, SLOT(OnUpdate(const QString&)));
	connect(ui.lblPreview, SIGNAL(linkActivated(const QString&)), this, SLOT(OnUpdate(const QString&)));
	//connect(ui.lblInsider, SIGNAL(linkActivated(const QString&)), this, SLOT(OnUpdate(const QString&)));
	//connect(ui.lblInsiderInfo, SIGNAL(linkActivated(const QString&)), this, SLOT(OnUpdate(const QString&)));

	connect(ui.chkAutoUpdate, SIGNAL(toggled(bool)), this, SLOT(UpdateUpdater()));

	connect(ui.radStable, SIGNAL(toggled(bool)), this, SLOT(UpdateUpdater()));
	connect(ui.radPreview, SIGNAL(toggled(bool)), this, SLOT(UpdateUpdater()));
	//connect(ui.radInsider, SIGNAL(toggled(bool)), this, SLOT(UpdateUpdater()));

	connect(ui.cmbUpdate, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptChanged()));
	connect(ui.cmbRelease, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptChanged()));

	connect(ui.chkNoCheck, SIGNAL(stateChanged(int)), this, SLOT(OnOptChanged()));
	//

	connect(ui.tabs, SIGNAL(currentChanged(int)), this, SLOT(OnTab()));

	// Ini Edit
	connect(ui.btnEditIni, SIGNAL(clicked(bool)), this, SLOT(OnEditIni()));
	connect(ui.btnSaveIni, SIGNAL(clicked(bool)), this, SLOT(OnSaveIni()));
	connect(ui.btnCancelEdit, SIGNAL(clicked(bool)), this, SLOT(OnCancelEdit()));
	connect(ui.txtIniSection, SIGNAL(textChanged()), this, SLOT(OnIniChanged()));
	//

	connect(ui.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)), this, SLOT(ok()));
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	this->installEventFilter(this); // prevent enter from closing the dialog

	restoreGeometry(theConf->GetBlob("SettingsWindow/Window_Geometry"));

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) {
		QByteArray Columns = theConf->GetBlob("SettingsWindow/" + pTree->objectName() + "_Columns");
		if (!Columns.isEmpty()) 
			pTree->header()->restoreState(Columns);
	}

	int iOptionTree = theConf->GetInt("Options/OptionTree", 2);
	if (iOptionTree == 2)
		iOptionTree = iViewMode == 2 ? 1 : 0;

	if (iOptionTree) 
		OnSetTree();
	else {
		QWidget* pSearch = AddConfigSearch(ui.tabs);
		ui.horizontalLayout->insertWidget(0, pSearch);
		QTimer::singleShot(0, [this]() {
			m_pSearch->setMaximumWidth(m_pTabs->tabBar()->width());
		});

		QAction* pSetTree = new QAction();
		connect(pSetTree, SIGNAL(triggered()), this, SLOT(OnSetTree()));
		pSetTree->setShortcut(QKeySequence("Ctrl+Alt+T"));
		pSetTree->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		this->addAction(pSetTree);
	}
	m_pSearch->setPlaceholderText(tr("Search for settings"));
}

void CSettingsWindow::OnSetTree()
{
	if (!ui.tabs) return;
	QWidget* pAltView = ConvertToTree(ui.tabs);
	ui.verticalLayout->replaceWidget(ui.tabs, pAltView);
	ui.tabs->deleteLater();
	ui.tabs = NULL;
}

CSettingsWindow::~CSettingsWindow()
{
	theConf->SetBlob("SettingsWindow/Window_Geometry",saveGeometry());

	foreach(QTreeWidget * pTree, this->findChildren<QTreeWidget*>()) 
		theConf->SetBlob("SettingsWindow/" + pTree->objectName() + "_Columns", pTree->header()->saveState());
}

void CSettingsWindow::showTab(int Tab, bool bExclusive)
{
	QWidget* pWidget = NULL;
	switch (Tab)
	{
	case eOptions: pWidget = ui.tabGeneral; break;
	case eShell: pWidget = ui.tabWindows; break;
	case eGuiConfig: pWidget = ui.tabGUI; break;
	case eAdvanced: pWidget = ui.tabSandbox; break;
	case eProgCtrl: pWidget = ui.tabAlert; break;
	case eConfigLock: pWidget = ui.tabLock; break;
	case eSoftCompat: pWidget = ui.tabAppCompat; break;
	case eEditIni: pWidget = ui.tabEdit; break;
	case eSupport: pWidget = ui.tabSupport; break;
	}

	if (ui.tabs) {
		
		for (int i = 0; i < ui.tabs->count(); i++) {
			QGridLayout* pGrid = qobject_cast<QGridLayout*>(ui.tabs->widget(i)->layout());
			QTabWidget* pSubTabs = pGrid ? qobject_cast<QTabWidget*>(pGrid->itemAt(0)->widget()) : NULL;
			if (!pSubTabs) {
				if(ui.tabs->widget(i) == pWidget)
					ui.tabs->setCurrentIndex(i);
			}
			else {
				for (int j = 0; j < pSubTabs->count(); j++) {
					if (pSubTabs->widget(j) == pWidget) {
						ui.tabs->setCurrentIndex(i);
						pSubTabs->setCurrentIndex(j);
					}
				}
			}
		}
		
	}
	else
		m_pStack->setCurrentWidget(pWidget);

	if(pWidget == ui.tabCompat)
		m_CompatLoaded = 2;
	if(pWidget == ui.tabSupport)
		ui.chkNoCheck->setVisible(true);

	if (bExclusive) {
		if (ui.tabs)
			ui.tabs->tabBar()->setVisible(false);
		else {
			m_pTree->setVisible(false);
			m_pSearch->setVisible(false);
		}
	}

	SafeShow(this);
}

void CSettingsWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

bool CSettingsWindow::eventFilter(QObject *source, QEvent *event)
{
	//if (event->type() == QEvent::KeyPress && ((QKeyEvent*)event)->key() == Qt::Key_Escape 
	//	&& ((QKeyEvent*)event)->modifiers() == Qt::NoModifier)
	//{
	//	return true; // cancel event
	//}

	if (event->type() == QEvent::KeyPress && (((QKeyEvent*)event)->key() == Qt::Key_Enter || ((QKeyEvent*)event)->key() == Qt::Key_Return) 
		&& (((QKeyEvent*)event)->modifiers() == Qt::NoModifier || ((QKeyEvent*)event)->modifiers() == Qt::KeypadModifier))
	{
		return true; // cancel event
	}

	return QDialog::eventFilter(source, event);
}

void CSettingsWindow::OnAddMessage()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter message"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	StrPair IdText = Split2(Value, ",");

	AddMessageItem(IdText.first, IdText.second);

	OnMessageChanged();
}

void CSettingsWindow::AddMessageItem(const QString& ID, const QString& Text)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, ID);
	pItem->setText(1, Text);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeMessages->addTopLevelItem(pItem);
}

void CSettingsWindow::OnDelMessage()
{
	QTreeWidgetItem* pItem = ui.treeMessages->currentItem();
	if (!pItem)
		return;

	delete pItem;
	OnMessageChanged();
}

void CSettingsWindow::OnBrowsePath()
{
	QString Value = QFileDialog::getOpenFileName(this, tr("Select Program"), "", tr("Executables (*.exe *.cmd)")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal);
	if (Name.isEmpty())
		return;

	AddRunItem(Name, "", "\"" + Value + "\"");
}

void CSettingsWindow::OnAddCommand()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a command"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	QString Name = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter a menu title"), QLineEdit::Normal);
	if (Name.isEmpty())
		return;

	AddRunItem(Name, "", Value);
	OnRunChanged();
}

void CSettingsWindow::AddRunItem(const QString& Name, const QString& Icon, const QString& Command)
{
	QTreeWidgetItem* pItem = new QTreeWidgetItem();
	pItem->setText(0, Name);
	pItem->setText(1, Icon);
	pItem->setText(2, Command);
	pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
	ui.treeRun->addTopLevelItem(pItem);
}

void CSettingsWindow::OnCommandUp()
{
	int index = ui.treeRun->indexOfTopLevelItem(ui.treeRun->currentItem());
	if (index > 0) {
		QTreeWidgetItem* pItem = ui.treeRun->takeTopLevelItem(index);
		ui.treeRun->insertTopLevelItem(index - 1, pItem);
		ui.treeRun->setCurrentItem(pItem);
		OnRunChanged();
	}
}

void CSettingsWindow::OnCommandDown()
{
	int index = ui.treeRun->indexOfTopLevelItem(ui.treeRun->currentItem());
	if (index < ui.treeRun->topLevelItemCount()-1) {
		QTreeWidgetItem* pItem = ui.treeRun->takeTopLevelItem(index);
		ui.treeRun->insertTopLevelItem(index + 1, pItem);
		ui.treeRun->setCurrentItem(pItem);
		OnRunChanged();
	}
}

void CSettingsWindow::OnDelCommand()
{
	QTreeWidgetItem* pItem = ui.treeRun->currentItem();
	if (!pItem)
		return;

	delete pItem;
	OnRunChanged();
}

Qt::CheckState CSettingsWindow__IsContextMenu()
{
	//QSettings Package("HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\PackagedCom\\Package", QSettings::NativeFormat);
	QSettings Package("HKEY_CURRENT_USER\\Software\\Classes\\PackagedCom\\Package", QSettings::NativeFormat);
	foreach(const QString & Key, Package.childGroups()) {
		if (Key.indexOf("SandboxieShell") == 0)
			return Qt::Checked;
	}

	QString cmd = CSbieUtils::GetContextMenuStartCmd();
	if (cmd.contains("SandMan.exe", Qt::CaseInsensitive)) 
		return Qt::Checked; // set up and sandman
	if (!cmd.isEmpty()) // ... probably sbiectrl.exe
		return Qt::PartiallyChecked; 
	return Qt::Unchecked; // not set up
}

void CSettingsWindow__AddContextMenu(bool bAlwaysClassic)
{
	QSettings CurrentVersion("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", QSettings::NativeFormat);
	if (CurrentVersion.value("CurrentBuild").toInt() >= 22000 && !bAlwaysClassic) // Windows 11
	{
		QSettings MyReg("HKEY_CURRENT_USER\\SOFTWARE\\Xanasoft\\Sandboxie-Plus\\SbieShellExt\\Lang", QSettings::NativeFormat);
		MyReg.setValue("Open Sandboxed", CSettingsWindow::tr("Run &Sandboxed"));
		MyReg.setValue("Explore Sandboxed", CSettingsWindow::tr("Run &Sandboxed"));
		
		QDir::setCurrent(QCoreApplication::applicationDirPath());
		QProcess Proc;
		Proc.execute("rundll32.exe", QStringList() << "SbieShellExt.dll,RegisterPackage");
		Proc.waitForFinished();
		return;
	}

	CSbieUtils::AddContextMenu(QApplication::applicationDirPath().replace("/", "\\") + "\\SandMan.exe",
		CSettingsWindow::tr("Run &Sandboxed"), //CSettingsWindow::tr("Explore &Sandboxed"),
			QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe");
}

void CSettingsWindow__RemoveContextMenu()
{
	QSettings CurrentVersion("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", QSettings::NativeFormat);
	if (CurrentVersion.value("CurrentBuild").toInt() >= 22000) // Windows 11
	{
		QDir::setCurrent(QCoreApplication::applicationDirPath());
		QProcess Proc;
		Proc.execute("rundll32.exe", QStringList() << "SbieShellExt.dll,RemovePackage");
		Proc.waitForFinished();
	}

	CSbieUtils::RemoveContextMenu();
}

void CSettingsWindow__AddBrowserIcon()
{
	QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
	Path += "\\" + CSettingsWindow::tr("Sandboxed Web Browser") + ".lnk";
	CSbieUtils::CreateShortcut(theAPI, Path, "", "", "default_browser");
}

void CSettingsWindow::LoadSettings()
{
	ui.uiLang->setCurrentIndex(ui.uiLang->findData(theConf->GetString("Options/UiLanguage")));

	ui.chkAutoStart->setChecked(IsAutorunEnabled());
	if (theAPI->IsConnected()) {
		if (theAPI->GetUserSettings()->GetBool("SbieCtrl_EnableAutoStart", true)) {
			if (theAPI->GetUserSettings()->GetText("SbieCtrl_AutoStartAgent", "") != "SandMan.exe")
				ui.chkSvcStart->setCheckState(Qt::PartiallyChecked);
			else
				ui.chkSvcStart->setChecked(true);
		}
		else
			ui.chkSvcStart->setChecked(false);
	}
	else {
		ui.chkSvcStart->setEnabled(false);
	}

	ui.chkShellMenu->setCheckState(CSettingsWindow__IsContextMenu());
	ui.chkShellMenu2->setChecked(CSbieUtils::HasContextMenu2());
	ui.chkAlwaysDefault->setChecked(theConf->GetBool("Options/RunInDefaultBox", false));

	ui.cmbDPI->setCurrentIndex(theConf->GetInt("Options/DPIScaling", 1));

	ui.chkDarkTheme->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/UseDarkTheme", 2)));
	ui.chkFusionTheme->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/UseFusionTheme", 2)));
	ui.chkAltRows->setChecked(theConf->GetBool("Options/AltRowColors", false));
	ui.chkBackground->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/UseBackground", 2)));
	ui.chkLargeIcons->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/LargeIcons", 2)));
	ui.chkNoIcons->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/NoIcons", 2)));
	ui.chkOptTree->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/OptionTree", 2)));
	ui.chkNewLayout->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/NewConfigLayout", 2)));
	ui.chkColorIcons->setChecked(theConf->GetBool("Options/ColorBoxIcons", false));
	ui.chkOverlayIcons->setChecked(theConf->GetBool("Options/UseOverlayIcons", true));

	//ui.cmbFontScale->setCurrentIndex(ui.cmbFontScale->findData(theConf->GetInt("Options/FontScaling", 100)));
	ui.cmbFontScale->setCurrentText(QString::number(theConf->GetInt("Options/FontScaling", 100)));

	ui.chkSilentMode->setChecked(theConf->GetBool("Options/CheckSilentMode", true));
	ui.chkCopyProgress->setChecked(theConf->GetBool("Options/ShowMigrationProgress", true));
	ui.chkNoMessages->setChecked(!theConf->GetBool("Options/ShowNotifications", true));

	ui.chkSandboxUrls->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/OpenUrlsSandboxed", 2)));

	ui.chkShowRecovery->setChecked(theConf->GetBool("Options/ShowRecovery", false));
	ui.chkNotifyRecovery->setChecked(!theConf->GetBool("Options/InstantRecovery", true));
	ui.chkRecoveryTop->setChecked(theConf->GetBool("Options/RecoveryOnTop", true));
	ui.chkAsyncBoxOps->setChecked(theConf->GetBool("Options/UseAsyncBoxOps", false));

	ui.chkPanic->setChecked(theConf->GetBool("Options/EnablePanicKey", false));
	ui.keyPanic->setKeySequence(QKeySequence(theConf->GetString("Options/PanicKeySequence", "Shift+Pause")));

	ui.chkMonitorSize->setChecked(theConf->GetBool("Options/WatchBoxSize", false));

	ui.chkWatchConfig->setChecked(theConf->GetBool("Options/WatchIni", true));

	ui.chkScanMenu->setChecked(theConf->GetBool("Options/ScanStartMenu", true));
	ui.cmbIntegrateMenu->setCurrentIndex(theConf->GetInt("Options/IntegrateStartMenu", 0));
	
	ui.cmbSysTray->setCurrentIndex(theConf->GetInt("Options/SysTrayIcon", 1));
	ui.cmbTrayBoxes->setCurrentIndex(theConf->GetInt("Options/SysTrayFilter", 0));
	ui.chkCompactTray->setChecked(theConf->GetBool("Options/CompactTray", false));
	ui.chkBoxOpsNotify->setChecked(theConf->GetBool("Options/AutoBoxOpsNotify", false));


	if (theAPI->IsConnected())
	{
		ui.treeMessages->clear();
		foreach(const QString & Value, theAPI->GetUserSettings()->GetTextList("SbieCtrl_HideMessage", false))
		{
			StrPair NameIcon = Split2(Value, ",");
			AddMessageItem(NameIcon.first, NameIcon.second);
		}
		m_MessagesChanged = false;

		ui.treeRun->clear();
		foreach(const QString& Value, theAPI->GetGlobalSettings()->GetTextList("RunCommand", false))
		{
			StrPair NameCmd = Split2(Value, "|");
			StrPair NameIcon = Split2(NameCmd.first, ",");
			AddRunItem(NameIcon.first, NameIcon.second, NameCmd.second);
		}
		m_RunChanged = false;
		
		ui.cmbDefault->clear();
		foreach(const CSandBoxPtr & pBox, theAPI->GetAllBoxes())
			ui.cmbDefault->addItem(pBox->GetName().replace("_", " "), pBox->GetName());
		int pos = ui.cmbDefault->findData(theAPI->GetGlobalSettings()->GetText("DefaultBox", "DefaultBox"));
		if(pos == -1)
			pos = ui.cmbDefault->findData("DefaultBox");
		ui.cmbDefault->setCurrentIndex(pos);

		QString FileRootPath_Default = "\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%";
		QString KeyRootPath_Default  = "\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%";
		QString IpcRootPath_Default  = "\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%";

		ui.fileRoot->setText(theAPI->GetGlobalSettings()->GetText("FileRootPath", FileRootPath_Default));
		//ui.chkSeparateUserFolders->setChecked(theAPI->GetGlobalSettings()->GetBool("SeparateUserFolders", true));
		ui.regRoot->setText(theAPI->GetGlobalSettings()->GetText("KeyRootPath", KeyRootPath_Default));
		ui.ipcRoot->setText(theAPI->GetGlobalSettings()->GetText("IpcRootPath", IpcRootPath_Default));

		ui.chkWFP->setChecked(theAPI->GetGlobalSettings()->GetBool("NetworkEnableWFP", false));
		ui.chkObjCb->setChecked(theAPI->GetGlobalSettings()->GetBool("EnableObjectFiltering", true));
		ui.chkWin32k->setChecked(theAPI->GetGlobalSettings()->GetBool("EnableWin32kHooks", true));
		ui.chkSbieLogon->setChecked(theAPI->GetGlobalSettings()->GetBool("SandboxieLogon", false));

		ui.chkAdminOnly->setChecked(theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false));
		ui.chkAdminOnly->setEnabled(IsAdminUser());
		ui.chkPassRequired->setChecked(!theAPI->GetGlobalSettings()->GetText("EditPassword", "").isEmpty());
		ui.chkAdminOnlyFP->setChecked(theAPI->GetGlobalSettings()->GetBool("ForceDisableAdminOnly", false));
		ui.chkClearPass->setChecked(theAPI->GetGlobalSettings()->GetBool("ForgetPassword", false));
		m_HoldChange = true;
		OnProtectionChange();
		m_HoldChange = false;

		m_GeneralChanged = false;

		ui.chkStartBlock->setChecked(theAPI->GetGlobalSettings()->GetBool("StartRunAlertDenied", false));
		ui.chkStartBlockMsg->setChecked(theAPI->GetGlobalSettings()->GetBool("AlertStartRunAccessDenied", true));
		ui.chkNotForcedMsg->setChecked(theAPI->GetGlobalSettings()->GetBool("NotifyForceProcessDisabled", false));

		ui.treeWarnProgs->clear();

		foreach(const QString& Value, theAPI->GetGlobalSettings()->GetTextList("AlertProcess", false))
			AddWarnEntry(Value, 1);
		
		foreach(const QString& Value, theAPI->GetGlobalSettings()->GetTextList("AlertFolder", false))
			AddWarnEntry(Value, 2);

		m_WarnProgsChanged = false;
	}
	
	if(!theAPI->IsConnected() || (theAPI->GetGlobalSettings()->GetBool("EditAdminOnly", false) && !IsAdminUser()))
	{
		ui.cmbDefault->setEnabled(false);
		ui.fileRoot->setEnabled(false);
		//ui.chkSeparateUserFolders->setEnabled(false);
		ui.chkAutoRoot->setEnabled(false);
		ui.chkWFP->setEnabled(false);
		ui.chkObjCb->setEnabled(false);
		ui.chkWin32k->setEnabled(false);
		ui.chkSbieLogon->setEnabled(false);
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
		ui.treeTemplates->setEnabled(false);
		ui.btnAddTemplate->setEnabled(false);
		ui.btnDelTemplate->setEnabled(false);
		ui.btnEditIni->setEnabled(false);
	}


	if (theGUI->IsFullyPortable()) {
		ui.chkAutoRoot->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/PortableRootDir", 2)));
		m_HoldChange = true;
		OnRootChanged();
		m_HoldChange = false;
	}
	else
		ui.chkAutoRoot->setVisible(false);

	UpdateCert();

	ui.chkAutoUpdate->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/CheckForUpdates", 2)));
	//ui.chkAutoDownload->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/DownloadUpdates", 0)));
	//ui.chkAutoInstall->setCheckState(CSettingsWindow__Int2Chk(theConf->GetInt("Options/InstallUpdates", 0)));

	QString ReleaseChannel = theConf->GetString("Options/ReleaseChannel", "stable");
	ui.radStable->setChecked(ReleaseChannel == "stable");
	ui.radPreview->setChecked(ReleaseChannel == "preview");
	//ui.radInsider->setChecked(ReleaseChannel == "insider");

	m_HoldChange = true;
	UpdateUpdater();
	m_HoldChange = false;

	ui.cmbUpdate->setCurrentIndex(ui.cmbUpdate->findData(theConf->GetString("Options/OnNewUpdate", "ignore")));
	ui.cmbRelease->setCurrentIndex(ui.cmbRelease->findData(theConf->GetString("Options/OnNewRelease", "download")));


	ui.chkNoCheck->setChecked(theConf->GetBool("Options/NoSupportCheck", false));
	if(ui.chkNoCheck->isCheckable() && !g_CertInfo.expired)
		ui.chkNoCheck->setVisible(false); // hide if not relevant
}

void CSettingsWindow::UpdateCert()
{
	ui.lblCertExp->setVisible(false);
	if (!g_Certificate.isEmpty()) 
	{
		ui.txtCertificate->setPlainText(g_Certificate);
		//ui.lblSupport->setVisible(false);

		QPalette palette = QApplication::palette();
		if (theGUI->m_DarkTheme)
			palette.setColor(QPalette::Text, Qt::black);
		if (g_CertInfo.expired) {
			palette.setColor(QPalette::Base, QColor(255, 255, 192));
			QString infoMsg = tr("This supporter certificate has expired, please <a href=\"sbie://update/cert\">get an updated certificate</a>.");
			if (g_CertInfo.valid) {
				if (g_CertInfo.grace_period)
					infoMsg.append(tr("<br /><font color='red'>Plus features will be disabled in %1 days.</font>").arg(30 + g_CertInfo.expirers_in_sec / (60*60*24)));
				else if (!g_CertInfo.outdated) // must be an expiren medium or large cert on an old build
					infoMsg.append(tr("<br /><font color='red'>For this build Plus features remain enabled.</font>"));
			} else
				infoMsg.append(tr("<br />Plus features are no longer enabled."));
			ui.lblCertExp->setText(infoMsg);
			ui.lblCertExp->setVisible(true);
		}
		else {
			if (g_CertInfo.about_to_expire) {
				ui.lblCertExp->setText(tr("This supporter certificate will <font color='red'>expire in %1 days</font>, please <a href=\"sbie://update/cert\">get an updated certificate</a>.").arg(g_CertInfo.expirers_in_sec / (60*60*24)));
				ui.lblCertExp->setVisible(true);
			}
/*#ifdef _DEBUG
			else {
				ui.lblCertExp->setText(tr("This supporter certificate is valid, <a href=\"sbie://update/cert\">check for an updated certificate</a>."));
				ui.lblCertExp->setVisible(true);
			}
#endif*/
			palette.setColor(QPalette::Base, QColor(192, 255, 192));
		}
		ui.txtCertificate->setPalette(palette);
	}

	//ui.radInsider->setEnabled(g_CertInfo.insider);
}

void CSettingsWindow::UpdateUpdater()
{
	//ui.radLive->setEnabled(false);
	if (!ui.chkAutoUpdate->isChecked()) {
		ui.cmbUpdate->setEnabled(false);
		ui.cmbRelease->setEnabled(false);
		ui.lblRevision->setText(QString());
	}
	else {
		if (ui.radStable->isChecked() && (!g_CertInfo.valid)) {
			ui.cmbUpdate->setEnabled(false);
			ui.cmbUpdate->setCurrentIndex(ui.cmbUpdate->findData("ignore"));
			ui.lblRevision->setText(tr("Supporter certificate required"));
		} 
		else {
			ui.cmbUpdate->setEnabled(true);
			ui.lblRevision->setText(QString());
		}
		ui.cmbRelease->setEnabled(true);
	}

	OnOptChanged();
}

void CSettingsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue)
{
	//if (pCheck->checkState() == Qt::PartiallyChecked)
	//	return;

	if (!pCheck->isEnabled())
		return;

	SB_STATUS Status;
	if (pCheck->checkState() == Qt::Checked)
	{
		if(!OnValue.isEmpty())
			Status = theAPI->GetGlobalSettings()->SetText(Name, OnValue);
		else
			Status = theAPI->GetGlobalSettings()->DelValue(Name);
	}
	else if (pCheck->checkState() == Qt::Unchecked)
	{
		if (!OffValue.isEmpty())
			Status = theAPI->GetGlobalSettings()->SetText(Name, OffValue);
		else
			Status = theAPI->GetGlobalSettings()->DelValue(Name);
	}

	if (!Status)
		throw Status;
}

void CSettingsWindow::WriteText(const QString& Name, const QString& Value)
{
	SB_STATUS Status;
	if(Value.isEmpty())
		Status = theAPI->GetGlobalSettings()->DelValue(Name);
	else
		Status = theAPI->GetGlobalSettings()->SetText(Name, Value);
	if (!Status)
		throw Status;
}

void CSettingsWindow::WriteTextList(const QString& Setting, const QStringList& List)
{
	SB_STATUS Status = theAPI->GetGlobalSettings()->UpdateTextList(Setting, List, false);
	if (!Status)
		throw Status;
}

void CSettingsWindow::SaveSettings()
{
	theConf->SetValue("Options/UiLanguage", ui.uiLang->currentData());

	theConf->SetValue("Options/DPIScaling", ui.cmbDPI->currentData());

	theConf->SetValue("Options/UseDarkTheme", CSettingsWindow__Chk2Int(ui.chkDarkTheme->checkState()));
	theConf->SetValue("Options/UseFusionTheme", CSettingsWindow__Chk2Int(ui.chkFusionTheme->checkState()));
	theConf->SetValue("Options/AltRowColors", ui.chkAltRows->isChecked());
	theConf->SetValue("Options/UseBackground", CSettingsWindow__Chk2Int(ui.chkBackground->checkState()));
	theConf->SetValue("Options/LargeIcons", CSettingsWindow__Chk2Int(ui.chkLargeIcons->checkState()));
	theConf->SetValue("Options/NoIcons", CSettingsWindow__Chk2Int(ui.chkNoIcons->checkState()));
	theConf->SetValue("Options/OptionTree", CSettingsWindow__Chk2Int(ui.chkOptTree->checkState()));
	theConf->SetValue("Options/NewConfigLayout", CSettingsWindow__Chk2Int(ui.chkNewLayout->checkState()));
	theConf->SetValue("Options/ColorBoxIcons", ui.chkColorIcons->isChecked());
	theConf->SetValue("Options/UseOverlayIcons", ui.chkOverlayIcons->isChecked());

	int Scaling = ui.cmbFontScale->currentText().toInt();
	if (Scaling < 75)
		Scaling = 75;
	else if (Scaling > 500)
		Scaling = 500;
	theConf->SetValue("Options/FontScaling", Scaling);

	AutorunEnable(ui.chkAutoStart->isChecked());

	if (theAPI->IsConnected()) {
		if (ui.chkSvcStart->checkState() == Qt::Checked) {
			theAPI->GetUserSettings()->SetBool("SbieCtrl_EnableAutoStart", true);
			theAPI->GetUserSettings()->SetText("SbieCtrl_AutoStartAgent", "SandMan.exe");
		}
		else if (ui.chkSvcStart->checkState() == Qt::Unchecked)
			theAPI->GetUserSettings()->SetBool("SbieCtrl_EnableAutoStart", false);
	}

	if (ui.chkShellMenu->checkState() != CSettingsWindow__IsContextMenu())
	{
		if (ui.chkShellMenu->isChecked()) {
			CSecretCheckBox* SecretCheckBox = qobject_cast<CSecretCheckBox*>(ui.chkShellMenu);
			CSettingsWindow__AddContextMenu(SecretCheckBox && SecretCheckBox->IsSecretSet());
		}
		else
			CSettingsWindow__RemoveContextMenu();
	}

	if (ui.chkShellMenu2->isChecked() != CSbieUtils::HasContextMenu2()) {
		if (ui.chkShellMenu2->isChecked()) {
			CSbieUtils::AddContextMenu2(QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe",
				tr("Run &Un-Sandboxed"),
				QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe");
		} else
			CSbieUtils::RemoveContextMenu2();
	}

	theConf->SetValue("Options/RunInDefaultBox", ui.chkAlwaysDefault->isChecked());

	theConf->SetValue("Options/CheckSilentMode", ui.chkSilentMode->isChecked());
	theConf->SetValue("Options/ShowMigrationProgress", ui.chkCopyProgress->isChecked());
	theConf->SetValue("Options/ShowNotifications", !ui.chkNoMessages->isChecked());

	theConf->SetValue("Options/OpenUrlsSandboxed", CSettingsWindow__Chk2Int(ui.chkSandboxUrls->checkState()));

	theConf->SetValue("Options/ShowRecovery", ui.chkShowRecovery->isChecked());
	theConf->SetValue("Options/InstantRecovery", !ui.chkNotifyRecovery->isChecked());
	theConf->SetValue("Options/RecoveryOnTop", ui.chkRecoveryTop->isChecked());
	theConf->SetValue("Options/UseAsyncBoxOps", ui.chkAsyncBoxOps->isChecked());

	theConf->SetValue("Options/EnablePanicKey", ui.chkPanic->isChecked());
	theConf->SetValue("Options/PanicKeySequence", ui.keyPanic->keySequence().toString());
	
	theConf->SetValue("Options/WatchBoxSize", ui.chkMonitorSize->isChecked());

	theConf->SetValue("Options/WatchIni", ui.chkWatchConfig->isChecked());

	theConf->SetValue("Options/ScanStartMenu", ui.chkScanMenu->isChecked());
	int OldIntegrateStartMenu = theConf->GetInt("Options/IntegrateStartMenu", 0);
	theConf->SetValue("Options/IntegrateStartMenu", ui.cmbIntegrateMenu->currentIndex());
	if (ui.cmbIntegrateMenu->currentIndex() != OldIntegrateStartMenu) {
		if (ui.cmbIntegrateMenu->currentIndex() == 0)
			theGUI->ClearStartMenu();
		else
			theGUI->SyncStartMenu();
	}

	theConf->SetValue("Options/SysTrayIcon", ui.cmbSysTray->currentIndex());
	theConf->SetValue("Options/SysTrayFilter", ui.cmbTrayBoxes->currentIndex());
	theConf->SetValue("Options/CompactTray", ui.chkCompactTray->isChecked());
	theConf->SetValue("Options/AutoBoxOpsNotify", ui.chkBoxOpsNotify->isChecked());

	if (theAPI->IsConnected())
	{
		try
		{
			if (m_MessagesChanged)
			{
				m_MessagesChanged = false;
				
				QStringList HiddenMessages;
				for (int i = 0; i < ui.treeMessages->topLevelItemCount(); i++) {
					QTreeWidgetItem* pItem = ui.treeMessages->topLevelItem(i);
					if (!pItem->text(1).isEmpty())
						HiddenMessages.append(pItem->text(0) + "," + pItem->text(1));
					else
						HiddenMessages.append(pItem->text(0));
				}
				theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_HideMessage", HiddenMessages, false);
			}

			if (m_RunChanged) 
			{
				m_RunChanged = false;

				QStringList RunCommands;
				for (int i = 0; i < ui.treeRun->topLevelItemCount(); i++) {
					QTreeWidgetItem* pItem = ui.treeRun->topLevelItem(i);
					if (pItem->text(1).isEmpty())
						RunCommands.prepend(pItem->text(0) + "|" + pItem->text(2));
					else
						RunCommands.prepend(pItem->text(0) + "," + pItem->text(1) + "|" + pItem->text(2));
				}
				//WriteTextList("RunCommand", RunCommands);
				theAPI->GetGlobalSettings()->DelValue("RunCommand");
				foreach(const QString & Value, RunCommands)
					theAPI->GetGlobalSettings()->InsertText("RunCommand", Value);
			}

			if (m_GeneralChanged)
			{
				m_GeneralChanged = false;

				WriteText("DefaultBox", ui.cmbDefault->currentData().toString());

				WriteText("FileRootPath", ui.fileRoot->text()); //ui.fileRoot->setText("\\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%");
				//WriteAdvancedCheck(ui.chkSeparateUserFolders, "SeparateUserFolders", "", "n");
				WriteText("KeyRootPath", ui.regRoot->text()); //ui.regRoot->setText("\\REGISTRY\\USER\\Sandbox_%USER%_%SANDBOX%");
				WriteText("IpcRootPath", ui.ipcRoot->text()); //ui.ipcRoot->setText("\\Sandbox\\%USER%\\%SANDBOX%\\Session_%SESSION%");

				WriteAdvancedCheck(ui.chkWFP, "NetworkEnableWFP", "y", "");
				WriteAdvancedCheck(ui.chkObjCb, "EnableObjectFiltering", "", "n");
				WriteAdvancedCheck(ui.chkWin32k, "EnableWin32kHooks", "", "n");
				WriteAdvancedCheck(ui.chkSbieLogon, "SandboxieLogon", "y", "");

				if (m_FeaturesChanged) {
					m_FeaturesChanged = false;
					theAPI->ReloadConfig(true);
				}
			}

			if (m_ProtectionChanged)
			{
				m_ProtectionChanged = false;

				WriteAdvancedCheck(ui.chkAdminOnly, "EditAdminOnly", "y", "");

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

				WriteAdvancedCheck(ui.chkAdminOnlyFP, "ForceDisableAdminOnly", "y", "");
				WriteAdvancedCheck(ui.chkClearPass, "ForgetPassword", "y", "");
			}

			if (m_WarnProgsChanged)
			{
				m_WarnProgsChanged = false;

				WriteAdvancedCheck(ui.chkStartBlock, "StartRunAlertDenied", "y", "");
				WriteAdvancedCheck(ui.chkStartBlockMsg, "AlertStartRunAccessDenied", "", "n");
				WriteAdvancedCheck(ui.chkNotForcedMsg, "NotifyForceProcessDisabled", "y", "");

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

				WriteTextList("AlertProcess", AlertProcess);
				WriteTextList("AlertFolder", AlertFolder);
			}

			if (m_CompatChanged)
			{
				m_CompatChanged = false;

				QStringList Used;
				QStringList Rejected;
				for (int i = 0; i < ui.treeCompat->topLevelItemCount(); i++) {
					QTreeWidgetItem* pItem = ui.treeCompat->topLevelItem(i);
					QString Template = pItem->data(0, Qt::UserRole).toString().mid(9);
					if (pItem->checkState(0) == Qt::Unchecked)
						Rejected.append(Template);
					else
						Used.append(Template);
				}

				// retain local templates
				foreach(const QString& Template, theAPI->GetGlobalSettings()->GetTextList("Template", false)) {
					if (Template.left(6) == "Local_") {
						Used.append(Template);
					}
				}

				WriteTextList("Template", Used);
				WriteTextList("TemplateReject", Rejected);
			}
		}
		catch (SB_STATUS Status)
		{
			theGUI->CheckResults(QList<SB_STATUS>() << Status);
		}
	}

	if (ui.chkAutoRoot->isVisible())
		theConf->SetValue("Options/PortableRootDir", CSettingsWindow__Chk2Int(ui.chkAutoRoot->checkState()));

	theConf->SetValue("Options/AutoRunSoftCompat", !ui.chkNoCompat->isChecked());

	if (m_CertChanged && theAPI->IsConnected())
	{
		QByteArray Certificate = ui.txtCertificate->toPlainText().toUtf8();	
		if (g_Certificate != Certificate) {

			QPalette palette = QApplication::palette();

			if (theGUI->m_DarkTheme)
				palette.setColor(QPalette::Text, Qt::black);

			ui.lblCertExp->setVisible(false);

			bool bRet = ApplyCertificate(Certificate, this);

			if (Certificate.isEmpty())
				palette.setColor(QPalette::Base, Qt::white);
			else if (!bRet) 
				palette.setColor(QPalette::Base, QColor(255, 192, 192));
			else 
				palette.setColor(QPalette::Base, QColor(192, 255, 192));

			ui.txtCertificate->setPalette(palette);
		}

		m_CertChanged = false;
	}

	theConf->SetValue("Options/CheckForUpdates", CSettingsWindow__Chk2Int(ui.chkAutoUpdate->checkState()));
	//theConf->SetValue("Options/DownloadUpdates", CSettingsWindow__Chk2Int(ui.chkAutoDownload->checkState()));
	//theConf->SetValue("Options/InstallUpdates", CSettingsWindow__Chk2Int(ui.chkAutoInstall->checkState()));

	QString ReleaseChannel;
	if (ui.radStable->isChecked())
		ReleaseChannel = "stable";
	else if (ui.radPreview->isChecked())
		ReleaseChannel = "preview";
	//else if (ui.radInsider->isChecked())
	//	ReleaseChannel = "insider";
	if(!ReleaseChannel.isEmpty()) theConf->SetValue("Options/ReleaseChannel", ReleaseChannel);

	theConf->SetValue("Options/OnNewUpdate", ui.cmbUpdate->currentData());
	theConf->SetValue("Options/OnNewRelease", ui.cmbRelease->currentData());

	theConf->SetValue("Options/NoSupportCheck", ui.chkNoCheck->isChecked());

	emit OptionsChanged(m_bRebuildUI);
}

bool CSettingsWindow::ApplyCertificate(const QByteArray &Certificate, QWidget* widget)
{
	QString CertPath = theAPI->GetSbiePath() + "\\Certificate.dat";
	if (!Certificate.isEmpty()) {

		auto Args = GetArguments(Certificate, L'\n', L':');

		bool bLooksOk = true;
		if (Args.value("NAME").isEmpty()) // mandatory
			bLooksOk = false;
		//if (Args.value("UPDATEKEY").isEmpty())
		//	bLooksOk = false;
		if (Args.value("SIGNATURE").isEmpty()) // absolutely mandatory
			bLooksOk = false;

		if (bLooksOk) {
			QString TempPath = QDir::tempPath() + "/Sbie+Certificate.dat";
			QFile CertFile(TempPath);
			if (CertFile.open(QFile::WriteOnly)) {
				CertFile.write(Certificate);
				CertFile.close();
			}

			WindowsMoveFile(TempPath.replace("/", "\\"), CertPath.replace("/", "\\"));
		}
		else {
			QMessageBox::critical(widget, "Sandboxie-Plus", tr("This does not look like a certificate. Please enter the entire certificate, not just a portion of it."));
			return false;
		}
	}
	else if(!g_Certificate.isEmpty()){
		WindowsMoveFile(CertPath.replace("/", "\\"), "");
	}

	if (Certificate.isEmpty())
		return false;

	if (!theAPI->ReloadCert().IsError())
	{
		g_FeatureFlags = theAPI->GetFeatureFlags();
		g_Certificate = Certificate;
		theGUI->UpdateCertState();

		if (g_CertInfo.expired || g_CertInfo.outdated) {
			if(g_CertInfo.expired)
				QMessageBox::information(widget, "Sandboxie-Plus", tr("This certificate is unfortunately expired."));
			else
				QMessageBox::information(widget, "Sandboxie-Plus", tr("This certificate is unfortunately outdated."));
		}
		else {
			QMessageBox::information(widget, "Sandboxie-Plus", tr("Thank you for supporting the development of Sandboxie-Plus."));
		}

		return true;
	}
	else
	{
		QMessageBox::critical(widget, "Sandboxie-Plus", tr("This support certificate is not valid."));

		g_CertInfo.State = 0;
		g_Certificate.clear();
		return false;
	}
}

void CSettingsWindow::apply()
{
	if (!ui.btnEditIni->isEnabled())
		SaveIniSection();
	else
		SaveSettings();
	LoadSettings();
}

void CSettingsWindow::ok()
{
	apply();

	this->close();
}

void CSettingsWindow::reject()
{
	this->close();
}

void CSettingsWindow::OnOptChanged()
{
	if (m_HoldChange)
		return;
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void CSettingsWindow::OnBrowse()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;

	ui.fileRoot->setText(Value + "\\%SANDBOX%");
}

void CSettingsWindow::OnRootChanged()
{
	if (ui.chkAutoRoot->isVisible())
		ui.fileRoot->setEnabled(ui.chkAutoRoot->checkState() != Qt::Checked);
	OnOptChanged();
}

void CSettingsWindow::OnTab()
{
	OnTab(ui.tabs->currentWidget());
}

void CSettingsWindow::OnTab(QWidget* pTab)
{
	m_pCurrentTab = pTab;

	if (pTab == ui.tabSupport)
	{
		if (ui.lblCurrent->text().isEmpty()) {
			if (ui.chkAutoUpdate->checkState())
				GetUpdates();
			else
				ui.lblCurrent->setText(tr("<a href=\"check\">Check Now</a>"));
		}
	}
	else if (pTab == ui.tabEdit)
	{
		LoadIniSection();
		//ui.txtIniSection->setReadOnly(true);
	}
	else if (pTab == ui.tabCompat && m_CompatLoaded != 1 && theAPI->IsConnected())
	{
		if(m_CompatLoaded == 0)
			theGUI->GetCompat()->RunCheck();

		ui.treeCompat->clear();

		bool bNew = false;

		QMap<QString, int> Templates = theGUI->GetCompat()->GetTemplates();
		for (QMap<QString, int>::iterator I = Templates.begin(); I != Templates.end(); ++I)
		{
			if (I.value() == CSbieTemplates::eNone)
				continue;

			QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + I.key(), theAPI));

			QString Title = pTemplate->GetText("Tmpl.Title", "", false, true, true);
			if (Title.left(1) == "#")
			{
				int End = Title.mid(1).indexOf(",");
				if (End == -1) End = Title.length() - 1;
				int MsgNum = Title.mid(1, End).toInt();
				Title = theAPI->GetSbieMsgStr(MsgNum, theGUI->m_LanguageId).arg(Title.mid(End + 2)).arg("");
			}
			//if (Title.isEmpty()) Title = Name;

			QTreeWidgetItem* pItem = new QTreeWidgetItem();
			pItem->setText(0, Title);
			pItem->setData(0, Qt::UserRole, "Template_" + I.key());
			if((I.value() & CSbieTemplates::eDisabled) != 0)
				pItem->setCheckState(0, Qt::Unchecked);
			else if((I.value() & CSbieTemplates::eEnabled) != 0)
				pItem->setCheckState(0, Qt::Checked);
			else {
				pItem->setCheckState(0, Qt::PartiallyChecked);
				bNew = true;
			}
			ui.treeCompat->addTopLevelItem(pItem);
		}

		m_CompatLoaded = 1;
		if(bNew)
			OnCompatChanged();

		LoadTemplates();
	}
}

void CSettingsWindow::OnProtectionChange()
{
	ui.btnSetPassword->setEnabled(ui.chkPassRequired->isChecked());
	m_ProtectionChanged = true;
	OnOptChanged();
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
	m_ProtectionChanged = true;
	OnOptChanged();
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
	OnWarnChanged();
}

void CSettingsWindow::OnAddWarnFolder()
{
	QString Value = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Value.isEmpty())
		return;
	AddWarnEntry(Value, 2);
	OnWarnChanged();
}

void CSettingsWindow::OnDelWarnProg()
{
	QTreeWidgetItem* pItem = ui.treeWarnProgs->currentItem();
	if (!pItem)
		return;

	delete pItem;
	OnWarnChanged();
}

void CSettingsWindow::OnTemplateClicked(QTreeWidgetItem* pItem, int Column)
{
	if (sender() == ui.treeCompat) {
		// todo: check if really changed
		OnCompatChanged();
	}
}

void CSettingsWindow::OnAddCompat()
{
	QTreeWidgetItem* pItem = ui.treeCompat->currentItem();
	if (!pItem)
		return;

	pItem->setCheckState(0, Qt::Checked);
	OnCompatChanged();
}

void CSettingsWindow::OnDelCompat()
{
	QTreeWidgetItem* pItem = ui.treeCompat->currentItem();
	if (!pItem)
		return;

	pItem->setCheckState(0, Qt::Unchecked);
	OnCompatChanged();
}

void CSettingsWindow::OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column)
{
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni(pItem->data(0, Qt::UserRole).toString(), theAPI));

	COptionsWindow OptionsWindow(pTemplate, pItem->text(0));
	QPoint ParentPos = mapToGlobal(rect().topLeft());
	OptionsWindow.move(ParentPos.x() + 30, ParentPos.y() + 10);
	OptionsWindow.exec();

	// todo update name if it changed
}

void CSettingsWindow::OnAddTemplates()
{
	QString Value = QInputDialog::getText(this, "Sandboxie-Plus", tr("Please enter the template identifier"), QLineEdit::Normal);
	if (Value.isEmpty())
		return;

	QString Name = QString(Value).replace(" ", "_");

	SB_STATUS Status = theAPI->ValidateName(Name);
	if (Status.IsError()) {
		QMessageBox::critical(this, "Sandboxie-Plus", tr("Error: %1").arg(CSandMan::FormatError(Status)));
		return;
	}

	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_Local_" + Name, theAPI));

	pTemplate->SetText("Tmpl.Title", Value);
	pTemplate->SetText("Tmpl.Class", "Local");

	COptionsWindow OptionsWindow(pTemplate, Value);
	OptionsWindow.exec();

	LoadTemplates();
}

void CSettingsWindow::OnTemplateWizard()
{
	CTemplateWizard::ETemplateType Type = (CTemplateWizard::ETemplateType)((QAction*)sender())->data().toInt();
	if (CTemplateWizard::CreateNewTemplate(NULL, Type, this)) {
		LoadTemplates();
	}
}

void CSettingsWindow::OnDelTemplates()
{
	if (QMessageBox("Sandboxie-Plus", tr("Do you really want to delete the selected local template(s)?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton, this).exec() != QMessageBox::Yes)
		return;

	foreach(QTreeWidgetItem * pItem, ui.treeTemplates->selectedItems())
	{
		QString Section = pItem->data(0, Qt::UserRole).toString();
		
		delete pItem;

		// delete section
		theAPI->SbieIniSet(Section, "*", "");
	}
}

void CSettingsWindow::LoadTemplates()
{
	QStringList Templates;
	for (int index = 0; ; index++)
	{
		QString Value = theAPI->SbieIniGet2("", "", index);
		if (Value.isNull())
			break;
		Templates.append(Value);
	}

	ui.treeTemplates->clear();

	QString TextFilter = ui.txtTemplates->text();

	foreach(const QString& Name, Templates)
	{
		if (Name.left(9).compare("Template_", Qt::CaseInsensitive) != 0)
			continue;

		if (Name.indexOf(TextFilter, 0, Qt::CaseInsensitive) == -1)
			continue;

		QString Title = theAPI->SbieIniGet2(Name, "Tmpl.Title");
		if (Title.left(1) == "#")
		{
			int End = Title.mid(1).indexOf(",");
			if (End == -1) End = Title.length() - 1;
			int MsgNum = Title.mid(1, End).toInt();
			Title = theAPI->GetSbieMsgStr(MsgNum, theGUI->m_LanguageId).arg(Title.mid(End + 2)).arg("");
		}
		if (Title.isEmpty()) Title = Name;

		QTreeWidgetItem* pItem = new QTreeWidgetItem();
		pItem->setData(0, Qt::UserRole, Name);
		pItem->setText(0, Title);
		ui.treeTemplates->addTopLevelItem(pItem);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Raw section ini Editor
//

void CSettingsWindow::SetIniEdit(bool bEnable)
{
	if (m_pTree) {
		m_pTree->setEnabled(!bEnable);
	}
	else {
		for (int i = 4; i < ui.tabs->count() - 1; i++) {
			bool Enabled = ui.tabs->widget(i)->isEnabled();
			ui.tabs->setTabEnabled(i, !bEnable && Enabled);
			ui.tabs->widget(i)->setEnabled(Enabled);
		}
	}
	ui.btnSaveIni->setEnabled(bEnable);
	ui.btnCancelEdit->setEnabled(bEnable);
	//ui.txtIniSection->setReadOnly(!bEnable);
	ui.btnEditIni->setEnabled(!bEnable);
}

void CSettingsWindow::OnEditIni()
{
	SetIniEdit(true);
}

void CSettingsWindow::OnSaveIni()
{
	SaveIniSection();
	SetIniEdit(false);
	LoadSettings();
}

void CSettingsWindow::OnIniChanged()
{
	if (m_HoldChange)
		return;
	if(ui.btnEditIni->isEnabled())
		SetIniEdit(true);
	ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void CSettingsWindow::OnCancelEdit()
{
	SetIniEdit(false);
	LoadIniSection();
}

void CSettingsWindow::LoadIniSection()
{
	QString Section;

	if(theAPI->IsConnected())
		Section = theAPI->SbieIniGetEx("GlobalSettings", "");

	m_HoldChange = true;
	ui.txtIniSection->setPlainText(Section);
	m_HoldChange = false;
}

void CSettingsWindow::SaveIniSection()
{
	if(theAPI->IsConnected())
		theAPI->SbieIniSet("GlobalSettings", "", ui.txtIniSection->toPlainText());

	LoadIniSection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update
//

void CSettingsWindow::GetUpdates()
{
	QVariantMap Params;
	Params["channel"] = "all";
	theGUI->m_pUpdater->GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)), Params);
}

QString CSettingsWindow__MkVersion(const QString& Name, const QVariantMap& Releases)
{
	QVariantMap Release = Releases[Name].toMap();
	QString Version = Release.value("version").toString();
	//if (Release["build"].type() != QVariant::Invalid) 
	int iUpdate = Release["update"].toInt();
	if(iUpdate) Version += QChar('a' + (iUpdate - 1));
	return QString("<a href=\"%1\">%2</a>").arg(Name, Version);
}

void CSettingsWindow::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
	if (Data.isEmpty() || Data["error"].toBool())
		return;

	QString Version = QString::number(VERSION_MJR) + "." + QString::number(VERSION_MIN) + "." + QString::number(VERSION_REV);
	int iUpdate = COnlineUpdater::GetCurrentUpdate();
	if(iUpdate) 
		Version += QChar('a' + (iUpdate - 1));

	m_UpdateData = Data;
	QVariantMap Releases = m_UpdateData["releases"].toMap();
	ui.lblCurrent->setText(tr("%1 (Current)").arg(Version));
	ui.lblStable->setText(CSettingsWindow__MkVersion("stable", Releases));
	ui.lblPreview->setText(CSettingsWindow__MkVersion("preview", Releases));
	//if(ui.radInsider->isEnabled())
	//	ui.lblInsider->setText(CSettingsWindow__MkVersion("insider", Releases));
}

void CSettingsWindow::OnUpdate(const QString& Channel)
{
	if (Channel == "check") {
		GetUpdates();
		return;
	}
	
	QVariantMap Releases = m_UpdateData["releases"].toMap();
	QVariantMap Release = Releases[Channel].toMap();

	QString VersionStr = Release["version"].toString();
	if (VersionStr.isEmpty())
		return;

	QVariantMap Installer = Releases["installer"].toMap();
	QString DownloadUrl = Installer["downloadUrl"].toString();
	//QString DownloadSig = Installer["signature"].toString();
	// todo xxx
	//if (!DownloadUrl.isEmpty() /*&& !DownloadSig.isEmpty()*/)
	//{
	//	// todo: signature
	//	if (QMessageBox("Sandboxie-Plus", tr("Do you want to download the installer for v%1?").arg(VersionStr), QMessageBox::Question, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton, this).exec() == QMessageBox::Yes)
	//		COnlineUpdater::Instance()->DownloadInstaller(DownloadUrl, true);
	//}
	//else
	{
		QString InfoUrl = Release["infoUrl"].toString();
		if (InfoUrl.isEmpty())
			InfoUrl = "https://sandboxie-plus.com/go.php?to=sbie-get";
		QDesktopServices::openUrl(InfoUrl);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Support
//

void CSettingsWindow::CertChanged()
{ 
	m_CertChanged = true; 
	QPalette palette = QApplication::palette();
	ui.txtCertificate->setPalette(palette);
	OnOptChanged();
}

void CSettingsWindow::LoadCertificate(QString CertPath)
{
#ifdef _DEBUG
	if (GetKeyState(VK_CONTROL) & 0x8000) {
		g_Certificate.clear();
		return;
	}
#endif

	if (theAPI && theAPI->IsConnected())
		CertPath = theAPI->GetSbiePath() + "\\Certificate.dat";
		
	QFile CertFile(CertPath);
	if (CertFile.open(QFile::ReadOnly)) {
		g_Certificate = CertFile.readAll();
		CertFile.close();
	}
}

void WindowsMoveFile(const QString& From, const QString& To)
{
	std::wstring from = From.toStdWString();
	from.append(L"\0", 1);
	std::wstring to = To.toStdWString();
	to.append(L"\0", 1);

	SHFILEOPSTRUCT SHFileOp;
    memset(&SHFileOp, 0, sizeof(SHFILEOPSTRUCT));
    SHFileOp.hwnd = NULL;
    SHFileOp.wFunc = To.isEmpty() ? FO_DELETE : FO_MOVE;
	SHFileOp.pFrom = from.c_str();
    SHFileOp.pTo = to.c_str();
    SHFileOp.fFlags = NULL;    

    //The Copying Function
    SHFileOperation(&SHFileOp);
}
