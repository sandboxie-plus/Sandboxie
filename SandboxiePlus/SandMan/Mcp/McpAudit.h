#pragma once
#include <QFile>
#include <QJsonObject>
#include <QString>

//
// CMcpAudit - what the agent interface did, in a file the user can read later.
//
// The message log is for the person watching now; this is for the person asking
// afterwards what happened. It survives a restart, it is append-only, and it is
// one JSON object per line so that it is both greppable by a human and parseable
// by whatever eventually distils these into fix scripts.
//
// What never goes in: the pairing code. It is a credential for the length of its
// life, and a log is exactly the wrong place for one - an audit trail that leaks
// the secret it exists to protect is worse than none. The log records that a
// code was issued, not what it was.
//
class CMcpAudit
{
public:
	CMcpAudit();
	~CMcpAudit();

	// Opens the log in the configuration directory, rotating it first if it has
	// grown past the cap. Returns false if it could not be opened, in which case
	// Record is a no-op rather than an error - losing the audit trail must not
	// take the feature down with it.
	bool			Open();
	void			Close();
	bool			IsOpen() const		{ return m_File.isOpen(); }
	QString			GetPath() const		{ return m_File.fileName(); }

	// Category is a short stable slug - "session", "pair", "tool", "refused" -
	// so the file can be filtered without parsing prose.
	void			Record(const QString& Category, const QString& Message, const QJsonObject& Data = QJsonObject());

private:
	void			Rotate(const QString& Path);

	QFile			m_File;
};
