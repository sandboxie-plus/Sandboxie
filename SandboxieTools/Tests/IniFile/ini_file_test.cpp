#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>

#include "ini.h"
#include "defines.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// This is only the value-edge rule from SbieDrv's Conf_Read_Settings.
// The production driver is not linked into this user-mode test.
static std::wstring DriverValue(const std::wstring& value)
{
    size_t first = 0, last = value.size();
    while (first < last && value[first] <= 32) ++first;
    while (last > first && value[last - 1] <= 32) --last;
    return value.substr(first, last - first);
}

static std::string Describe(const std::wstring& value)
{
    std::ostringstream out;
    size_t displayed = 0;
    for (wchar_t ch : value) {
        if (displayed++ == 96) {
            out << "... (" << std::dec << value.size() << " UTF-16 units)";
            break;
        }
        if (ch >= 32 && ch < 127) out << static_cast<char>(ch);
        else out << "<U+" << std::hex << std::uppercase << std::setfill('0')
                 << std::setw(4) << static_cast<unsigned>(ch) << ">";
    }
    return out.str();
}

static void Require(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

static void Success(NTSTATUS status, const char* operation)
{
    if (status < 0) {
        std::ostringstream text;
        text << operation << " returned 0x" << std::hex << static_cast<ULONG>(status);
        throw std::runtime_error(text.str());
    }
}

static void Equal(const std::wstring& actual, const std::wstring& expected, const char* detail)
{
    Require(actual == expected, std::string(detail) + ": expected [" + Describe(expected)
        + "] but got [" + Describe(actual) + "]");
}

enum class Encoding { Utf8Bom, Utf16LE, Utf16BE };

// Only create fresh test-owned files. SaveIni subsequently writes those same files.
static void CreateIni(const fs::path& path, const std::wstring& text, Encoding encoding)
{
    std::string bytes;
    if (encoding == Encoding::Utf8Bom) {
        bytes = "\xEF\xBB\xBF";
        const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            nullptr, 0, nullptr, nullptr);
        Require(size > 0, "UTF-8 conversion failed");
        const size_t start = bytes.size();
        bytes.resize(start + size);
        Require(WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            bytes.data() + start, size, nullptr, nullptr) == size, "UTF-8 conversion failed");
    } else {
        const bool big = encoding == Encoding::Utf16BE;
        bytes = big ? "\xFE\xFF" : "\xFF\xFE";
        for (wchar_t ch : text) {
            bytes.push_back(static_cast<char>(big ? ch >> 8 : ch & 255));
            bytes.push_back(static_cast<char>(big ? ch & 255 : ch >> 8));
        }
    }
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    Require(file != INVALID_HANDLE_VALUE, "Could not create a fresh test INI");
    DWORD written = 0;
    const BOOL ok = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr);
    CloseHandle(file);
    Require(ok && written == bytes.size(), "Could not write the test INI");
}

static fs::path OutputDirectory;
static unsigned FileSequence = 0;
static constexpr const wchar_t* Section = L"UserSettings_TEST_ONLY";
static constexpr const wchar_t* Key = L"SbieCtrl_HideMessage";

class Fixture
{
public:
    fs::path Path;
    CIniFile Ini;

    explicit Fixture(const std::wstring& body = L"", Encoding encoding = Encoding::Utf16LE)
    {
        Path = OutputDirectory / (L"fixture-" + std::to_wstring(++FileSequence) + L".ini");
        CreateIni(Path, L"# Test fixture, never a user configuration\r\n[UserSettings_TEST_ONLY]\r\n"
            + body, encoding);
        Success(Ini.LoadIni(Path.c_str()), "LoadIni");
    }

    std::wstring Read(const wchar_t* key = Key)
    {
        std::wstring value;
        Success(Ini.GetValue(Section, key, value), "GetValue");
        return value;
    }

    void SaveReload()
    {
        Success(Ini.SaveIni(Path.c_str()), "SaveIni");
        Success(Ini.LoadIni(Path.c_str()), "LoadIni after SaveIni");
    }
};

static void LoadedTailDelete(Encoding encoding)
{
    Fixture f(std::wstring(Key) + L"=2112,test" + wchar_t(3) + L"\r\n", encoding);
    const std::wstring cached = f.Read();
    const std::wstring driver = DriverValue(cached);
    Equal(driver, L"2112,test", "driver-side value model");
    Success(f.Ini.RemoveValue(Section, Key, driver.c_str()), "RemoveValue using driver value");
    f.SaveReload();
    Equal(f.Read(), L"", "reported #5552 delete/save/reload");
}

static void LoadedValueEdges()
{
    for (wchar_t ch = 1; ch <= 32; ++ch) {
        // CR/LF delimit physical lines and have separate compatibility cases.
        if (ch == L'\r' || ch == L'\n') continue;
        Fixture f(std::wstring(Key) + L"=" + ch + L"2112,test" + ch + L"\r\n");
        Equal(f.Read(), L"2112,test", "parsed value edges");
        Success(f.Ini.RemoveValue(Section, Key, L"2112,test"), "RemoveValue");
        f.SaveReload();
        Equal(f.Read(), L"", "deletion of normalized loaded value");
    }
}

static void AddValueEdges()
{
    for (wchar_t ch = 1; ch <= 32; ++ch) {
        if (ch == L'\r' || ch == L'\n') continue;
        Fixture f;
        const std::wstring value = std::wstring(1, ch) + L"2112,test" + ch;
        Success(f.Ini.AddValue(Section, Key, value.c_str()), "AddValue");
        Equal(f.Read(), L"2112,test", "AddValue cache must match driver before reload");
        Success(f.Ini.RemoveValue(Section, Key, L"2112,test"), "RemoveValue after AddValue");
        f.SaveReload();
        Equal(f.Read(), L"", "deletion of normalized added value");
    }
}

static void SetValueEdges()
{
    for (wchar_t ch = 1; ch <= 32; ++ch) {
        if (ch == L'\r' || ch == L'\n') continue;
        Fixture f(std::wstring(Key) + L"=old\r\n");
        const std::wstring value = std::wstring(1, ch) + L"2112,test" + ch;
        Success(f.Ini.SetValue(Section, Key, value.c_str()), "SetValue");
        Equal(f.Read(), L"2112,test", "SetValue cache must match driver before reload");
        f.SaveReload();
        Success(f.Ini.RemoveValue(Section, Key, L"2112,test"), "RemoveValue after SetValue");
        f.SaveReload();
        Equal(f.Read(), L"", "deletion of normalized replaced value");
    }
}

static void ClassicControlledList()
{
    Fixture f(std::wstring(Key) + L"=old\r\n");
    const std::wstring list = std::wstring(1, wchar_t(3)) + L"2112,first" + wchar_t(3)
        + L"\n\t2113,second\t";
    Success(f.Ini.SetValue(Section, Key, list.c_str()), "Classic LF list SetValue");
    Equal(f.Read(), L"2112,first\n2113,second", "each Classic list item is normalized");
    f.SaveReload();
    Equal(f.Read(), L"2112,first\n2113,second", "normalized Classic list persists");
}

static void RawSectionControl()
{
    Fixture f(L"Old=gone\r\n");
    const std::wstring section = L"# retained comment\r\n" + std::wstring(Key)
        + L"=\x0003" L"2112,test\x0003\r\nOther=kept\r\n";
    Success(f.Ini.SetValue(Section, L"", section.c_str()), "raw section SetValue");
    Equal(f.Read(), L"2112,test", "raw section value is normalized");
    Equal(f.Read(L"Old"), L"", "raw section replaces old entries");
    f.SaveReload();
    Equal(f.Read(L"Other"), L"kept", "raw section preserves neighbors");
}

static void WhitespaceOnlyParsedValue()
{
    Fixture f(L"Empty= \t \r\nOther=kept\r\n");
    Equal(f.Read(L"Empty"), L"", "whitespace-only value must not throw or cross the line");
    Equal(f.Read(L"Other"), L"kept", "neighbor after empty value");
    f.SaveReload();
    Equal(f.Read(L"Other"), L"kept", "neighbor survives save of empty value");
}

static void NormalLoadPreservation(Encoding encoding)
{
    const std::wstring unicode = L"中文，正常值\u00A0\u3000";
    const std::wstring interior = L"first" + std::wstring(1, wchar_t(3)) + L"second";
    Fixture f(L"  Spaces \t= \talpha beta\t \r\nUnicode=" + unicode
        + L"\r\nInterior=" + interior + L"\r\nNbsp=\u00A0value\u00A0\r\n", encoding);
    Equal(f.Read(L"Spaces"), L"alpha beta", "ordinary space/tab parsing");
    Equal(f.Read(L"Unicode"), unicode, "Unicode is not <=32 whitespace");
    Equal(f.Read(L"Interior"), interior, "interior controls are preserved");
    Equal(f.Read(L"Nbsp"), L"\u00A0value\u00A0", "NBSP is not trimmed");
    f.SaveReload();
    Equal(f.Read(L"Unicode"), unicode, "Unicode roundtrip");
    Equal(f.Read(L"Interior"), interior, "interior control roundtrip");
}

static void AddInsertAndDuplicateDelete()
{
    Fixture f;
    Success(f.Ini.AddValue(Section, Key, L"middle"), "append first");
    Success(f.Ini.AddValue(Section, Key, L"last"), "append second");
    Success(f.Ini.AddValue(Section, Key, L"first", true), "insert");
    Success(f.Ini.AddValue(Section, Key, L"middle"), "append duplicate");
    Equal(f.Read(), L"first\nmiddle\nlast\nmiddle", "list order and duplicate preservation");
    Success(f.Ini.RemoveValue(Section, Key, L"MIDDLE"), "case-insensitive duplicate delete");
    Success(f.Ini.RemoveValue(Section, Key, L"missing"), "idempotent missing delete");
    f.SaveReload();
    Equal(f.Read(), L"first\nlast", "delete all matching duplicates only");
}

static void AddRawThenDeleteRaw()
{
    Fixture f;
    const std::wstring raw = L" \t" + std::wstring(1, wchar_t(3)) + L"test"
        + wchar_t(3) + L"\t ";
    Success(f.Ini.AddValue(Section, Key, raw.c_str()), "add raw value");
    Success(f.Ini.RemoveValue(Section, Key, raw.c_str()), "remove original raw value");
    Equal(f.Read(), L"", "raw removal argument must match normalized cached value");
    f.SaveReload();
    Equal(f.Read(), L"", "raw add/remove roundtrip");
}

static void NonemptyWhitespaceDelete()
{
    Fixture f(std::wstring(Key) + L"=first\r\n" + Key + L"=second\r\n");
    for (const auto& raw : {std::wstring(L" \t "), std::wstring(1, wchar_t(3)),
             std::wstring(L"\r\n")}) {
        Success(f.Ini.RemoveValue(Section, Key, raw.c_str()), "remove a nonempty whitespace target");
        Equal(f.Read(), L"first\nsecond", "normalizing a target must not turn it into delete-all");
    }
    f.SaveReload();
    Equal(f.Read(), L"first\nsecond", "nonempty whitespace deletion leaves normal values alone");
}

static void TruncateBeforeTrim()
{
    Fixture f;
    const std::wstring wanted(CONF_LINE_LEN - 1, L'x');
    const std::wstring input = L" " + wanted + L"tail";
    Success(f.Ini.SetValue(Section, Key, input.c_str()), "oversized SetValue with leading space");
    Equal(f.Read(), wanted, "SetValue keeps its original truncate-before-trim order");
    const std::wstring overLimit(CONF_LINE_LEN + 40, L'z');
    Success(f.Ini.SetValue(Section, Key, overLimit.c_str()), "oversized ordinary SetValue");
    Equal(f.Read(), std::wstring(CONF_LINE_LEN, L'z'), "ordinary CONF_LINE_LEN truncation remains");
}

static void LongClassicSegment()
{
    Fixture f;
    const std::wstring first(CONF_LINE_LEN + 40, L'x');
    const std::wstring input = first + L"\nsecond";
    Success(f.Ini.SetValue(Section, Key, input.c_str()), "Classic list with oversized first segment");
    Equal(f.Read(), std::wstring(CONF_LINE_LEN, L'x') + L"\nsecond",
        "skip the full original segment, not its truncated length");
}

static void EmptyEntryDeleteIsolation()
{
    const std::wstring spaces = L" \t" + std::wstring(1, wchar_t(3)) + L" ";
    Fixture added(std::wstring(Key) + L"=kept\r\n");
    Success(added.Ini.AddValue(Section, Key, spaces.c_str()), "add whitespace-only value");
    Equal(added.Read(), L"kept\n", "a new whitespace-only entry has an empty normalized value");
    Success(added.Ini.RemoveValue(Section, Key, spaces.c_str()), "delete normalized empty entry");
    Equal(added.Read(), L"kept", "deleting empty entry must preserve the normal entry");

    Fixture loaded(std::wstring(Key) + L"=\r\n" + Key + L"=kept\r\n");
    Success(loaded.Ini.RemoveValue(Section, Key, spaces.c_str()), "delete already parsed empty entry");
    Equal(loaded.Read(L""), std::wstring(Key) + L"=kept\r\n",
        "only the parsed empty entry is removed");

    Fixture replaced;
    Success(replaced.Ini.SetValue(Section, Key, (spaces + L"\nkept").c_str()),
        "SetValue list containing a whitespace-only item");
    Success(replaced.Ini.RemoveValue(Section, Key, spaces.c_str()), "delete normalized SetValue empty item");
    Equal(replaced.Read(L""), std::wstring(Key) + L"=kept\r\n",
        "SetValue empty item deletion preserves normal same-name item");
}

static void ClassicPlainList()
{
    Fixture f(std::wstring(Key) + L"=old\r\nOther=kept\r\n");
    Success(f.Ini.SetValue(Section, Key, L"first\nsecond\nfirst"), "Classic LF list");
    Equal(f.Read(), L"first\nsecond\nfirst", "Classic LF list in cache");
    f.SaveReload();
    Equal(f.Read(), L"first\nsecond\nfirst", "Classic LF list roundtrip");
    Equal(f.Read(L"Other"), L"kept", "Classic list leaves neighbors alone");
}

static void ClassicEmptyListItems()
{
    Fixture f;
    Success(f.Ini.SetValue(Section, Key, L"\nfirst\n\nsecond\n"), "Classic list with empty segments");
    // GetValue does not prefix a list with an empty item; an internal empty
    // item is present until serialization writes it as an ordinary blank line.
    Equal(f.Read(), L"first\n\nsecond", "existing in-cache empty-segment behavior");
    f.SaveReload();
    Equal(f.Read(), L"first\nsecond", "existing serialized empty-segment behavior");
}

static void ClassicCRLFList()
{
    Fixture f;
    Success(f.Ini.SetValue(Section, Key, L"first\r\nsecond\r\n"), "Classic CRLF-separated list");
    Equal(f.Read(), L"first\nsecond", "CR at an LF-separated value edge is normalized");
    f.SaveReload();
    Equal(f.Read(), L"first\nsecond", "CRLF list roundtrip");
}

static void WriteInteriorAndUnicode()
{
    Fixture f;
    const std::wstring first = L"first" + std::wstring(1, wchar_t(3)) + L"second";
    const std::wstring second = L"\u00A0中文\u3000";
    Success(f.Ini.AddValue(Section, Key, first.c_str()), "add interior control");
    Equal(f.Read(), first, "AddValue must keep interior controls");
    Success(f.Ini.SetValue(Section, Key, second.c_str()), "set non-ASCII edge spaces");
    Equal(f.Read(), second, "SetValue must keep non-ASCII edge spaces");
    f.SaveReload();
    Equal(f.Read(), second, "written Unicode roundtrip");
}

static void EmptySetAndDeleteAll()
{
    Fixture f(L"Test=old\r\nOther=kept\r\n");
    Success(f.Ini.SetValue(Section, L"Test", L""), "empty SetValue");
    Equal(f.Read(L"Test"), L"", "empty SetValue deletes old value");
    Success(f.Ini.AddValue(Section, L"Test", L"one"), "AddValue one");
    Success(f.Ini.AddValue(Section, L"Test", L"two"), "AddValue two");
    Success(f.Ini.RemoveValue(Section, L"Test", L""), "delete all values");
    f.SaveReload();
    Equal(f.Read(L"Test"), L"", "empty delete target still means delete all");
    Equal(f.Read(L"Other"), L"kept", "delete all leaves neighbors alone");
    Success(f.Ini.AddValue(Section, L"Test", L"one"), "add before null delete");
    Success(f.Ini.AddValue(Section, L"Test", L"two"), "add second before null delete");
    Success(f.Ini.RemoveValue(Section, L"Test", nullptr), "null delete-all argument");
    Equal(f.Read(L"Test"), L"", "null delete target still means delete all");
}

static void RejectCRLFAdd()
{
    Fixture f(L"Test=kept\r\n");
    Require(f.Ini.AddValue(Section, L"Bad\nName", L"value") < 0, "LF in add name must fail");
    Require(f.Ini.AddValue(Section, L"Test", L"bad\nvalue") < 0, "LF in add value must fail");
    Require(f.Ini.AddValue(Section, L"Test", L"bad\rvalue") < 0, "CR in add value must fail");
    Equal(f.Read(L"Test"), L"kept", "invalid add leaves old value intact");
}

static void RawSectionCommentsAndOrder()
{
    Fixture f(L"Old=gone\r\n");
    const std::wstring body = L"# test comment\r\nA=one\r\nB=中文\r\nA=two\r\n";
    Success(f.Ini.SetValue(Section, L"", body.c_str()), "replace whole section");
    Equal(f.Read(L""), body, "whole section preserves comments and order");
    f.SaveReload();
    Equal(f.Read(L""), body, "whole section roundtrip");
}

int wmain(int argc, wchar_t** argv)
{
    if (argc != 2 || !fs::is_directory(argv[1])) {
        std::cerr << "Usage: ini_file_test.exe <existing-test-output-directory>\n";
        return 2;
    }
    LARGE_INTEGER runId;
    QueryPerformanceCounter(&runId);
    OutputDirectory = fs::canonical(argv[1]) / (L"ini-values-"
        + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(runId.QuadPart));
    if (!fs::create_directory(OutputDirectory)) {
        std::cerr << "Could not create a fresh fixture directory\n";
        return 2;
    }
    std::cout << "TEST_DATA=" << OutputDirectory.u8string() << '\n';
    const std::vector<std::pair<const char*, std::function<void()>>> cases = {
        {"regression-loaded-U0003-utf8", [] { LoadedTailDelete(Encoding::Utf8Bom); }},
        {"regression-loaded-U0003-utf16le", [] { LoadedTailDelete(Encoding::Utf16LE); }},
        {"regression-loaded-U0003-utf16be", [] { LoadedTailDelete(Encoding::Utf16BE); }},
        {"regression-loaded-value-edges", LoadedValueEdges},
        {"regression-add-value-edges", AddValueEdges},
        {"regression-set-value-edges", SetValueEdges},
        {"regression-classic-controlled-list", ClassicControlledList},
        {"regression-raw-section-control", RawSectionControl},
        {"regression-whitespace-only-value", WhitespaceOnlyParsedValue},
        {"regression-set-truncate-before-trim", TruncateBeforeTrim},
        {"regression-classic-crlf-list", ClassicCRLFList},
        {"regression-empty-entry-delete-isolation", EmptyEntryDeleteIsolation},
        {"compat-load-unicode-interior-whitespace-utf8", [] { NormalLoadPreservation(Encoding::Utf8Bom); }},
        {"compat-load-unicode-interior-whitespace-utf16le", [] { NormalLoadPreservation(Encoding::Utf16LE); }},
        {"compat-load-unicode-interior-whitespace-utf16be", [] { NormalLoadPreservation(Encoding::Utf16BE); }},
        {"compat-add-insert-duplicate-delete", AddInsertAndDuplicateDelete},
        {"compat-add-raw-remove-raw", AddRawThenDeleteRaw},
        {"compat-nonempty-whitespace-delete", NonemptyWhitespaceDelete},
        {"compat-classic-lf-list", ClassicPlainList},
        {"compat-classic-empty-list-items", ClassicEmptyListItems},
        {"compat-classic-oversized-first-segment", LongClassicSegment},
        {"compat-write-interior-unicode", WriteInteriorAndUnicode},
        {"compat-empty-set-delete-all", EmptySetAndDeleteAll},
        {"compat-add-crlf-rejection", RejectCRLFAdd},
        {"compat-raw-section-comments-order", RawSectionCommentsAndOrder},
    };
    int failed = 0;
    for (const auto& test : cases) {
        try {
            test.second();
            std::cout << "PASS " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cout << "FAIL " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << "RESULT total=" << cases.size() << " passed=" << cases.size() - failed
              << " failed=" << failed << " generated_files=" << FileSequence << '\n';
    return failed ? 1 : 0;
}
