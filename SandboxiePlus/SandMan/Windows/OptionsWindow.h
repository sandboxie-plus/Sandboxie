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

	void OnForceProg();
	void OnForceDir();
	void OnDelForce();
	void OnShowForceTmpl()			{ LoadForced(); }

	void OnAddLingering();
	void OnAddLeader();
	void OnDelStopProg();
	void OnShowStopTmpl()			{ LoadStop(); }

	void OnRestrictStart();
	void OnAddStartProg();
	void OnDelStartProg();

	void OnBlockINet();
	void OnAddINetProg();
	void OnDelINetProg();

	void OnAccessItemClicked(QTreeWidgetItem* pItem, int Column);
	void OnAccessItemDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnAccessSelectionChanged() { CloseAccessEdit(); }

	void OnAddFile()				{ AddAccessEntry(eFile, eDirect, "", ""); }
	void OnBrowseFile();
	void OnBrowseFolder();
	void OnAddKey()					{ AddAccessEntry(eKey, eDirect, "", ""); }
	void OnAddIPC()					{ AddAccessEntry(eIPC, eDirect, "", ""); }
	void OnAddWnd()					{ AddAccessEntry(eWnd, eDirect, "", ""); }
	void OnAddCOM()					{ AddAccessEntry(eCOM, eDirect, "", ""); }
	void OnDelAccess();
	void OnShowAccessTmpl()			{ LoadAccessList(); }

	void OnAddRecFolder();
	void OnAddRecIgnore();
	void OnAddRecIgnoreExt();
	void OnDelRecEntry();
	void OnShowRecoveryTmpl()		{ LoadRecoveryList(); }

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
	void OnAdvancedChanged();
	void OnDebugChanged();

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnCancelEdit();

protected:
	void closeEvent(QCloseEvent *e);

	bool eventFilter(QObject *watched, QEvent *e);

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

	void CopyGroupToList(const QString& Groupe, QTreeWidget* pTree);
	QTreeWidgetItem* GetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void SetAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void DelAccessEntry(EAccessType Type, const QString& Program, EAccessMode Mode, const QString& Path);
	void AddProgToGroup(QTreeWidget* pTree, const QString& Groupe);
	void AddProgToGroup(const QString& Value, const QString& Groupe);
	void DelProgFromGroup(QTreeWidget* pTree, const QString& Groupe);

	void LoadConfig();
	void SaveConfig();

	void AddAutoRunItem(const QString& Value, int Type);

	void AddRunItem(const QString& Name, const QString& Command);

	void LoadGroups();
	void SaveGroups();

	void LoadForced();
	void AddForcedEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveForced();

	void LoadStop();
	void AddStopEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveStop();

	QString	AccessTypeToName(EAccessEntry Type);
	void LoadAccessList();
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

	void LoadRecoveryList();
	void AddRecoveryEntry(const QString& Name, int type, const QString& Template = QString());
	void SaveRecoveryList();

	void LoadTemplates();
	void ShowTemplates();
	void SaveTemplates();

	void LoadFolders();
	void ShowFolders();
	void SaveFolders();

	void LoadIniSection();
	void SaveIniSection();

	bool m_ConfigDirty;
	QColor m_BorderColor;

	bool m_GeneralChanged;
	bool m_GroupsChanged;
	bool m_ForcedChanged;
	bool m_StopChanged;
	bool m_StartChanged;
	//bool m_RestrictionChanged;
	bool m_INetBlockChanged;
	bool m_AccessChanged;
	bool m_TemplatesChanged;
	bool m_FoldersChanged;
	bool m_RecoveryChanged;
	bool m_AdvancedChanged;

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

	Ui::OptionsWindow ui;

	struct SDbgOpt {
		QString Name;
		QString Value;
		bool Changed;
	};
	QMap<QCheckBox*, SDbgOpt> m_DebugOptions;
};
