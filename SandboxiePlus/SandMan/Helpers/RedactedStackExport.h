#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <QVector>

// Input model for one captured frame. All fields are private diagnostic data
// that must never appear in the redacted output. `symbol` is the resolver's
// display string ("Module.dll!Function+0x<hex>" or "Module.dll+0x<hex>" or
// empty when unresolved). `moduleKey` is the private module identity the
// caller uses to distinguish two modules that happen to share a base name;
// pass the module base address, a stable handle, or an integer, never the
// display name alone.
struct SStackFrame
{
	quint64 address = 0;
	QString symbol;
	QVariant moduleKey;
};

struct SStackCapture
{
	enum ECaptureState { eComplete, ePartial };
	QVector<SStackFrame> frames;
	ECaptureState state = eComplete;
};

// Output model. Only the fields listed on the redaction allow-list are
// populated; every other input field stays inside the capture and does not
// reach the public model.
struct SRedactedFrame
{
	int index = 0;
	int moduleId = 0;			// opaque per-report id, 1-based; 0 = unresolved / no module
	bool moduleResolved = false;
	bool symbolResolved = false;	// true only when the resolver returned a symbol name
	QString line;				// canonical text line, uses only the allowed markers
};

struct SRedactedStack
{
	int version = 1;
	QString captureState;		// "complete" or "partial"
	int frameCount = 0;
	int moduleCount = 0;		// distinct modules referenced in this report
	QVector<SRedactedFrame> frames;

	QJsonObject toJson() const;
	QString toText() const;
};

// Transformer contract:
//  * takes an immutable capture, returns a new object built from the
//    allow-list only, and never mutates the input
//  * two references to the same `moduleKey` share the same opaque id
//    inside one report; different keys never collide, even when the
//    original module name matches
//  * an unresolved frame is marked, never guessed
//  * frame order and capture state are preserved
class CRedactedStackExport
{
public:
	static SRedactedStack transform(const SStackCapture& capture);

	// Write the redacted content atomically via QSaveFile with the direct
	// write fallback disabled. Returns false and fills `error` on any I/O
	// or commit failure; the previous file is left intact and no temporary
	// file survives. `content` must already be the transformed bytes,
	// never a raw diagnostic buffer.
	static bool writeAtomic(const QString& path, const QByteArray& content, QString* error);

	// Fallible transform + write in one call. `wrote` is set only on true.
	static bool exportToFile(const SStackCapture& capture, const QString& path,
		bool asJson, QString* error);
};
