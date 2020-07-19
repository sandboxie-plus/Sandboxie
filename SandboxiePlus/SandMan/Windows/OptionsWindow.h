#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OptionsWindow.h"
#include "SbiePlusAPI.h"

class COptionsWindow : public QMainWindow
{
	Q_OBJECT

public:
	COptionsWindow(const QSharedPointer<CSbieIni>& pBox, const QString& Name, QWidget *parent = Q_NULLPTR);
	~COptionsWindow();

signals:
	void OptionsChanged();

public slots:
	void apply();
	void accept();
	void reject();

private slots:

	void OnPickColor();

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

	void OnAddFile()				{ AddAccessEntry(eFile, eDirect, "", ""); }
	void OnAddKey()					{ AddAccessEntry(eKey, eDirect, "", ""); }
	void OnAddIPC()					{ AddAccessEntry(eIPC, eDirect, "", ""); }
	void OnAddClsId()				{ AddAccessEntry(eWndCls, eDirect, "", ""); }
	void OnAddCOM()					{ AddAccessEntry(eClsId, eDirect, "", ""); }
	void OnDelAccess();
	void OnShowAccessTmpl()			{ LoadAccessList(); }

	void OnAddUser();
	void OnDelUser();

	void OnFilterTemplates()		{ ShowTemplates(); }
	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column);

	void OnTab();

	void OnGeneralChanged();
	void OnStartChanged()			{ m_StartChanged = true; }
	void OnRestrictionChanged()		{ m_RestrictionChanged = true; }
	void OnINetBlockChanged()		{ m_INetBlockChanged = true; }
	void OnAdvancedChanged();

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnCancelEdit();

protected:
	void closeEvent(QCloseEvent *e);

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

		eOpenClsid,

		eMaxAccessType
	};

	enum EAccessType
	{
		eFile,
		eKey,
		eIPC,
		eWndCls,
		eClsId
	};

	enum EAccessMode
	{
		eDirect,
		eFull,
		eClosed,
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

	void LoadTemplates();
	void ShowTemplates();
	void SaveTemplates();

	void LoadIniSection();
	void SaveIniSection();

	bool m_ConfigDirty;
	QColor m_BorderColor;

	bool m_GeneralChanged;
	bool m_GroupsChanged;
	bool m_ForcedChanged;
	bool m_StopChanged;
	bool m_StartChanged;
	bool m_RestrictionChanged;
	bool m_INetBlockChanged;
	bool m_AccessChanged;
	bool m_TemplatesChanged;
	bool m_AdvancedChanged;

	bool m_Template;

	QSet<QString> m_TemplateGroups;

	QMultiMap<QString, QPair<QString, QString>> m_AllTemplates;
	QStringList m_GlobalTemplates;
	QStringList m_BoxTemplates;

	QList<QPair<QString, QString>> m_Settings;

	QSharedPointer<CSbieIni> m_pBox;

private:
	Ui::OptionsWindow ui;
};
