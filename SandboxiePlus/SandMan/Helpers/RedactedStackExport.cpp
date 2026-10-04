#include "RedactedStackExport.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QSaveFile>

//---------------------------------------------------------------------------
// SRedactedStack
//---------------------------------------------------------------------------

QJsonObject SRedactedStack::toJson() const
{
	QJsonObject json;
	json["format"] = "sandboxie-stack-export";
	json["version"] = version;
	json["captureState"] = captureState;
	json["frameCount"] = frameCount;
	json["moduleCount"] = moduleCount;
	QJsonArray array;
	foreach (const SRedactedFrame& frame, frames) {
		QJsonObject entry;
		entry["index"] = frame.index;
		entry["moduleId"] = frame.moduleId;
		entry["moduleResolved"] = frame.moduleResolved;
		entry["symbolResolved"] = frame.symbolResolved;
		entry["line"] = frame.line;
		array.append(entry);
	}
	json["frames"] = array;
	json["note"] = QStringLiteral("Module identifiers are opaque and valid only inside this report. "
		"Paths, module and symbol names, absolute addresses and offsets are omitted by design. "
		"Preserving frame order and grouping does not prevent correlation between reports.");
	return json;
}

QString SRedactedStack::toText() const
{
	QString out;
	out += QStringLiteral("# Sandboxie-Plus redacted stack export v%1 (%2, %3 frames, %4 modules)\n")
		.arg(version).arg(captureState).arg(frameCount).arg(moduleCount);
	foreach (const SRedactedFrame& frame, frames)
		out += QString::number(frame.index) + QLatin1Char('\t') + frame.line + QLatin1Char('\n');
	return out;
}

//---------------------------------------------------------------------------
// Redaction contract
//---------------------------------------------------------------------------

// Whitelist markers. They must be recognisable as markers, never as an
// attribution to a real product.
static const QLatin1String kModuleMarker("[module-%1]");
static const QLatin1String kSymbolRedacted("[symbol redacted]");
static const QLatin1String kUnresolvedFrame("[unresolved]");
static const QLatin1String kUnresolvedModule("[unresolved module]");

static QString moduleLabel(int id)
{
	return QString(kModuleMarker).arg(id, 2, 10, QLatin1Char('0'));
}

static QString frameLine(int moduleId, bool moduleResolved, bool symbolResolved)
{
	if (!moduleResolved && !symbolResolved)
		return kUnresolvedFrame;
	if (!moduleResolved)
		return QString(kUnresolvedModule) + QLatin1Char('!') + kSymbolRedacted;
	QString line = moduleLabel(moduleId);
	line += QLatin1Char('!');
	line += symbolResolved ? QString(kSymbolRedacted) : QString(kUnresolvedFrame);
	return line;
}

static bool hasModuleInSymbol(const QString& symbol)
{
	// The resolver emits "Module!Function..." or "Module+0x..." when the
	// module is known and the empty string when it is not.
	return symbol.contains(QLatin1Char('!')) || symbol.contains(QLatin1Char('+'));
}

static bool hasSymbolName(const QString& symbol)
{
	// A resolved symbol always includes the "!Function" delimiter.
	return symbol.contains(QLatin1Char('!'));
}

//---------------------------------------------------------------------------
// CRedactedStackExport
//---------------------------------------------------------------------------

SRedactedStack CRedactedStackExport::transform(const SStackCapture& capture)
{
	SRedactedStack out;
	out.captureState = capture.state == SStackCapture::ePartial
		? QStringLiteral("partial") : QStringLiteral("complete");
	out.frameCount = capture.frames.size();

	QMap<QString, int> moduleIds;	// QMap keyed on a canonical string so any moduleKey type works
	int nextModule = 0;
	out.frames.reserve(capture.frames.size());
	int index = 0;
	foreach (const SStackFrame& frame, capture.frames) {
		SRedactedFrame red;
		red.index = index++;
		red.moduleResolved = frame.moduleKey.isValid() && !frame.moduleKey.isNull() && hasModuleInSymbol(frame.symbol);
		red.symbolResolved = red.moduleResolved && hasSymbolName(frame.symbol);
		if (red.moduleResolved) {
			// Canonical key: type-tagged so a QString and the same string
			// wrapped in another variant type never collide.
			QString key = QString::number(int(frame.moduleKey.type())) + QLatin1Char(':') + frame.moduleKey.toString();
			QMap<QString, int>::const_iterator it = moduleIds.constFind(key);
			if (it == moduleIds.constEnd()) {
				++nextModule;
				moduleIds.insert(key, nextModule);
				red.moduleId = nextModule;
			} else {
				red.moduleId = it.value();
			}
		}
		red.line = frameLine(red.moduleId, red.moduleResolved, red.symbolResolved);
		out.frames.append(red);
	}
	out.moduleCount = nextModule;
	return out;
}

bool CRedactedStackExport::writeAtomic(const QString& path, const QByteArray& content, QString* error)
{
	QSaveFile file(path);
	file.setDirectWriteFallback(false);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		if (error) *error = QStringLiteral("cannot open the export destination");
		return false;
	}
	if (file.write(content) != content.size()) {
		if (error) *error = QStringLiteral("cannot write the redacted export");
		file.cancelWriting();
		return false;
	}
	if (!file.commit()) {
		if (error) *error = QStringLiteral("cannot commit the redacted export");
		return false;
	}
	return true;
}

bool CRedactedStackExport::exportToFile(const SStackCapture& capture, const QString& path,
	bool asJson, QString* error)
{
	SRedactedStack red = transform(capture);
	QByteArray bytes;
	if (asJson)
		bytes = QJsonDocument(red.toJson()).toJson(QJsonDocument::Indented);
	else
		bytes = red.toText().toUtf8();
	return writeAtomic(path, bytes, error);
}
