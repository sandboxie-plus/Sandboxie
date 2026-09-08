#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QMap>
#include <QPushButton>
#include <QStringList>
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
#define SB_OK SB_STATUS()

class CSbieIni
{
public:
	QMap<QString, QStringList> Values;
	QStringList Calls;
	int FailCall = 0;
	bool WithTemplates = false;
	bool Refresh = true;
	int Commits = 0;
	CSbieIni* GetAPI() { return this; }
	bool IsConnected() const { return true; }

	SB_STATUS SetBoolSafe(const QString& Setting, bool Value);
	SB_STATUS UpdateTextList(const QString& Setting, const QStringList& List, bool withTemplates);
	QStringList GetTextList(const QString& Setting, bool withTemplates)
	{
		WithTemplates = withTemplates;
		return Values.value(Setting);
	}
	SB_STATUS DelValue(const QString& Setting, const QString& Value)
	{
		Calls.append("delete:" + Value);
		if (Calls.size() == FailCall) return {-123, Calls.last()};
		Values[Setting].removeAll(Value);
		return SB_OK;
	}
	SB_STATUS AppendText(const QString& Setting, const QString& Value)
	{
		Calls.append("append:" + Value);
		if (Calls.size() == FailCall) return {-123, Calls.last()};
		Values[Setting].append(Value);
		return SB_OK;
	}
	QString GetText(const QString&) { return "y"; }
	QString GetName() { return "SyntheticBox"; }
	void SetRefreshOnChange(bool Value) { Refresh = Value; }
	void CommitIniChanges() { ++Commits; }
	QString GetBoxImagePath() { return QString(); }
	void ImBoxCreate(qint64, const QString&) { CHECK(false); }
};
using CSandBoxPlus = CSbieIni;

struct SBoxPointer {
	CSbieIni* Box;
	CSbieIni* operator->() const { return Box; }
	template<class T> T* objectCast() { return static_cast<T*>(Box); }
};
struct SGui {
	QList<SB_STATUS> Errors;
	void CheckResults(const QList<SB_STATUS>& Results, SGui*) { Errors.append(Results); }
} Gui;
static SGui* theGUI = &Gui;
struct SMessageBox {
	static void critical(void*, const char*, const QString&) { CHECK(false); }
};
#define QMessageBox SMessageBox

#include "fixture_return.inc"

class COptionsWindow
{
public:
	CSbieIni Box;
	SBoxPointer m_pBox{&Box};
	bool m_Template = false;
	bool m_ConfigDirty = false;
	QString m_Password;
	qint64 m_ImageSize = 0;
	QStringList Desired{"HarddiskVolume1,1234-ABCD"};
	int Loads = 0, TabUpdates = 0, Closed = 0, PathReloads = 0;
	bool FailStage = false;
	QPushButton EditIni;
	QCheckBox Encrypt;
	QDialogButtonBox Buttons{QDialogButtonBox::Apply};
	struct {
		QPushButton* btnEditIni;
		QCheckBox* chkEncrypt;
		QDialogButtonBox* buttonBox;
	} ui;
	COptionsWindow()
	{
		ui.btnEditIni = &EditIni;
		ui.chkEncrypt = &Encrypt;
		ui.buttonBox = &Buttons;
		ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
	}
	void WriteAdvancedCheck(QCheckBox*, const QString&, const QString&, const QString&);
	void WriteTextList(const QString&, const QStringList&);
	SAVE_CONFIG_RESULT SaveConfig();
	bool apply();
	void ok();
	QString tr(const char* text) { return QString::fromUtf8(text); }
	void close() { ++Closed; }
	void TriggerPathReload() { ++PathReloads; }
	void OnSetPassword() { CHECK(false); }
	bool SaveIniSection() { CHECK(false); return false; }
	void LoadConfig() { ++Loads; }
	void UpdateCurrentTab() { ++TabUpdates; }
	void CloseINetEdit() {}
	void CloseNetFwEdit() {}
	void CloseAccessEdit() {}
	void CloseOptionEdit() {}
	void CloseCopyEdit() {}
	void CloseNetProxyEdit() {}
	void SaveDebug() {}
#define STAGE(flag, method) bool flag = false; void method() { flag = false; }
	STAGE(m_GeneralChanged, SaveGeneral)
	STAGE(m_CopyRulesChanged, SaveCopyRules)
	STAGE(m_GroupsChanged, SaveGroups)
	STAGE(m_ForcedChanged, SaveForced)
	STAGE(m_StopChanged, SaveStop)
	STAGE(m_StartChanged, SaveStart)
	STAGE(m_INetBlockChanged, SaveINetAccess)
	STAGE(m_NetFwRulesChanged, SaveNetFwRules)
	STAGE(m_DnsFilterChanged, SaveDnsFilter)
	STAGE(m_NetProxyChanged, SaveNetProxy)
	STAGE(m_NetworkChanged, SaveNetwork)
	STAGE(m_AccessChanged, SaveAccessList)
	STAGE(m_RecoveryChanged, SaveRecoveryList)
	STAGE(m_TemplatesChanged, SaveTemplates)
	STAGE(m_FoldersChanged, SaveFolders)
#undef STAGE
	bool m_AdvancedChanged = true;
	void SaveAdvanced()
	{
		if (FailStage) throw SB_STATUS{-321, "stage"};
		WriteTextList("DiskSerialNumber", Desired);
		m_AdvancedChanged = false;
	}
};

#include "ini_write_under_test.inc"

static void ListDelete()
{
	CSbieIni Ini;
	Ini.Values["Test"] = QStringList{"old-a", "old-b"};
	Ini.FailCall = 1;
	SB_STATUS Result = Ini.UpdateTextList("Test", {"new"}, false);
	CHECK(!Result && Result.Code == -123 && Result.Detail == "delete:old-a");
	CHECK(Ini.Calls == QStringList{"delete:old-a"});
	CHECK((Ini.Values["Test"] == QStringList{"old-a", "old-b"}));
}
static void ListAppend()
{
	CSbieIni Ini;
	Ini.FailCall = 1;
	SB_STATUS Result = Ini.UpdateTextList("Test", {"a", "b"}, false);
	CHECK(!Result && Result.Code == -123 && Result.Detail == "append:a");
	CHECK(Ini.Calls == QStringList{"append:a"} && Ini.Values["Test"].isEmpty());
}
static void ListRetry()
{
	CSbieIni Ini;
	Ini.Values["Test"] = QStringList{"old"};
	Ini.FailCall = 3;
	SB_STATUS Result = Ini.UpdateTextList("Test", {"a", "b", "c"}, false);
	CHECK(!Result && Result.Detail == "append:b");
	CHECK(Ini.Values["Test"] == QStringList{"a"});
	CHECK((Ini.Calls == QStringList{"delete:old", "append:a", "append:b"}));
	Ini.FailCall = 0;
	CHECK(Ini.UpdateTextList("Test", {"a", "b", "c"}, false));
	CHECK((Ini.Values["Test"] == QStringList{"a", "b", "c"}));
	CHECK(Ini.Calls.size() == 5);
}
static void ListNoop()
{
	CSbieIni Ini;
	Ini.Values["Test"] = QStringList{"a", "a", "b"};
	CHECK(Ini.UpdateTextList("Test", {"a", "a", "b"}, true));
	CHECK(Ini.WithTemplates && Ini.Calls.isEmpty());
	CHECK(Ini.UpdateTextList("Test", {"b", "a", "a"}, false));
	CHECK(!Ini.WithTemplates && Ini.Calls.isEmpty());
}
static void BoolDelete()
{
	CSbieIni Ini;
	Ini.Values["Test"] = QStringList{"excluded.exe,n", "n"};
	Ini.FailCall = 1;
	SB_STATUS Result = Ini.SetBoolSafe("Test", true);
	CHECK(!Result && Result.Detail == "delete:n");
	CHECK(Ini.Calls == QStringList{"delete:n"});
	CHECK((Ini.Values["Test"] == QStringList{"excluded.exe,n", "n"}));
}
static void BoolSuccess()
{
	CSbieIni Ini;
	Ini.Values["Test"] = QStringList{"excluded.exe,n", "n"};
	CHECK(Ini.SetBoolSafe("Test", true));
	CHECK((Ini.Values["Test"] == QStringList{"excluded.exe,n", "y"}));
	Ini.Calls.clear();
	CHECK(Ini.SetBoolSafe("Test", true) && Ini.Calls.isEmpty());
}
static void CheckboxDelete()
{
	COptionsWindow Window;
	Window.Box.Values["Test"] = QStringList{"excluded.exe,n", "n"};
	Window.Box.FailCall = 1;
	QCheckBox Check;
	Check.setChecked(true);
	bool Thrown = false;
	try { Window.WriteAdvancedCheck(&Check, "Test", "y", ""); }
	catch (SB_STATUS Result) { Thrown = !Result && Result.Detail == "delete:n"; }
	CHECK(Thrown && Window.Box.Calls == QStringList{"delete:n"});
}
static void OptionsFailure()
{
	COptionsWindow Window;
	Window.Box.FailCall = 1;
	CHECK(!Window.apply());
	CHECK(Window.Loads == 0 && Window.TabUpdates == 0);
	CHECK(Window.m_AdvancedChanged && Window.Box.Refresh);
	CHECK(Window.ui.buttonBox->button(QDialogButtonBox::Apply)->isEnabled());
	CHECK(Gui.Errors.size() == 1 && Gui.Errors[0].Code == -123);
	Window.Box.FailCall = 2;
	Window.ok();
	CHECK(Window.Closed == 0 && Window.Loads == 0);
	Window.Box.FailCall = 0;
	Window.ok();
	CHECK(Window.Closed == 1 && Window.Loads == 1 && Window.TabUpdates == 1);
	CHECK(!Window.m_AdvancedChanged && Window.Box.Refresh);
	CHECK(Window.Box.Values["DiskSerialNumber"] == Window.Desired);
}
static void OptionsReportedFailure()
{
	COptionsWindow Window;
	Window.FailStage = true;
	Window.ok();
	CHECK(Window.Closed == 0 && Window.Loads == 0 && Window.TabUpdates == 0);
	CHECK(Window.m_AdvancedChanged && Window.Box.Refresh);
	CHECK(Window.ui.buttonBox->button(QDialogButtonBox::Apply)->isEnabled());
	CHECK(Gui.Errors.size() == 1 && Gui.Errors[0].Code == -321);
}
static void OptionsSuccess()
{
	COptionsWindow Window;
	Window.ok();
	CHECK(Window.Closed == 1 && Window.Loads == 1 && Window.TabUpdates == 1);
	CHECK(Window.Box.Refresh && Window.Box.Commits == 1);
	CHECK(!Window.m_AdvancedChanged && Gui.Errors.isEmpty());
	CHECK(!Window.ui.buttonBox->button(QDialogButtonBox::Apply)->isEnabled());
}
int main(int argc, char** argv)
{
	QApplication App(argc, argv);
	QMap<QString, std::function<void()>> Cases{
		{"list-delete", ListDelete}, {"list-append", ListAppend}, {"list-retry", ListRetry},
		{"list-noop", ListNoop}, {"bool-delete", BoolDelete}, {"bool-success", BoolSuccess},
		{"checkbox-delete", CheckboxDelete}, {"options-failure", OptionsFailure}, {"options-success", OptionsSuccess}, {"options-reported-failure", OptionsReportedFailure}
	};
	if (argc != 2 || !Cases.contains(QString::fromUtf8(argv[1]))) return 2;
	Cases.value(QString::fromUtf8(argv[1]))();
	std::printf("PASS: %s\n", argv[1]);
	return 0;
}
