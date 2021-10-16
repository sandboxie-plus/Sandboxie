#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OptionsWindow.h"
#include "SbiePlusAPI.h"

class COptionsWindow : public QDialog
{
	Q_OBJECT

public:
	COptionsWindow(const QSharedPointer<CSbieIni>& pBox, const QString& Name, QWidget *parent = Q_NULLPTR);
	~COptionsWindow();

	virtual void accept() {}
	virtual void reject();

signals:
	void OptionsChanged();
	void Closed();

public slots:
	void ok();
	void apply();

private slots:

	//void OnWithTemplates();

	void OnPickColor();

	void OnBrowsePath();
	void OnAddCommand();
	void OnDelCommand();

	void OnAddAutoCmd();
	void OnAddAutoExe();
	void OnDelAutoSvc();
	void OnDelAuto();

	void OnAddGroup();
	void OnAddProg();
	void OnDelProg();
	void OnShowGroupTmpl()			{ LoadGroupsTmpl(true); }

	void OnForceProg();
	void OnForceDir();
	void OnDelForce();
	void OnShowForceTmpl()			{ LoadForcedTmpl(true); }

	void OnAddLingering();
	void OnAddLeader();
	void OnDelStopProg();
	void OnShowStopTmpl()			{ LoadStopTmpl(true); }

	void OnRestrictStart();
	void OnAddStartProg();
	void OnDelStartProg();

	// net
	void OnINetItemDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnINetSelectionChanged()	{ CloseINetEdit(); }
	void OnBlockINet();
	void OnAddINetProg();
	void OnDelINetProg();

	void OnNetFwItemDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnNetFwSelectionChanged()	{ CloseNetFwEdit(); }
	void OnAddNetFwRule();
	void OnDelNetFwRule();

	void OnShowNetFwTmpl()			{ LoadNetFwRulesTmpl(true); }

	void OnTestNetFwRule();
	void OnClearNetFwTest();
	//net
	// 
	// access
	//void OnAccessItemClicked(QTreeWidgetItem* pItem, int Column);
	void OnAccessItemDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnAccessSelectionChanged() { CloseAccessEdit(); }

	void OnAddFile()				{ AddAccessEntry(eFile, eDirect, "", ""); m_AccessChanged = true; }
	void OnBrowseFile();
	void OnBrowseFolder();
	void OnAddKey()					{ AddAccessEntry(eKey, eDirect, "", ""); m_AccessChanged = true; }
	void OnAddIPC()					{ AddAccessEntry(eIPC, eDirect, "", ""); m_AccessChanged = true; }
	void OnAddWnd()					{ AddAccessEntry(eWnd, eDirect, "", ""); m_AccessChanged = true; }
	void OnAddCOM()					{ AddAccessEntry(eCOM, eDirect, "", ""); m_AccessChanged = true; }
	void OnDelAccess();
	void OnShowAccessTmpl()			{ LoadAccessListTmpl(true); }
	//

	void OnAddRecFolder();
	void OnAddRecIgnore();
	void OnAddRecIgnoreExt();
	void OnDelRecEntry();
	void OnShowRecoveryTmpl()		{ LoadRecoveryListTmpl(true); }

	void OnAddAutoExec();
	void OnDelAutoExec();

	void OnAddProcess();
	void OnDelProcess();

	void OnNoWindowRename();

	void OnAddUser();
	void OnDelUser();

	void OnFilterTemplates()		{ ShowTemplates(); }
	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnAddTemplates();
	void OnDelTemplates();
	void OnFolderChanged();
	void OnScreenReaders();

	void OnTab();

	void OnGeneralChanged();
	void OnStartChanged()			{ m_StartChanged = true; }
	//void OnRestrictionChanged()		{ m_RestrictionChanged = true; }
	void OnINetBlockChanged()		{ m_INetBlockChanged = true; }
	void OnRecoveryChanged()		{ m_RecoveryChanged = true; }
	void OnAccessChanged()			{ m_AccessChanged = true; }
	void OnAdvancedChanged();
	void OnDebugChanged();

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnCancelEdit();

protected:
	friend struct SFirewallRule;

	void closeEvent(QCloseEvent *e);

	bool eventFilter(QObject *watched, QEvent *e);

	enum ENetWfAction
	{
		eAllow,
		eBlock
	};

	enum ENetWfProt
	{
		eAny,
		eTcp,
		eUdp,
		eIcmp,
	};

	enum EAccessEntry
	{
		eOpenFilePath,
		eOpenPipePath,
		eClosedFilePath,
		eReadFilePath,
		eWriteFilePath,

		eOpenKeyPath,
		eClosedKeyPath,
		eReadKeyPath,
		eWriteKeyPath,

		eOpenIpcPath,
		eClosedIpcPath,

		eOpenWinClass,

		eOpenCOM,
		eClosedCOM,
		eClosedCOM_RT,

		eMaxAccessType
	};

	enum EAccessType
	{
		eFile,
		eKey,
		eIPC,
		eWnd,
		eCOM
	};

	enum EAccessMode
	{
		eDirect,
		eDirectAll,
		eClosed,
		eClosedRT,
		eReadOnly,
		eWriteOnly
	};

	void SetProgramItem(QString Program, QTreeWidgetItem* pItem, int Column);

	QString SelectProgram(bool bOrGroup = true);
	void AddProgramToGroup(const QString& Program, const QString& Group);
	void DelProgramFromGroup(const QString& Program, const QString& Group);
	QTreeWidgetItem* FindGroupByName(const QString& Group, bool bAdd = false);

	void CopyGroupToList(const QString& Groupe, QTreeWidget* pTree);
	QTreeWidgetItem* GetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void SetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void DelAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void AddProgToGroup(QTreeWidget* pTree, const QString& Groupe);
	void AddProgToGroup(const QString& Value, const QString& Groupe);
	void DelProgFromGroup(QTreeWidget* pTree, const QString& Groupe);

	void LoadConfig();
	void SaveConfig();
	void UpdateCurrentTab();

	void AddAutoRunItem(const QString& Value, int Type);

	void AddRunItem(const QString& Name, const QString& Command);

	void CreateGeneral();
	void LoadGeneral();
	void SaveGeneral();

	void LoadGroups();
	void LoadGroupsTmpl(bool bUpdate = false);
	void SaveGroups();

	void LoadForced();
	void LoadForcedTmpl(bool bUpdate = false);
	void AddForcedEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveForced();

	void LoadStop();
	void LoadStopTmpl(bool bUpdate = false);
	void AddStopEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveStop();

	void LoadStart();
	void SaveStart();

	// Network
	void CreateNetwork();

	int GroupToINetMode(const QString& Mode);
	QString INetModeToGroup(int Mode);
	void LoadBlockINet();
	QString	GetINetModeStr(int Mode);
	void CloseINetEdit(bool bSave = true);
	void CloseINetEdit(QTreeWidgetItem* pItem, bool bSave = true);
	void CheckINetBlock();
	bool FindEntryInSettingList(const QString& Name, const QString& Value);
	void LoadINetAccess();
	void SaveINetAccess();

	void ParseAndAddFwRule(const QString& Value, const QString& Template = QString());
	void CloseNetFwEdit(bool bSave = true);
	void CloseNetFwEdit(QTreeWidgetItem* pItem, bool bSave = true);
	ENetWfProt GetFwRuleProt(const QString& Value);
	ENetWfAction GetFwRuleAction(const QString& Value);
	QString GetFwRuleActionStr(ENetWfAction Action);
	void LoadNetFwRules();
	void SaveNetFwRules();
	void LoadNetFwRulesTmpl(bool bUpdate = false);
	//
	
	// access
	void CreateAccess();

	QString	AccessTypeToName(EAccessEntry Type);
	void LoadAccessList();
	void LoadAccessListTmpl(bool bUpdate = false);
	QString	GetAccessTypeStr(EAccessType Type);
	QString	GetAccessModeStr(EAccessMode Mode);
	void ParseAndAddAccessEntry(EAccessEntry EntryType, const QString& Value, const QString& Template = QString());
	void AddAccessEntry(EAccessType	Type, EAccessMode Mode, QString Program, const QString& Path, const QString& Template = QString());
	QString MakeAccessStr(EAccessType Type, EAccessMode Mode);
	void SaveAccessList();
	QList<EAccessMode> GetAccessModes(EAccessType Type);
	void DeleteAccessEntry(QTreeWidgetItem* pItem);

	void CloseAccessEdit(bool bSave = true);
	void CloseAccessEdit(QTreeWidgetItem* pItem, bool bSave = true);
	//

	void LoadRecoveryList();
	void LoadRecoveryListTmpl(bool bUpdate = false);
	void AddRecoveryEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveRecoveryList();

	void CreateAdvanced();
	void LoadAdvanced();
	void SaveAdvanced();

	void CreateDebug();
	void LoadDebug();
	void SaveDebug();

	void LoadTemplates();
	void ShowTemplates();
	void SaveTemplates();

	void LoadFolders();
	void ShowFolders();
	void SaveFolders();

	void LoadIniSection();
	void SaveIniSection();

	QString GetCategoryName(const QString& Category);

	bool m_ConfigDirty;
	QColor m_BorderColor;

	bool m_GeneralChanged;
	bool m_GroupsChanged;
	bool m_ForcedChanged;
	bool m_StopChanged;
	bool m_StartChanged;
	//bool m_RestrictionChanged;
	bool m_INetBlockChanged;
	bool m_NetFwRulesChanged;
	bool m_AccessChanged;
	bool m_TemplatesChanged;
	bool m_FoldersChanged;
	bool m_RecoveryChanged;
	bool m_AdvancedChanged;

	bool m_IsEnabledWFP;
	bool m_WFPisBlocking;

	bool m_Template;

	QSet<QString> m_TemplateGroups;

	QMultiMap<QString, QPair<QString, QString>> m_AllTemplates;
	QStringList m_GlobalTemplates;
	QStringList m_BoxTemplates;
	QStringList m_BoxFolders;

	QList<QPair<QString, QString>> m_Settings;

	QSharedPointer<CSbieIni> m_pBox;

	QSet<QString> m_Programs;

private:
	void ReadAdvancedCheck(const QString& Name, QCheckBox* pCheck, const QString& Value = "y");
	void WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& Value = "y");
	void WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue);
	void WriteText(const QString& Name, const QString& Value);
	void WriteTextList(const QString& Setting, const QStringList& List);

	Ui::OptionsWindow ui;

	struct SDbgOpt {
		QString Name;
		QString Value;
		bool Changed;
	};
	QMap<QCheckBox*, SDbgOpt> m_DebugOptions;
};
