/********************************************************************************
** Form generated from reading UI file 'OptionsWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPTIONSWINDOW_H
#define UI_OPTIONSWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OptionsWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabs;
    QWidget *tabGeneral;
    QGridLayout *gridLayout_9;
    QTabWidget *tabsGeneral;
    QWidget *tabOptions;
    QGridLayout *gridLayout_27;
    QGridLayout *gridLayout_8;
    QLabel *label_30;
    QCheckBox *chkPinToTray;
    QLabel *label_20;
    QLabel *label_21;
    QSpacerItem *horizontalSpacer_2;
    QSpinBox *spinBorderWidth;
    QLabel *lblSupportCert;
    QComboBox *cmbBoxIndicator;
    QToolButton *btnBorderColor;
    QLabel *label_58;
    QCheckBox *chkShowForRun;
    QLabel *lblBoxInfo;
    QSpacerItem *verticalSpacer_17;
    QComboBox *cmbBoxType;
    QComboBox *cmbBoxBorder;
    QLabel *label_57;
    QLabel *label_14;
    QLabel *lblBoxType;
    QLabel *lblAppearance;
    QLabel *label_70;
    QComboBox *cmbDblClick;
    QWidget *tabFile;
    QGridLayout *gridLayout_30;
    QGridLayout *gridLayout_28;
    QLabel *lblRawDisk;
    QCheckBox *chkUseVolumeSerialNumbers;
    QLabel *label_37;
    QSpacerItem *horizontalSpacer_4;
    QCheckBox *chkEncrypt;
    QCheckBox *chkAutoEmpty;
    QLabel *lblCrypto;
    QLabel *lblDelete;
    QSpacerItem *verticalSpacer_2;
    QCheckBox *chkRawDiskRead;
    QCheckBox *chkProtectBox;
    QLabel *lblImDisk;
    QCheckBox *chkSeparateUserFolders;
    QLabel *lblStructure;
    QCheckBox *chkRamBox;
    QCheckBox *chkRawDiskNotify;
    QComboBox *cmbVersion;
    QToolButton *btnPassword;
    QLabel *lblScheme;
    QLabel *lblWhenEmpty;
    QWidget *tabMigration;
    QGridLayout *gridLayout_79;
    QGridLayout *gridLayout_78;
    QCheckBox *chkCopyLimit;
    QCheckBox *chkCopyPrompt;
    QLabel *lblMigration;
    QCheckBox *chkNoCopyMsg;
    QLineEdit *txtCopyLimit;
    QLabel *lblCopyLimit;
    QPushButton *btnAddCopy;
    QPushButton *btnDelCopy;
    QSpacerItem *horizontalSpacer_17;
    QSpacerItem *verticalSpacer_37;
    QCheckBox *chkShowCopyTmpl;
    QTreeWidget *treeCopy;
    QLabel *label_31;
    QLabel *label_34;
    QCheckBox *chkNoCopyWarn;
    QCheckBox *chkDenyWrite;
    QWidget *tabRestrictions;
    QGridLayout *gridLayout_37;
    QGridLayout *gridLayout_36;
    QCheckBox *chkOpenCredentials;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *chkCloseClipBoard;
    QSpacerItem *verticalSpacer_12;
    QCheckBox *chkBlockNetParam;
    QCheckBox *chkVmRead;
    QCheckBox *chkBlockSpooler;
    QCheckBox *chkPrintToFile;
    QCheckBox *chkBlockNetShare;
    QCheckBox *chkOpenSpooler;
    QCheckBox *chkOpenProtectedStorage;
    QCheckBox *chkVmReadNotify;
    QLabel *lblOther;
    QLabel *label_39;
    QLabel *lblPrinting;
    QLabel *lblNetwork;
    QWidget *tabRun;
    QGridLayout *gridLayout_33;
    QToolButton *btnDelCmd;
    QToolButton *btnAddCmd;
    QSpacerItem *verticalSpacer_15;
    QTreeWidget *treeRun;
    QToolButton *btnCmdUp;
    QToolButton *btnCmdDown;
    QLabel *label_8;
    QSpacerItem *verticalSpacer_38;
    QWidget *tabSecurity;
    QGridLayout *gridLayout_7;
    QTabWidget *tabsSecurity;
    QWidget *tabHarden;
    QGridLayout *gridLayout_53;
    QGridLayout *gridLayout_52;
    QCheckBox *chkLockDown;
    QCheckBox *chkSecurityMode;
    QLabel *lblElevation;
    QCheckBox *chkFakeElevation;
    QLabel *label_40;
    QCheckBox *chkRestrictDevices;
    QLabel *lblSecurity;
    QCheckBox *chkMsiExemptions;
    QSpacerItem *horizontalSpacer_10;
    QSpacerItem *verticalSpacer_29;
    QCheckBox *chkDropRights;
    QLabel *lblAdmin;
    QLabel *label_28;
    QLabel *label_35;
    QLabel *label_41;
    QWidget *tabIsolation;
    QGridLayout *gridLayout_39;
    QGridLayout *gridLayout_38;
    QSpacerItem *verticalSpacer_25;
    QLabel *label_42;
    QSpacerItem *horizontalSpacer_8;
    QCheckBox *chkNoSecurityIsolation;
    QCheckBox *chkOpenSamEndpoint;
    QCheckBox *chkNoSecurityFiltering;
    QLabel *label_61;
    QLabel *label_60;
    QCheckBox *chkOpenLsaEndpoint;
    QLabel *lblIsolation;
    QLabel *lblAccess;
    QCheckBox *chkOpenDevCMApi;
    QLabel *label_62;
    QWidget *tabPrivate;
    QGridLayout *gridLayout_83;
    QGridLayout *gridLayout_82;
    QSpacerItem *horizontalSpacer_19;
    QPushButton *btnHostProcessDeny;
    QSpacerItem *verticalSpacer_31;
    QPushButton *btnDelHostProcess;
    QCheckBox *chkNotifyProtect;
    QTreeWidget *treeHostProc;
    QSpacerItem *verticalSpacer_40;
    QCheckBox *chkShowHostProcTmpl;
    QLabel *lblBoxProtection;
    QLabel *label_52;
    QPushButton *btnHostProcessAllow;
    QLabel *label_5;
    QLabel *label_47;
    QCheckBox *chkLessConfidential;
    QCheckBox *chkConfidential;
    QWidget *tabPrivileges;
    QGridLayout *gridLayout_26;
    QGridLayout *gridLayout_2;
    QCheckBox *chkSbieLogon;
    QLabel *lblFence;
    QCheckBox *chkElevateRpcss;
    QLabel *lblPrivilege;
    QCheckBox *chkProtectSystem;
    QLabel *lblToken;
    QLabel *label_65;
    QCheckBox *chkDropPrivileges;
    QCheckBox *chkRestrictServices;
    QSpacerItem *verticalSpacer;
    QCheckBox *chkProtectSCM;
    QLabel *label_64;
    QSpacerItem *horizontalSpacer_13;
    QCheckBox *chkAddToJob;
    QLabel *label_74;
    QWidget *tabGroups;
    QGridLayout *gridLayout_18;
    QGridLayout *tabGroups_2;
    QCheckBox *chkShowGroupTmpl;
    QSpacerItem *verticalSpacer_7;
    QPushButton *btnAddGroup;
    QPushButton *btnAddProg;
    QLabel *label_3;
    QTreeWidget *treeGroups;
    QPushButton *btnDelProg;
    QWidget *tabForce;
    QGridLayout *gridLayout_6;
    QTabWidget *tabsForce;
    QWidget *tabForceProgs;
    QGridLayout *gridLayout_54;
    QGridLayout *gridLayout_35;
    QLabel *label;
    QSpacerItem *verticalSpacer_5;
    QToolButton *btnForceDir;
    QCheckBox *chkShowForceTmpl;
    QTreeWidget *treeForced;
    QToolButton *btnForceProg;
    QPushButton *btnDelForce;
    QCheckBox *chkDisableForced;
    QWidget *tabBreakout;
    QGridLayout *gridLayout_13;
    QGridLayout *gridLayout_55;
    QTreeWidget *treeBreakout;
    QPushButton *btnDelBreakout;
    QCheckBox *chkShowBreakoutTmpl;
    QLabel *label_69;
    QLabel *lblBreakOut;
    QToolButton *btnBreakoutDir;
    QToolButton *btnBreakoutProg;
    QSpacerItem *verticalSpacer_26;
    QWidget *tabStop;
    QGridLayout *gridLayout_17;
    QTabWidget *tabsStop;
    QWidget *tabLingerer;
    QGridLayout *gridLayout_57;
    QGridLayout *gridLayout_14;
    QSpacerItem *verticalSpacer_6;
    QPushButton *btnDelStopProg;
    QCheckBox *chkShowStopTmpl;
    QPushButton *btnAddLingering;
    QTreeWidget *treeStop;
    QLabel *label_2;
    QWidget *tabLeader;
    QGridLayout *gridLayout_61;
    QGridLayout *gridLayout_58;
    QTreeWidget *treeLeader;
    QSpacerItem *verticalSpacer_27;
    QPushButton *btnAddLeader;
    QPushButton *btnDelLeader;
    QCheckBox *chkShowLeaderTmpl;
    QLabel *label_71;
    QWidget *tabStart;
    QGridLayout *gridLayout_19;
    QGridLayout *gridLayout_16;
    QCheckBox *chkStartBlockMsg;
    QTreeWidget *treeStart;
    QSpacerItem *verticalSpacer_8;
    QPushButton *btnAddStartProg;
    QPushButton *btnDelStartProg;
    QGridLayout *gridLayout_23;
    QRadioButton *radStartSelected;
    QRadioButton *radStartExcept;
    QRadioButton *radStartAll;
    QLabel *label_4;
    QCheckBox *chkShowStartTmpl;
    QWidget *tabAccess;
    QGridLayout *gridLayout_11;
    QTabWidget *tabsAccess;
    QWidget *tabFiles;
    QGridLayout *gridLayout_72;
    QGridLayout *gridLayout_67;
    QCheckBox *chkShowFilesTmpl;
    QPushButton *btnDelFile;
    QTreeWidget *treeFiles;
    QToolButton *btnAddFile;
    QSpacerItem *verticalSpacer_11;
    QLabel *label_7;
    QWidget *tabKeys;
    QGridLayout *gridLayout_73;
    QGridLayout *gridLayout_68;
    QCheckBox *chkShowKeysTmpl;
    QTreeWidget *treeKeys;
    QPushButton *btnDelKey;
    QToolButton *btnAddKey;
    QSpacerItem *verticalSpacer_18;
    QLabel *label_10;
    QWidget *tabIPC;
    QGridLayout *gridLayout_74;
    QGridLayout *gridLayout_69;
    QToolButton *btnAddIPC;
    QCheckBox *chkShowIPCTmpl;
    QPushButton *btnDelIPC;
    QTreeWidget *treeIPC;
    QSpacerItem *verticalSpacer_21;
    QLabel *label_22;
    QWidget *tabWnd;
    QGridLayout *gridLayout_75;
    QGridLayout *gridLayout_70;
    QPushButton *btnDelWnd;
    QCheckBox *chkShowWndTmpl;
    QToolButton *btnAddWnd;
    QSpacerItem *verticalSpacer_34;
    QTreeWidget *treeWnd;
    QCheckBox *chkNoWindowRename;
    QLabel *label_27;
    QWidget *tabCOM;
    QGridLayout *gridLayout_76;
    QGridLayout *gridLayout_71;
    QPushButton *btnDelCOM;
    QTreeWidget *treeCOM;
    QCheckBox *chkShowCOMTmpl;
    QToolButton *btnAddCOM;
    QSpacerItem *verticalSpacer_35;
    QLabel *label_29;
    QCheckBox *chkOpenCOM;
    QWidget *tabPolicy;
    QGridLayout *gridLayout_51;
    QGridLayout *gridLayout_50;
    QLabel *lblPolicy;
    QCheckBox *chkCloseForBox;
    QCheckBox *chkUseSpecificity;
    QCheckBox *chkNoOpenForBox;
    QSpacerItem *horizontalSpacer_9;
    QLabel *lblMode;
    QLabel *label_54;
    QCheckBox *chkPrivacy;
    QSpacerItem *verticalSpacer_24;
    QLabel *label_55;
    QWidget *tabInternet;
    QGridLayout *gridLayout_21;
    QTabWidget *tabsInternet;
    QWidget *tabINet;
    QGridLayout *gridLayout_46;
    QGridLayout *gridLayout_20;
    QCheckBox *chkINetBlockMsg;
    QPushButton *btnAddINetProg;
    QCheckBox *chkINetBlockPrompt;
    QPushButton *btnDelINetProg;
    QLabel *label_11;
    QSpacerItem *verticalSpacer_9;
    QTreeWidget *treeINet;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_12;
    QComboBox *cmbBlockINet;
    QWidget *tabNetFw;
    QGridLayout *gridLayout_48;
    QGridLayout *gridLayout_47;
    QSpacerItem *verticalSpacer_23;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_48;
    QLineEdit *txtProgFwTest;
    QLabel *label_49;
    QLineEdit *txtPortFwTest;
    QLabel *label_50;
    QLineEdit *txtIPFwTest;
    QLabel *label_51;
    QComboBox *cmbProtFwTest;
    QToolButton *btnClearFwTest;
    QPushButton *btnDelFwRule;
    QPushButton *btnAddFwRule;
    QTreeWidget *treeNetFw;
    QCheckBox *chkShowNetFwTmpl;
    QLabel *lblNoWfp;
    QWidget *tabRecovery;
    QGridLayout *gridLayout_24;
    QTabWidget *tabsRecovery;
    QWidget *tabQuickRecovery;
    QGridLayout *gridLayout_10;
    QGridLayout *gridLayout_22;
    QTreeWidget *treeRecovery;
    QLabel *label_16;
    QSpacerItem *verticalSpacer_13;
    QCheckBox *chkShowRecoveryTmpl;
    QPushButton *btnDelRecovery;
    QPushButton *btnAddRecovery;
    QWidget *tabImmediateRecovery;
    QGridLayout *gridLayout_56;
    QGridLayout *gridLayout_40;
    QLabel *label_9;
    QPushButton *btnAddRecIgnoreExt;
    QCheckBox *chkAutoRecovery;
    QSpacerItem *verticalSpacer_3;
    QTreeWidget *treeRecIgnore;
    QPushButton *btnDelRecIgnore;
    QPushButton *btnAddRecIgnore;
    QCheckBox *chkShowRecIgnoreTmpl;
    QWidget *tabOther;
    QGridLayout *gridLayout_12;
    QTabWidget *tabsOther;
    QWidget *tabCompat;
    QGridLayout *gridLayout_63;
    QGridLayout *gridLayout_62;
    QCheckBox *chkElevateCreateProcessFix;
    QCheckBox *chkUseSbieWndStation;
    QLabel *lblCompatibility;
    QCheckBox *chkComTimeout;
    QCheckBox *chkPreferExternalManifest;
    QCheckBox *chkUseSbieDeskHack;
    QSpacerItem *verticalSpacer_28;
    QSpacerItem *horizontalSpacer_12;
    QCheckBox *chkNestedJobs;
    QCheckBox *chkNoPanic;
    QWidget *tabDlls;
    QGridLayout *gridLayout_77;
    QGridLayout *gridLayout_49;
    QSpacerItem *verticalSpacer_36;
    QLabel *lblProtection;
    QTreeWidget *treeInjectDll;
    QCheckBox *chkHostProtect;
    QCheckBox *chkHostProtectMsg;
    QLabel *label_36;
    QSpacerItem *horizontalSpacer_16;
    QSpacerItem *verticalSpacer_39;
    QWidget *tabAdvanced;
    QGridLayout *gridLayout_121;
    QTabWidget *tabsAdvanced;
    QWidget *tabMisc;
    QGridLayout *gridLayout_59;
    QGridLayout *gridLayout_60;
    QToolButton *btnAddOption;
    QPushButton *btnDelOption;
    QLabel *lblAdvanced;
    QTreeWidget *treeOptions;
    QCheckBox *chkShowOptionsTmpl;
    QSpacerItem *verticalSpacer_30;
    QWidget *tabTriggers;
    QGridLayout *gridLayout_4;
    QToolButton *btnDelAuto;
    QLabel *label_68;
    QToolButton *btnAddAutoRun;
    QLabel *label_66;
    QSpacerItem *verticalSpacer_4;
    QLabel *label_67;
    QTreeWidget *treeTriggers;
    QLabel *label_18;
    QLabel *label_32;
    QToolButton *btnAddRecoveryCmd;
    QLabel *label_33;
    QToolButton *btnAddDeleteCmd;
    QCheckBox *chkShowTriggersTmpl;
    QToolButton *btnAddAutoSvc;
    QToolButton *btnAddAutoExec;
    QToolButton *btnAddTerminateCmd;
    QWidget *tabHideProc;
    QGridLayout *gridLayout_29;
    QPushButton *btnAddProcess;
    QCheckBox *chkHideOtherBoxes;
    QSpacerItem *verticalSpacer_16;
    QLabel *label_24;
    QCheckBox *chkShowHiddenProcTmpl;
    QTreeWidget *treeHideProc;
    QPushButton *btnDelProcess;
    QWidget *tabUsers;
    QGridLayout *gridLayout_25;
    QCheckBox *chkMonitorAdminOnly;
    QPushButton *btnAddUser;
    QSpacerItem *verticalSpacer_14;
    QListWidget *lstUsers;
    QPushButton *btnDelUser;
    QLabel *label_23;
    QWidget *tabTracing;
    QGridLayout *gridLayout_34;
    QGridLayout *gridLayout_32;
    QCheckBox *chkPipeTrace;
    QCheckBox *chkHookTrace;
    QCheckBox *chkErrTrace;
    QLabel *lblMonitor;
    QLabel *label_38;
    QCheckBox *chkKeyTrace;
    QCheckBox *chkFileTrace;
    QCheckBox *chkIpcTrace;
    QCheckBox *chkDbgTrace;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_3;
    QCheckBox *chkNetFwTrace;
    QCheckBox *chkGuiTrace;
    QLabel *label_26;
    QCheckBox *chkDisableMonitor;
    QLabel *lblTracing;
    QSpacerItem *verticalSpacer_19;
    QCheckBox *chkComTrace;
    QCheckBox *chkCallTrace;
    QWidget *tabDebug;
    QGridLayout *gridLayout_31;
    QScrollArea *scrollArea;
    QWidget *dbgWidget;
    QGridLayout *dbgLayout;
    QLabel *label_25;
    QLabel *label_17;
    QWidget *tabTemplates;
    QGridLayout *gridLayout_5;
    QTabWidget *tabsTemplates;
    QWidget *tab_11;
    QGridLayout *gridLayout_41;
    QGridLayout *gridLayout_3;
    QComboBox *cmbCategories;
    QLabel *label_13;
    QSpacerItem *verticalSpacer_20;
    QLabel *label_15;
    QLineEdit *txtTemplates;
    QToolButton *btnAddTemplate;
    QLabel *label_6;
    QTreeWidget *treeTemplates;
    QToolButton *btnDelTemplate;
    QWidget *tab_12;
    QGridLayout *gridLayout_45;
    QGridLayout *gridLayout_42;
    QLabel *label_46;
    QSpacerItem *verticalSpacer_22;
    QTreeWidget *treeFolders;
    QWidget *tab_13;
    QGridLayout *gridLayout_44;
    QGridLayout *gridLayout_43;
    QCheckBox *chkScreenReaders;
    QLabel *label_45;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *horizontalSpacer_11;
    QLabel *label_43;
    QSpacerItem *horizontalSpacer_15;
    QSpacerItem *horizontalSpacer_14;
    QLabel *label_44;
    QWidget *tabEdit;
    QGridLayout *gridLayout;
    QPushButton *btnEditIni;
    QPushButton *btnCancelEdit;
    QSpacerItem *horizontalSpacer_7;
    QPushButton *btnSaveIni;
    QPlainTextEdit *txtIniSection;
    QHBoxLayout *horizontalLayout;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *OptionsWindow)
    {
        if (OptionsWindow->objectName().isEmpty())
            OptionsWindow->setObjectName(QString::fromUtf8("OptionsWindow"));
        OptionsWindow->resize(785, 526);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OptionsWindow->sizePolicy().hasHeightForWidth());
        OptionsWindow->setSizePolicy(sizePolicy);
        OptionsWindow->setMinimumSize(QSize(0, 0));
        OptionsWindow->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(OptionsWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tabs = new QTabWidget(OptionsWindow);
        tabs->setObjectName(QString::fromUtf8("tabs"));
        QFont font;
        font.setKerning(true);
        tabs->setFont(font);
        tabs->setTabPosition(QTabWidget::North);
        tabGeneral = new QWidget();
        tabGeneral->setObjectName(QString::fromUtf8("tabGeneral"));
        gridLayout_9 = new QGridLayout(tabGeneral);
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        tabsGeneral = new QTabWidget(tabGeneral);
        tabsGeneral->setObjectName(QString::fromUtf8("tabsGeneral"));
        tabOptions = new QWidget();
        tabOptions->setObjectName(QString::fromUtf8("tabOptions"));
        gridLayout_27 = new QGridLayout(tabOptions);
        gridLayout_27->setObjectName(QString::fromUtf8("gridLayout_27"));
        gridLayout_8 = new QGridLayout();
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        label_30 = new QLabel(tabOptions);
        label_30->setObjectName(QString::fromUtf8("label_30"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_30->sizePolicy().hasHeightForWidth());
        label_30->setSizePolicy(sizePolicy1);
        label_30->setMaximumSize(QSize(16, 16777215));

        gridLayout_8->addWidget(label_30, 0, 1, 1, 1);

        chkPinToTray = new QCheckBox(tabOptions);
        chkPinToTray->setObjectName(QString::fromUtf8("chkPinToTray"));

        gridLayout_8->addWidget(chkPinToTray, 4, 1, 1, 7);

        label_20 = new QLabel(tabOptions);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_8->addWidget(label_20, 1, 0, 1, 3);

        label_21 = new QLabel(tabOptions);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_8->addWidget(label_21, 2, 0, 1, 3);

        horizontalSpacer_2 = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_8->addItem(horizontalSpacer_2, 11, 2, 1, 5);

        spinBorderWidth = new QSpinBox(tabOptions);
        spinBorderWidth->setObjectName(QString::fromUtf8("spinBorderWidth"));
        spinBorderWidth->setMinimum(1);
        spinBorderWidth->setMaximum(10);
        spinBorderWidth->setValue(1);

        gridLayout_8->addWidget(spinBorderWidth, 2, 6, 1, 1);

        lblSupportCert = new QLabel(tabOptions);
        lblSupportCert->setObjectName(QString::fromUtf8("lblSupportCert"));
        lblSupportCert->setTextFormat(Qt::RichText);
        lblSupportCert->setWordWrap(true);
        lblSupportCert->setOpenExternalLinks(true);

        gridLayout_8->addWidget(lblSupportCert, 10, 1, 1, 6);

        cmbBoxIndicator = new QComboBox(tabOptions);
        cmbBoxIndicator->setObjectName(QString::fromUtf8("cmbBoxIndicator"));

        gridLayout_8->addWidget(cmbBoxIndicator, 1, 3, 1, 2);

        btnBorderColor = new QToolButton(tabOptions);
        btnBorderColor->setObjectName(QString::fromUtf8("btnBorderColor"));

        gridLayout_8->addWidget(btnBorderColor, 2, 5, 1, 1);

        label_58 = new QLabel(tabOptions);
        label_58->setObjectName(QString::fromUtf8("label_58"));

        gridLayout_8->addWidget(label_58, 8, 1, 1, 6);

        chkShowForRun = new QCheckBox(tabOptions);
        chkShowForRun->setObjectName(QString::fromUtf8("chkShowForRun"));

        gridLayout_8->addWidget(chkShowForRun, 3, 1, 1, 7);

        lblBoxInfo = new QLabel(tabOptions);
        lblBoxInfo->setObjectName(QString::fromUtf8("lblBoxInfo"));
        lblBoxInfo->setTextFormat(Qt::AutoText);
        lblBoxInfo->setWordWrap(true);
        lblBoxInfo->setOpenExternalLinks(true);

        gridLayout_8->addWidget(lblBoxInfo, 9, 1, 1, 6);

        verticalSpacer_17 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_8->addItem(verticalSpacer_17, 11, 1, 1, 1);

        cmbBoxType = new QComboBox(tabOptions);
        cmbBoxType->setObjectName(QString::fromUtf8("cmbBoxType"));

        gridLayout_8->addWidget(cmbBoxType, 7, 3, 1, 2);

        cmbBoxBorder = new QComboBox(tabOptions);
        cmbBoxBorder->setObjectName(QString::fromUtf8("cmbBoxBorder"));

        gridLayout_8->addWidget(cmbBoxBorder, 2, 3, 1, 2);

        label_57 = new QLabel(tabOptions);
        label_57->setObjectName(QString::fromUtf8("label_57"));
        label_57->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_8->addWidget(label_57, 7, 0, 1, 3);

        label_14 = new QLabel(tabOptions);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_8->addWidget(label_14, 2, 7, 1, 1);

        lblBoxType = new QLabel(tabOptions);
        lblBoxType->setObjectName(QString::fromUtf8("lblBoxType"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        font1.setKerning(true);
        lblBoxType->setFont(font1);

        gridLayout_8->addWidget(lblBoxType, 6, 0, 1, 2);

        lblAppearance = new QLabel(tabOptions);
        lblAppearance->setObjectName(QString::fromUtf8("lblAppearance"));
        lblAppearance->setFont(font1);

        gridLayout_8->addWidget(lblAppearance, 0, 0, 1, 1);

        label_70 = new QLabel(tabOptions);
        label_70->setObjectName(QString::fromUtf8("label_70"));
        label_70->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_8->addWidget(label_70, 5, 0, 1, 3);

        cmbDblClick = new QComboBox(tabOptions);
        cmbDblClick->setObjectName(QString::fromUtf8("cmbDblClick"));
        cmbDblClick->setEditable(true);

        gridLayout_8->addWidget(cmbDblClick, 5, 3, 1, 4);


        gridLayout_27->addLayout(gridLayout_8, 1, 0, 1, 1);

        tabsGeneral->addTab(tabOptions, QString());
        tabFile = new QWidget();
        tabFile->setObjectName(QString::fromUtf8("tabFile"));
        gridLayout_30 = new QGridLayout(tabFile);
        gridLayout_30->setObjectName(QString::fromUtf8("gridLayout_30"));
        gridLayout_28 = new QGridLayout();
        gridLayout_28->setObjectName(QString::fromUtf8("gridLayout_28"));
        lblRawDisk = new QLabel(tabFile);
        lblRawDisk->setObjectName(QString::fromUtf8("lblRawDisk"));
        lblRawDisk->setFont(font1);

        gridLayout_28->addWidget(lblRawDisk, 11, 0, 1, 2);

        chkUseVolumeSerialNumbers = new QCheckBox(tabFile);
        chkUseVolumeSerialNumbers->setObjectName(QString::fromUtf8("chkUseVolumeSerialNumbers"));

        gridLayout_28->addWidget(chkUseVolumeSerialNumbers, 3, 1, 1, 5);

        label_37 = new QLabel(tabFile);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        label_37->setMaximumSize(QSize(20, 16777215));

        gridLayout_28->addWidget(label_37, 13, 1, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_28->addItem(horizontalSpacer_4, 14, 2, 1, 5);

        chkEncrypt = new QCheckBox(tabFile);
        chkEncrypt->setObjectName(QString::fromUtf8("chkEncrypt"));

        gridLayout_28->addWidget(chkEncrypt, 5, 1, 1, 4);

        chkAutoEmpty = new QCheckBox(tabFile);
        chkAutoEmpty->setObjectName(QString::fromUtf8("chkAutoEmpty"));

        gridLayout_28->addWidget(chkAutoEmpty, 9, 1, 1, 6);

        lblCrypto = new QLabel(tabFile);
        lblCrypto->setObjectName(QString::fromUtf8("lblCrypto"));
        lblCrypto->setWordWrap(true);
        lblCrypto->setOpenExternalLinks(true);

        gridLayout_28->addWidget(lblCrypto, 6, 2, 1, 4);

        lblDelete = new QLabel(tabFile);
        lblDelete->setObjectName(QString::fromUtf8("lblDelete"));
        lblDelete->setFont(font1);

        gridLayout_28->addWidget(lblDelete, 8, 0, 1, 3);

        verticalSpacer_2 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_28->addItem(verticalSpacer_2, 14, 1, 1, 1);

        chkRawDiskRead = new QCheckBox(tabFile);
        chkRawDiskRead->setObjectName(QString::fromUtf8("chkRawDiskRead"));

        gridLayout_28->addWidget(chkRawDiskRead, 12, 1, 1, 6);

        chkProtectBox = new QCheckBox(tabFile);
        chkProtectBox->setObjectName(QString::fromUtf8("chkProtectBox"));
        chkProtectBox->setTristate(true);

        gridLayout_28->addWidget(chkProtectBox, 10, 1, 1, 6);

        lblImDisk = new QLabel(tabFile);
        lblImDisk->setObjectName(QString::fromUtf8("lblImDisk"));

        gridLayout_28->addWidget(lblImDisk, 7, 2, 1, 4);

        chkSeparateUserFolders = new QCheckBox(tabFile);
        chkSeparateUserFolders->setObjectName(QString::fromUtf8("chkSeparateUserFolders"));
        chkSeparateUserFolders->setTristate(false);

        gridLayout_28->addWidget(chkSeparateUserFolders, 2, 1, 1, 5);

        lblStructure = new QLabel(tabFile);
        lblStructure->setObjectName(QString::fromUtf8("lblStructure"));
        lblStructure->setFont(font1);

        gridLayout_28->addWidget(lblStructure, 0, 0, 1, 1);

        chkRamBox = new QCheckBox(tabFile);
        chkRamBox->setObjectName(QString::fromUtf8("chkRamBox"));

        gridLayout_28->addWidget(chkRamBox, 4, 1, 1, 4);

        chkRawDiskNotify = new QCheckBox(tabFile);
        chkRawDiskNotify->setObjectName(QString::fromUtf8("chkRawDiskNotify"));

        gridLayout_28->addWidget(chkRawDiskNotify, 13, 2, 1, 5);

        cmbVersion = new QComboBox(tabFile);
        cmbVersion->setObjectName(QString::fromUtf8("cmbVersion"));

        gridLayout_28->addWidget(cmbVersion, 1, 3, 1, 1);

        btnPassword = new QToolButton(tabFile);
        btnPassword->setObjectName(QString::fromUtf8("btnPassword"));

        gridLayout_28->addWidget(btnPassword, 5, 5, 1, 1);

        lblScheme = new QLabel(tabFile);
        lblScheme->setObjectName(QString::fromUtf8("lblScheme"));

        gridLayout_28->addWidget(lblScheme, 1, 1, 1, 2);

        lblWhenEmpty = new QLabel(tabFile);
        lblWhenEmpty->setObjectName(QString::fromUtf8("lblWhenEmpty"));

        gridLayout_28->addWidget(lblWhenEmpty, 0, 2, 1, 4);


        gridLayout_30->addLayout(gridLayout_28, 0, 0, 1, 1);

        tabsGeneral->addTab(tabFile, QString());
        tabMigration = new QWidget();
        tabMigration->setObjectName(QString::fromUtf8("tabMigration"));
        gridLayout_79 = new QGridLayout(tabMigration);
        gridLayout_79->setObjectName(QString::fromUtf8("gridLayout_79"));
        gridLayout_79->setContentsMargins(9, 9, 9, 9);
        gridLayout_78 = new QGridLayout();
        gridLayout_78->setObjectName(QString::fromUtf8("gridLayout_78"));
        chkCopyLimit = new QCheckBox(tabMigration);
        chkCopyLimit->setObjectName(QString::fromUtf8("chkCopyLimit"));

        gridLayout_78->addWidget(chkCopyLimit, 2, 1, 1, 1);

        chkCopyPrompt = new QCheckBox(tabMigration);
        chkCopyPrompt->setObjectName(QString::fromUtf8("chkCopyPrompt"));

        gridLayout_78->addWidget(chkCopyPrompt, 3, 1, 1, 2);

        lblMigration = new QLabel(tabMigration);
        lblMigration->setObjectName(QString::fromUtf8("lblMigration"));
        lblMigration->setFont(font1);

        gridLayout_78->addWidget(lblMigration, 1, 0, 1, 1);

        chkNoCopyMsg = new QCheckBox(tabMigration);
        chkNoCopyMsg->setObjectName(QString::fromUtf8("chkNoCopyMsg"));

        gridLayout_78->addWidget(chkNoCopyMsg, 11, 0, 1, 6);

        txtCopyLimit = new QLineEdit(tabMigration);
        txtCopyLimit->setObjectName(QString::fromUtf8("txtCopyLimit"));
        txtCopyLimit->setMaximumSize(QSize(100, 16777215));

        gridLayout_78->addWidget(txtCopyLimit, 2, 2, 1, 1);

        lblCopyLimit = new QLabel(tabMigration);
        lblCopyLimit->setObjectName(QString::fromUtf8("lblCopyLimit"));

        gridLayout_78->addWidget(lblCopyLimit, 2, 3, 1, 1);

        btnAddCopy = new QPushButton(tabMigration);
        btnAddCopy->setObjectName(QString::fromUtf8("btnAddCopy"));

        gridLayout_78->addWidget(btnAddCopy, 7, 6, 1, 1);

        btnDelCopy = new QPushButton(tabMigration);
        btnDelCopy->setObjectName(QString::fromUtf8("btnDelCopy"));

        gridLayout_78->addWidget(btnDelCopy, 10, 6, 1, 1);

        horizontalSpacer_17 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_78->addItem(horizontalSpacer_17, 5, 4, 1, 2);

        verticalSpacer_37 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_78->addItem(verticalSpacer_37, 8, 6, 1, 1);

        chkShowCopyTmpl = new QCheckBox(tabMigration);
        chkShowCopyTmpl->setObjectName(QString::fromUtf8("chkShowCopyTmpl"));

        gridLayout_78->addWidget(chkShowCopyTmpl, 9, 6, 1, 1);

        treeCopy = new QTreeWidget(tabMigration);
        treeCopy->setObjectName(QString::fromUtf8("treeCopy"));
        treeCopy->setSortingEnabled(true);

        gridLayout_78->addWidget(treeCopy, 7, 0, 4, 6);

        label_31 = new QLabel(tabMigration);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        label_31->setWordWrap(true);

        gridLayout_78->addWidget(label_31, 0, 0, 1, 7);

        label_34 = new QLabel(tabMigration);
        label_34->setObjectName(QString::fromUtf8("label_34"));

        gridLayout_78->addWidget(label_34, 6, 0, 1, 6);

        chkNoCopyWarn = new QCheckBox(tabMigration);
        chkNoCopyWarn->setObjectName(QString::fromUtf8("chkNoCopyWarn"));

        gridLayout_78->addWidget(chkNoCopyWarn, 3, 3, 1, 4);

        chkDenyWrite = new QCheckBox(tabMigration);
        chkDenyWrite->setObjectName(QString::fromUtf8("chkDenyWrite"));

        gridLayout_78->addWidget(chkDenyWrite, 4, 1, 1, 5);


        gridLayout_79->addLayout(gridLayout_78, 0, 0, 1, 1);

        tabsGeneral->addTab(tabMigration, QString());
        tabRestrictions = new QWidget();
        tabRestrictions->setObjectName(QString::fromUtf8("tabRestrictions"));
        gridLayout_37 = new QGridLayout(tabRestrictions);
        gridLayout_37->setObjectName(QString::fromUtf8("gridLayout_37"));
        gridLayout_36 = new QGridLayout();
        gridLayout_36->setObjectName(QString::fromUtf8("gridLayout_36"));
        chkOpenCredentials = new QCheckBox(tabRestrictions);
        chkOpenCredentials->setObjectName(QString::fromUtf8("chkOpenCredentials"));

        gridLayout_36->addWidget(chkOpenCredentials, 9, 2, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_36->addItem(horizontalSpacer_5, 13, 2, 1, 1);

        chkCloseClipBoard = new QCheckBox(tabRestrictions);
        chkCloseClipBoard->setObjectName(QString::fromUtf8("chkCloseClipBoard"));

        gridLayout_36->addWidget(chkCloseClipBoard, 10, 1, 1, 2);

        verticalSpacer_12 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_36->addItem(verticalSpacer_12, 13, 0, 1, 1);

        chkBlockNetParam = new QCheckBox(tabRestrictions);
        chkBlockNetParam->setObjectName(QString::fromUtf8("chkBlockNetParam"));

        gridLayout_36->addWidget(chkBlockNetParam, 2, 1, 1, 2);

        chkVmRead = new QCheckBox(tabRestrictions);
        chkVmRead->setObjectName(QString::fromUtf8("chkVmRead"));

        gridLayout_36->addWidget(chkVmRead, 11, 1, 1, 2);

        chkBlockSpooler = new QCheckBox(tabRestrictions);
        chkBlockSpooler->setObjectName(QString::fromUtf8("chkBlockSpooler"));

        gridLayout_36->addWidget(chkBlockSpooler, 4, 1, 1, 2);

        chkPrintToFile = new QCheckBox(tabRestrictions);
        chkPrintToFile->setObjectName(QString::fromUtf8("chkPrintToFile"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(chkPrintToFile->sizePolicy().hasHeightForWidth());
        chkPrintToFile->setSizePolicy(sizePolicy2);

        gridLayout_36->addWidget(chkPrintToFile, 6, 2, 1, 1);

        chkBlockNetShare = new QCheckBox(tabRestrictions);
        chkBlockNetShare->setObjectName(QString::fromUtf8("chkBlockNetShare"));

        gridLayout_36->addWidget(chkBlockNetShare, 1, 1, 1, 2);

        chkOpenSpooler = new QCheckBox(tabRestrictions);
        chkOpenSpooler->setObjectName(QString::fromUtf8("chkOpenSpooler"));

        gridLayout_36->addWidget(chkOpenSpooler, 5, 2, 1, 1);

        chkOpenProtectedStorage = new QCheckBox(tabRestrictions);
        chkOpenProtectedStorage->setObjectName(QString::fromUtf8("chkOpenProtectedStorage"));

        gridLayout_36->addWidget(chkOpenProtectedStorage, 8, 1, 1, 2);

        chkVmReadNotify = new QCheckBox(tabRestrictions);
        chkVmReadNotify->setObjectName(QString::fromUtf8("chkVmReadNotify"));

        gridLayout_36->addWidget(chkVmReadNotify, 12, 2, 1, 1);

        lblOther = new QLabel(tabRestrictions);
        lblOther->setObjectName(QString::fromUtf8("lblOther"));
        lblOther->setFont(font1);

        gridLayout_36->addWidget(lblOther, 7, 0, 1, 2);

        label_39 = new QLabel(tabRestrictions);
        label_39->setObjectName(QString::fromUtf8("label_39"));
        label_39->setMinimumSize(QSize(20, 0));
        label_39->setMaximumSize(QSize(20, 16777215));

        gridLayout_36->addWidget(label_39, 9, 1, 1, 1);

        lblPrinting = new QLabel(tabRestrictions);
        lblPrinting->setObjectName(QString::fromUtf8("lblPrinting"));
        lblPrinting->setFont(font1);

        gridLayout_36->addWidget(lblPrinting, 3, 0, 1, 2);

        lblNetwork = new QLabel(tabRestrictions);
        lblNetwork->setObjectName(QString::fromUtf8("lblNetwork"));
        lblNetwork->setFont(font1);

        gridLayout_36->addWidget(lblNetwork, 0, 0, 1, 2);


        gridLayout_37->addLayout(gridLayout_36, 0, 0, 1, 1);

        tabsGeneral->addTab(tabRestrictions, QString());
        tabRun = new QWidget();
        tabRun->setObjectName(QString::fromUtf8("tabRun"));
        gridLayout_33 = new QGridLayout(tabRun);
        gridLayout_33->setObjectName(QString::fromUtf8("gridLayout_33"));
        gridLayout_33->setContentsMargins(3, 6, 3, 3);
        btnDelCmd = new QToolButton(tabRun);
        btnDelCmd->setObjectName(QString::fromUtf8("btnDelCmd"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(btnDelCmd->sizePolicy().hasHeightForWidth());
        btnDelCmd->setSizePolicy(sizePolicy3);
        btnDelCmd->setMinimumSize(QSize(0, 23));

        gridLayout_33->addWidget(btnDelCmd, 7, 1, 1, 1);

        btnAddCmd = new QToolButton(tabRun);
        btnAddCmd->setObjectName(QString::fromUtf8("btnAddCmd"));
        sizePolicy3.setHeightForWidth(btnAddCmd->sizePolicy().hasHeightForWidth());
        btnAddCmd->setSizePolicy(sizePolicy3);
        btnAddCmd->setMinimumSize(QSize(0, 23));

        gridLayout_33->addWidget(btnAddCmd, 1, 1, 1, 1);

        verticalSpacer_15 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_33->addItem(verticalSpacer_15, 3, 1, 1, 1);

        treeRun = new QTreeWidget(tabRun);
        treeRun->setObjectName(QString::fromUtf8("treeRun"));
        treeRun->setSortingEnabled(true);

        gridLayout_33->addWidget(treeRun, 1, 0, 7, 1);

        btnCmdUp = new QToolButton(tabRun);
        btnCmdUp->setObjectName(QString::fromUtf8("btnCmdUp"));
        sizePolicy3.setHeightForWidth(btnCmdUp->sizePolicy().hasHeightForWidth());
        btnCmdUp->setSizePolicy(sizePolicy3);
        btnCmdUp->setMinimumSize(QSize(0, 23));

        gridLayout_33->addWidget(btnCmdUp, 4, 1, 1, 1);

        btnCmdDown = new QToolButton(tabRun);
        btnCmdDown->setObjectName(QString::fromUtf8("btnCmdDown"));
        sizePolicy3.setHeightForWidth(btnCmdDown->sizePolicy().hasHeightForWidth());
        btnCmdDown->setSizePolicy(sizePolicy3);
        btnCmdDown->setMinimumSize(QSize(0, 23));

        gridLayout_33->addWidget(btnCmdDown, 5, 1, 1, 1);

        label_8 = new QLabel(tabRun);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setWordWrap(true);

        gridLayout_33->addWidget(label_8, 0, 0, 1, 1);

        verticalSpacer_38 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_33->addItem(verticalSpacer_38, 6, 1, 1, 1);

        tabsGeneral->addTab(tabRun, QString());

        gridLayout_9->addWidget(tabsGeneral, 0, 0, 1, 1);

        tabs->addTab(tabGeneral, QString());
        tabSecurity = new QWidget();
        tabSecurity->setObjectName(QString::fromUtf8("tabSecurity"));
        gridLayout_7 = new QGridLayout(tabSecurity);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        tabsSecurity = new QTabWidget(tabSecurity);
        tabsSecurity->setObjectName(QString::fromUtf8("tabsSecurity"));
        tabHarden = new QWidget();
        tabHarden->setObjectName(QString::fromUtf8("tabHarden"));
        gridLayout_53 = new QGridLayout(tabHarden);
        gridLayout_53->setObjectName(QString::fromUtf8("gridLayout_53"));
        gridLayout_52 = new QGridLayout();
        gridLayout_52->setObjectName(QString::fromUtf8("gridLayout_52"));
        chkLockDown = new QCheckBox(tabHarden);
        chkLockDown->setObjectName(QString::fromUtf8("chkLockDown"));

        gridLayout_52->addWidget(chkLockDown, 2, 2, 1, 4);

        chkSecurityMode = new QCheckBox(tabHarden);
        chkSecurityMode->setObjectName(QString::fromUtf8("chkSecurityMode"));

        gridLayout_52->addWidget(chkSecurityMode, 1, 1, 1, 5);

        lblElevation = new QLabel(tabHarden);
        lblElevation->setObjectName(QString::fromUtf8("lblElevation"));
        lblElevation->setFont(font1);

        gridLayout_52->addWidget(lblElevation, 4, 0, 1, 3);

        chkFakeElevation = new QCheckBox(tabHarden);
        chkFakeElevation->setObjectName(QString::fromUtf8("chkFakeElevation"));

        gridLayout_52->addWidget(chkFakeElevation, 7, 2, 1, 4);

        label_40 = new QLabel(tabHarden);
        label_40->setObjectName(QString::fromUtf8("label_40"));
        label_40->setFont(font1);

        gridLayout_52->addWidget(label_40, 6, 4, 1, 1);

        chkRestrictDevices = new QCheckBox(tabHarden);
        chkRestrictDevices->setObjectName(QString::fromUtf8("chkRestrictDevices"));

        gridLayout_52->addWidget(chkRestrictDevices, 3, 2, 1, 4);

        lblSecurity = new QLabel(tabHarden);
        lblSecurity->setObjectName(QString::fromUtf8("lblSecurity"));
        lblSecurity->setFont(font1);

        gridLayout_52->addWidget(lblSecurity, 0, 0, 1, 3);

        chkMsiExemptions = new QCheckBox(tabHarden);
        chkMsiExemptions->setObjectName(QString::fromUtf8("chkMsiExemptions"));

        gridLayout_52->addWidget(chkMsiExemptions, 9, 1, 1, 5);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_52->addItem(horizontalSpacer_10, 11, 3, 1, 3);

        verticalSpacer_29 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_52->addItem(verticalSpacer_29, 11, 1, 1, 1);

        chkDropRights = new QCheckBox(tabHarden);
        chkDropRights->setObjectName(QString::fromUtf8("chkDropRights"));

        gridLayout_52->addWidget(chkDropRights, 6, 1, 1, 3);

        lblAdmin = new QLabel(tabHarden);
        lblAdmin->setObjectName(QString::fromUtf8("lblAdmin"));
        lblAdmin->setFont(font1);
        lblAdmin->setWordWrap(true);

        gridLayout_52->addWidget(lblAdmin, 8, 1, 1, 4);

        label_28 = new QLabel(tabHarden);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        label_28->setWordWrap(true);

        gridLayout_52->addWidget(label_28, 10, 2, 1, 3);

        label_35 = new QLabel(tabHarden);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        label_35->setFont(font1);
        label_35->setWordWrap(true);

        gridLayout_52->addWidget(label_35, 5, 1, 1, 4);

        label_41 = new QLabel(tabHarden);
        label_41->setObjectName(QString::fromUtf8("label_41"));
        label_41->setMinimumSize(QSize(20, 0));
        label_41->setMaximumSize(QSize(20, 16777215));

        gridLayout_52->addWidget(label_41, 10, 1, 1, 1);


        gridLayout_53->addLayout(gridLayout_52, 0, 0, 1, 1);

        tabsSecurity->addTab(tabHarden, QString());
        tabIsolation = new QWidget();
        tabIsolation->setObjectName(QString::fromUtf8("tabIsolation"));
        gridLayout_39 = new QGridLayout(tabIsolation);
        gridLayout_39->setObjectName(QString::fromUtf8("gridLayout_39"));
        gridLayout_38 = new QGridLayout();
        gridLayout_38->setObjectName(QString::fromUtf8("gridLayout_38"));
        verticalSpacer_25 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_38->addItem(verticalSpacer_25, 11, 1, 1, 1);

        label_42 = new QLabel(tabIsolation);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        label_42->setWordWrap(true);

        gridLayout_38->addWidget(label_42, 0, 0, 1, 3);

        horizontalSpacer_8 = new QSpacerItem(40, 5, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_38->addItem(horizontalSpacer_8, 11, 2, 1, 1);

        chkNoSecurityIsolation = new QCheckBox(tabIsolation);
        chkNoSecurityIsolation->setObjectName(QString::fromUtf8("chkNoSecurityIsolation"));

        gridLayout_38->addWidget(chkNoSecurityIsolation, 2, 1, 1, 2);

        chkOpenSamEndpoint = new QCheckBox(tabIsolation);
        chkOpenSamEndpoint->setObjectName(QString::fromUtf8("chkOpenSamEndpoint"));

        gridLayout_38->addWidget(chkOpenSamEndpoint, 9, 1, 1, 2);

        chkNoSecurityFiltering = new QCheckBox(tabIsolation);
        chkNoSecurityFiltering->setObjectName(QString::fromUtf8("chkNoSecurityFiltering"));

        gridLayout_38->addWidget(chkNoSecurityFiltering, 4, 1, 1, 2);

        label_61 = new QLabel(tabIsolation);
        label_61->setObjectName(QString::fromUtf8("label_61"));
        label_61->setWordWrap(true);

        gridLayout_38->addWidget(label_61, 5, 2, 1, 1);

        label_60 = new QLabel(tabIsolation);
        label_60->setObjectName(QString::fromUtf8("label_60"));
        label_60->setWordWrap(true);

        gridLayout_38->addWidget(label_60, 3, 2, 1, 1);

        chkOpenLsaEndpoint = new QCheckBox(tabIsolation);
        chkOpenLsaEndpoint->setObjectName(QString::fromUtf8("chkOpenLsaEndpoint"));

        gridLayout_38->addWidget(chkOpenLsaEndpoint, 10, 1, 1, 2);

        lblIsolation = new QLabel(tabIsolation);
        lblIsolation->setObjectName(QString::fromUtf8("lblIsolation"));
        lblIsolation->setFont(font1);

        gridLayout_38->addWidget(lblIsolation, 1, 0, 1, 3);

        lblAccess = new QLabel(tabIsolation);
        lblAccess->setObjectName(QString::fromUtf8("lblAccess"));
        lblAccess->setFont(font1);

        gridLayout_38->addWidget(lblAccess, 6, 0, 1, 1);

        chkOpenDevCMApi = new QCheckBox(tabIsolation);
        chkOpenDevCMApi->setObjectName(QString::fromUtf8("chkOpenDevCMApi"));

        gridLayout_38->addWidget(chkOpenDevCMApi, 8, 1, 1, 2);

        label_62 = new QLabel(tabIsolation);
        label_62->setObjectName(QString::fromUtf8("label_62"));
        label_62->setWordWrap(true);

        gridLayout_38->addWidget(label_62, 7, 1, 1, 2);


        gridLayout_39->addLayout(gridLayout_38, 0, 0, 1, 1);

        tabsSecurity->addTab(tabIsolation, QString());
        tabPrivate = new QWidget();
        tabPrivate->setObjectName(QString::fromUtf8("tabPrivate"));
        gridLayout_83 = new QGridLayout(tabPrivate);
        gridLayout_83->setObjectName(QString::fromUtf8("gridLayout_83"));
        gridLayout_82 = new QGridLayout();
        gridLayout_82->setObjectName(QString::fromUtf8("gridLayout_82"));
        horizontalSpacer_19 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_82->addItem(horizontalSpacer_19, 1, 2, 1, 1);

        btnHostProcessDeny = new QPushButton(tabPrivate);
        btnHostProcessDeny->setObjectName(QString::fromUtf8("btnHostProcessDeny"));

        gridLayout_82->addWidget(btnHostProcessDeny, 7, 3, 1, 1);

        verticalSpacer_31 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_82->addItem(verticalSpacer_31, 8, 3, 1, 1);

        btnDelHostProcess = new QPushButton(tabPrivate);
        btnDelHostProcess->setObjectName(QString::fromUtf8("btnDelHostProcess"));

        gridLayout_82->addWidget(btnDelHostProcess, 10, 3, 1, 1);

        chkNotifyProtect = new QCheckBox(tabPrivate);
        chkNotifyProtect->setObjectName(QString::fromUtf8("chkNotifyProtect"));

        gridLayout_82->addWidget(chkNotifyProtect, 11, 0, 1, 4);

        treeHostProc = new QTreeWidget(tabPrivate);
        treeHostProc->headerItem()->setText(2, QString());
        treeHostProc->setObjectName(QString::fromUtf8("treeHostProc"));
        treeHostProc->setSortingEnabled(true);

        gridLayout_82->addWidget(treeHostProc, 6, 0, 5, 3);

        verticalSpacer_40 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_82->addItem(verticalSpacer_40, 4, 3, 1, 1);

        chkShowHostProcTmpl = new QCheckBox(tabPrivate);
        chkShowHostProcTmpl->setObjectName(QString::fromUtf8("chkShowHostProcTmpl"));

        gridLayout_82->addWidget(chkShowHostProcTmpl, 9, 3, 1, 1);

        lblBoxProtection = new QLabel(tabPrivate);
        lblBoxProtection->setObjectName(QString::fromUtf8("lblBoxProtection"));
        lblBoxProtection->setFont(font1);

        gridLayout_82->addWidget(lblBoxProtection, 1, 0, 1, 1);

        label_52 = new QLabel(tabPrivate);
        label_52->setObjectName(QString::fromUtf8("label_52"));
        label_52->setWordWrap(true);

        gridLayout_82->addWidget(label_52, 0, 0, 1, 4);

        btnHostProcessAllow = new QPushButton(tabPrivate);
        btnHostProcessAllow->setObjectName(QString::fromUtf8("btnHostProcessAllow"));

        gridLayout_82->addWidget(btnHostProcessAllow, 6, 3, 1, 1);

        label_5 = new QLabel(tabPrivate);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setWordWrap(false);

        gridLayout_82->addWidget(label_5, 5, 0, 1, 3);

        label_47 = new QLabel(tabPrivate);
        label_47->setObjectName(QString::fromUtf8("label_47"));
        label_47->setMinimumSize(QSize(20, 0));
        label_47->setMaximumSize(QSize(20, 16777215));

        gridLayout_82->addWidget(label_47, 3, 1, 1, 1);

        chkLessConfidential = new QCheckBox(tabPrivate);
        chkLessConfidential->setObjectName(QString::fromUtf8("chkLessConfidential"));

        gridLayout_82->addWidget(chkLessConfidential, 3, 2, 1, 2);

        chkConfidential = new QCheckBox(tabPrivate);
        chkConfidential->setObjectName(QString::fromUtf8("chkConfidential"));

        gridLayout_82->addWidget(chkConfidential, 2, 1, 1, 3);


        gridLayout_83->addLayout(gridLayout_82, 0, 0, 1, 1);

        tabsSecurity->addTab(tabPrivate, QString());
        tabPrivileges = new QWidget();
        tabPrivileges->setObjectName(QString::fromUtf8("tabPrivileges"));
        QFont font2;
        font2.setBold(false);
        font2.setWeight(50);
        font2.setKerning(true);
        tabPrivileges->setFont(font2);
        gridLayout_26 = new QGridLayout(tabPrivileges);
        gridLayout_26->setObjectName(QString::fromUtf8("gridLayout_26"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        chkSbieLogon = new QCheckBox(tabPrivileges);
        chkSbieLogon->setObjectName(QString::fromUtf8("chkSbieLogon"));

        gridLayout_2->addWidget(chkSbieLogon, 9, 1, 1, 4);

        lblFence = new QLabel(tabPrivileges);
        lblFence->setObjectName(QString::fromUtf8("lblFence"));
        lblFence->setFont(font1);

        gridLayout_2->addWidget(lblFence, 6, 0, 1, 1);

        chkElevateRpcss = new QCheckBox(tabPrivileges);
        chkElevateRpcss->setObjectName(QString::fromUtf8("chkElevateRpcss"));

        gridLayout_2->addWidget(chkElevateRpcss, 3, 1, 1, 3);

        lblPrivilege = new QLabel(tabPrivileges);
        lblPrivilege->setObjectName(QString::fromUtf8("lblPrivilege"));
        lblPrivilege->setFont(font1);

        gridLayout_2->addWidget(lblPrivilege, 0, 0, 1, 1);

        chkProtectSystem = new QCheckBox(tabPrivileges);
        chkProtectSystem->setObjectName(QString::fromUtf8("chkProtectSystem"));

        gridLayout_2->addWidget(chkProtectSystem, 4, 1, 1, 3);

        lblToken = new QLabel(tabPrivileges);
        lblToken->setObjectName(QString::fromUtf8("lblToken"));
        lblToken->setFont(font1);

        gridLayout_2->addWidget(lblToken, 8, 0, 1, 2);

        label_65 = new QLabel(tabPrivileges);
        label_65->setObjectName(QString::fromUtf8("label_65"));
        label_65->setFont(font1);

        gridLayout_2->addWidget(label_65, 4, 4, 1, 1);

        chkDropPrivileges = new QCheckBox(tabPrivileges);
        chkDropPrivileges->setObjectName(QString::fromUtf8("chkDropPrivileges"));

        gridLayout_2->addWidget(chkDropPrivileges, 5, 1, 1, 3);

        chkRestrictServices = new QCheckBox(tabPrivileges);
        chkRestrictServices->setObjectName(QString::fromUtf8("chkRestrictServices"));

        gridLayout_2->addWidget(chkRestrictServices, 2, 1, 1, 3);

        verticalSpacer = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 11, 1, 1, 1);

        chkProtectSCM = new QCheckBox(tabPrivileges);
        chkProtectSCM->setObjectName(QString::fromUtf8("chkProtectSCM"));

        gridLayout_2->addWidget(chkProtectSCM, 1, 1, 1, 3);

        label_64 = new QLabel(tabPrivileges);
        label_64->setObjectName(QString::fromUtf8("label_64"));
        label_64->setFont(font1);

        gridLayout_2->addWidget(label_64, 5, 4, 1, 1);

        horizontalSpacer_13 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_13, 11, 2, 1, 1);

        chkAddToJob = new QCheckBox(tabPrivileges);
        chkAddToJob->setObjectName(QString::fromUtf8("chkAddToJob"));

        gridLayout_2->addWidget(chkAddToJob, 7, 1, 1, 2);

        label_74 = new QLabel(tabPrivileges);
        label_74->setObjectName(QString::fromUtf8("label_74"));
        label_74->setWordWrap(true);

        gridLayout_2->addWidget(label_74, 10, 2, 1, 2);


        gridLayout_26->addLayout(gridLayout_2, 0, 1, 1, 1);

        tabsSecurity->addTab(tabPrivileges, QString());

        gridLayout_7->addWidget(tabsSecurity, 0, 0, 1, 1);

        tabs->addTab(tabSecurity, QString());
        tabGroups = new QWidget();
        tabGroups->setObjectName(QString::fromUtf8("tabGroups"));
        gridLayout_18 = new QGridLayout(tabGroups);
        gridLayout_18->setObjectName(QString::fromUtf8("gridLayout_18"));
        tabGroups_2 = new QGridLayout();
        tabGroups_2->setObjectName(QString::fromUtf8("tabGroups_2"));
        chkShowGroupTmpl = new QCheckBox(tabGroups);
        chkShowGroupTmpl->setObjectName(QString::fromUtf8("chkShowGroupTmpl"));

        tabGroups_2->addWidget(chkShowGroupTmpl, 4, 1, 1, 1);

        verticalSpacer_7 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        tabGroups_2->addItem(verticalSpacer_7, 3, 1, 1, 1);

        btnAddGroup = new QPushButton(tabGroups);
        btnAddGroup->setObjectName(QString::fromUtf8("btnAddGroup"));

        tabGroups_2->addWidget(btnAddGroup, 1, 1, 1, 1);

        btnAddProg = new QPushButton(tabGroups);
        btnAddProg->setObjectName(QString::fromUtf8("btnAddProg"));

        tabGroups_2->addWidget(btnAddProg, 2, 1, 1, 1);

        label_3 = new QLabel(tabGroups);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        tabGroups_2->addWidget(label_3, 0, 0, 1, 2);

        treeGroups = new QTreeWidget(tabGroups);
        treeGroups->setObjectName(QString::fromUtf8("treeGroups"));
        treeGroups->setSortingEnabled(true);

        tabGroups_2->addWidget(treeGroups, 1, 0, 5, 1);

        btnDelProg = new QPushButton(tabGroups);
        btnDelProg->setObjectName(QString::fromUtf8("btnDelProg"));

        tabGroups_2->addWidget(btnDelProg, 5, 1, 1, 1);


        gridLayout_18->addLayout(tabGroups_2, 0, 0, 1, 1);

        tabs->addTab(tabGroups, QString());
        tabForce = new QWidget();
        tabForce->setObjectName(QString::fromUtf8("tabForce"));
        gridLayout_6 = new QGridLayout(tabForce);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setContentsMargins(9, 9, 9, 9);
        tabsForce = new QTabWidget(tabForce);
        tabsForce->setObjectName(QString::fromUtf8("tabsForce"));
        tabForceProgs = new QWidget();
        tabForceProgs->setObjectName(QString::fromUtf8("tabForceProgs"));
        gridLayout_54 = new QGridLayout(tabForceProgs);
        gridLayout_54->setObjectName(QString::fromUtf8("gridLayout_54"));
        gridLayout_54->setContentsMargins(3, 6, 3, 3);
        gridLayout_35 = new QGridLayout();
        gridLayout_35->setObjectName(QString::fromUtf8("gridLayout_35"));
        label = new QLabel(tabForceProgs);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        gridLayout_35->addWidget(label, 0, 0, 1, 2);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_35->addItem(verticalSpacer_5, 4, 1, 1, 1);

        btnForceDir = new QToolButton(tabForceProgs);
        btnForceDir->setObjectName(QString::fromUtf8("btnForceDir"));
        sizePolicy3.setHeightForWidth(btnForceDir->sizePolicy().hasHeightForWidth());
        btnForceDir->setSizePolicy(sizePolicy3);
        btnForceDir->setMinimumSize(QSize(0, 23));

        gridLayout_35->addWidget(btnForceDir, 3, 1, 1, 1);

        chkShowForceTmpl = new QCheckBox(tabForceProgs);
        chkShowForceTmpl->setObjectName(QString::fromUtf8("chkShowForceTmpl"));

        gridLayout_35->addWidget(chkShowForceTmpl, 5, 1, 1, 1);

        treeForced = new QTreeWidget(tabForceProgs);
        treeForced->setObjectName(QString::fromUtf8("treeForced"));
        treeForced->setSortingEnabled(true);

        gridLayout_35->addWidget(treeForced, 2, 0, 5, 1);

        btnForceProg = new QToolButton(tabForceProgs);
        btnForceProg->setObjectName(QString::fromUtf8("btnForceProg"));
        sizePolicy3.setHeightForWidth(btnForceProg->sizePolicy().hasHeightForWidth());
        btnForceProg->setSizePolicy(sizePolicy3);
        btnForceProg->setMinimumSize(QSize(0, 23));

        gridLayout_35->addWidget(btnForceProg, 2, 1, 1, 1);

        btnDelForce = new QPushButton(tabForceProgs);
        btnDelForce->setObjectName(QString::fromUtf8("btnDelForce"));

        gridLayout_35->addWidget(btnDelForce, 6, 1, 1, 1);

        chkDisableForced = new QCheckBox(tabForceProgs);
        chkDisableForced->setObjectName(QString::fromUtf8("chkDisableForced"));

        gridLayout_35->addWidget(chkDisableForced, 7, 0, 1, 1);


        gridLayout_54->addLayout(gridLayout_35, 0, 0, 1, 1);

        tabsForce->addTab(tabForceProgs, QString());
        tabBreakout = new QWidget();
        tabBreakout->setObjectName(QString::fromUtf8("tabBreakout"));
        gridLayout_13 = new QGridLayout(tabBreakout);
        gridLayout_13->setObjectName(QString::fromUtf8("gridLayout_13"));
        gridLayout_13->setContentsMargins(3, 6, 3, 3);
        gridLayout_55 = new QGridLayout();
        gridLayout_55->setObjectName(QString::fromUtf8("gridLayout_55"));
        treeBreakout = new QTreeWidget(tabBreakout);
        treeBreakout->setObjectName(QString::fromUtf8("treeBreakout"));
        treeBreakout->setSortingEnabled(true);

        gridLayout_55->addWidget(treeBreakout, 2, 0, 5, 1);

        btnDelBreakout = new QPushButton(tabBreakout);
        btnDelBreakout->setObjectName(QString::fromUtf8("btnDelBreakout"));

        gridLayout_55->addWidget(btnDelBreakout, 6, 1, 1, 1);

        chkShowBreakoutTmpl = new QCheckBox(tabBreakout);
        chkShowBreakoutTmpl->setObjectName(QString::fromUtf8("chkShowBreakoutTmpl"));

        gridLayout_55->addWidget(chkShowBreakoutTmpl, 5, 1, 1, 1);

        label_69 = new QLabel(tabBreakout);
        label_69->setObjectName(QString::fromUtf8("label_69"));
        label_69->setWordWrap(true);

        gridLayout_55->addWidget(label_69, 0, 0, 1, 1);

        lblBreakOut = new QLabel(tabBreakout);
        lblBreakOut->setObjectName(QString::fromUtf8("lblBreakOut"));
        lblBreakOut->setWordWrap(true);
        lblBreakOut->setOpenExternalLinks(true);

        gridLayout_55->addWidget(lblBreakOut, 7, 0, 1, 1);

        btnBreakoutDir = new QToolButton(tabBreakout);
        btnBreakoutDir->setObjectName(QString::fromUtf8("btnBreakoutDir"));
        sizePolicy3.setHeightForWidth(btnBreakoutDir->sizePolicy().hasHeightForWidth());
        btnBreakoutDir->setSizePolicy(sizePolicy3);
        btnBreakoutDir->setMinimumSize(QSize(0, 23));

        gridLayout_55->addWidget(btnBreakoutDir, 3, 1, 1, 1);

        btnBreakoutProg = new QToolButton(tabBreakout);
        btnBreakoutProg->setObjectName(QString::fromUtf8("btnBreakoutProg"));
        sizePolicy3.setHeightForWidth(btnBreakoutProg->sizePolicy().hasHeightForWidth());
        btnBreakoutProg->setSizePolicy(sizePolicy3);
        btnBreakoutProg->setMinimumSize(QSize(0, 23));

        gridLayout_55->addWidget(btnBreakoutProg, 2, 1, 1, 1);

        verticalSpacer_26 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_55->addItem(verticalSpacer_26, 4, 1, 1, 1);


        gridLayout_13->addLayout(gridLayout_55, 0, 0, 1, 1);

        tabsForce->addTab(tabBreakout, QString());

        gridLayout_6->addWidget(tabsForce, 0, 1, 1, 1);

        tabs->addTab(tabForce, QString());
        tabStop = new QWidget();
        tabStop->setObjectName(QString::fromUtf8("tabStop"));
        gridLayout_17 = new QGridLayout(tabStop);
        gridLayout_17->setObjectName(QString::fromUtf8("gridLayout_17"));
        tabsStop = new QTabWidget(tabStop);
        tabsStop->setObjectName(QString::fromUtf8("tabsStop"));
        tabLingerer = new QWidget();
        tabLingerer->setObjectName(QString::fromUtf8("tabLingerer"));
        gridLayout_57 = new QGridLayout(tabLingerer);
        gridLayout_57->setObjectName(QString::fromUtf8("gridLayout_57"));
        gridLayout_57->setContentsMargins(3, 6, 3, 3);
        gridLayout_14 = new QGridLayout();
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        verticalSpacer_6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_14->addItem(verticalSpacer_6, 3, 1, 1, 1);

        btnDelStopProg = new QPushButton(tabLingerer);
        btnDelStopProg->setObjectName(QString::fromUtf8("btnDelStopProg"));

        gridLayout_14->addWidget(btnDelStopProg, 5, 1, 1, 1);

        chkShowStopTmpl = new QCheckBox(tabLingerer);
        chkShowStopTmpl->setObjectName(QString::fromUtf8("chkShowStopTmpl"));

        gridLayout_14->addWidget(chkShowStopTmpl, 4, 1, 1, 1);

        btnAddLingering = new QPushButton(tabLingerer);
        btnAddLingering->setObjectName(QString::fromUtf8("btnAddLingering"));

        gridLayout_14->addWidget(btnAddLingering, 1, 1, 1, 1);

        treeStop = new QTreeWidget(tabLingerer);
        treeStop->setObjectName(QString::fromUtf8("treeStop"));
        treeStop->setSortingEnabled(true);

        gridLayout_14->addWidget(treeStop, 1, 0, 5, 1);

        label_2 = new QLabel(tabLingerer);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(true);

        gridLayout_14->addWidget(label_2, 0, 0, 1, 2);


        gridLayout_57->addLayout(gridLayout_14, 0, 0, 1, 1);

        tabsStop->addTab(tabLingerer, QString());
        tabLeader = new QWidget();
        tabLeader->setObjectName(QString::fromUtf8("tabLeader"));
        gridLayout_61 = new QGridLayout(tabLeader);
        gridLayout_61->setObjectName(QString::fromUtf8("gridLayout_61"));
        gridLayout_61->setContentsMargins(3, 6, 3, 3);
        gridLayout_58 = new QGridLayout();
        gridLayout_58->setObjectName(QString::fromUtf8("gridLayout_58"));
        treeLeader = new QTreeWidget(tabLeader);
        treeLeader->setObjectName(QString::fromUtf8("treeLeader"));
        treeLeader->setSortingEnabled(true);

        gridLayout_58->addWidget(treeLeader, 1, 0, 4, 1);

        verticalSpacer_27 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_58->addItem(verticalSpacer_27, 2, 1, 1, 1);

        btnAddLeader = new QPushButton(tabLeader);
        btnAddLeader->setObjectName(QString::fromUtf8("btnAddLeader"));

        gridLayout_58->addWidget(btnAddLeader, 1, 1, 1, 1);

        btnDelLeader = new QPushButton(tabLeader);
        btnDelLeader->setObjectName(QString::fromUtf8("btnDelLeader"));

        gridLayout_58->addWidget(btnDelLeader, 4, 1, 1, 1);

        chkShowLeaderTmpl = new QCheckBox(tabLeader);
        chkShowLeaderTmpl->setObjectName(QString::fromUtf8("chkShowLeaderTmpl"));

        gridLayout_58->addWidget(chkShowLeaderTmpl, 3, 1, 1, 1);

        label_71 = new QLabel(tabLeader);
        label_71->setObjectName(QString::fromUtf8("label_71"));
        label_71->setWordWrap(true);

        gridLayout_58->addWidget(label_71, 0, 0, 1, 2);


        gridLayout_61->addLayout(gridLayout_58, 0, 0, 1, 1);

        tabsStop->addTab(tabLeader, QString());

        gridLayout_17->addWidget(tabsStop, 0, 0, 1, 1);

        tabs->addTab(tabStop, QString());
        tabStart = new QWidget();
        tabStart->setObjectName(QString::fromUtf8("tabStart"));
        gridLayout_19 = new QGridLayout(tabStart);
        gridLayout_19->setObjectName(QString::fromUtf8("gridLayout_19"));
        gridLayout_16 = new QGridLayout();
        gridLayout_16->setObjectName(QString::fromUtf8("gridLayout_16"));
        chkStartBlockMsg = new QCheckBox(tabStart);
        chkStartBlockMsg->setObjectName(QString::fromUtf8("chkStartBlockMsg"));

        gridLayout_16->addWidget(chkStartBlockMsg, 9, 0, 1, 1);

        treeStart = new QTreeWidget(tabStart);
        treeStart->setObjectName(QString::fromUtf8("treeStart"));
        treeStart->setSortingEnabled(true);

        gridLayout_16->addWidget(treeStart, 5, 0, 4, 1);

        verticalSpacer_8 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_16->addItem(verticalSpacer_8, 6, 1, 1, 1);

        btnAddStartProg = new QPushButton(tabStart);
        btnAddStartProg->setObjectName(QString::fromUtf8("btnAddStartProg"));

        gridLayout_16->addWidget(btnAddStartProg, 5, 1, 1, 1);

        btnDelStartProg = new QPushButton(tabStart);
        btnDelStartProg->setObjectName(QString::fromUtf8("btnDelStartProg"));

        gridLayout_16->addWidget(btnDelStartProg, 8, 1, 1, 1);

        gridLayout_23 = new QGridLayout();
        gridLayout_23->setObjectName(QString::fromUtf8("gridLayout_23"));
        gridLayout_23->setVerticalSpacing(0);
        radStartSelected = new QRadioButton(tabStart);
        radStartSelected->setObjectName(QString::fromUtf8("radStartSelected"));

        gridLayout_23->addWidget(radStartSelected, 2, 0, 1, 1);

        radStartExcept = new QRadioButton(tabStart);
        radStartExcept->setObjectName(QString::fromUtf8("radStartExcept"));

        gridLayout_23->addWidget(radStartExcept, 1, 0, 1, 1);

        radStartAll = new QRadioButton(tabStart);
        radStartAll->setObjectName(QString::fromUtf8("radStartAll"));

        gridLayout_23->addWidget(radStartAll, 0, 0, 1, 1);

        label_4 = new QLabel(tabStart);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setWordWrap(true);

        gridLayout_23->addWidget(label_4, 3, 0, 1, 1);


        gridLayout_16->addLayout(gridLayout_23, 3, 0, 1, 1);

        chkShowStartTmpl = new QCheckBox(tabStart);
        chkShowStartTmpl->setObjectName(QString::fromUtf8("chkShowStartTmpl"));

        gridLayout_16->addWidget(chkShowStartTmpl, 7, 1, 1, 1);


        gridLayout_19->addLayout(gridLayout_16, 0, 0, 1, 1);

        tabs->addTab(tabStart, QString());
        tabAccess = new QWidget();
        tabAccess->setObjectName(QString::fromUtf8("tabAccess"));
        gridLayout_11 = new QGridLayout(tabAccess);
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        tabsAccess = new QTabWidget(tabAccess);
        tabsAccess->setObjectName(QString::fromUtf8("tabsAccess"));
        tabFiles = new QWidget();
        tabFiles->setObjectName(QString::fromUtf8("tabFiles"));
        gridLayout_72 = new QGridLayout(tabFiles);
        gridLayout_72->setObjectName(QString::fromUtf8("gridLayout_72"));
        gridLayout_72->setContentsMargins(3, 6, 3, 3);
        gridLayout_67 = new QGridLayout();
        gridLayout_67->setObjectName(QString::fromUtf8("gridLayout_67"));
        chkShowFilesTmpl = new QCheckBox(tabFiles);
        chkShowFilesTmpl->setObjectName(QString::fromUtf8("chkShowFilesTmpl"));

        gridLayout_67->addWidget(chkShowFilesTmpl, 3, 1, 1, 1);

        btnDelFile = new QPushButton(tabFiles);
        btnDelFile->setObjectName(QString::fromUtf8("btnDelFile"));

        gridLayout_67->addWidget(btnDelFile, 4, 1, 1, 1);

        treeFiles = new QTreeWidget(tabFiles);
        treeFiles->setObjectName(QString::fromUtf8("treeFiles"));
        treeFiles->setSortingEnabled(true);

        gridLayout_67->addWidget(treeFiles, 1, 0, 4, 1);

        btnAddFile = new QToolButton(tabFiles);
        btnAddFile->setObjectName(QString::fromUtf8("btnAddFile"));
        sizePolicy3.setHeightForWidth(btnAddFile->sizePolicy().hasHeightForWidth());
        btnAddFile->setSizePolicy(sizePolicy3);
        btnAddFile->setMinimumSize(QSize(0, 23));

        gridLayout_67->addWidget(btnAddFile, 1, 1, 1, 1);

        verticalSpacer_11 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_67->addItem(verticalSpacer_11, 2, 1, 1, 1);

        label_7 = new QLabel(tabFiles);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setWordWrap(true);

        gridLayout_67->addWidget(label_7, 0, 0, 1, 1);


        gridLayout_72->addLayout(gridLayout_67, 0, 0, 1, 1);

        tabsAccess->addTab(tabFiles, QString());
        tabKeys = new QWidget();
        tabKeys->setObjectName(QString::fromUtf8("tabKeys"));
        gridLayout_73 = new QGridLayout(tabKeys);
        gridLayout_73->setObjectName(QString::fromUtf8("gridLayout_73"));
        gridLayout_73->setContentsMargins(3, 6, 3, 3);
        gridLayout_68 = new QGridLayout();
        gridLayout_68->setObjectName(QString::fromUtf8("gridLayout_68"));
        chkShowKeysTmpl = new QCheckBox(tabKeys);
        chkShowKeysTmpl->setObjectName(QString::fromUtf8("chkShowKeysTmpl"));

        gridLayout_68->addWidget(chkShowKeysTmpl, 3, 1, 1, 1);

        treeKeys = new QTreeWidget(tabKeys);
        treeKeys->setObjectName(QString::fromUtf8("treeKeys"));
        treeKeys->setSortingEnabled(true);

        gridLayout_68->addWidget(treeKeys, 1, 0, 4, 1);

        btnDelKey = new QPushButton(tabKeys);
        btnDelKey->setObjectName(QString::fromUtf8("btnDelKey"));

        gridLayout_68->addWidget(btnDelKey, 4, 1, 1, 1);

        btnAddKey = new QToolButton(tabKeys);
        btnAddKey->setObjectName(QString::fromUtf8("btnAddKey"));
        sizePolicy3.setHeightForWidth(btnAddKey->sizePolicy().hasHeightForWidth());
        btnAddKey->setSizePolicy(sizePolicy3);
        btnAddKey->setMinimumSize(QSize(0, 23));

        gridLayout_68->addWidget(btnAddKey, 1, 1, 1, 1);

        verticalSpacer_18 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_68->addItem(verticalSpacer_18, 2, 1, 1, 1);

        label_10 = new QLabel(tabKeys);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setWordWrap(true);

        gridLayout_68->addWidget(label_10, 0, 0, 1, 1);


        gridLayout_73->addLayout(gridLayout_68, 0, 0, 1, 1);

        tabsAccess->addTab(tabKeys, QString());
        tabIPC = new QWidget();
        tabIPC->setObjectName(QString::fromUtf8("tabIPC"));
        gridLayout_74 = new QGridLayout(tabIPC);
        gridLayout_74->setObjectName(QString::fromUtf8("gridLayout_74"));
        gridLayout_74->setContentsMargins(3, 6, 3, 3);
        gridLayout_69 = new QGridLayout();
        gridLayout_69->setObjectName(QString::fromUtf8("gridLayout_69"));
        btnAddIPC = new QToolButton(tabIPC);
        btnAddIPC->setObjectName(QString::fromUtf8("btnAddIPC"));
        sizePolicy3.setHeightForWidth(btnAddIPC->sizePolicy().hasHeightForWidth());
        btnAddIPC->setSizePolicy(sizePolicy3);
        btnAddIPC->setMinimumSize(QSize(0, 23));

        gridLayout_69->addWidget(btnAddIPC, 1, 1, 1, 1);

        chkShowIPCTmpl = new QCheckBox(tabIPC);
        chkShowIPCTmpl->setObjectName(QString::fromUtf8("chkShowIPCTmpl"));

        gridLayout_69->addWidget(chkShowIPCTmpl, 3, 1, 1, 1);

        btnDelIPC = new QPushButton(tabIPC);
        btnDelIPC->setObjectName(QString::fromUtf8("btnDelIPC"));

        gridLayout_69->addWidget(btnDelIPC, 4, 1, 1, 1);

        treeIPC = new QTreeWidget(tabIPC);
        treeIPC->setObjectName(QString::fromUtf8("treeIPC"));
        treeIPC->setSortingEnabled(true);

        gridLayout_69->addWidget(treeIPC, 1, 0, 4, 1);

        verticalSpacer_21 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_69->addItem(verticalSpacer_21, 2, 1, 1, 1);

        label_22 = new QLabel(tabIPC);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setWordWrap(true);

        gridLayout_69->addWidget(label_22, 0, 0, 1, 1);


        gridLayout_74->addLayout(gridLayout_69, 0, 0, 1, 1);

        tabsAccess->addTab(tabIPC, QString());
        tabWnd = new QWidget();
        tabWnd->setObjectName(QString::fromUtf8("tabWnd"));
        gridLayout_75 = new QGridLayout(tabWnd);
        gridLayout_75->setObjectName(QString::fromUtf8("gridLayout_75"));
        gridLayout_75->setContentsMargins(3, 6, 3, 3);
        gridLayout_70 = new QGridLayout();
        gridLayout_70->setObjectName(QString::fromUtf8("gridLayout_70"));
        btnDelWnd = new QPushButton(tabWnd);
        btnDelWnd->setObjectName(QString::fromUtf8("btnDelWnd"));

        gridLayout_70->addWidget(btnDelWnd, 4, 1, 1, 1);

        chkShowWndTmpl = new QCheckBox(tabWnd);
        chkShowWndTmpl->setObjectName(QString::fromUtf8("chkShowWndTmpl"));

        gridLayout_70->addWidget(chkShowWndTmpl, 3, 1, 1, 1);

        btnAddWnd = new QToolButton(tabWnd);
        btnAddWnd->setObjectName(QString::fromUtf8("btnAddWnd"));
        sizePolicy3.setHeightForWidth(btnAddWnd->sizePolicy().hasHeightForWidth());
        btnAddWnd->setSizePolicy(sizePolicy3);
        btnAddWnd->setMinimumSize(QSize(0, 23));

        gridLayout_70->addWidget(btnAddWnd, 1, 1, 1, 1);

        verticalSpacer_34 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_70->addItem(verticalSpacer_34, 2, 1, 1, 1);

        treeWnd = new QTreeWidget(tabWnd);
        treeWnd->setObjectName(QString::fromUtf8("treeWnd"));
        treeWnd->setSortingEnabled(true);

        gridLayout_70->addWidget(treeWnd, 1, 0, 4, 1);

        chkNoWindowRename = new QCheckBox(tabWnd);
        chkNoWindowRename->setObjectName(QString::fromUtf8("chkNoWindowRename"));

        gridLayout_70->addWidget(chkNoWindowRename, 5, 0, 1, 1);

        label_27 = new QLabel(tabWnd);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setWordWrap(true);

        gridLayout_70->addWidget(label_27, 0, 0, 1, 1);


        gridLayout_75->addLayout(gridLayout_70, 0, 0, 1, 1);

        tabsAccess->addTab(tabWnd, QString());
        tabCOM = new QWidget();
        tabCOM->setObjectName(QString::fromUtf8("tabCOM"));
        gridLayout_76 = new QGridLayout(tabCOM);
        gridLayout_76->setObjectName(QString::fromUtf8("gridLayout_76"));
        gridLayout_76->setContentsMargins(3, 6, 3, 3);
        gridLayout_71 = new QGridLayout();
        gridLayout_71->setObjectName(QString::fromUtf8("gridLayout_71"));
        btnDelCOM = new QPushButton(tabCOM);
        btnDelCOM->setObjectName(QString::fromUtf8("btnDelCOM"));

        gridLayout_71->addWidget(btnDelCOM, 4, 1, 1, 1);

        treeCOM = new QTreeWidget(tabCOM);
        treeCOM->setObjectName(QString::fromUtf8("treeCOM"));
        treeCOM->setSortingEnabled(true);

        gridLayout_71->addWidget(treeCOM, 1, 0, 4, 1);

        chkShowCOMTmpl = new QCheckBox(tabCOM);
        chkShowCOMTmpl->setObjectName(QString::fromUtf8("chkShowCOMTmpl"));

        gridLayout_71->addWidget(chkShowCOMTmpl, 3, 1, 1, 1);

        btnAddCOM = new QToolButton(tabCOM);
        btnAddCOM->setObjectName(QString::fromUtf8("btnAddCOM"));
        sizePolicy3.setHeightForWidth(btnAddCOM->sizePolicy().hasHeightForWidth());
        btnAddCOM->setSizePolicy(sizePolicy3);
        btnAddCOM->setMinimumSize(QSize(0, 23));

        gridLayout_71->addWidget(btnAddCOM, 1, 1, 1, 1);

        verticalSpacer_35 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_71->addItem(verticalSpacer_35, 2, 1, 1, 1);

        label_29 = new QLabel(tabCOM);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setWordWrap(true);

        gridLayout_71->addWidget(label_29, 0, 0, 1, 1);


        gridLayout_76->addLayout(gridLayout_71, 0, 0, 1, 1);

        chkOpenCOM = new QCheckBox(tabCOM);
        chkOpenCOM->setObjectName(QString::fromUtf8("chkOpenCOM"));

        gridLayout_76->addWidget(chkOpenCOM, 1, 0, 1, 1);

        tabsAccess->addTab(tabCOM, QString());
        tabPolicy = new QWidget();
        tabPolicy->setObjectName(QString::fromUtf8("tabPolicy"));
        gridLayout_51 = new QGridLayout(tabPolicy);
        gridLayout_51->setObjectName(QString::fromUtf8("gridLayout_51"));
        gridLayout_51->setContentsMargins(9, 9, 9, 9);
        gridLayout_50 = new QGridLayout();
        gridLayout_50->setObjectName(QString::fromUtf8("gridLayout_50"));
        lblPolicy = new QLabel(tabPolicy);
        lblPolicy->setObjectName(QString::fromUtf8("lblPolicy"));
        lblPolicy->setFont(font1);

        gridLayout_50->addWidget(lblPolicy, 3, 1, 1, 1);

        chkCloseForBox = new QCheckBox(tabPolicy);
        chkCloseForBox->setObjectName(QString::fromUtf8("chkCloseForBox"));

        gridLayout_50->addWidget(chkCloseForBox, 6, 2, 1, 2);

        chkUseSpecificity = new QCheckBox(tabPolicy);
        chkUseSpecificity->setObjectName(QString::fromUtf8("chkUseSpecificity"));

        gridLayout_50->addWidget(chkUseSpecificity, 4, 2, 1, 2);

        chkNoOpenForBox = new QCheckBox(tabPolicy);
        chkNoOpenForBox->setObjectName(QString::fromUtf8("chkNoOpenForBox"));

        gridLayout_50->addWidget(chkNoOpenForBox, 7, 2, 1, 2);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_50->addItem(horizontalSpacer_9, 8, 3, 1, 1);

        lblMode = new QLabel(tabPolicy);
        lblMode->setObjectName(QString::fromUtf8("lblMode"));
        lblMode->setFont(font1);

        gridLayout_50->addWidget(lblMode, 0, 1, 1, 1);

        label_54 = new QLabel(tabPolicy);
        label_54->setObjectName(QString::fromUtf8("label_54"));
        label_54->setWordWrap(true);

        gridLayout_50->addWidget(label_54, 5, 3, 1, 1);

        chkPrivacy = new QCheckBox(tabPolicy);
        chkPrivacy->setObjectName(QString::fromUtf8("chkPrivacy"));

        gridLayout_50->addWidget(chkPrivacy, 1, 2, 1, 2);

        verticalSpacer_24 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_50->addItem(verticalSpacer_24, 8, 2, 1, 1);

        label_55 = new QLabel(tabPolicy);
        label_55->setObjectName(QString::fromUtf8("label_55"));
        label_55->setWordWrap(true);

        gridLayout_50->addWidget(label_55, 2, 3, 1, 1);


        gridLayout_51->addLayout(gridLayout_50, 0, 0, 1, 1);

        tabsAccess->addTab(tabPolicy, QString());

        gridLayout_11->addWidget(tabsAccess, 0, 0, 1, 1);

        tabs->addTab(tabAccess, QString());
        tabInternet = new QWidget();
        tabInternet->setObjectName(QString::fromUtf8("tabInternet"));
        gridLayout_21 = new QGridLayout(tabInternet);
        gridLayout_21->setObjectName(QString::fromUtf8("gridLayout_21"));
        tabsInternet = new QTabWidget(tabInternet);
        tabsInternet->setObjectName(QString::fromUtf8("tabsInternet"));
        tabINet = new QWidget();
        tabINet->setObjectName(QString::fromUtf8("tabINet"));
        gridLayout_46 = new QGridLayout(tabINet);
        gridLayout_46->setObjectName(QString::fromUtf8("gridLayout_46"));
        gridLayout_46->setContentsMargins(3, 6, 3, 3);
        gridLayout_20 = new QGridLayout();
        gridLayout_20->setObjectName(QString::fromUtf8("gridLayout_20"));
        chkINetBlockMsg = new QCheckBox(tabINet);
        chkINetBlockMsg->setObjectName(QString::fromUtf8("chkINetBlockMsg"));

        gridLayout_20->addWidget(chkINetBlockMsg, 6, 0, 1, 1);

        btnAddINetProg = new QPushButton(tabINet);
        btnAddINetProg->setObjectName(QString::fromUtf8("btnAddINetProg"));

        gridLayout_20->addWidget(btnAddINetProg, 3, 1, 1, 1);

        chkINetBlockPrompt = new QCheckBox(tabINet);
        chkINetBlockPrompt->setObjectName(QString::fromUtf8("chkINetBlockPrompt"));

        gridLayout_20->addWidget(chkINetBlockPrompt, 1, 0, 1, 1);

        btnDelINetProg = new QPushButton(tabINet);
        btnDelINetProg->setObjectName(QString::fromUtf8("btnDelINetProg"));

        gridLayout_20->addWidget(btnDelINetProg, 5, 1, 1, 1);

        label_11 = new QLabel(tabINet);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_20->addWidget(label_11, 2, 0, 1, 1);

        verticalSpacer_9 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_20->addItem(verticalSpacer_9, 4, 1, 1, 1);

        treeINet = new QTreeWidget(tabINet);
        treeINet->setObjectName(QString::fromUtf8("treeINet"));
        treeINet->setSortingEnabled(true);

        gridLayout_20->addWidget(treeINet, 3, 0, 3, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_12 = new QLabel(tabINet);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_2->addWidget(label_12);

        cmbBlockINet = new QComboBox(tabINet);
        cmbBlockINet->setObjectName(QString::fromUtf8("cmbBlockINet"));

        horizontalLayout_2->addWidget(cmbBlockINet);


        gridLayout_20->addLayout(horizontalLayout_2, 0, 0, 1, 1);


        gridLayout_46->addLayout(gridLayout_20, 0, 0, 1, 1);

        tabsInternet->addTab(tabINet, QString());
        tabNetFw = new QWidget();
        tabNetFw->setObjectName(QString::fromUtf8("tabNetFw"));
        gridLayout_48 = new QGridLayout(tabNetFw);
        gridLayout_48->setObjectName(QString::fromUtf8("gridLayout_48"));
        gridLayout_48->setContentsMargins(3, -1, 3, 3);
        gridLayout_47 = new QGridLayout();
        gridLayout_47->setObjectName(QString::fromUtf8("gridLayout_47"));
        verticalSpacer_23 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_47->addItem(verticalSpacer_23, 2, 1, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_48 = new QLabel(tabNetFw);
        label_48->setObjectName(QString::fromUtf8("label_48"));

        horizontalLayout_3->addWidget(label_48);

        txtProgFwTest = new QLineEdit(tabNetFw);
        txtProgFwTest->setObjectName(QString::fromUtf8("txtProgFwTest"));

        horizontalLayout_3->addWidget(txtProgFwTest);

        label_49 = new QLabel(tabNetFw);
        label_49->setObjectName(QString::fromUtf8("label_49"));

        horizontalLayout_3->addWidget(label_49);

        txtPortFwTest = new QLineEdit(tabNetFw);
        txtPortFwTest->setObjectName(QString::fromUtf8("txtPortFwTest"));

        horizontalLayout_3->addWidget(txtPortFwTest);

        label_50 = new QLabel(tabNetFw);
        label_50->setObjectName(QString::fromUtf8("label_50"));

        horizontalLayout_3->addWidget(label_50);

        txtIPFwTest = new QLineEdit(tabNetFw);
        txtIPFwTest->setObjectName(QString::fromUtf8("txtIPFwTest"));

        horizontalLayout_3->addWidget(txtIPFwTest);

        label_51 = new QLabel(tabNetFw);
        label_51->setObjectName(QString::fromUtf8("label_51"));

        horizontalLayout_3->addWidget(label_51);

        cmbProtFwTest = new QComboBox(tabNetFw);
        cmbProtFwTest->setObjectName(QString::fromUtf8("cmbProtFwTest"));

        horizontalLayout_3->addWidget(cmbProtFwTest);

        btnClearFwTest = new QToolButton(tabNetFw);
        btnClearFwTest->setObjectName(QString::fromUtf8("btnClearFwTest"));

        horizontalLayout_3->addWidget(btnClearFwTest);


        gridLayout_47->addLayout(horizontalLayout_3, 5, 0, 1, 1);

        btnDelFwRule = new QPushButton(tabNetFw);
        btnDelFwRule->setObjectName(QString::fromUtf8("btnDelFwRule"));

        gridLayout_47->addWidget(btnDelFwRule, 4, 1, 1, 1);

        btnAddFwRule = new QPushButton(tabNetFw);
        btnAddFwRule->setObjectName(QString::fromUtf8("btnAddFwRule"));

        gridLayout_47->addWidget(btnAddFwRule, 1, 1, 1, 1);

        treeNetFw = new QTreeWidget(tabNetFw);
        treeNetFw->setObjectName(QString::fromUtf8("treeNetFw"));
        treeNetFw->setSortingEnabled(true);

        gridLayout_47->addWidget(treeNetFw, 1, 0, 4, 1);

        chkShowNetFwTmpl = new QCheckBox(tabNetFw);
        chkShowNetFwTmpl->setObjectName(QString::fromUtf8("chkShowNetFwTmpl"));

        gridLayout_47->addWidget(chkShowNetFwTmpl, 3, 1, 1, 1);

        lblNoWfp = new QLabel(tabNetFw);
        lblNoWfp->setObjectName(QString::fromUtf8("lblNoWfp"));
        lblNoWfp->setFont(font1);
        lblNoWfp->setWordWrap(true);

        gridLayout_47->addWidget(lblNoWfp, 0, 0, 1, 2);


        gridLayout_48->addLayout(gridLayout_47, 0, 0, 1, 1);

        tabsInternet->addTab(tabNetFw, QString());

        gridLayout_21->addWidget(tabsInternet, 1, 0, 1, 1);

        tabs->addTab(tabInternet, QString());
        tabRecovery = new QWidget();
        tabRecovery->setObjectName(QString::fromUtf8("tabRecovery"));
        gridLayout_24 = new QGridLayout(tabRecovery);
        gridLayout_24->setObjectName(QString::fromUtf8("gridLayout_24"));
        tabsRecovery = new QTabWidget(tabRecovery);
        tabsRecovery->setObjectName(QString::fromUtf8("tabsRecovery"));
        tabQuickRecovery = new QWidget();
        tabQuickRecovery->setObjectName(QString::fromUtf8("tabQuickRecovery"));
        gridLayout_10 = new QGridLayout(tabQuickRecovery);
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        gridLayout_10->setContentsMargins(3, 6, 3, 3);
        gridLayout_22 = new QGridLayout();
        gridLayout_22->setObjectName(QString::fromUtf8("gridLayout_22"));
        treeRecovery = new QTreeWidget(tabQuickRecovery);
        treeRecovery->setObjectName(QString::fromUtf8("treeRecovery"));
        treeRecovery->setSortingEnabled(true);

        gridLayout_22->addWidget(treeRecovery, 4, 0, 8, 1);

        label_16 = new QLabel(tabQuickRecovery);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setWordWrap(true);

        gridLayout_22->addWidget(label_16, 1, 0, 1, 2);

        verticalSpacer_13 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_22->addItem(verticalSpacer_13, 7, 1, 1, 1);

        chkShowRecoveryTmpl = new QCheckBox(tabQuickRecovery);
        chkShowRecoveryTmpl->setObjectName(QString::fromUtf8("chkShowRecoveryTmpl"));

        gridLayout_22->addWidget(chkShowRecoveryTmpl, 8, 1, 1, 1);

        btnDelRecovery = new QPushButton(tabQuickRecovery);
        btnDelRecovery->setObjectName(QString::fromUtf8("btnDelRecovery"));

        gridLayout_22->addWidget(btnDelRecovery, 9, 1, 1, 1);

        btnAddRecovery = new QPushButton(tabQuickRecovery);
        btnAddRecovery->setObjectName(QString::fromUtf8("btnAddRecovery"));

        gridLayout_22->addWidget(btnAddRecovery, 4, 1, 1, 1);


        gridLayout_10->addLayout(gridLayout_22, 0, 0, 1, 1);

        tabsRecovery->addTab(tabQuickRecovery, QString());
        tabImmediateRecovery = new QWidget();
        tabImmediateRecovery->setObjectName(QString::fromUtf8("tabImmediateRecovery"));
        gridLayout_56 = new QGridLayout(tabImmediateRecovery);
        gridLayout_56->setObjectName(QString::fromUtf8("gridLayout_56"));
        gridLayout_56->setContentsMargins(3, 6, 3, 3);
        gridLayout_40 = new QGridLayout();
        gridLayout_40->setObjectName(QString::fromUtf8("gridLayout_40"));
        label_9 = new QLabel(tabImmediateRecovery);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setWordWrap(true);

        gridLayout_40->addWidget(label_9, 1, 0, 1, 1);

        btnAddRecIgnoreExt = new QPushButton(tabImmediateRecovery);
        btnAddRecIgnoreExt->setObjectName(QString::fromUtf8("btnAddRecIgnoreExt"));

        gridLayout_40->addWidget(btnAddRecIgnoreExt, 3, 1, 1, 1);

        chkAutoRecovery = new QCheckBox(tabImmediateRecovery);
        chkAutoRecovery->setObjectName(QString::fromUtf8("chkAutoRecovery"));

        gridLayout_40->addWidget(chkAutoRecovery, 0, 0, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_40->addItem(verticalSpacer_3, 4, 1, 1, 1);

        treeRecIgnore = new QTreeWidget(tabImmediateRecovery);
        treeRecIgnore->setObjectName(QString::fromUtf8("treeRecIgnore"));
        treeRecIgnore->setSortingEnabled(true);

        gridLayout_40->addWidget(treeRecIgnore, 2, 0, 5, 1);

        btnDelRecIgnore = new QPushButton(tabImmediateRecovery);
        btnDelRecIgnore->setObjectName(QString::fromUtf8("btnDelRecIgnore"));

        gridLayout_40->addWidget(btnDelRecIgnore, 6, 1, 1, 1);

        btnAddRecIgnore = new QPushButton(tabImmediateRecovery);
        btnAddRecIgnore->setObjectName(QString::fromUtf8("btnAddRecIgnore"));

        gridLayout_40->addWidget(btnAddRecIgnore, 2, 1, 1, 1);

        chkShowRecIgnoreTmpl = new QCheckBox(tabImmediateRecovery);
        chkShowRecIgnoreTmpl->setObjectName(QString::fromUtf8("chkShowRecIgnoreTmpl"));

        gridLayout_40->addWidget(chkShowRecIgnoreTmpl, 5, 1, 1, 1);


        gridLayout_56->addLayout(gridLayout_40, 0, 0, 1, 1);

        tabsRecovery->addTab(tabImmediateRecovery, QString());

        gridLayout_24->addWidget(tabsRecovery, 0, 0, 1, 1);

        tabs->addTab(tabRecovery, QString());
        tabOther = new QWidget();
        tabOther->setObjectName(QString::fromUtf8("tabOther"));
        gridLayout_12 = new QGridLayout(tabOther);
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        tabsOther = new QTabWidget(tabOther);
        tabsOther->setObjectName(QString::fromUtf8("tabsOther"));
        tabsOther->setFont(font2);
        tabCompat = new QWidget();
        tabCompat->setObjectName(QString::fromUtf8("tabCompat"));
        gridLayout_63 = new QGridLayout(tabCompat);
        gridLayout_63->setObjectName(QString::fromUtf8("gridLayout_63"));
        gridLayout_62 = new QGridLayout();
        gridLayout_62->setObjectName(QString::fromUtf8("gridLayout_62"));
        chkElevateCreateProcessFix = new QCheckBox(tabCompat);
        chkElevateCreateProcessFix->setObjectName(QString::fromUtf8("chkElevateCreateProcessFix"));

        gridLayout_62->addWidget(chkElevateCreateProcessFix, 3, 1, 1, 1);

        chkUseSbieWndStation = new QCheckBox(tabCompat);
        chkUseSbieWndStation->setObjectName(QString::fromUtf8("chkUseSbieWndStation"));

        gridLayout_62->addWidget(chkUseSbieWndStation, 6, 1, 1, 1);

        lblCompatibility = new QLabel(tabCompat);
        lblCompatibility->setObjectName(QString::fromUtf8("lblCompatibility"));
        lblCompatibility->setFont(font1);

        gridLayout_62->addWidget(lblCompatibility, 1, 0, 1, 1);

        chkComTimeout = new QCheckBox(tabCompat);
        chkComTimeout->setObjectName(QString::fromUtf8("chkComTimeout"));

        gridLayout_62->addWidget(chkComTimeout, 7, 1, 1, 1);

        chkPreferExternalManifest = new QCheckBox(tabCompat);
        chkPreferExternalManifest->setObjectName(QString::fromUtf8("chkPreferExternalManifest"));

        gridLayout_62->addWidget(chkPreferExternalManifest, 2, 1, 1, 1);

        chkUseSbieDeskHack = new QCheckBox(tabCompat);
        chkUseSbieDeskHack->setObjectName(QString::fromUtf8("chkUseSbieDeskHack"));

        gridLayout_62->addWidget(chkUseSbieDeskHack, 5, 1, 1, 1);

        verticalSpacer_28 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_62->addItem(verticalSpacer_28, 8, 0, 1, 1);

        horizontalSpacer_12 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_62->addItem(horizontalSpacer_12, 8, 1, 1, 1);

        chkNestedJobs = new QCheckBox(tabCompat);
        chkNestedJobs->setObjectName(QString::fromUtf8("chkNestedJobs"));

        gridLayout_62->addWidget(chkNestedJobs, 4, 1, 1, 1);

        chkNoPanic = new QCheckBox(tabCompat);
        chkNoPanic->setObjectName(QString::fromUtf8("chkNoPanic"));

        gridLayout_62->addWidget(chkNoPanic, 0, 1, 1, 1);


        gridLayout_63->addLayout(gridLayout_62, 0, 0, 1, 1);

        tabsOther->addTab(tabCompat, QString());
        tabDlls = new QWidget();
        tabDlls->setObjectName(QString::fromUtf8("tabDlls"));
        gridLayout_77 = new QGridLayout(tabDlls);
        gridLayout_77->setObjectName(QString::fromUtf8("gridLayout_77"));
        gridLayout_49 = new QGridLayout();
        gridLayout_49->setObjectName(QString::fromUtf8("gridLayout_49"));
        verticalSpacer_36 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_49->addItem(verticalSpacer_36, 6, 1, 1, 1);

        lblProtection = new QLabel(tabDlls);
        lblProtection->setObjectName(QString::fromUtf8("lblProtection"));
        lblProtection->setFont(font1);

        gridLayout_49->addWidget(lblProtection, 3, 0, 1, 2);

        treeInjectDll = new QTreeWidget(tabDlls);
        treeInjectDll->setObjectName(QString::fromUtf8("treeInjectDll"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(treeInjectDll->sizePolicy().hasHeightForWidth());
        treeInjectDll->setSizePolicy(sizePolicy4);

        gridLayout_49->addWidget(treeInjectDll, 1, 0, 1, 3);

        chkHostProtect = new QCheckBox(tabDlls);
        chkHostProtect->setObjectName(QString::fromUtf8("chkHostProtect"));

        gridLayout_49->addWidget(chkHostProtect, 4, 1, 1, 2);

        chkHostProtectMsg = new QCheckBox(tabDlls);
        chkHostProtectMsg->setObjectName(QString::fromUtf8("chkHostProtectMsg"));

        gridLayout_49->addWidget(chkHostProtectMsg, 5, 2, 1, 1);

        label_36 = new QLabel(tabDlls);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        sizePolicy.setHeightForWidth(label_36->sizePolicy().hasHeightForWidth());
        label_36->setSizePolicy(sizePolicy);
        label_36->setWordWrap(true);

        gridLayout_49->addWidget(label_36, 0, 0, 1, 3);

        horizontalSpacer_16 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_49->addItem(horizontalSpacer_16, 2, 2, 1, 1);

        verticalSpacer_39 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_49->addItem(verticalSpacer_39, 1, 3, 1, 1);


        gridLayout_77->addLayout(gridLayout_49, 1, 0, 1, 1);

        tabsOther->addTab(tabDlls, QString());

        gridLayout_12->addWidget(tabsOther, 0, 0, 1, 1);

        tabs->addTab(tabOther, QString());
        tabAdvanced = new QWidget();
        tabAdvanced->setObjectName(QString::fromUtf8("tabAdvanced"));
        gridLayout_121 = new QGridLayout(tabAdvanced);
        gridLayout_121->setObjectName(QString::fromUtf8("gridLayout_121"));
        tabsAdvanced = new QTabWidget(tabAdvanced);
        tabsAdvanced->setObjectName(QString::fromUtf8("tabsAdvanced"));
        tabsAdvanced->setFont(font2);
        tabMisc = new QWidget();
        tabMisc->setObjectName(QString::fromUtf8("tabMisc"));
        gridLayout_59 = new QGridLayout(tabMisc);
        gridLayout_59->setObjectName(QString::fromUtf8("gridLayout_59"));
        gridLayout_59->setContentsMargins(3, 6, 3, 0);
        gridLayout_60 = new QGridLayout();
        gridLayout_60->setObjectName(QString::fromUtf8("gridLayout_60"));
        btnAddOption = new QToolButton(tabMisc);
        btnAddOption->setObjectName(QString::fromUtf8("btnAddOption"));
        sizePolicy3.setHeightForWidth(btnAddOption->sizePolicy().hasHeightForWidth());
        btnAddOption->setSizePolicy(sizePolicy3);
        btnAddOption->setMinimumSize(QSize(0, 23));

        gridLayout_60->addWidget(btnAddOption, 1, 1, 1, 1);

        btnDelOption = new QPushButton(tabMisc);
        btnDelOption->setObjectName(QString::fromUtf8("btnDelOption"));

        gridLayout_60->addWidget(btnDelOption, 4, 1, 1, 1);

        lblAdvanced = new QLabel(tabMisc);
        lblAdvanced->setObjectName(QString::fromUtf8("lblAdvanced"));

        gridLayout_60->addWidget(lblAdvanced, 0, 0, 1, 2);

        treeOptions = new QTreeWidget(tabMisc);
        treeOptions->setObjectName(QString::fromUtf8("treeOptions"));
        treeOptions->setSortingEnabled(true);

        gridLayout_60->addWidget(treeOptions, 1, 0, 4, 1);

        chkShowOptionsTmpl = new QCheckBox(tabMisc);
        chkShowOptionsTmpl->setObjectName(QString::fromUtf8("chkShowOptionsTmpl"));

        gridLayout_60->addWidget(chkShowOptionsTmpl, 3, 1, 1, 1);

        verticalSpacer_30 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_60->addItem(verticalSpacer_30, 2, 1, 1, 1);


        gridLayout_59->addLayout(gridLayout_60, 0, 0, 1, 1);

        tabsAdvanced->addTab(tabMisc, QString());
        tabTriggers = new QWidget();
        tabTriggers->setObjectName(QString::fromUtf8("tabTriggers"));
        gridLayout_4 = new QGridLayout(tabTriggers);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(3, 6, 3, 3);
        btnDelAuto = new QToolButton(tabTriggers);
        btnDelAuto->setObjectName(QString::fromUtf8("btnDelAuto"));
        sizePolicy3.setHeightForWidth(btnDelAuto->sizePolicy().hasHeightForWidth());
        btnDelAuto->setSizePolicy(sizePolicy3);
        btnDelAuto->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnDelAuto, 21, 1, 1, 1);

        label_68 = new QLabel(tabTriggers);
        label_68->setObjectName(QString::fromUtf8("label_68"));
        label_68->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_68, 13, 1, 1, 1);

        btnAddAutoRun = new QToolButton(tabTriggers);
        btnAddAutoRun->setObjectName(QString::fromUtf8("btnAddAutoRun"));
        sizePolicy3.setHeightForWidth(btnAddAutoRun->sizePolicy().hasHeightForWidth());
        btnAddAutoRun->setSizePolicy(sizePolicy3);
        btnAddAutoRun->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddAutoRun, 9, 1, 1, 1);

        label_66 = new QLabel(tabTriggers);
        label_66->setObjectName(QString::fromUtf8("label_66"));
        label_66->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_66, 1, 1, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_4->addItem(verticalSpacer_4, 19, 1, 1, 1);

        label_67 = new QLabel(tabTriggers);
        label_67->setObjectName(QString::fromUtf8("label_67"));
        label_67->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_67, 16, 1, 1, 1);

        treeTriggers = new QTreeWidget(tabTriggers);
        treeTriggers->headerItem()->setText(2, QString());
        treeTriggers->setObjectName(QString::fromUtf8("treeTriggers"));
        treeTriggers->setSortingEnabled(true);

        gridLayout_4->addWidget(treeTriggers, 1, 0, 21, 1);

        label_18 = new QLabel(tabTriggers);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_18, 11, 1, 1, 1);

        label_32 = new QLabel(tabTriggers);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setWordWrap(true);

        gridLayout_4->addWidget(label_32, 0, 0, 1, 1);

        btnAddRecoveryCmd = new QToolButton(tabTriggers);
        btnAddRecoveryCmd->setObjectName(QString::fromUtf8("btnAddRecoveryCmd"));
        sizePolicy3.setHeightForWidth(btnAddRecoveryCmd->sizePolicy().hasHeightForWidth());
        btnAddRecoveryCmd->setSizePolicy(sizePolicy3);
        btnAddRecoveryCmd->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddRecoveryCmd, 15, 1, 1, 1);

        label_33 = new QLabel(tabTriggers);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        label_33->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_33, 8, 1, 1, 1);

        btnAddDeleteCmd = new QToolButton(tabTriggers);
        btnAddDeleteCmd->setObjectName(QString::fromUtf8("btnAddDeleteCmd"));
        sizePolicy3.setHeightForWidth(btnAddDeleteCmd->sizePolicy().hasHeightForWidth());
        btnAddDeleteCmd->setSizePolicy(sizePolicy3);
        btnAddDeleteCmd->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddDeleteCmd, 18, 1, 1, 1);

        chkShowTriggersTmpl = new QCheckBox(tabTriggers);
        chkShowTriggersTmpl->setObjectName(QString::fromUtf8("chkShowTriggersTmpl"));

        gridLayout_4->addWidget(chkShowTriggersTmpl, 20, 1, 1, 1);

        btnAddAutoSvc = new QToolButton(tabTriggers);
        btnAddAutoSvc->setObjectName(QString::fromUtf8("btnAddAutoSvc"));
        sizePolicy3.setHeightForWidth(btnAddAutoSvc->sizePolicy().hasHeightForWidth());
        btnAddAutoSvc->setSizePolicy(sizePolicy3);
        btnAddAutoSvc->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddAutoSvc, 10, 1, 1, 1);

        btnAddAutoExec = new QToolButton(tabTriggers);
        btnAddAutoExec->setObjectName(QString::fromUtf8("btnAddAutoExec"));
        sizePolicy3.setHeightForWidth(btnAddAutoExec->sizePolicy().hasHeightForWidth());
        btnAddAutoExec->setSizePolicy(sizePolicy3);
        btnAddAutoExec->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddAutoExec, 2, 1, 1, 1);

        btnAddTerminateCmd = new QToolButton(tabTriggers);
        btnAddTerminateCmd->setObjectName(QString::fromUtf8("btnAddTerminateCmd"));
        sizePolicy.setHeightForWidth(btnAddTerminateCmd->sizePolicy().hasHeightForWidth());
        btnAddTerminateCmd->setSizePolicy(sizePolicy);
        btnAddTerminateCmd->setMinimumSize(QSize(0, 23));

        gridLayout_4->addWidget(btnAddTerminateCmd, 12, 1, 1, 1);

        tabsAdvanced->addTab(tabTriggers, QString());
        tabHideProc = new QWidget();
        tabHideProc->setObjectName(QString::fromUtf8("tabHideProc"));
        tabHideProc->setFont(font2);
        gridLayout_29 = new QGridLayout(tabHideProc);
        gridLayout_29->setObjectName(QString::fromUtf8("gridLayout_29"));
        gridLayout_29->setContentsMargins(3, 6, 3, 3);
        btnAddProcess = new QPushButton(tabHideProc);
        btnAddProcess->setObjectName(QString::fromUtf8("btnAddProcess"));

        gridLayout_29->addWidget(btnAddProcess, 2, 1, 1, 1);

        chkHideOtherBoxes = new QCheckBox(tabHideProc);
        chkHideOtherBoxes->setObjectName(QString::fromUtf8("chkHideOtherBoxes"));

        gridLayout_29->addWidget(chkHideOtherBoxes, 0, 0, 1, 1);

        verticalSpacer_16 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_29->addItem(verticalSpacer_16, 3, 1, 1, 1);

        label_24 = new QLabel(tabHideProc);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setWordWrap(true);

        gridLayout_29->addWidget(label_24, 1, 0, 1, 2);

        chkShowHiddenProcTmpl = new QCheckBox(tabHideProc);
        chkShowHiddenProcTmpl->setObjectName(QString::fromUtf8("chkShowHiddenProcTmpl"));

        gridLayout_29->addWidget(chkShowHiddenProcTmpl, 5, 1, 1, 1);

        treeHideProc = new QTreeWidget(tabHideProc);
        treeHideProc->headerItem()->setText(1, QString());
        treeHideProc->setObjectName(QString::fromUtf8("treeHideProc"));
        treeHideProc->setSortingEnabled(true);

        gridLayout_29->addWidget(treeHideProc, 2, 0, 5, 1);

        btnDelProcess = new QPushButton(tabHideProc);
        btnDelProcess->setObjectName(QString::fromUtf8("btnDelProcess"));

        gridLayout_29->addWidget(btnDelProcess, 6, 1, 1, 1);

        tabsAdvanced->addTab(tabHideProc, QString());
        tabUsers = new QWidget();
        tabUsers->setObjectName(QString::fromUtf8("tabUsers"));
        tabUsers->setFont(font2);
        gridLayout_25 = new QGridLayout(tabUsers);
        gridLayout_25->setObjectName(QString::fromUtf8("gridLayout_25"));
        gridLayout_25->setContentsMargins(3, 6, 3, 3);
        chkMonitorAdminOnly = new QCheckBox(tabUsers);
        chkMonitorAdminOnly->setObjectName(QString::fromUtf8("chkMonitorAdminOnly"));

        gridLayout_25->addWidget(chkMonitorAdminOnly, 4, 0, 1, 1);

        btnAddUser = new QPushButton(tabUsers);
        btnAddUser->setObjectName(QString::fromUtf8("btnAddUser"));

        gridLayout_25->addWidget(btnAddUser, 1, 1, 1, 1);

        verticalSpacer_14 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_25->addItem(verticalSpacer_14, 2, 1, 1, 1);

        lstUsers = new QListWidget(tabUsers);
        lstUsers->setObjectName(QString::fromUtf8("lstUsers"));

        gridLayout_25->addWidget(lstUsers, 1, 0, 3, 1);

        btnDelUser = new QPushButton(tabUsers);
        btnDelUser->setObjectName(QString::fromUtf8("btnDelUser"));

        gridLayout_25->addWidget(btnDelUser, 3, 1, 1, 1);

        label_23 = new QLabel(tabUsers);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setWordWrap(true);

        gridLayout_25->addWidget(label_23, 0, 0, 1, 2);

        tabsAdvanced->addTab(tabUsers, QString());
        tabTracing = new QWidget();
        tabTracing->setObjectName(QString::fromUtf8("tabTracing"));
        gridLayout_34 = new QGridLayout(tabTracing);
        gridLayout_34->setObjectName(QString::fromUtf8("gridLayout_34"));
        gridLayout_32 = new QGridLayout();
        gridLayout_32->setObjectName(QString::fromUtf8("gridLayout_32"));
        chkPipeTrace = new QCheckBox(tabTracing);
        chkPipeTrace->setObjectName(QString::fromUtf8("chkPipeTrace"));

        gridLayout_32->addWidget(chkPipeTrace, 5, 1, 1, 2);

        chkHookTrace = new QCheckBox(tabTracing);
        chkHookTrace->setObjectName(QString::fromUtf8("chkHookTrace"));

        gridLayout_32->addWidget(chkHookTrace, 11, 1, 1, 4);

        chkErrTrace = new QCheckBox(tabTracing);
        chkErrTrace->setObjectName(QString::fromUtf8("chkErrTrace"));

        gridLayout_32->addWidget(chkErrTrace, 13, 1, 1, 4);

        lblMonitor = new QLabel(tabTracing);
        lblMonitor->setObjectName(QString::fromUtf8("lblMonitor"));
        lblMonitor->setFont(font1);

        gridLayout_32->addWidget(lblMonitor, 0, 0, 1, 2);

        label_38 = new QLabel(tabTracing);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        label_38->setMaximumSize(QSize(20, 16777215));

        gridLayout_32->addWidget(label_38, 2, 1, 1, 1);

        chkKeyTrace = new QCheckBox(tabTracing);
        chkKeyTrace->setObjectName(QString::fromUtf8("chkKeyTrace"));

        gridLayout_32->addWidget(chkKeyTrace, 6, 1, 1, 2);

        chkFileTrace = new QCheckBox(tabTracing);
        chkFileTrace->setObjectName(QString::fromUtf8("chkFileTrace"));

        gridLayout_32->addWidget(chkFileTrace, 4, 1, 1, 2);

        chkIpcTrace = new QCheckBox(tabTracing);
        chkIpcTrace->setObjectName(QString::fromUtf8("chkIpcTrace"));

        gridLayout_32->addWidget(chkIpcTrace, 7, 1, 1, 2);

        chkDbgTrace = new QCheckBox(tabTracing);
        chkDbgTrace->setObjectName(QString::fromUtf8("chkDbgTrace"));

        gridLayout_32->addWidget(chkDbgTrace, 12, 1, 1, 4);

        horizontalSpacer = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_32->addItem(horizontalSpacer, 14, 4, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_32->addItem(horizontalSpacer_3, 14, 5, 1, 1);

        chkNetFwTrace = new QCheckBox(tabTracing);
        chkNetFwTrace->setObjectName(QString::fromUtf8("chkNetFwTrace"));

        gridLayout_32->addWidget(chkNetFwTrace, 10, 1, 1, 3);

        chkGuiTrace = new QCheckBox(tabTracing);
        chkGuiTrace->setObjectName(QString::fromUtf8("chkGuiTrace"));

        gridLayout_32->addWidget(chkGuiTrace, 8, 1, 1, 2);

        label_26 = new QLabel(tabTracing);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_26->setWordWrap(true);

        gridLayout_32->addWidget(label_26, 4, 3, 5, 2);

        chkDisableMonitor = new QCheckBox(tabTracing);
        chkDisableMonitor->setObjectName(QString::fromUtf8("chkDisableMonitor"));

        gridLayout_32->addWidget(chkDisableMonitor, 1, 1, 1, 4);

        lblTracing = new QLabel(tabTracing);
        lblTracing->setObjectName(QString::fromUtf8("lblTracing"));
        lblTracing->setFont(font1);

        gridLayout_32->addWidget(lblTracing, 2, 0, 1, 1);

        verticalSpacer_19 = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_32->addItem(verticalSpacer_19, 14, 0, 1, 1);

        chkComTrace = new QCheckBox(tabTracing);
        chkComTrace->setObjectName(QString::fromUtf8("chkComTrace"));

        gridLayout_32->addWidget(chkComTrace, 9, 1, 1, 3);

        chkCallTrace = new QCheckBox(tabTracing);
        chkCallTrace->setObjectName(QString::fromUtf8("chkCallTrace"));

        gridLayout_32->addWidget(chkCallTrace, 3, 1, 1, 4);


        gridLayout_34->addLayout(gridLayout_32, 0, 0, 1, 1);

        tabsAdvanced->addTab(tabTracing, QString());
        tabDebug = new QWidget();
        tabDebug->setObjectName(QString::fromUtf8("tabDebug"));
        tabDebug->setFont(font2);
        gridLayout_31 = new QGridLayout(tabDebug);
        gridLayout_31->setObjectName(QString::fromUtf8("gridLayout_31"));
        gridLayout_31->setContentsMargins(3, 6, 3, 3);
        scrollArea = new QScrollArea(tabDebug);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setWidgetResizable(true);
        dbgWidget = new QWidget();
        dbgWidget->setObjectName(QString::fromUtf8("dbgWidget"));
        dbgWidget->setGeometry(QRect(0, 0, 92, 16));
        dbgLayout = new QGridLayout(dbgWidget);
        dbgLayout->setObjectName(QString::fromUtf8("dbgLayout"));
        dbgLayout->setContentsMargins(0, 0, 0, 0);
        scrollArea->setWidget(dbgWidget);

        gridLayout_31->addWidget(scrollArea, 2, 0, 1, 1);

        label_25 = new QLabel(tabDebug);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setFont(font1);
        label_25->setWordWrap(true);

        gridLayout_31->addWidget(label_25, 0, 0, 1, 1);

        label_17 = new QLabel(tabDebug);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setWordWrap(true);

        gridLayout_31->addWidget(label_17, 1, 0, 1, 1);

        tabsAdvanced->addTab(tabDebug, QString());

        gridLayout_121->addWidget(tabsAdvanced, 0, 0, 1, 1);

        tabs->addTab(tabAdvanced, QString());
        tabTemplates = new QWidget();
        tabTemplates->setObjectName(QString::fromUtf8("tabTemplates"));
        gridLayout_5 = new QGridLayout(tabTemplates);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        tabsTemplates = new QTabWidget(tabTemplates);
        tabsTemplates->setObjectName(QString::fromUtf8("tabsTemplates"));
        tab_11 = new QWidget();
        tab_11->setObjectName(QString::fromUtf8("tab_11"));
        gridLayout_41 = new QGridLayout(tab_11);
        gridLayout_41->setObjectName(QString::fromUtf8("gridLayout_41"));
        gridLayout_41->setContentsMargins(3, 6, 3, 3);
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        cmbCategories = new QComboBox(tab_11);
        cmbCategories->setObjectName(QString::fromUtf8("cmbCategories"));

        gridLayout_3->addWidget(cmbCategories, 1, 1, 1, 1);

        label_13 = new QLabel(tab_11);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setWordWrap(true);

        gridLayout_3->addWidget(label_13, 0, 0, 1, 4);

        verticalSpacer_20 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_3->addItem(verticalSpacer_20, 3, 4, 1, 1);

        label_15 = new QLabel(tab_11);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_3->addWidget(label_15, 1, 2, 1, 1);

        txtTemplates = new QLineEdit(tab_11);
        txtTemplates->setObjectName(QString::fromUtf8("txtTemplates"));

        gridLayout_3->addWidget(txtTemplates, 1, 3, 1, 1);

        btnAddTemplate = new QToolButton(tab_11);
        btnAddTemplate->setObjectName(QString::fromUtf8("btnAddTemplate"));
        sizePolicy3.setHeightForWidth(btnAddTemplate->sizePolicy().hasHeightForWidth());
        btnAddTemplate->setSizePolicy(sizePolicy3);
        btnAddTemplate->setMinimumSize(QSize(0, 23));

        gridLayout_3->addWidget(btnAddTemplate, 2, 4, 1, 1);

        label_6 = new QLabel(tab_11);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(label_6, 1, 0, 1, 1);

        treeTemplates = new QTreeWidget(tab_11);
        treeTemplates->setObjectName(QString::fromUtf8("treeTemplates"));
        treeTemplates->setSelectionMode(QAbstractItemView::ExtendedSelection);
        treeTemplates->setSortingEnabled(true);

        gridLayout_3->addWidget(treeTemplates, 2, 0, 4, 4);

        btnDelTemplate = new QToolButton(tab_11);
        btnDelTemplate->setObjectName(QString::fromUtf8("btnDelTemplate"));
        sizePolicy3.setHeightForWidth(btnDelTemplate->sizePolicy().hasHeightForWidth());
        btnDelTemplate->setSizePolicy(sizePolicy3);
        btnDelTemplate->setMinimumSize(QSize(0, 23));

        gridLayout_3->addWidget(btnDelTemplate, 4, 4, 1, 1);


        gridLayout_41->addLayout(gridLayout_3, 0, 0, 1, 1);

        tabsTemplates->addTab(tab_11, QString());
        tab_12 = new QWidget();
        tab_12->setObjectName(QString::fromUtf8("tab_12"));
        gridLayout_45 = new QGridLayout(tab_12);
        gridLayout_45->setObjectName(QString::fromUtf8("gridLayout_45"));
        gridLayout_45->setContentsMargins(3, 6, 3, 3);
        gridLayout_42 = new QGridLayout();
        gridLayout_42->setObjectName(QString::fromUtf8("gridLayout_42"));
        label_46 = new QLabel(tab_12);
        label_46->setObjectName(QString::fromUtf8("label_46"));

        gridLayout_42->addWidget(label_46, 0, 0, 1, 1);

        verticalSpacer_22 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_42->addItem(verticalSpacer_22, 0, 1, 1, 1);

        treeFolders = new QTreeWidget(tab_12);
        treeFolders->setObjectName(QString::fromUtf8("treeFolders"));
        treeFolders->setSortingEnabled(true);

        gridLayout_42->addWidget(treeFolders, 1, 0, 1, 2);


        gridLayout_45->addLayout(gridLayout_42, 0, 0, 1, 1);

        tabsTemplates->addTab(tab_12, QString());
        tab_13 = new QWidget();
        tab_13->setObjectName(QString::fromUtf8("tab_13"));
        gridLayout_44 = new QGridLayout(tab_13);
        gridLayout_44->setObjectName(QString::fromUtf8("gridLayout_44"));
        gridLayout_44->setContentsMargins(3, 6, 3, 3);
        gridLayout_43 = new QGridLayout();
        gridLayout_43->setObjectName(QString::fromUtf8("gridLayout_43"));
        chkScreenReaders = new QCheckBox(tab_13);
        chkScreenReaders->setObjectName(QString::fromUtf8("chkScreenReaders"));

        gridLayout_43->addWidget(chkScreenReaders, 2, 2, 1, 1);

        label_45 = new QLabel(tab_13);
        label_45->setObjectName(QString::fromUtf8("label_45"));

        gridLayout_43->addWidget(label_45, 3, 2, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_43->addItem(horizontalSpacer_6, 1, 0, 1, 1);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_43->addItem(horizontalSpacer_11, 1, 4, 1, 1);

        label_43 = new QLabel(tab_13);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setWordWrap(true);

        gridLayout_43->addWidget(label_43, 0, 1, 1, 3);

        horizontalSpacer_15 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_43->addItem(horizontalSpacer_15, 2, 1, 1, 1);

        horizontalSpacer_14 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_43->addItem(horizontalSpacer_14, 2, 3, 1, 1);

        label_44 = new QLabel(tab_13);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        label_44->setWordWrap(true);

        gridLayout_43->addWidget(label_44, 1, 1, 1, 2);


        gridLayout_44->addLayout(gridLayout_43, 0, 0, 1, 1);

        tabsTemplates->addTab(tab_13, QString());

        gridLayout_5->addWidget(tabsTemplates, 1, 0, 1, 1);

        tabs->addTab(tabTemplates, QString());
        tabEdit = new QWidget();
        tabEdit->setObjectName(QString::fromUtf8("tabEdit"));
        gridLayout = new QGridLayout(tabEdit);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnEditIni = new QPushButton(tabEdit);
        btnEditIni->setObjectName(QString::fromUtf8("btnEditIni"));
        btnEditIni->setCheckable(false);

        gridLayout->addWidget(btnEditIni, 0, 0, 1, 1);

        btnCancelEdit = new QPushButton(tabEdit);
        btnCancelEdit->setObjectName(QString::fromUtf8("btnCancelEdit"));
        btnCancelEdit->setEnabled(false);

        gridLayout->addWidget(btnCancelEdit, 0, 3, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_7, 0, 1, 1, 1);

        btnSaveIni = new QPushButton(tabEdit);
        btnSaveIni->setObjectName(QString::fromUtf8("btnSaveIni"));
        btnSaveIni->setEnabled(false);

        gridLayout->addWidget(btnSaveIni, 0, 2, 1, 1);

        txtIniSection = new QPlainTextEdit(tabEdit);
        txtIniSection->setObjectName(QString::fromUtf8("txtIniSection"));
        txtIniSection->setLineWrapMode(QPlainTextEdit::NoWrap);

        gridLayout->addWidget(txtIniSection, 1, 0, 1, 4);

        tabs->addTab(tabEdit, QString());

        verticalLayout->addWidget(tabs);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        buttonBox = new QDialogButtonBox(OptionsWindow);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);

        QWidget::setTabOrder(tabs, tabsGeneral);
        QWidget::setTabOrder(tabsGeneral, cmbBoxIndicator);
        QWidget::setTabOrder(cmbBoxIndicator, cmbBoxBorder);
        QWidget::setTabOrder(cmbBoxBorder, btnBorderColor);
        QWidget::setTabOrder(btnBorderColor, spinBorderWidth);
        QWidget::setTabOrder(spinBorderWidth, treeRun);
        QWidget::setTabOrder(treeRun, btnAddCmd);
        QWidget::setTabOrder(btnAddCmd, btnDelCmd);
        QWidget::setTabOrder(btnDelCmd, chkAutoEmpty);
        QWidget::setTabOrder(chkAutoEmpty, chkProtectBox);
        QWidget::setTabOrder(chkProtectBox, treeTriggers);
        QWidget::setTabOrder(treeTriggers, btnDelAuto);
        QWidget::setTabOrder(btnDelAuto, treeGroups);
        QWidget::setTabOrder(treeGroups, btnAddGroup);
        QWidget::setTabOrder(btnAddGroup, btnAddProg);
        QWidget::setTabOrder(btnAddProg, btnDelProg);
        QWidget::setTabOrder(btnDelProg, treeStop);
        QWidget::setTabOrder(treeStop, btnAddLingering);
        QWidget::setTabOrder(btnAddLingering, chkShowStopTmpl);
        QWidget::setTabOrder(chkShowStopTmpl, btnDelStopProg);
        QWidget::setTabOrder(btnDelStopProg, radStartAll);
        QWidget::setTabOrder(radStartAll, radStartExcept);
        QWidget::setTabOrder(radStartExcept, radStartSelected);
        QWidget::setTabOrder(radStartSelected, treeStart);
        QWidget::setTabOrder(treeStart, btnAddStartProg);
        QWidget::setTabOrder(btnAddStartProg, btnDelStartProg);
        QWidget::setTabOrder(btnDelStartProg, chkStartBlockMsg);
        QWidget::setTabOrder(chkStartBlockMsg, chkINetBlockPrompt);
        QWidget::setTabOrder(chkINetBlockPrompt, treeINet);
        QWidget::setTabOrder(treeINet, btnAddINetProg);
        QWidget::setTabOrder(btnAddINetProg, btnDelINetProg);
        QWidget::setTabOrder(btnDelINetProg, chkINetBlockMsg);
        QWidget::setTabOrder(chkINetBlockMsg, treeRecovery);
        QWidget::setTabOrder(treeRecovery, btnAddRecovery);
        QWidget::setTabOrder(btnAddRecovery, chkShowRecoveryTmpl);
        QWidget::setTabOrder(chkShowRecoveryTmpl, btnDelRecovery);
        QWidget::setTabOrder(btnDelRecovery, tabsAdvanced);
        QWidget::setTabOrder(tabsAdvanced, chkHideOtherBoxes);
        QWidget::setTabOrder(chkHideOtherBoxes, btnAddProcess);
        QWidget::setTabOrder(btnAddProcess, btnDelProcess);
        QWidget::setTabOrder(btnDelProcess, lstUsers);
        QWidget::setTabOrder(lstUsers, btnAddUser);
        QWidget::setTabOrder(btnAddUser, btnDelUser);
        QWidget::setTabOrder(btnDelUser, chkMonitorAdminOnly);
        QWidget::setTabOrder(chkMonitorAdminOnly, chkFileTrace);
        QWidget::setTabOrder(chkFileTrace, chkPipeTrace);
        QWidget::setTabOrder(chkPipeTrace, chkKeyTrace);
        QWidget::setTabOrder(chkKeyTrace, chkIpcTrace);
        QWidget::setTabOrder(chkIpcTrace, chkGuiTrace);
        QWidget::setTabOrder(chkGuiTrace, chkComTrace);
        QWidget::setTabOrder(chkComTrace, chkDbgTrace);
        QWidget::setTabOrder(chkDbgTrace, scrollArea);
        QWidget::setTabOrder(scrollArea, treeTemplates);
        QWidget::setTabOrder(treeTemplates, cmbCategories);
        QWidget::setTabOrder(cmbCategories, txtTemplates);
        QWidget::setTabOrder(txtTemplates, btnEditIni);
        QWidget::setTabOrder(btnEditIni, txtIniSection);
        QWidget::setTabOrder(txtIniSection, btnSaveIni);
        QWidget::setTabOrder(btnSaveIni, btnCancelEdit);

        retranslateUi(OptionsWindow);

        tabs->setCurrentIndex(10);
        tabsGeneral->setCurrentIndex(0);
        tabsSecurity->setCurrentIndex(0);
        tabsForce->setCurrentIndex(0);
        tabsStop->setCurrentIndex(0);
        tabsAccess->setCurrentIndex(5);
        tabsInternet->setCurrentIndex(0);
        tabsRecovery->setCurrentIndex(0);
        tabsOther->setCurrentIndex(0);
        tabsAdvanced->setCurrentIndex(1);
        tabsTemplates->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(OptionsWindow);
    } // setupUi

    void retranslateUi(QWidget *OptionsWindow)
    {
        OptionsWindow->setWindowTitle(QCoreApplication::translate("OptionsWindow", "SandboxiePlus Options", nullptr));
        label_30->setText(QString());
        chkPinToTray->setText(QCoreApplication::translate("OptionsWindow", "Always show this sandbox in the systray list (Pinned)", nullptr));
        label_20->setText(QCoreApplication::translate("OptionsWindow", "Sandbox Indicator in title:", nullptr));
        label_21->setText(QCoreApplication::translate("OptionsWindow", "Sandboxed window border:", nullptr));
        lblSupportCert->setText(QCoreApplication::translate("OptionsWindow", "<b>More Box Types</b> are exclusively available to <u>project supporters</u>, the Privacy Enhanced boxes <b><font color='red'>protect user data from illicit access</font></b> by the sandboxed programs.<br />If you are not yet a supporter, then please consider <a href=\"https://sandboxie-plus.com/go.php?to=sbie-get-cert\">supporting the project</a>, to receive a <a href=\"https://sandboxie-plus.com/go.php?to=sbie-cert\">supporter certificate</a>.<br />You can test the other box types by creating new sandboxes of those types, however processes in these will be auto terminated after 5 minutes.", nullptr));
        btnBorderColor->setText(QString());
        label_58->setText(QString());
        chkShowForRun->setText(QCoreApplication::translate("OptionsWindow", "Show this box in the 'run in box' selection prompt", nullptr));
        lblBoxInfo->setText(QCoreApplication::translate("OptionsWindow", "Box info", nullptr));
        label_57->setText(QCoreApplication::translate("OptionsWindow", "Box Type Preset:", nullptr));
        label_14->setText(QCoreApplication::translate("OptionsWindow", "px Width", nullptr));
        lblBoxType->setText(QCoreApplication::translate("OptionsWindow", "General Configuration", nullptr));
        lblAppearance->setText(QCoreApplication::translate("OptionsWindow", "Appearance", nullptr));
        label_70->setText(QCoreApplication::translate("OptionsWindow", "Double click action:", nullptr));
        tabsGeneral->setTabText(tabsGeneral->indexOf(tabOptions), QCoreApplication::translate("OptionsWindow", "Box Options", nullptr));
        lblRawDisk->setText(QCoreApplication::translate("OptionsWindow", "Disk/File access", nullptr));
        chkUseVolumeSerialNumbers->setText(QCoreApplication::translate("OptionsWindow", "Use volume serial numbers for drives, like: \\drive\\C~1234-ABCD", nullptr));
        label_37->setText(QString());
        chkEncrypt->setText(QCoreApplication::translate("OptionsWindow", "Encrypt sandbox content", nullptr));
        chkAutoEmpty->setText(QCoreApplication::translate("OptionsWindow", "Auto delete content when last sandboxed process terminates", nullptr));
        lblCrypto->setText(QCoreApplication::translate("OptionsWindow", "When <a href=\"sbie://docs/boxencryption\">Box Encryption</a> is enabled the box\342\200\231s root folder, including its registry hive, is stored in an encrypted disk image, using <a href=\"https://diskcryptor.org\">Disk Cryptor's</a> AES-XTS implementation.", nullptr));
        lblDelete->setText(QCoreApplication::translate("OptionsWindow", "Box Delete options", nullptr));
        chkRawDiskRead->setText(QCoreApplication::translate("OptionsWindow", "Allow elevated sandboxed applications to read the harddrive", nullptr));
#if QT_CONFIG(tooltip)
        chkProtectBox->setToolTip(QCoreApplication::translate("OptionsWindow", "Partially checked means prevent box removal but not content deletion.", nullptr));
#endif // QT_CONFIG(tooltip)
        chkProtectBox->setText(QCoreApplication::translate("OptionsWindow", "Protect this sandbox from deletion or emptying", nullptr));
        lblImDisk->setText(QCoreApplication::translate("OptionsWindow", "<a href=\"addon://ImDisk\">Install ImDisk</a> driver to enable Ram Disk and Disk Image support.", nullptr));
        chkSeparateUserFolders->setText(QCoreApplication::translate("OptionsWindow", "Separate user folders", nullptr));
        lblStructure->setText(QCoreApplication::translate("OptionsWindow", "Box Structure", nullptr));
        chkRamBox->setText(QCoreApplication::translate("OptionsWindow", "Store the sandbox content in a Ram Disk", nullptr));
        chkRawDiskNotify->setText(QCoreApplication::translate("OptionsWindow", "Warn when an application opens a harddrive handle", nullptr));
        btnPassword->setText(QCoreApplication::translate("OptionsWindow", "Set Password", nullptr));
        lblScheme->setText(QCoreApplication::translate("OptionsWindow", "Virtualization scheme", nullptr));
        lblWhenEmpty->setText(QCoreApplication::translate("OptionsWindow", "The box structure can only be changed when the sandbox is empty", nullptr));
        tabsGeneral->setTabText(tabsGeneral->indexOf(tabFile), QCoreApplication::translate("OptionsWindow", "File Options", nullptr));
        chkCopyLimit->setText(QCoreApplication::translate("OptionsWindow", "Copy file size limit:", nullptr));
        chkCopyPrompt->setText(QCoreApplication::translate("OptionsWindow", "Prompt user for large file migration", nullptr));
        lblMigration->setText(QCoreApplication::translate("OptionsWindow", "File Migration", nullptr));
#if QT_CONFIG(tooltip)
        chkNoCopyMsg->setToolTip(QCoreApplication::translate("OptionsWindow", "2113: Content of migrated file was discarded\n"
"2114: File was not migrated, write access to file was denied\n"
"2115: File was not migrated, file will be opened read only", nullptr));
#endif // QT_CONFIG(tooltip)
        chkNoCopyMsg->setText(QCoreApplication::translate("OptionsWindow", "Issue message 2113/2114/2115 when a file is not fully migrated", nullptr));
        lblCopyLimit->setText(QCoreApplication::translate("OptionsWindow", "kilobytes", nullptr));
        btnAddCopy->setText(QCoreApplication::translate("OptionsWindow", "Add Pattern", nullptr));
        btnDelCopy->setText(QCoreApplication::translate("OptionsWindow", "Remove Pattern", nullptr));
        chkShowCopyTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeCopy->headerItem();
        ___qtreewidgetitem->setText(2, QCoreApplication::translate("OptionsWindow", "Pattern", nullptr));
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("OptionsWindow", "Action", nullptr));
        label_31->setText(QCoreApplication::translate("OptionsWindow", "Sandboxie does not allow writing to host files, unless permitted by the user. When a sandboxed application attempts to modify a file, the entire file must be copied into the sandbox, for large files this can take a significate amount of time. Sandboxie offers options for handling these cases, which can be configured on this page.", nullptr));
        label_34->setText(QCoreApplication::translate("OptionsWindow", "Using wildcard patterns file specific behavior can be configured in the list below:", nullptr));
        chkNoCopyWarn->setText(QCoreApplication::translate("OptionsWindow", "Issue message 2102 when a file is too large", nullptr));
        chkDenyWrite->setText(QCoreApplication::translate("OptionsWindow", "When a file cannot be migrated, open it in read-only mode instead", nullptr));
        tabsGeneral->setTabText(tabsGeneral->indexOf(tabMigration), QCoreApplication::translate("OptionsWindow", "File Migration", nullptr));
        chkOpenCredentials->setText(QCoreApplication::translate("OptionsWindow", "Open Windows Credentials Store (user mode)", nullptr));
        chkCloseClipBoard->setText(QCoreApplication::translate("OptionsWindow", "Block read access to the clipboard", nullptr));
        chkBlockNetParam->setText(QCoreApplication::translate("OptionsWindow", "Prevent change to network and firewall parameters (user mode)", nullptr));
        chkVmRead->setText(QCoreApplication::translate("OptionsWindow", "Allow to read memory of unsandboxed processes (not recommended)", nullptr));
        chkBlockSpooler->setText(QCoreApplication::translate("OptionsWindow", "Block access to the printer spooler", nullptr));
        chkPrintToFile->setText(QCoreApplication::translate("OptionsWindow", "Allow the print spooler to print to files outside the sandbox", nullptr));
        chkBlockNetShare->setText(QCoreApplication::translate("OptionsWindow", "Block network files and folders, unless specifically opened.", nullptr));
        chkOpenSpooler->setText(QCoreApplication::translate("OptionsWindow", "Remove spooler restriction, printers can be installed outside the sandbox", nullptr));
        chkOpenProtectedStorage->setText(QCoreApplication::translate("OptionsWindow", "Open System Protected Storage", nullptr));
        chkVmReadNotify->setText(QCoreApplication::translate("OptionsWindow", "Issue message 2111 when a process access is denied", nullptr));
#if QT_CONFIG(tooltip)
        lblOther->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the system from sandboxed processes", nullptr));
#endif // QT_CONFIG(tooltip)
        lblOther->setText(QCoreApplication::translate("OptionsWindow", "Other restrictions", nullptr));
        label_39->setText(QString());
#if QT_CONFIG(tooltip)
        lblPrinting->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the system from sandboxed processes", nullptr));
#endif // QT_CONFIG(tooltip)
        lblPrinting->setText(QCoreApplication::translate("OptionsWindow", "Printing restrictions", nullptr));
#if QT_CONFIG(tooltip)
        lblNetwork->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the system from sandboxed processes", nullptr));
#endif // QT_CONFIG(tooltip)
        lblNetwork->setText(QCoreApplication::translate("OptionsWindow", "Network restrictions", nullptr));
        tabsGeneral->setTabText(tabsGeneral->indexOf(tabRestrictions), QCoreApplication::translate("OptionsWindow", "Restrictions", nullptr));
        btnDelCmd->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        btnAddCmd->setText(QCoreApplication::translate("OptionsWindow", "Add program", nullptr));
        QTreeWidgetItem *___qtreewidgetitem1 = treeRun->headerItem();
        ___qtreewidgetitem1->setText(1, QCoreApplication::translate("OptionsWindow", "Command Line", nullptr));
        ___qtreewidgetitem1->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        btnCmdUp->setText(QCoreApplication::translate("OptionsWindow", "Move Up", nullptr));
        btnCmdDown->setText(QCoreApplication::translate("OptionsWindow", "Move Down", nullptr));
        label_8->setText(QCoreApplication::translate("OptionsWindow", "You can configure custom entries for the sandbox run menu.", nullptr));
        tabsGeneral->setTabText(tabsGeneral->indexOf(tabRun), QCoreApplication::translate("OptionsWindow", "Run Menu", nullptr));
        tabs->setTabText(tabs->indexOf(tabGeneral), QCoreApplication::translate("OptionsWindow", "General Options", nullptr));
        chkLockDown->setText(QCoreApplication::translate("OptionsWindow", "Use the original token only for approved NT system calls", nullptr));
        chkSecurityMode->setText(QCoreApplication::translate("OptionsWindow", "Enable all security enhancements (make security hardened box)", nullptr));
#if QT_CONFIG(tooltip)
        lblElevation->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the system from sandboxed processes", nullptr));
#endif // QT_CONFIG(tooltip)
        lblElevation->setText(QCoreApplication::translate("OptionsWindow", "Elevation restrictions", nullptr));
        chkFakeElevation->setText(QCoreApplication::translate("OptionsWindow", "Make applications think they are running elevated (allows to run installers safely)", nullptr));
        label_40->setText(QCoreApplication::translate("OptionsWindow", "(Recommended)", nullptr));
        chkRestrictDevices->setText(QCoreApplication::translate("OptionsWindow", "Restrict driver/device access to only approved ones", nullptr));
#if QT_CONFIG(tooltip)
        lblSecurity->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the system from sandboxed processes", nullptr));
#endif // QT_CONFIG(tooltip)
        lblSecurity->setText(QCoreApplication::translate("OptionsWindow", "Security enhancements", nullptr));
        chkMsiExemptions->setText(QCoreApplication::translate("OptionsWindow", "Allow MSIServer to run with a sandboxed system token and apply other exceptions if required", nullptr));
        chkDropRights->setText(QCoreApplication::translate("OptionsWindow", "Drop rights from Administrators and Power Users groups", nullptr));
        lblAdmin->setText(QCoreApplication::translate("OptionsWindow", "CAUTION: When running under the built in administrator, processes can not drop administrative privileges.", nullptr));
        label_28->setText(QCoreApplication::translate("OptionsWindow", "Note: Msi Installer Exemptions should not be required, but if you encounter issues installing a msi package which you trust, this option may help the installation complete successfully. You can also try disabling drop admin rights.", nullptr));
        label_35->setText(QCoreApplication::translate("OptionsWindow", "Security note: Elevated applications running under the supervision of Sandboxie, with an admin or system token, have more opportunities to bypass isolation and modify the system outside the sandbox.", nullptr));
        label_41->setText(QString());
        tabsSecurity->setTabText(tabsSecurity->indexOf(tabHarden), QCoreApplication::translate("OptionsWindow", "Security Hardening", nullptr));
        label_42->setText(QCoreApplication::translate("OptionsWindow", "Various isolation features can break compatibility with some applications. If you are using this sandbox <b>NOT for Security</b> but for application portability, by changing these options you can restore compatibility by sacrificing some security.", nullptr));
        chkNoSecurityIsolation->setText(QCoreApplication::translate("OptionsWindow", "Disable Security Isolation", nullptr));
        chkOpenSamEndpoint->setText(QCoreApplication::translate("OptionsWindow", "Open access to Windows Security Account Manager", nullptr));
        chkNoSecurityFiltering->setText(QCoreApplication::translate("OptionsWindow", "Disable Security Filtering (not recommended)", nullptr));
        label_61->setText(QCoreApplication::translate("OptionsWindow", "Security Filtering used by Sandboxie to enforce filesystem and registry access restrictions, as well as to restrict process access.", nullptr));
        label_60->setText(QCoreApplication::translate("OptionsWindow", "Security Isolation through the usage of a heavily restricted process token is Sandboxie's primary means of enforcing sandbox restrictions, when this is disabled the box is operated in the application compartment mode, i.e. it\342\200\231s no longer providing reliable security, just simple application compartmentalization.", nullptr));
        chkOpenLsaEndpoint->setText(QCoreApplication::translate("OptionsWindow", "Open access to Windows Local Security Authority", nullptr));
#if QT_CONFIG(tooltip)
        lblIsolation->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblIsolation->setText(QCoreApplication::translate("OptionsWindow", "Security Isolation & Filtering", nullptr));
#if QT_CONFIG(tooltip)
        lblAccess->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblAccess->setText(QCoreApplication::translate("OptionsWindow", "Access Isolation", nullptr));
        chkOpenDevCMApi->setText(QCoreApplication::translate("OptionsWindow", "Allow sandboxed programs to manage Hardware/Devices", nullptr));
        label_62->setText(QCoreApplication::translate("OptionsWindow", "The below options can be used safely when you don't grant admin rights.", nullptr));
        tabsSecurity->setTabText(tabsSecurity->indexOf(tabIsolation), QCoreApplication::translate("OptionsWindow", "Security Isolation", nullptr));
        btnHostProcessDeny->setText(QCoreApplication::translate("OptionsWindow", "Deny Process", nullptr));
        btnDelHostProcess->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkNotifyProtect->setText(QCoreApplication::translate("OptionsWindow", "Issue message 1318/1317 when a host process tries to access a sandboxed process/the box root", nullptr));
        QTreeWidgetItem *___qtreewidgetitem2 = treeHostProc->headerItem();
        ___qtreewidgetitem2->setText(1, QCoreApplication::translate("OptionsWindow", "Action", nullptr));
        ___qtreewidgetitem2->setText(0, QCoreApplication::translate("OptionsWindow", "Process", nullptr));
        chkShowHostProcTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
#if QT_CONFIG(tooltip)
        lblBoxProtection->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblBoxProtection->setText(QCoreApplication::translate("OptionsWindow", "Box Protection", nullptr));
        label_52->setText(QCoreApplication::translate("OptionsWindow", "Sandboxie-Plus is able to create confidential sandboxes that provide robust protection against unauthorized surveillance or tampering by host processes. By utilizing an encrypted sandbox image, this feature delivers the highest level of operational confidentiality, ensuring the safety and integrity of sandboxed processes.", nullptr));
        btnHostProcessAllow->setText(QCoreApplication::translate("OptionsWindow", "Allow Process", nullptr));
        label_5->setText(QCoreApplication::translate("OptionsWindow", "Protect processes in this box from being accessed by specified unsandboxed host processes.", nullptr));
        label_47->setText(QString());
        chkLessConfidential->setText(QCoreApplication::translate("OptionsWindow", "Allow useful Windows processes access to protected processes", nullptr));
        chkConfidential->setText(QCoreApplication::translate("OptionsWindow", "Protect processes within this box from host processes", nullptr));
        tabsSecurity->setTabText(tabsSecurity->indexOf(tabPrivate), QCoreApplication::translate("OptionsWindow", "Box Protection", nullptr));
        chkSbieLogon->setText(QCoreApplication::translate("OptionsWindow", "Use a Sandboxie login instead of an anonymous token", nullptr));
#if QT_CONFIG(tooltip)
        lblFence->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblFence->setText(QCoreApplication::translate("OptionsWindow", "Other isolation", nullptr));
        chkElevateRpcss->setText(QCoreApplication::translate("OptionsWindow", "Start the sandboxed RpcSs as a SYSTEM process (not recommended)", nullptr));
#if QT_CONFIG(tooltip)
        lblPrivilege->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblPrivilege->setText(QCoreApplication::translate("OptionsWindow", "Privilege isolation", nullptr));
        chkProtectSystem->setText(QCoreApplication::translate("OptionsWindow", "Protect sandboxed SYSTEM processes from unprivileged processes", nullptr));
#if QT_CONFIG(tooltip)
        lblToken->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblToken->setText(QCoreApplication::translate("OptionsWindow", "Sandboxie token", nullptr));
        label_65->setText(QCoreApplication::translate("OptionsWindow", "(Security Critical)", nullptr));
        chkDropPrivileges->setText(QCoreApplication::translate("OptionsWindow", "Drop critical privileges from processes running with a SYSTEM token", nullptr));
        chkRestrictServices->setText(QCoreApplication::translate("OptionsWindow", "Do not start sandboxed services using a system token (recommended)", nullptr));
        chkProtectSCM->setText(QCoreApplication::translate("OptionsWindow", "Allow only privileged processes to access the Service Control Manager", nullptr));
        label_64->setText(QCoreApplication::translate("OptionsWindow", "(Security Critical)", nullptr));
        chkAddToJob->setText(QCoreApplication::translate("OptionsWindow", "Add sandboxed processes to job objects (recommended)", nullptr));
        label_74->setText(QCoreApplication::translate("OptionsWindow", "Using a custom Sandboxie Token allows to isolate individual sandboxes from each other better, and it shows in the user column of task managers the name of the box a process belongs to. Some 3rd party security solutions may however have problems with custom tokens.", nullptr));
        tabsSecurity->setTabText(tabsSecurity->indexOf(tabPrivileges), QCoreApplication::translate("OptionsWindow", "Advanced Security", nullptr));
        tabs->setTabText(tabs->indexOf(tabSecurity), QCoreApplication::translate("OptionsWindow", "Security Options", nullptr));
        chkShowGroupTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnAddGroup->setText(QCoreApplication::translate("OptionsWindow", "Add Group", nullptr));
        btnAddProg->setText(QCoreApplication::translate("OptionsWindow", "Add Program", nullptr));
        label_3->setText(QCoreApplication::translate("OptionsWindow", "You can group programs together and give them a group name.  Program groups can be used with some of the settings instead of program names. Groups defined for the box overwrite groups defined in templates.", nullptr));
        QTreeWidgetItem *___qtreewidgetitem3 = treeGroups->headerItem();
        ___qtreewidgetitem3->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        btnDelProg->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        tabs->setTabText(tabs->indexOf(tabGroups), QCoreApplication::translate("OptionsWindow", "Program Groups", nullptr));
        label->setText(QCoreApplication::translate("OptionsWindow", "Programs entered here, or programs started from entered locations, will be put in this sandbox automatically, unless they are explicitly started in another sandbox.", nullptr));
        btnForceDir->setText(QCoreApplication::translate("OptionsWindow", "Force Folder", nullptr));
        chkShowForceTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        QTreeWidgetItem *___qtreewidgetitem4 = treeForced->headerItem();
        ___qtreewidgetitem4->setText(1, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        ___qtreewidgetitem4->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        btnForceProg->setText(QCoreApplication::translate("OptionsWindow", "Force Program", nullptr));
        btnDelForce->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkDisableForced->setText(QCoreApplication::translate("OptionsWindow", "Disable forced Process and Folder for this sandbox", nullptr));
        tabsForce->setTabText(tabsForce->indexOf(tabForceProgs), QCoreApplication::translate("OptionsWindow", "Force Programs", nullptr));
        QTreeWidgetItem *___qtreewidgetitem5 = treeBreakout->headerItem();
        ___qtreewidgetitem5->setText(1, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        ___qtreewidgetitem5->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        btnDelBreakout->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkShowBreakoutTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        label_69->setText(QCoreApplication::translate("OptionsWindow", "Programs entered here will be allowed to break out of this sandbox when they start. It is also possible to capture them into another sandbox, for example to have your web browser always open in a dedicated box.", nullptr));
        lblBreakOut->setText(QCoreApplication::translate("OptionsWindow", "<b><font color='red'>SECURITY ADVISORY</font>:</b> Using <a href=\"sbie://docs/breakoutfolder\">BreakoutFolder</a> and/or <a href=\"sbie://docs/breakoutprocess\">BreakoutProcess</a> in combination with Open[File/Pipe]Path directives can compromise security, as can the use of <a href=\"sbie://docs/breakoutdocument\">BreakoutDocument</a> allowing any * or insecure (*.exe;*.dll;*.ocx;*.cmd;*.bat;*.lnk;*.pif;*.url;*.ps1;etc\342\200\246) extensions. Please review the security section for each option in the documentation before use.", nullptr));
        btnBreakoutDir->setText(QCoreApplication::translate("OptionsWindow", "Breakout Folder", nullptr));
        btnBreakoutProg->setText(QCoreApplication::translate("OptionsWindow", "Breakout Program", nullptr));
        tabsForce->setTabText(tabsForce->indexOf(tabBreakout), QCoreApplication::translate("OptionsWindow", "Breakout Programs", nullptr));
        tabs->setTabText(tabs->indexOf(tabForce), QCoreApplication::translate("OptionsWindow", "Program Control", nullptr));
        btnDelStopProg->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkShowStopTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnAddLingering->setText(QCoreApplication::translate("OptionsWindow", "Add Program", nullptr));
        QTreeWidgetItem *___qtreewidgetitem6 = treeStop->headerItem();
        ___qtreewidgetitem6->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        label_2->setText(QCoreApplication::translate("OptionsWindow", "Lingering programs will be automatically terminated if they are still running after all other processes have been terminated.", nullptr));
        tabsStop->setTabText(tabsStop->indexOf(tabLingerer), QCoreApplication::translate("OptionsWindow", "Lingering Programs", nullptr));
        QTreeWidgetItem *___qtreewidgetitem7 = treeLeader->headerItem();
        ___qtreewidgetitem7->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        btnAddLeader->setText(QCoreApplication::translate("OptionsWindow", "Add Program", nullptr));
        btnDelLeader->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkShowLeaderTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        label_71->setText(QCoreApplication::translate("OptionsWindow", "If leader processes are defined, all others are treated as lingering processes.", nullptr));
        tabsStop->setTabText(tabsStop->indexOf(tabLeader), QCoreApplication::translate("OptionsWindow", "Leader Programs", nullptr));
        tabs->setTabText(tabs->indexOf(tabStop), QCoreApplication::translate("OptionsWindow", "Stop Behaviour", nullptr));
        chkStartBlockMsg->setText(QCoreApplication::translate("OptionsWindow", "Issue message 1308 when a program fails to start", nullptr));
        QTreeWidgetItem *___qtreewidgetitem8 = treeStart->headerItem();
        ___qtreewidgetitem8->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        btnAddStartProg->setText(QCoreApplication::translate("OptionsWindow", "Add Program", nullptr));
        btnDelStartProg->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        radStartSelected->setText(QCoreApplication::translate("OptionsWindow", "Allow only selected programs to start in this sandbox. *", nullptr));
        radStartExcept->setText(QCoreApplication::translate("OptionsWindow", "Prevent selected programs from starting in this sandbox.", nullptr));
        radStartAll->setText(QCoreApplication::translate("OptionsWindow", "Allow all programs to start in this sandbox.", nullptr));
        label_4->setText(QCoreApplication::translate("OptionsWindow", "* Note: Programs installed to this sandbox won't be able to start at all.", nullptr));
        chkShowStartTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        tabs->setTabText(tabs->indexOf(tabStart), QCoreApplication::translate("OptionsWindow", "Start Restrictions", nullptr));
        chkShowFilesTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnDelFile->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        QTreeWidgetItem *___qtreewidgetitem9 = treeFiles->headerItem();
        ___qtreewidgetitem9->setText(3, QCoreApplication::translate("OptionsWindow", "Path", nullptr));
        ___qtreewidgetitem9->setText(2, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem9->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem9->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        btnAddFile->setText(QCoreApplication::translate("OptionsWindow", "Add File/Folder", nullptr));
        label_7->setText(QCoreApplication::translate("OptionsWindow", "Configure which processes can access Files, Folders and Pipes. \n"
"'Open' access only applies to program binaries located outside the sandbox, you can use 'Open for All' instead to make it apply to all programs, or change this behavior in the Policies tab.", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabFiles), QCoreApplication::translate("OptionsWindow", "Files", nullptr));
        chkShowKeysTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        QTreeWidgetItem *___qtreewidgetitem10 = treeKeys->headerItem();
        ___qtreewidgetitem10->setText(3, QCoreApplication::translate("OptionsWindow", "Path", nullptr));
        ___qtreewidgetitem10->setText(2, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem10->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem10->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        btnDelKey->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        btnAddKey->setText(QCoreApplication::translate("OptionsWindow", "Add Reg Key", nullptr));
        label_10->setText(QCoreApplication::translate("OptionsWindow", "Configure which processes can access the Registry. \n"
"'Open' access only applies to program binaries located outside the sandbox, you can use 'Open for All' instead to make it apply to all programs, or change this behavior in the Policies tab.", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabKeys), QCoreApplication::translate("OptionsWindow", "Registry", nullptr));
        btnAddIPC->setText(QCoreApplication::translate("OptionsWindow", "Add IPC Path", nullptr));
        chkShowIPCTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnDelIPC->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        QTreeWidgetItem *___qtreewidgetitem11 = treeIPC->headerItem();
        ___qtreewidgetitem11->setText(3, QCoreApplication::translate("OptionsWindow", "Path", nullptr));
        ___qtreewidgetitem11->setText(2, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem11->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem11->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        label_22->setText(QCoreApplication::translate("OptionsWindow", "Configure which processes can access NT IPC objects like ALPC ports and other processes memory and context.\n"
"To specify a process use '$:program.exe' as path.", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabIPC), QCoreApplication::translate("OptionsWindow", "IPC", nullptr));
        btnDelWnd->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        chkShowWndTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnAddWnd->setText(QCoreApplication::translate("OptionsWindow", "Add Wnd Class", nullptr));
        QTreeWidgetItem *___qtreewidgetitem12 = treeWnd->headerItem();
        ___qtreewidgetitem12->setText(3, QCoreApplication::translate("OptionsWindow", "Wnd Class", nullptr));
        ___qtreewidgetitem12->setText(2, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem12->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem12->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        chkNoWindowRename->setText(QCoreApplication::translate("OptionsWindow", "Don't alter window class names created by sandboxed programs", nullptr));
        label_27->setText(QCoreApplication::translate("OptionsWindow", "Configure which processes can access Desktop objects like Windows and alike.", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabWnd), QCoreApplication::translate("OptionsWindow", "Wnd", nullptr));
        btnDelCOM->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        QTreeWidgetItem *___qtreewidgetitem13 = treeCOM->headerItem();
        ___qtreewidgetitem13->setText(3, QCoreApplication::translate("OptionsWindow", "Class Id", nullptr));
        ___qtreewidgetitem13->setText(2, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem13->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem13->setText(0, QCoreApplication::translate("OptionsWindow", "Type", nullptr));
        chkShowCOMTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnAddCOM->setText(QCoreApplication::translate("OptionsWindow", "Add COM Object", nullptr));
        label_29->setText(QCoreApplication::translate("OptionsWindow", "Configure which processes can access COM objects.", nullptr));
        chkOpenCOM->setText(QCoreApplication::translate("OptionsWindow", "Don't use virtualized COM, Open access to hosts COM infrastructure (not recommended)", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabCOM), QCoreApplication::translate("OptionsWindow", "COM", nullptr));
        lblPolicy->setText(QCoreApplication::translate("OptionsWindow", "Rule Policies", nullptr));
        chkCloseForBox->setText(QCoreApplication::translate("OptionsWindow", "Apply Close...=!<program>,... rules also to all binaries located in the sandbox.", nullptr));
        chkUseSpecificity->setText(QCoreApplication::translate("OptionsWindow", "Prioritize rules based on their Specificity and Process Match Level", nullptr));
        chkNoOpenForBox->setText(QCoreApplication::translate("OptionsWindow", "Apply File and Key Open directives only to binaries located outside the sandbox.", nullptr));
        lblMode->setText(QCoreApplication::translate("OptionsWindow", "Access Mode", nullptr));
        label_54->setText(QCoreApplication::translate("OptionsWindow", "The rule specificity is a measure to how well a given rule matches a particular path, simply put the specificity is the length of characters from the begin of the path up to and including the last matching non-wildcard substring. A rule which matches only file types like \"*.tmp\" would have the highest specificity as it would always match the entire file path.\n"
"The process match level has a higher priority than the specificity and describes how a rule applies to a given process. Rules applying by process name or group have the strongest match level, followed by the match by negation (i.e. rules applying to all processes but the given one), while the lowest match levels have global matches, i.e. rules that apply to any process.", nullptr));
        chkPrivacy->setText(QCoreApplication::translate("OptionsWindow", "Privacy Mode, block file and registry access to all locations except the generic system ones", nullptr));
        label_55->setText(QCoreApplication::translate("OptionsWindow", "When the Privacy Mode is enabled, sandboxed processes will be only able to read C:\\Windows\\*, C:\\Program Files\\*, and parts of the HKLM registry, all other locations will need explicit access to be readable and/or writable. In this mode, Rule Specificity is always enabled.", nullptr));
        tabsAccess->setTabText(tabsAccess->indexOf(tabPolicy), QCoreApplication::translate("OptionsWindow", "Access Policies", nullptr));
        tabs->setTabText(tabs->indexOf(tabAccess), QCoreApplication::translate("OptionsWindow", "Resource Access", nullptr));
        chkINetBlockMsg->setText(QCoreApplication::translate("OptionsWindow", "Issue message 1307 when a program is denied internet access", nullptr));
        btnAddINetProg->setText(QCoreApplication::translate("OptionsWindow", "Add Program", nullptr));
        chkINetBlockPrompt->setText(QCoreApplication::translate("OptionsWindow", "Prompt user whether to allow an exemption from the blockade.", nullptr));
        btnDelINetProg->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        label_11->setText(QCoreApplication::translate("OptionsWindow", "Note: Programs installed to this sandbox won't be able to access the internet at all.", nullptr));
        QTreeWidgetItem *___qtreewidgetitem14 = treeINet->headerItem();
        ___qtreewidgetitem14->setText(1, QCoreApplication::translate("OptionsWindow", "Access", nullptr));
        ___qtreewidgetitem14->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        label_12->setText(QCoreApplication::translate("OptionsWindow", "Set network/internet access for unlisted processes:", nullptr));
        tabsInternet->setTabText(tabsInternet->indexOf(tabINet), QCoreApplication::translate("OptionsWindow", "Process Restrictions", nullptr));
        label_48->setText(QCoreApplication::translate("OptionsWindow", "Test Rules, Program:", nullptr));
        label_49->setText(QCoreApplication::translate("OptionsWindow", "Port:", nullptr));
        label_50->setText(QCoreApplication::translate("OptionsWindow", "IP:", nullptr));
        label_51->setText(QCoreApplication::translate("OptionsWindow", "Protocol:", nullptr));
        btnClearFwTest->setText(QCoreApplication::translate("OptionsWindow", "X", nullptr));
        btnDelFwRule->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        btnAddFwRule->setText(QCoreApplication::translate("OptionsWindow", "Add Rule", nullptr));
        QTreeWidgetItem *___qtreewidgetitem15 = treeNetFw->headerItem();
        ___qtreewidgetitem15->setText(4, QCoreApplication::translate("OptionsWindow", "Protocol", nullptr));
        ___qtreewidgetitem15->setText(3, QCoreApplication::translate("OptionsWindow", "IP", nullptr));
        ___qtreewidgetitem15->setText(2, QCoreApplication::translate("OptionsWindow", "Port", nullptr));
        ___qtreewidgetitem15->setText(1, QCoreApplication::translate("OptionsWindow", "Action", nullptr));
        ___qtreewidgetitem15->setText(0, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        chkShowNetFwTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        lblNoWfp->setText(QCoreApplication::translate("OptionsWindow", "CAUTION: Windows Filtering Platform is not enabled with the driver, therefore these rules will be applied only in user mode and can not be enforced!!! This means that malicious applications may bypass them.", nullptr));
        tabsInternet->setTabText(tabsInternet->indexOf(tabNetFw), QCoreApplication::translate("OptionsWindow", "Network Firewall", nullptr));
        tabs->setTabText(tabs->indexOf(tabInternet), QCoreApplication::translate("OptionsWindow", "Network Options", nullptr));
        QTreeWidgetItem *___qtreewidgetitem16 = treeRecovery->headerItem();
        ___qtreewidgetitem16->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        label_16->setText(QCoreApplication::translate("OptionsWindow", "When the Quick Recovery function is invoked, the following folders will be checked for sandboxed content. ", nullptr));
        chkShowRecoveryTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnDelRecovery->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        btnAddRecovery->setText(QCoreApplication::translate("OptionsWindow", "Add Folder", nullptr));
        tabsRecovery->setTabText(tabsRecovery->indexOf(tabQuickRecovery), QCoreApplication::translate("OptionsWindow", "Quick Recovery", nullptr));
        label_9->setText(QCoreApplication::translate("OptionsWindow", "You can exclude folders and file types (or file extensions) from Immediate Recovery.", nullptr));
        btnAddRecIgnoreExt->setText(QCoreApplication::translate("OptionsWindow", "Ignore Extension", nullptr));
        chkAutoRecovery->setText(QCoreApplication::translate("OptionsWindow", "Enable Immediate Recovery prompt to be able to recover files as soon as they are created.", nullptr));
        QTreeWidgetItem *___qtreewidgetitem17 = treeRecIgnore->headerItem();
        ___qtreewidgetitem17->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        btnDelRecIgnore->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        btnAddRecIgnore->setText(QCoreApplication::translate("OptionsWindow", "Ignore Folder", nullptr));
        chkShowRecIgnoreTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        tabsRecovery->setTabText(tabsRecovery->indexOf(tabImmediateRecovery), QCoreApplication::translate("OptionsWindow", "Immediate Recovery", nullptr));
        tabs->setTabText(tabs->indexOf(tabRecovery), QCoreApplication::translate("OptionsWindow", "File Recovery", nullptr));
        chkElevateCreateProcessFix->setText(QCoreApplication::translate("OptionsWindow", "Apply ElevateCreateProcess Workaround (legacy behaviour)", nullptr));
        chkUseSbieWndStation->setText(QCoreApplication::translate("OptionsWindow", "Emulate sandboxed window station for all processes", nullptr));
        lblCompatibility->setText(QCoreApplication::translate("OptionsWindow", "Compatibility", nullptr));
        chkComTimeout->setText(QCoreApplication::translate("OptionsWindow", "Disable the use of RpcMgmtSetComTimeout by default (this may resolve compatibility issues)", nullptr));
        chkPreferExternalManifest->setText(QCoreApplication::translate("OptionsWindow", "Force usage of custom dummy Manifest files (legacy behaviour)", nullptr));
        chkUseSbieDeskHack->setText(QCoreApplication::translate("OptionsWindow", "Use desktop object workaround for all processes", nullptr));
        chkNestedJobs->setText(QCoreApplication::translate("OptionsWindow", "Allow use of nested job objects (works on Windows 8 and later)", nullptr));
#if QT_CONFIG(tooltip)
        chkNoPanic->setToolTip(QCoreApplication::translate("OptionsWindow", "When the global hotkey is pressed 3 times in short succession this exception will be ignored.", nullptr));
#endif // QT_CONFIG(tooltip)
        chkNoPanic->setText(QCoreApplication::translate("OptionsWindow", "Exclude this sandbox from being terminated when \"Terminate All Processes\" is invoked.", nullptr));
        tabsOther->setTabText(tabsOther->indexOf(tabCompat), QCoreApplication::translate("OptionsWindow", "Compatibility", nullptr));
#if QT_CONFIG(tooltip)
        lblProtection->setToolTip(QCoreApplication::translate("OptionsWindow", "Protect the sandbox integrity itself", nullptr));
#endif // QT_CONFIG(tooltip)
        lblProtection->setText(QCoreApplication::translate("OptionsWindow", "Image Protection", nullptr));
        QTreeWidgetItem *___qtreewidgetitem18 = treeInjectDll->headerItem();
        ___qtreewidgetitem18->setText(1, QCoreApplication::translate("OptionsWindow", "Description", nullptr));
        ___qtreewidgetitem18->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
#if QT_CONFIG(tooltip)
        chkHostProtect->setToolTip(QCoreApplication::translate("OptionsWindow", "Sandboxie\342\200\231s resource access rules often discriminate against program binaries located inside the sandbox. OpenFilePath and OpenKeyPath work only for application binaries located on the host natively. In order to define a rule without this restriction, OpenPipePath or OpenConfPath must be used. Likewise, all Closed(File|Key|Ipc)Path directives which are defined by negation e.g. \342\200\230ClosedFilePath=! iexplore.exe,C:Users*\342\200\231 will be always closed for binaries located inside a sandbox. Both restriction policies can be disabled on the \342\200\234Access policies\342\200\235 page.\n"
"This is done to prevent rogue processes inside the sandbox from creating a renamed copy of themselves and accessing protected resources. Another exploit vector is the injection of a library into an authorized process to get access to everything it is allowed to access. Using Host Image Protection, this can be prevented by blocking applications (installed on the host) running inside a sandbox from loading lib"
                        "raries from the sandbox itself.", nullptr));
#endif // QT_CONFIG(tooltip)
        chkHostProtect->setText(QCoreApplication::translate("OptionsWindow", "Prevent sandboxes programs installed on host from loading dll's from the sandbox", nullptr));
        chkHostProtectMsg->setText(QCoreApplication::translate("OptionsWindow", "Issue message 1305 when a program tries to load a sandboxed dll", nullptr));
        label_36->setText(QCoreApplication::translate("OptionsWindow", "Sandboxie's functionality can be enhanced by using optional DLLs which can be loaded into each sandboxed process on start by the SbieDll.dll file, the add-on manager in the global settings offers a couple of useful extensions, once installed they can be enabled here for the current box.", nullptr));
        tabsOther->setTabText(tabsOther->indexOf(tabDlls), QCoreApplication::translate("OptionsWindow", "Dlls && Extensions", nullptr));
        tabs->setTabText(tabs->indexOf(tabOther), QCoreApplication::translate("OptionsWindow", "Various Options", nullptr));
        btnAddOption->setText(QCoreApplication::translate("OptionsWindow", "Add Option", nullptr));
        btnDelOption->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        lblAdvanced->setText(QCoreApplication::translate("OptionsWindow", "Here you can configure advanced per process options to improve compatibility and/or customize sandboxing behavior.", nullptr));
        QTreeWidgetItem *___qtreewidgetitem19 = treeOptions->headerItem();
        ___qtreewidgetitem19->setText(2, QCoreApplication::translate("OptionsWindow", "Value", nullptr));
        ___qtreewidgetitem19->setText(1, QCoreApplication::translate("OptionsWindow", "Program", nullptr));
        ___qtreewidgetitem19->setText(0, QCoreApplication::translate("OptionsWindow", "Option", nullptr));
        chkShowOptionsTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabMisc), QCoreApplication::translate("OptionsWindow", "Miscellaneous", nullptr));
        btnDelAuto->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
#if QT_CONFIG(tooltip)
        label_68->setToolTip(QCoreApplication::translate("OptionsWindow", "These commands are run UNBOXED just before the box content is deleted", nullptr));
#endif // QT_CONFIG(tooltip)
        label_68->setText(QCoreApplication::translate("OptionsWindow", "On File Recovery", nullptr));
        btnAddAutoRun->setText(QCoreApplication::translate("OptionsWindow", "Run Command", nullptr));
#if QT_CONFIG(tooltip)
        label_66->setToolTip(QCoreApplication::translate("OptionsWindow", "These commands are executed only when a box is initialized. To make them run again, the box content must be deleted.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_66->setText(QCoreApplication::translate("OptionsWindow", "On Box Init", nullptr));
#if QT_CONFIG(tooltip)
        label_67->setToolTip(QCoreApplication::translate("OptionsWindow", "These commands are run UNBOXED just before the box content is deleted", nullptr));
#endif // QT_CONFIG(tooltip)
        label_67->setText(QCoreApplication::translate("OptionsWindow", "On Delete Content", nullptr));
        QTreeWidgetItem *___qtreewidgetitem20 = treeTriggers->headerItem();
        ___qtreewidgetitem20->setText(1, QCoreApplication::translate("OptionsWindow", "Action", nullptr));
        ___qtreewidgetitem20->setText(0, QCoreApplication::translate("OptionsWindow", "Event", nullptr));
#if QT_CONFIG(tooltip)
        label_18->setToolTip(QCoreApplication::translate("OptionsWindow", "This command runs after all processes in the sandbox have finished.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_18->setText(QCoreApplication::translate("OptionsWindow", "On Box Terminate", nullptr));
        label_32->setText(QCoreApplication::translate("OptionsWindow", "Here you can specify actions to be executed automatically on various box events.", nullptr));
#if QT_CONFIG(tooltip)
        btnAddRecoveryCmd->setToolTip(QCoreApplication::translate("OptionsWindow", "This command will be run before a file is being recovered and the file path will be passed as the first argument. If this command returns anything other than 0, the recovery will be blocked", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAddRecoveryCmd->setText(QCoreApplication::translate("OptionsWindow", "Run File Checker", nullptr));
#if QT_CONFIG(tooltip)
        label_33->setToolTip(QCoreApplication::translate("OptionsWindow", "These events are executed each time a box is started", nullptr));
#endif // QT_CONFIG(tooltip)
        label_33->setText(QCoreApplication::translate("OptionsWindow", "On Box Start", nullptr));
#if QT_CONFIG(tooltip)
        btnAddDeleteCmd->setToolTip(QCoreApplication::translate("OptionsWindow", "This command will be run before the box content will be deleted", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAddDeleteCmd->setText(QCoreApplication::translate("OptionsWindow", "Run Command", nullptr));
        chkShowTriggersTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        btnAddAutoSvc->setText(QCoreApplication::translate("OptionsWindow", "Start Service", nullptr));
        btnAddAutoExec->setText(QCoreApplication::translate("OptionsWindow", "Run Command", nullptr));
        btnAddTerminateCmd->setText(QCoreApplication::translate("OptionsWindow", "Run Command", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabTriggers), QCoreApplication::translate("OptionsWindow", "Triggers", nullptr));
        btnAddProcess->setText(QCoreApplication::translate("OptionsWindow", "Add Process", nullptr));
        chkHideOtherBoxes->setText(QCoreApplication::translate("OptionsWindow", "Don't allow sandboxed processes to see processes running in other boxes", nullptr));
        label_24->setText(QCoreApplication::translate("OptionsWindow", "Hide host processes from processes running in the sandbox.", nullptr));
        chkShowHiddenProcTmpl->setText(QCoreApplication::translate("OptionsWindow", "Show Templates", nullptr));
        QTreeWidgetItem *___qtreewidgetitem21 = treeHideProc->headerItem();
        ___qtreewidgetitem21->setText(0, QCoreApplication::translate("OptionsWindow", "Process", nullptr));
        btnDelProcess->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabHideProc), QCoreApplication::translate("OptionsWindow", "Hide Processes", nullptr));
        chkMonitorAdminOnly->setText(QCoreApplication::translate("OptionsWindow", "Restrict Resource Access monitor to administrators only", nullptr));
        btnAddUser->setText(QCoreApplication::translate("OptionsWindow", "Add User", nullptr));
        btnDelUser->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        label_23->setText(QCoreApplication::translate("OptionsWindow", "Add user accounts and user groups to the list below to limit use of the sandbox to only those accounts.  If the list is empty, the sandbox can be used by all user accounts.\n"
"\n"
"Note:  Forced Programs and Force Folders settings for a sandbox do not apply to user accounts which cannot use the sandbox.", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabUsers), QCoreApplication::translate("OptionsWindow", "Users", nullptr));
        chkPipeTrace->setText(QCoreApplication::translate("OptionsWindow", "Pipe Trace", nullptr));
        chkHookTrace->setText(QCoreApplication::translate("OptionsWindow", "API call Trace (traces all SBIE hooks)", nullptr));
        chkErrTrace->setText(QCoreApplication::translate("OptionsWindow", "Log all SetError's to Trace log (creates a lot of output)", nullptr));
        lblMonitor->setText(QCoreApplication::translate("OptionsWindow", "Resource Access Monitor", nullptr));
        label_38->setText(QString());
        chkKeyTrace->setText(QCoreApplication::translate("OptionsWindow", "Key Trace", nullptr));
        chkFileTrace->setText(QCoreApplication::translate("OptionsWindow", "File Trace", nullptr));
        chkIpcTrace->setText(QCoreApplication::translate("OptionsWindow", "IPC Trace", nullptr));
        chkDbgTrace->setText(QCoreApplication::translate("OptionsWindow", "Log Debug Output to the Trace Log", nullptr));
        chkNetFwTrace->setText(QCoreApplication::translate("OptionsWindow", "Network Firewall", nullptr));
        chkGuiTrace->setText(QCoreApplication::translate("OptionsWindow", "GUI Trace", nullptr));
        label_26->setText(QCoreApplication::translate("OptionsWindow", "Log all access events as seen by the driver to the resource access log.\n"
"\n"
"This options set the event mask to \"*\" - All access events\n"
"You can customize the logging using the ini by specifying\n"
"\"A\" - Allowed accesses\n"
"\"D\" - Denied accesses\n"
"\"I\" - Ignore access requests\n"
"instead of \"*\".", nullptr));
        chkDisableMonitor->setText(QCoreApplication::translate("OptionsWindow", "Disable Resource Access Monitor", nullptr));
        lblTracing->setText(QCoreApplication::translate("OptionsWindow", "Access Tracing", nullptr));
        chkComTrace->setText(QCoreApplication::translate("OptionsWindow", "COM Class Trace", nullptr));
        chkCallTrace->setText(QCoreApplication::translate("OptionsWindow", "Syscall Trace (creates a lot of output)", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabTracing), QCoreApplication::translate("OptionsWindow", "Tracing", nullptr));
        label_25->setText(QCoreApplication::translate("OptionsWindow", "WARNING, these options can disable core security guarantees and break sandbox security!!!", nullptr));
        label_17->setText(QCoreApplication::translate("OptionsWindow", "These options are intended for debugging compatibility issues, please do not use them in production use. ", nullptr));
        tabsAdvanced->setTabText(tabsAdvanced->indexOf(tabDebug), QCoreApplication::translate("OptionsWindow", "Debug", nullptr));
        tabs->setTabText(tabs->indexOf(tabAdvanced), QCoreApplication::translate("OptionsWindow", "Advanced Options", nullptr));
        label_13->setText(QCoreApplication::translate("OptionsWindow", "This list contains a large amount of sandbox compatibility enhancing templates", nullptr));
        label_15->setText(QCoreApplication::translate("OptionsWindow", "Text Filter", nullptr));
        btnAddTemplate->setText(QCoreApplication::translate("OptionsWindow", "Add Template", nullptr));
        label_6->setText(QCoreApplication::translate("OptionsWindow", "Filter Categories", nullptr));
        QTreeWidgetItem *___qtreewidgetitem22 = treeTemplates->headerItem();
        ___qtreewidgetitem22->setText(1, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        ___qtreewidgetitem22->setText(0, QCoreApplication::translate("OptionsWindow", "Category", nullptr));
        btnDelTemplate->setText(QCoreApplication::translate("OptionsWindow", "Remove", nullptr));
        tabsTemplates->setTabText(tabsTemplates->indexOf(tab_11), QCoreApplication::translate("OptionsWindow", "Templates", nullptr));
        label_46->setText(QCoreApplication::translate("OptionsWindow", "Configure the folder locations used by your other applications.\n"
"\n"
"Please note that this values are currently user specific and saved globally for all boxes.", nullptr));
        QTreeWidgetItem *___qtreewidgetitem23 = treeFolders->headerItem();
        ___qtreewidgetitem23->setText(1, QCoreApplication::translate("OptionsWindow", "Value", nullptr));
        ___qtreewidgetitem23->setText(0, QCoreApplication::translate("OptionsWindow", "Name", nullptr));
        tabsTemplates->setTabText(tabsTemplates->indexOf(tab_12), QCoreApplication::translate("OptionsWindow", "Template Folders", nullptr));
        chkScreenReaders->setText(QCoreApplication::translate("OptionsWindow", "Screen Readers: JAWS, NVDA, Window-Eyes, System Access", nullptr));
        label_45->setText(QString());
        label_43->setText(QCoreApplication::translate("OptionsWindow", "The following settings enable the use of Sandboxie in combination with accessibility software.  Please note that some measure of Sandboxie protection is necessarily lost when these settings are in effect.", nullptr));
        label_44->setText(QCoreApplication::translate("OptionsWindow", "To compensate for the lost protection, please consult the Drop Rights settings page in the Restrictions settings group.", nullptr));
        tabsTemplates->setTabText(tabsTemplates->indexOf(tab_13), QCoreApplication::translate("OptionsWindow", "Accessibility", nullptr));
        tabs->setTabText(tabs->indexOf(tabTemplates), QCoreApplication::translate("OptionsWindow", "App Templates", nullptr));
        btnEditIni->setText(QCoreApplication::translate("OptionsWindow", "Edit ini", nullptr));
        btnCancelEdit->setText(QCoreApplication::translate("OptionsWindow", "Cancel", nullptr));
        btnSaveIni->setText(QCoreApplication::translate("OptionsWindow", "Save", nullptr));
        tabs->setTabText(tabs->indexOf(tabEdit), QCoreApplication::translate("OptionsWindow", "Edit ini Section", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OptionsWindow: public Ui_OptionsWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSWINDOW_H
