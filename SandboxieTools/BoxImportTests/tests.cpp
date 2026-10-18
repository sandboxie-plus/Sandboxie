// Regression tests for archive configuration replacement, without Sandboxie IPC.
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <QtCore/QString>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtCore/QMap>
#include <memory>
#include "common/ini.h"

// Supplies effective configuration values and observes the query flags. It
// deliberately has no physical-root API, so capture cannot freeze a mount path.
struct TestBox {
    QMap<QString, QString> values;
    bool correctFlags = true;
    QString GetText(const QString& key, const QString& fallback, bool global, bool noExpand, bool templates) {
        correctFlags &= global && noExpand && templates;
        return values.value(key, fallback);
    }
};
using CSandBoxPtr = std::shared_ptr<TestBox>;
#include "import_config.generated.h"

static int failures = 0;

static void Set(CIniFile& ini, const wchar_t* section, const wchar_t* key, const QString& value)
{
    std::wstring data = value.toStdWString();
    if (ini.SetValue(section, key, data.c_str()) != STATUS_SUCCESS)
        ++failures;
}

static QString Get(CIniFile& ini, const wchar_t* section, const wchar_t* key)
{
    std::wstring value;
    ini.GetValue(section, key, value);
    return QString::fromStdWString(value);
}

static void Check(const char* name, const QString& archived, const QString& destination,
    bool replaceGlobal = false, const QString& archiveName = "SourceBox")
{
    CIniFile ini;
    Set(ini, L"GlobalSettings", L"FileRootPath", R"(C:\Default\%USER%\%SANDBOX%)");
    Set(ini, L"RestoredBox", L"FileRootPath", destination);
    if (replaceGlobal)
        Set(ini, L"GlobalSettings", L"", "FileRootPath=Z:\\OtherMachine\\%SANDBOX%\n");
    const QString config = PrepareImportedBoxConfig(archived, archiveName, destination);
    Set(ini, L"RestoredBox", L"", config);
    const QString actual = Get(ini, L"RestoredBox", L"FileRootPath");
    bool ok = actual == destination && Get(ini, L"RestoredBox", L"Enabled") == "y";
    if (archived.contains("UseFileImage=y"))
        ok &= Get(ini, L"RestoredBox", L"UseFileImage") == "y";
    if (archived.contains("Template=Browser"))
        ok &= Get(ini, L"RestoredBox", L"Template") == "Browser";
    QTextStream(stdout) << (ok ? "PASS " : "FAIL ") << name << ": root=" << actual << '\n';
    if (!ok) ++failures;
}

int main()
{
    const QString normal = "[SourceBox]\nEnabled=y\nFileRootPath=Z:\\SourceMachine\\Box\n";
    Check("new custom destination", normal, R"(D:\Imported\%SANDBOX%)");
    Check("destination inherited default", normal, R"(\??\%SystemDrive%\Sandbox\%USER%\%SANDBOX%)");
    Check("existing box destination", normal, R"(E:\Existing\%SANDBOX%)");
    Check("global config cannot relocate destination", normal, R"(C:\Default\%USER%\%SANDBOX%)", true);
    Check("custom destination plus global import", normal, R"(D:\Imported\%SANDBOX%)", true);
    Check("legacy headerless archive", "Enabled=y\nFileRootPath=Z:\\Old\n", R"(D:\Imported\%SANDBOX%)");
    Check("case insensitive source key", "[SourceBox]\nEnabled=y\nfilerootpath=Z:\\Old\n", R"(D:\Imported\%SANDBOX%)");
    Check("spaced source key", "[SourceBox]\nEnabled=y\n  FileRootPath = Z:\\Old\n", R"(D:\Imported\%SANDBOX%)");
    Check("duplicate source roots", normal + "FIlERootPath=Y:\\Other\n", R"(D:\Imported\%SANDBOX%)");
    Check("windows newlines", "[SourceBox]\r\nEnabled=y\r\nFileRootPath=Z:\\Old\r\n", R"(D:\Imported\%SANDBOX%)");
    Check("template variables remain unexpanded", "[SourceBox]\nEnabled=y\nTemplate=Browser\n", R"(%USERPROFILE%\%SANDBOX%)");
    Check("image box keeps image location pattern", "[SourceBox]\nEnabled=y\nUseFileImage=y\nFileRootPath=Z:\\Old\n", R"(D:\Images\%SANDBOX%)");
    Check("second section not imported", normal + "[Other]\nFileRootPath=Y:\\Other\n", R"(D:\Imported\%SANDBOX%)");
#ifdef HAVE_IMPORT_ROOT_SETTING
    auto box = std::make_shared<TestBox>();
    const QString builtIn = R"(\??\%SystemDrive%\Sandbox\%USER%\%SANDBOX%)";
    auto capture = [&](const char* name, const QString& expected) {
        const QString actual = GetImportRootSetting(box);
        const bool ok = actual == expected && box->correctFlags;
        QTextStream(stdout) << (ok ? "PASS " : "FAIL ") << name << ": root=" << actual << '\n';
        if (!ok) ++failures;
    };
    capture("capture built-in default", builtIn);
    box->values["BoxRootFolder"] = R"(E:\Legacy\%USER%)";
    capture("capture legacy root fallback", R"(E:\Legacy\%USER%\Sandbox\%SANDBOX%)");
    box->values["FileRootPath"] = R"(D:\Images\%USER%\%SANDBOX%)";
    capture("capture configured pattern before mounting", R"(D:\Images\%USER%\%SANDBOX%)");
    box->values["FileRootPath"] = R"(%Tmpl.Path%\%SANDBOX%)";
    capture("capture inherited template variables", R"(%Tmpl.Path%\%SANDBOX%)");
#endif
    // Empty or unrelated archive sections must retain the old no-op behavior.
    for (const QString& input : {QString(), QString("[Other]\nEnabled=y\n")}) {
        const bool ok = PrepareImportedBoxConfig(input, "SourceBox", R"(D:\Imported\%SANDBOX%)").isEmpty();
        QTextStream(stdout) << (ok ? "PASS " : "FAIL ") << "empty/missing archive section remains empty\n";
        if (!ok) ++failures;
    }
    QTextStream(stdout) << "failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
