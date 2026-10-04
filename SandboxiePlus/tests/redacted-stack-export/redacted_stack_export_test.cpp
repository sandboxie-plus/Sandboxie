#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QTemporaryDir>
#include <QThread>
#include <QVector>
#include <cstdio>
#include <cstdlib>

#include "Helpers/RedactedStackExport.h"

#define CHECK(condition) do { if (!(condition)) { \
	std::fprintf(stdout, "FAIL at line %d: %s\n", __LINE__, #condition); \
	std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); \
	std::fflush(stdout); std::fflush(stderr); std::exit(1); \
} } while (0)

//---------------------------------------------------------------------------
// Fixture: private diagnostic tokens that must never leak into the share view.
//---------------------------------------------------------------------------

// Synthetic private strings. If any of these ever appears in the redacted
// output, in the JSON or in the error message the test fails. They are the
// only "sensitive" data in this file.
static const QStringList kPrivateTokens = {
	QStringLiteral("C:\\Users\\alice\\projeto-interno"),
	QStringLiteral("C:/Users/alice/projeto-interno"),
	QStringLiteral("bridge.dll"),
	QStringLiteral("BridgeDispatchRequest"),
	QStringLiteral("SbieDll.dll"),
	QStringLiteral("Sbie_Init"),
	QStringLiteral("app.exe"),
	QStringLiteral("RunTask"),
	QStringLiteral("kernel32.dll"),
	QStringLiteral("CreateFileW"),
	QStringLiteral("HelloDlgProc"),
	QStringLiteral("0x7ffde1a04b30"),
	QStringLiteral("SecretPath\\file\\with spaces"),
	QStringLiteral("caixa-secreta"),
	QStringLiteral("alice@example.internal"),
};

static bool leaksPrivate(const QString& s)
{
	foreach (const QString& token, kPrivateTokens) {
		if (s.contains(token, Qt::CaseInsensitive)) return true;
	}
	// Hex byte sequences are also a leak signal for absolute addresses.
	// The frame index is decimal, the module label uses "%1" (int), so
	// there should be no "0x" in the redacted output at all.
	return s.contains(QStringLiteral("0x"), Qt::CaseInsensitive);
}

static SStackCapture makeCapture(SStackCapture::ECaptureState state = SStackCapture::eComplete)
{
	SStackCapture c;
	c.state = state;
	SStackFrame f0; f0.address = 0x7ffde1a04b30ULL; f0.symbol = QStringLiteral("bridge.dll!BridgeDispatchRequest+0x2a"); f0.moduleKey = QStringLiteral("bridge.dll"); c.frames.append(f0);
	SStackFrame f1; f1.address = 0x7ffde1a04a10ULL; f1.symbol = QStringLiteral("bridge.dll!Helper+0x10"); f1.moduleKey = QStringLiteral("bridge.dll"); c.frames.append(f1);
	SStackFrame f2; f2.address = 0x7fffab001000ULL; f2.symbol = QStringLiteral("kernel32.dll!CreateFileW+0x100"); f2.moduleKey = QStringLiteral("kernel32.dll"); c.frames.append(f2);
	SStackFrame f3; f3.address = 0x7fffde000000ULL; f3.symbol = QStringLiteral("SbieDll.dll+0x1234"); f3.moduleKey = QStringLiteral("SbieDll.dll"); c.frames.append(f3);
	SStackFrame f4; f4.address = 0x7ffdeadbeef0ULL; f4.symbol = QString(); f4.moduleKey = QVariant(); c.frames.append(f4);
	return c;
}

//---------------------------------------------------------------------------

static void AllowListPrivacy()
{
	SStackCapture cap = makeCapture();
	SRedactedStack red = CRedactedStackExport::transform(cap);
	QString text = red.toText();
	QString json = QString::fromUtf8(QJsonDocument(red.toJson()).toJson());
	CHECK(!leaksPrivate(text));
	CHECK(!leaksPrivate(json));

	QString error;
	QTemporaryDir dir; CHECK(dir.isValid());
	QString path = dir.path() + QStringLiteral("/out.json");
	CHECK(CRedactedStackExport::exportToFile(cap, path, true, &error));
	QFile file(path); CHECK(file.open(QIODevice::ReadOnly));
	QString onDisk = QString::fromUtf8(file.readAll());
	CHECK(!leaksPrivate(onDisk));
	CHECK(!leaksPrivate(error));

	// Unknown/free-form fields never make it in.
	SStackCapture withNoise = cap;
	SStackFrame extra; extra.address = 0xdeadbeef; extra.symbol = QStringLiteral("password=hunter2 bridge.dll!DispatchRequest"); extra.moduleKey = QStringLiteral("bridge.dll");
	withNoise.frames.append(extra);
	SRedactedStack redNoise = CRedactedStackExport::transform(withNoise);
	QString textNoise = redNoise.toText();
	CHECK(!textNoise.contains(QStringLiteral("password")));
	CHECK(!textNoise.contains(QStringLiteral("hunter2")));
}

static void AllowListFields()
{
	SStackCapture cap = makeCapture();
	QJsonObject json = CRedactedStackExport::transform(cap).toJson();
	QSet<QString> topKeys;
	foreach (const QString& k, json.keys()) topKeys.insert(k);
	CHECK(topKeys == (QSet<QString>() << QStringLiteral("format") << QStringLiteral("version") << QStringLiteral("captureState") << QStringLiteral("frameCount") << QStringLiteral("moduleCount") << QStringLiteral("frames") << QStringLiteral("note")));
	QJsonArray frames = json.value(QStringLiteral("frames")).toArray();
	CHECK(!frames.isEmpty());
	QSet<QString> frameKeys;
	foreach (const QString& k, frames[0].toObject().keys()) frameKeys.insert(k);
	CHECK(frameKeys == (QSet<QString>() << QStringLiteral("index") << QStringLiteral("moduleId") << QStringLiteral("moduleResolved") << QStringLiteral("symbolResolved") << QStringLiteral("line")));
	CHECK(json.value(QStringLiteral("format")).toString() == QStringLiteral("sandboxie-stack-export"));
	CHECK(json.value(QStringLiteral("version")).toInt() == 1);
}

static void OpaqueIdsPerReport()
{
	SStackCapture cap = makeCapture();
	SRedactedStack a = CRedactedStackExport::transform(cap);
	SRedactedStack b = CRedactedStackExport::transform(cap);
	// Order-of-appearance ids inside one report.
	QSet<int> aIds, bIds;
	foreach (const SRedactedFrame& f, a.frames) if (f.moduleId) aIds.insert(f.moduleId);
	foreach (const SRedactedFrame& f, b.frames) if (f.moduleId) bIds.insert(f.moduleId);
	CHECK(aIds == (QSet<int>() << 1 << 2 << 3));
	CHECK(bIds == aIds);
	// Frames of the same module share the id inside a report.
	CHECK(a.frames[0].moduleId == a.frames[1].moduleId);
	CHECK(a.frames[0].moduleId != a.frames[2].moduleId);
	CHECK(a.moduleCount == 3);
}

static void SameNameDistinctModules()
{
	// Same visible name, different private module identities: the transformer
	// must not merge them.
	SStackCapture cap;
	SStackFrame a; a.address = 0x100; a.symbol = QStringLiteral("bridge.dll!DispatchRequest+0x1"); a.moduleKey = 0x10000000ULL; cap.frames.append(a);
	SStackFrame b; b.address = 0x200; b.symbol = QStringLiteral("bridge.dll!AnotherFn+0x2"); b.moduleKey = 0x20000000ULL; cap.frames.append(b);
	SStackFrame c; c.address = 0x300; c.symbol = QStringLiteral("bridge.dll!AnotherFn+0x3"); c.moduleKey = 0x10000000ULL; cap.frames.append(c);
	SRedactedStack red = CRedactedStackExport::transform(cap);
	CHECK(red.frames[0].moduleId == 1);
	CHECK(red.frames[1].moduleId == 2);
	CHECK(red.frames[2].moduleId == 1);
	CHECK(red.moduleCount == 2);
}

static void ModuleNameShared()
{
	// If the caller passes the module name as the key (SandMan integration),
	// same-name frames collide by design. That behaviour is documented.
	SStackCapture cap;
	SStackFrame a; a.address = 0x100; a.symbol = QStringLiteral("bridge.dll!A+0x1"); a.moduleKey = QStringLiteral("bridge.dll"); cap.frames.append(a);
	SStackFrame b; b.address = 0x200; b.symbol = QStringLiteral("bridge.dll!B+0x2"); b.moduleKey = QStringLiteral("bridge.dll"); cap.frames.append(b);
	SRedactedStack red = CRedactedStackExport::transform(cap);
	CHECK(red.frames[0].moduleId == red.frames[1].moduleId);
	CHECK(red.moduleCount == 1);
}

static void FrameOrderPreserved()
{
	SStackCapture cap = makeCapture();
	SRedactedStack red = CRedactedStackExport::transform(cap);
	CHECK(red.frameCount == cap.frames.size());
	for (int i = 0; i < red.frames.size(); ++i)
		CHECK(red.frames[i].index == i);
}

static void UnresolvedFrameMarked()
{
	SStackCapture cap = makeCapture();
	SRedactedStack red = CRedactedStackExport::transform(cap);
	const SRedactedFrame& last = red.frames.last();
	CHECK(!last.moduleResolved && !last.symbolResolved);
	CHECK(last.moduleId == 0);
	CHECK(last.line == QStringLiteral("[unresolved]"));
	// A module-only symbol (no "!") is not resolved but the module is.
	const SRedactedFrame& mod = red.frames[3];
	CHECK(mod.moduleResolved && !mod.symbolResolved);
	CHECK(mod.line.contains(QStringLiteral("!")));
	CHECK(mod.line.endsWith(QStringLiteral("[unresolved]")));
}

static void PartialCapturePreserved()
{
	SStackCapture cap = makeCapture(SStackCapture::ePartial);
	SRedactedStack red = CRedactedStackExport::transform(cap);
	CHECK(red.captureState == QStringLiteral("partial"));
	CHECK(red.toText().contains(QStringLiteral("partial")));
	CHECK(red.toJson().value(QStringLiteral("captureState")).toString() == QStringLiteral("partial"));
	SRedactedStack complete = CRedactedStackExport::transform(makeCapture(SStackCapture::eComplete));
	CHECK(complete.captureState == QStringLiteral("complete"));
}

static void UnicodeAndPunctuation()
{
	SStackCapture cap;
	SStackFrame a; a.address = 0x100; a.symbol = QStringLiteral("módulo-ção.dll!Função<T>+0xFF");
	// Two different Unicode keys, then the same as key 1 again.
	a.moduleKey = QStringLiteral("módulo-ção.dll@base1");
	cap.frames.append(a);
	SStackFrame b; b.address = 0x200; b.symbol = QStringLiteral("模块.dll!調用+0x1");
	b.moduleKey = QStringLiteral("模块.dll@base1");
	cap.frames.append(b);
	SStackFrame c = a; c.address = 0x300;
	cap.frames.append(c);
	SRedactedStack red = CRedactedStackExport::transform(cap);
	QString text = red.toText();
	CHECK(!text.contains(QStringLiteral("módulo")));
	CHECK(!text.contains(QStringLiteral("Função")));
	CHECK(!text.contains(QStringLiteral("模块")));
	CHECK(red.frames[0].moduleId == red.frames[2].moduleId);
	CHECK(red.frames[0].moduleId != red.frames[1].moduleId);
}

static void TextAndJsonFidelity()
{
	SStackCapture cap = makeCapture();
	SRedactedStack red = CRedactedStackExport::transform(cap);
	QString text = red.toText();
	int lines = text.count(QLatin1Char('\n'));
	CHECK(lines == red.frameCount + 1);	// header + one line per frame

	// JSON roundtrip: writing then parsing must yield the same object.
	QByteArray bytes = QJsonDocument(red.toJson()).toJson(QJsonDocument::Indented);
	QJsonParseError err{};
	QJsonDocument parsed = QJsonDocument::fromJson(bytes, &err);
	CHECK(err.error == QJsonParseError::NoError);
	CHECK(parsed.object().value(QStringLiteral("frameCount")).toInt() == red.frameCount);
	CHECK(parsed.object().value(QStringLiteral("frames")).toArray().size() == red.frameCount);
}

//---------------------------------------------------------------------------

static void SaveAtomicHappy()
{
	QTemporaryDir dir; CHECK(dir.isValid());
	QString path = dir.path() + QStringLiteral("/happy.json");
	SStackCapture cap = makeCapture();
	QString error;
	CHECK(CRedactedStackExport::exportToFile(cap, path, true, &error) && error.isEmpty());
	QFile file(path); CHECK(file.open(QIODevice::ReadOnly));
	QString onDisk = QString::fromUtf8(file.readAll());
	CHECK(!leaksPrivate(onDisk));
	// One file, no temporary siblings.
	CHECK(QDir(dir.path()).entryList(QDir::Files) == (QStringList() << QStringLiteral("happy.json")));
}

static void SaveOpenFailure()
{
	QTemporaryDir dir; CHECK(dir.isValid());
	// Target sits inside a directory that does not exist: open fails.
	QString path = dir.path() + QStringLiteral("/missing-dir/out.json");
	SStackCapture cap = makeCapture();
	QString error;
	CHECK(!CRedactedStackExport::exportToFile(cap, path, true, &error) && !error.isEmpty());
	CHECK(!leaksPrivate(error));
	CHECK(!QFile::exists(path));
	CHECK(QDir(dir.path()).entryList(QDir::Files).isEmpty());
}

static void SaveCommitFailure()
{
	QTemporaryDir dir; CHECK(dir.isValid());
	QString path = dir.path() + QStringLiteral("/commit.json");
	// Occupy the destination path with a directory so QSaveFile::commit()
	// (rename over the file) fails; directWriteFallback is off, so it must
	// not sneak past the failure by writing directly.
	CHECK(QDir().mkpath(path));
	SStackCapture cap = makeCapture();
	QString error;
	CHECK(!CRedactedStackExport::exportToFile(cap, path, true, &error) && !error.isEmpty());
	CHECK(!leaksPrivate(error));
	CHECK(QDir(path).exists());	// directory intact
	CHECK(QDir(path).entryList(QDir::Files).isEmpty());	// no rogue file inside
}

static void SaveExistingFile()
{
	QTemporaryDir dir; CHECK(dir.isValid());
	QString path = dir.path() + QStringLiteral("/prev.json");
	{
		QFile prev(path);
		CHECK(prev.open(QIODevice::WriteOnly));
		prev.write("previous content");
	}
	SStackCapture cap = makeCapture();
	QString error;
	CHECK(CRedactedStackExport::exportToFile(cap, path, true, &error));
	QFile file(path); CHECK(file.open(QIODevice::ReadOnly));
	QString onDisk = QString::fromUtf8(file.readAll());
	CHECK(!onDisk.contains(QStringLiteral("previous content")));
	CHECK(onDisk.contains(QStringLiteral("sandboxie-stack-export")));
	CHECK(QDir(dir.path()).entryList(QDir::Files) == (QStringList() << QStringLiteral("prev.json")));
}

static void SaveDoesNotTouchInput()
{
	SStackCapture cap = makeCapture();
	SStackCapture before = cap;
	QTemporaryDir dir; CHECK(dir.isValid());
	QString error;
	CHECK(CRedactedStackExport::exportToFile(cap, dir.path() + QStringLiteral("/x.json"), true, &error));
	CHECK(cap.frames.size() == before.frames.size());
	for (int i = 0; i < cap.frames.size(); ++i) {
		CHECK(cap.frames[i].address == before.frames[i].address);
		CHECK(cap.frames[i].symbol == before.frames[i].symbol);
		CHECK(cap.frames[i].moduleKey == before.frames[i].moduleKey);
	}
	CHECK(cap.state == before.state);
}

static void ConcurrentTransformsDoNotShareIds()
{
	// Two exports of two distinct captures must not share the id space.
	// Even if a runs first and reserves id 1, running b with a different
	// module set has to start again from 1 inside its own report.
	SStackCapture a; { SStackFrame f; f.address = 1; f.symbol = QStringLiteral("A!x+0"); f.moduleKey = QStringLiteral("A"); a.frames.append(f); }
	SStackCapture b; { SStackFrame f; f.address = 2; f.symbol = QStringLiteral("B!x+0"); f.moduleKey = QStringLiteral("B"); b.frames.append(f); }
	SRedactedStack ra = CRedactedStackExport::transform(a);
	SRedactedStack rb = CRedactedStackExport::transform(b);
	CHECK(ra.frames[0].moduleId == 1 && rb.frames[0].moduleId == 1);
}

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	QMap<QString, void(*)()> cases = {
		{"allow-list-privacy", AllowListPrivacy},
		{"allow-list-fields", AllowListFields},
		{"opaque-ids-per-report", OpaqueIdsPerReport},
		{"same-name-distinct-modules", SameNameDistinctModules},
		{"module-name-shared", ModuleNameShared},
		{"frame-order-preserved", FrameOrderPreserved},
		{"unresolved-frame-marked", UnresolvedFrameMarked},
		{"partial-capture-preserved", PartialCapturePreserved},
		{"unicode-and-punctuation", UnicodeAndPunctuation},
		{"text-and-json-fidelity", TextAndJsonFidelity},
		{"save-atomic-happy", SaveAtomicHappy},
		{"save-open-failure", SaveOpenFailure},
		{"save-commit-failure", SaveCommitFailure},
		{"save-existing-file", SaveExistingFile},
		{"save-does-not-touch-input", SaveDoesNotTouchInput},
		{"concurrent-transforms-do-not-share-ids", ConcurrentTransformsDoNotShareIds},
	};
	if (argc != 2 || !cases.contains(QString::fromUtf8(argv[1]))) return 2;
	cases.value(QString::fromUtf8(argv[1]))();
	std::printf("PASS: %s\n", argv[1]);
	return 0;
}
