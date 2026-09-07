#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QMap>
#include <QPushButton>
#include <QStringList>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextCursor>
#include <QTreeWidget>
#include <cstdio>
#include <cstdlib>
#include <functional>

#define CHECK(condition) do { if (!(condition)) { \
	std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); std::exit(1); \
} } while (0)

struct SB_STATUS {
	int Code = 0;
	QString Detail;
	operator bool() const { return Code == 0; }
};

struct SStorage {
	bool Connected = true;
	bool Fail = false;
	bool PartialWrite = false;
	QString Persisted = "# synthetic baseline\nEnabled=y\n";
	QString Section, Setting, Submitted;
	int Writes = 0;
	SB_STATUS SbieIniSet(const QString& section, const QString& setting, const QString& value)
	{
		++Writes;
		Section = section; Setting = setting; Submitted = value;
		if (Fail) {
			if (PartialWrite) Persisted = "# synthetic partial write\nEnabled=y\n";
			return {-123, "synthetic_write_failure"};
		}
		Persisted = value;
		return {};
	}
	bool IsConnected() const { return Connected; }
	QString GetName() const { return "SyntheticBox"; }
	QString GetText(const QString&) const { return "y"; }
	QString GetBoxImagePath() const { return {}; }
	void ImBoxCreate(qint64, const QString&) { CHECK(false); }
};
using CSandBoxPlus = SStorage;
struct SBoxPointer {
	SStorage* Box;
	SStorage* operator->() const { return Box; }
	template<class T> T* objectCast() { return static_cast<T*>(Box); }
};
struct SGui {
	struct Addons { void UpdateAddonsWhenNotCached() { CHECK(false); } } addons;
	Addons* GetAddonManager() { return &addons; }
	void CheckCompat(QWidget*, const char*) { CHECK(false); }
	QList<SB_STATUS> Errors;
	QWidget* ErrorParent = nullptr;
	void CheckResults(const QList<SB_STATUS>& results, QWidget* parent)
	{
		Errors.append(results); ErrorParent = parent;
	}
} Gui;
[[maybe_unused]] static SGui* theGUI = &Gui;
static SStorage* theAPI = nullptr;
struct SConfig { int GetInt(const char*, int) { CHECK(false); return 0; } } Conf;
static SConfig* theConf = &Conf;
template<class... Args> static void TryRefreshCert(Args...) { CHECK(false); }
struct SMessageBox {
	static int Criticals;
	static void critical(QWidget*, const char*, const QString&) { ++Criticals; }
};
int SMessageBox::Criticals = 0;
#define QMessageBox SMessageBox

struct SCodeEdit {
	QTextEdit Editor;
	QString GetCode() const { return Editor.toPlainText(); }
	void SetCode(const QString& value) { Editor.setPlainText(value); }
};
struct SEditorWindow : QDialog {
	SStorage Storage;
	SCodeEdit Code;
	SCodeEdit* m_pCodeEdit = &Code;
	QTreeWidget Navigation;
	QTreeWidget* m_pTree = &Navigation;
	QTabWidget Tabs;
	QPushButton EditIni, SaveIni, CancelEdit;
	QCheckBox Encrypt;
	QDialogButtonBox Buttons{QDialogButtonBox::Apply};
	bool m_HoldChange = false;
	int Loads = 0, IniLoads = 0, Closed = 0, TabUpdates = 0, StructuredSaves = 0;
	struct {
		QWidget* tabEdit;
		QTabWidget* tabs;
		QPushButton* btnEditIni;
		QPushButton* btnSaveIni;
		QPushButton* btnCancelEdit;
		QCheckBox* chkEncrypt;
		QDialogButtonBox* buttonBox;
		QWidget* tabSupport = nullptr;
		QWidget* tabAddons = nullptr;
		QWidget* tabCompat = nullptr;
		QPushButton* lblCurrent = nullptr;
		QPushButton* lblUpdateAddons = nullptr;
		QCheckBox* chkAutoUpdate = nullptr;
	} ui;
	SEditorWindow(bool tree)
	{
		Gui.Errors.clear(); Gui.ErrorParent = nullptr; SMessageBox::Criticals = 0;
		if (!tree) m_pTree = nullptr;
		ui = {nullptr, &Tabs, &EditIni, &SaveIni, &CancelEdit, &Encrypt, &Buttons};
		for (int i = 0; i < 7; ++i) Tabs.addTab(new QWidget(), QString::number(i));
		ui.tabEdit = Tabs.widget(Tabs.count() - 1);
		Tabs.widget(5)->setEnabled(false);
		ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
		Code.SetCode(QString::fromUtf8("# synthetic \xc3\xa9\xe6\xbc\xa2\nEnabled=y\nTest=a\nTest=b\n\n"));
	}
	bool close() { ++Closed; return QDialog::close(); }
	void LoadIniSection() { ++IniLoads; Code.SetCode(Storage.Persisted); }
	void UpdateCurrentTab() { ++TabUpdates; }
	void CloseINetEdit() {}
	void CloseNetFwEdit() {}
	void CloseAccessEdit() {}
	void CloseOptionEdit() {}
	void CloseCopyEdit() {}
	void CloseNetProxyEdit() {}
};

#include "raw_ini_return.inc"

class COptionsWindow : public SEditorWindow {
public:
	using SEditorWindow::SEditorWindow;
	SBoxPointer m_pBox{&Storage};
	QWidget* m_pCurrentTab = nullptr;
	QString StructuredCode = Storage.Persisted;
	void OnTab(QWidget*);
	bool m_Template = false, m_GeneralChanged = false, m_ConfigDirty = false;
	QString m_Password;
	qint64 m_ImageSize = 0;
	OPTIONSWINDOW_SAVEINISECTION_RESULT SaveIniSection();
	OPTIONSWINDOW_APPLY_RESULT apply();
	void OnSaveIni();
	void ok();
	void SetIniEdit(bool);
	void OnIniChanged();
	void OnCancelEdit();
	void OnSetPassword() { CHECK(false); }
	bool SaveConfig() { ++StructuredSaves; return true; }
	void LoadConfig() { ++Loads; m_ConfigDirty = false; StructuredCode = Storage.Persisted; }
};
class CSettingsWindow : public SEditorWindow {
public:
	QWidget* m_pCurrentTab = nullptr;
	int m_CompatLoaded = 0;
	bool m_SettingsDirty = false;
	static bool CertRefreshRequired() { CHECK(false); return false; }
	void GetUpdates() { CHECK(false); }
	void OnTab(QWidget*);
	CSettingsWindow(bool tree) : SEditorWindow(tree) { theAPI = &Storage; }
	~CSettingsWindow() { theAPI = nullptr; }
	SETTINGSWINDOW_SAVEINISECTION_RESULT SaveIniSection();
	SETTINGSWINDOW_APPLY_RESULT apply();
	void OnSaveIni();
	void ok();
	void SetIniEdit(bool);
	void OnIniChanged();
	void OnCancelEdit();
	void SaveSettings() { ++StructuredSaves; }
	QString StructuredCode = Storage.Persisted;
	void LoadSettings() { ++Loads; StructuredCode = Storage.Persisted; }
};

#include "raw_ini_under_test.inc"

static void CheckEditing(const SEditorWindow& window)
{
	CHECK(!window.ui.btnEditIni->isEnabled());
	CHECK(window.ui.btnSaveIni->isEnabled() && window.ui.btnCancelEdit->isEnabled());
	CHECK(window.ui.buttonBox->button(QDialogButtonBox::Apply)->isEnabled());
	if (window.m_pTree) CHECK(!window.m_pTree->isEnabled());
	else CHECK(!window.ui.tabs->isTabEnabled(4));
}
template<class T> static void Invoke(T& window, const QString& action)
{
	if (action == "save") window.OnSaveIni();
	else if (action == "apply") window.apply();
	else if (action == "ok") window.ok();
	else CHECK(false);
}
template<class T> static void FailureRetry(bool tree, const QString& action)
{
	T Window(tree);
	Window.SetIniEdit(true);
	Window.Code.Editor.moveCursor(QTextCursor::End);
	Window.Code.Editor.insertPlainText("# pending edit\n");
	QTextCursor Cursor = Window.Code.Editor.textCursor();
	Cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 5);
	Window.Code.Editor.setTextCursor(Cursor);
	CHECK(Window.Code.Editor.document()->isUndoAvailable());
	const QString Pending = Window.Code.GetCode();
	const QString Persisted = Window.Storage.Persisted;
	Window.Storage.Fail = true;
	Invoke(Window, action);
	CHECK(Window.Storage.Writes == 1 && Window.Storage.Submitted == Pending);
	CHECK(Window.Storage.Persisted == Persisted && Window.Code.GetCode() == Pending);
	CHECK(Window.Loads == 0 && Window.IniLoads == 0 && Window.Closed == 0 && Window.TabUpdates == 0);
	CHECK(Window.StructuredSaves == 0 && SMessageBox::Criticals == 0);
	CHECK(Gui.Errors.size() == 1 && Gui.Errors[0].Code == -123);
	CHECK(Gui.Errors[0].Detail == "synthetic_write_failure" && Gui.ErrorParent == &Window);
	CheckEditing(Window);
	CHECK(Window.Code.Editor.textCursor().position() == Cursor.position());
	CHECK(Window.Code.Editor.textCursor().anchor() == Cursor.anchor());
	CHECK(Window.Code.Editor.document()->isUndoAvailable());
	Window.Storage.Fail = false;
	Invoke(Window, action);
	CHECK(Window.Storage.Writes == 2 && Window.Storage.Persisted == Pending);
	CHECK(Window.Code.GetCode() == Pending && Gui.Errors.size() == 1);
	CHECK(Window.StructuredSaves == 0);
	CHECK(Window.Closed == (action == "ok" ? 1 : 0));
	CHECK(Window.Storage.Setting.isEmpty());
	if (action == "save") {
		CHECK(Window.ui.btnEditIni->isEnabled());
		CHECK(!Window.ui.btnSaveIni->isEnabled() && !Window.ui.btnCancelEdit->isEnabled());
		if (Window.m_pTree) CHECK(Window.m_pTree->isEnabled());
		else {
			CHECK(Window.ui.tabs->isTabEnabled(4));
			CHECK(!Window.ui.tabs->isTabEnabled(5));
		}
	}
}
static void BoxDirty(bool tree)
{
	COptionsWindow Window(tree);
	Window.SetIniEdit(true);
	Window.Storage.Fail = true;
	Window.OnSaveIni();
	CHECK(Window.m_ConfigDirty);
	Window.m_ConfigDirty = true;
	Window.OnSaveIni();
	CHECK(Window.m_ConfigDirty && Window.Storage.Writes == 2);
	Window.m_ConfigDirty = false;
	Window.Storage.Fail = false;
	Window.OnSaveIni();
	CHECK(Window.m_ConfigDirty && Window.Storage.Section == "SyntheticBox");
}
static void GlobalOffline(bool tree)
{
	for (const auto& Action : {"save", "apply", "ok"}) {
		CSettingsWindow Window(tree);
		Window.SetIniEdit(true);
		const QString Pending = Window.Code.GetCode();
		Window.Storage.Connected = false;
		Invoke(Window, Action);
		CHECK(Window.Storage.Writes == 0 && Window.Loads == 0 && Window.Closed == 0);
		CHECK(Window.StructuredSaves == 0 && Window.Code.GetCode() == Pending);
		CHECK(SMessageBox::Criticals == 1 && Gui.Errors.isEmpty());
		CheckEditing(Window);
		Window.Storage.Connected = true;
		Invoke(Window, Action);
		CHECK(Window.Storage.Writes == 1 && Window.Storage.Persisted == Pending);
		CHECK(Window.Storage.Section == "GlobalSettings" && Window.Loads == 1);
	}
}
template<class T> static void Empty(bool tree)
{
	T Window(tree);
	Window.SetIniEdit(true);
	Window.Code.SetCode("");
	Window.OnSaveIni();
	CHECK(Window.Storage.Writes == 1 && Window.Storage.Submitted.isEmpty());
	CHECK(Window.Storage.Persisted.isEmpty() && Gui.Errors.isEmpty());
	CHECK(Window.ui.btnEditIni->isEnabled());
}
template<class T> static void Cancel(bool tree)
{
	T Window(tree);
	Window.SetIniEdit(true);
	Window.Storage.Fail = true;
	Window.OnSaveIni();
	Window.OnCancelEdit();
	CHECK(Window.IniLoads == 1 && Window.Storage.Writes == 1);
	CHECK(Window.Code.GetCode() == Window.Storage.Persisted);
	CHECK(Window.ui.btnEditIni->isEnabled() && Window.Closed == 0);
}
static void Structured(bool tree)
{
	CSettingsWindow Window(tree);
	Window.SetIniEdit(false);
	Window.ok();
	CHECK(Window.StructuredSaves == 1 && Window.Loads == 1 && Window.Closed == 1);
	CHECK(Window.Storage.Writes == 0 && Gui.Errors.isEmpty());
}

template<class T> static void PartialFailureRetry(bool tree)
{
	for (const auto& Action : {"save", "apply", "ok"}) {
		T Window(tree);
		Window.SetIniEdit(true);
		const QString Pending = Window.Code.GetCode();
		const QString Before = Window.Storage.Persisted;
		Window.Storage.Fail = true;
		Window.Storage.PartialWrite = true;
		Invoke(Window, Action);
		CHECK(Window.Storage.Writes == 1 && Window.Storage.Persisted != Before);
		CHECK(Window.Storage.Persisted != Pending && Window.Code.GetCode() == Pending);
		CHECK(Window.Loads == 0 && Window.IniLoads == 0 && Window.Closed == 0);
		CHECK(Window.StructuredSaves == 0 && Gui.Errors.size() == 1);
		CHECK(Gui.Errors[0].Code == -123 && Gui.ErrorParent == &Window);
		CheckEditing(Window);
		Window.Storage.Fail = false;
		Window.Storage.PartialWrite = false;
		Invoke(Window, Action);
		CHECK(Window.Storage.Writes == 2 && Window.Storage.Persisted == Pending);
		CHECK(Window.Code.GetCode() == Pending && Gui.Errors.size() == 1);
		CHECK(Window.StructuredSaves == 0 && Window.Closed == (QString(Action) == "ok" ? 1 : 0));
	}
}

static void PartialCancelReload(bool tree)
{
	COptionsWindow Window(tree);
	Window.SetIniEdit(true);
	const QString Pending = Window.Code.GetCode();
	const QString Before = Window.StructuredCode;
	Window.Storage.Fail = true;
	Window.Storage.PartialWrite = true;
	Window.OnSaveIni();
	CHECK(Window.Storage.Writes == 1 && Window.Storage.Persisted != Before);
	CHECK(Window.Code.GetCode() == Pending && Window.Loads == 0 && Window.Closed == 0);
	CHECK(Gui.Errors.size() == 1 && Gui.Errors[0].Code == -123);
	Window.OnCancelEdit();
	CHECK(Window.Code.GetCode() == Window.Storage.Persisted && Window.Storage.Writes == 1);
	Window.OnTab(Window.ui.tabs->widget(0));
	CHECK(Window.Loads == 1 && Window.StructuredCode == Window.Storage.Persisted);
	CHECK(Window.StructuredCode != Before && !Window.m_ConfigDirty);
	CHECK(Window.Storage.Writes == 1 && Window.Closed == 0 && Gui.Errors.size() == 1);
}

static void GlobalPartialCancelReload(bool tree)
{
 CSettingsWindow Window(tree);
 Window.SetIniEdit(true);
 const QString Before = Window.StructuredCode;
 Window.Storage.Fail = true;
 Window.Storage.PartialWrite = true;
 Window.OnSaveIni();
 CHECK(Window.Storage.Persisted != Before);
 CHECK(Window.Loads == 0);
 Window.OnCancelEdit();
 CHECK(Window.Code.GetCode() == Window.Storage.Persisted);
 CHECK(Window.ui.btnEditIni->isEnabled());
 Window.OnTab(Window.ui.tabs->widget(0));
 CHECK(Window.StructuredCode == Window.Storage.Persisted);
}

int main(int argc, char** argv)
{
	QApplication App(argc, argv);
	QMap<QString, std::function<void(bool)>> Cases{
		{"box-save", [](bool tree) { FailureRetry<COptionsWindow>(tree, "save"); }},
		{"box-apply", [](bool tree) { FailureRetry<COptionsWindow>(tree, "apply"); }},
		{"box-ok", [](bool tree) { FailureRetry<COptionsWindow>(tree, "ok"); }},
		{"box-dirty", BoxDirty}, {"box-empty", Empty<COptionsWindow>}, {"box-cancel", Cancel<COptionsWindow>},
		{"global-save", [](bool tree) { FailureRetry<CSettingsWindow>(tree, "save"); }},
		{"global-apply", [](bool tree) { FailureRetry<CSettingsWindow>(tree, "apply"); }},
		{"global-ok", [](bool tree) { FailureRetry<CSettingsWindow>(tree, "ok"); }},
		{"global-offline", GlobalOffline}, {"global-empty", Empty<CSettingsWindow>},
		{"global-cancel", Cancel<CSettingsWindow>}, {"global-structured", Structured},
		{"box-partial", PartialFailureRetry<COptionsWindow>}, {"global-partial", PartialFailureRetry<CSettingsWindow>},
		{"box-partial-cancel", PartialCancelReload}, {"global-partial-cancel", GlobalPartialCancelReload}
	};
	if (argc != 2 || !Cases.contains(QString::fromUtf8(argv[1]))) return 2;
	for (bool Tree : {false, true}) Cases.value(QString::fromUtf8(argv[1]))(Tree);
	std::printf("PASS: %s (tabs and tree)\n", argv[1]);
	return 0;
}
