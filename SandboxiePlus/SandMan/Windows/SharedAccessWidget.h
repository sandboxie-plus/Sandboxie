#pragma once

#include <QtWidgets/qwidget.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qset.h>
#include <QtCore/qmap.h>
#include <functional>

class QTreeWidget;
class QTreeWidgetItem;
class QCheckBox;
class QToolButton;
class QMenu;
class CSbieIni;


// modes supported inside the access lists, kept generic so that the widget
// can be reused on its own; values mirror COptionsWindow::EAccessMode so that
// the two can interoperate without conversions
namespace eAccessMode {
	enum EAccessMode {
		eNormal,
		eOpen,
		eOpen4All,
		eNoRename,
		eClosed,
		eClosedRT,
		eReadOnly,
		eBoxOnly,
		eIgnoreUIPI,
		eMaxMode
	};
};

// access type ids, mirrors COptionsWindow::EAccessType so entries stored from
// the legacy options window code can be found again
namespace eAccessType {
	enum EType {
		eFile,
		eKey,
		eIPC,
		eWnd,
		eCOM,
	};
};

// one row of GetAccessConfig:
//  Mode       -> generic mode (see eAccessMode)
//  Setting    -> ini setting that holds the respective access list (e.g. "OpenFilePath")
//  ModeName/ ModeTip -> localized display name and tooltip shown in column 2
struct SAccessEntryConfig {
	eAccessMode::EAccessMode Mode;
	QString	Setting;
	QString	ModeName;
	QString	ModeTip;
};

// NOTE: this widget keeps the same item data layout as the old Options window
// access lists, so that the existing entry-lookup helpers can operate on the
// widget tree directly:
//   column 0:  type id (or -1 for template entries)
//   column 1:  program (raw, may start with '!')
//   column 2:  access mode
//   column 3:  raw path (unexpanded)
class CSharedAccessWidget : public QWidget
{
	Q_OBJECT

public:
	CSharedAccessWidget(QWidget* parent = NULL);
	virtual ~CSharedAccessWidget();

public:
	void SetConfig(const QSharedPointer<CSbieIni>& pIni, bool bTemplate);
	void SetGroupsProvider(const std::function<QStringList()>& GetGroups) { m_GetGroups = GetGroups; }
	void SetPrograms(const QSet<QString>& Programs) { m_Programs = Programs; }

	// optionally pre-process a path before it is shown (e.g. variable/box expansion);
	// when not set the path is displayed as-is
	void SetPathExpander(const std::function<QString(const QString&)>& fn) { m_ExpandPathFn = fn; }

	void SetTemplatesEnabled(bool bEnabled);
	void SetShowTemplates(bool bShow);

	void LoadAccessList();
	void SaveAccessList();

	int GetItemCount() const { return m_pTree->topLevelItemCount(); }
	QTreeWidget* GetTree() const { return m_pTree; }
	QSet<QString> GetPrograms() const { return m_Programs; }

	// update the shown path of every item to the expanded one (column 3)
	void ReloadPathDisplay();

	// compatibility helpers, used by special box configuration code
	bool HasEntry(int Mode, const QString& Program, const QString& Path);
	void SetEntry(int Mode, const QString& Program, const QString& Path);
	void DelEntry(int Mode, const QString& Program, const QString& Path);

public slots:
	void CloseAccessEdit(bool bSave);

signals:
	void Changed();

protected:
	virtual QList<SAccessEntryConfig> GetAccessConfig() = 0;	// defines the settings/mode strings for the derived type
	virtual QString GetAccessTypeStr() const = 0;
	virtual int GetAccessTypeId() const = 0;
	virtual eAccessMode::EAccessMode GetDefaultMode() const { return eAccessMode::eOpen; }

	virtual QString ExpandPath(const QString& Path) const { return m_ExpandPathFn ? m_ExpandPathFn(Path) : Path; }

	virtual QTreeWidget* CreateListWidget();
	virtual QToolButton* CreateAddButton();
	virtual void FillAddMenu();	// may be re-implemented by derived classes

protected slots:
	void OnAdd();
	void OnDel();
	void OnItemDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnSelectionChanged();
	void OnItemChanged(QTreeWidgetItem* pItem, int Column);
	void OnChangeTemplates();

	virtual void OnBrowseFile() {}
	virtual void OnBrowseFolder() {}

protected:
	void ParseAndAddEntry(const QString& Setting, const QString& Value, bool disabled = false, const QString& Template = "");
	void AddEntry(eAccessMode::EAccessMode Mode, const QString& Program, const QString& Path, bool Enabled = true, const QString& Template = QString());
	void ClearEntries();
	void LoadTemplates(bool bShow);

	QString GetSettingName(eAccessMode::EAccessMode Mode) const;
	eAccessMode::EAccessMode GetModeFromSetting(const QString& Setting) const;
	QTreeWidgetItem* FindEntry(eAccessMode::EAccessMode Mode, const QString& Program, const QString& Path, bool bOnlyEnabled = false);
	void CloseAccessEdit(QTreeWidgetItem* pItem, bool bSave);

	void OnConfigChanged();
	void OnReloadPathDisplay();

	QSharedPointer<CSbieIni>	m_pIni;
	QTreeWidget*				m_pTree;
	QCheckBox*					m_pShowTemplates;
	QToolButton*				m_pAddButton;
	QToolButton*				m_pRemoveButton;
	QSet<QString>				m_Programs;

	std::function<QStringList()>			m_GetGroups;
	std::function<QString(const QString&)>	m_ExpandPathFn;

	bool m_bTemplate;

private:
	bool eventFilter(QObject* watched, QEvent* event);
};

////////////////////////////////////////////////////////////////////////////////////////
// CSharedFileWidget: file / folder / pipe access list
////////////////////////////////////////////////////////////////////////////////////////

class CSharedFileWidget : public CSharedAccessWidget
{
	Q_OBJECT

public:
	CSharedFileWidget(QWidget* parent = NULL);

protected:
	virtual QList<SAccessEntryConfig> GetAccessConfig();
	virtual QString GetAccessTypeStr() const { return tr("File/Folder"); }
	virtual int GetAccessTypeId() const { return eAccessType::eFile; }
	virtual eAccessMode::EAccessMode GetDefaultMode() const { return eAccessMode::eOpen; }

	// File and Registry entries auto append a '*' wildcard when they don't
	// contain any; prepending '|' disables this behaviour
	virtual QString ExpandPath(const QString& Path) const;

	virtual void FillAddMenu();
	virtual void OnBrowseFile();
	virtual void OnBrowseFolder();
};