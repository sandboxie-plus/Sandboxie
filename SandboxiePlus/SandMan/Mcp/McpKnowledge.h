#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

class CMcpDocs;

//
// CMcpKnowledge - the observation-to-cause mapping.
//
// The settings schema says what a setting IS. The reference documentation says
// what it DOES. Neither answers the question a user actually arrives with,
// which is "this is broken, why" - and without a third thing an agent fills
// that gap by guessing plausibly, which is the failure mode this whole feature
// exists to avoid.
//
// A knowledge-base entry is curated prose with a machine-readable head:
//
//   id          stable identifier, referenced by reports and by other entries
//   symptoms    how a user would describe it, in their words not ours
//   signals     what the machine would show - message codes, denied paths,
//               a template that is absent. This is the part that turns a trace
//               dump into a ranked shortlist instead of a narrative
//   diagnosis   why it happens
//   fix         the settings that resolve it, and the script that applies them
//   risk        what the fix costs, because every fix costs something
//   verify      how to know it worked
//
// Entries live in a kb/ tree inside the documentation pack, so they ship and
// update through the same channel as the documentation and are reviewable in a
// pull request. A loose kb/ directory beside the pack overrides it, which is
// how an entry is written and tried before it is packed.
//
// Matching is deliberately conservative. A signal that does not match counts
// against nothing - entries are ranked by what they explain, never by what they
// fail to - and an entry with no matching signal at all is not returned. An
// over-eager knowledge base is worse than none: it produces confident wrong
// answers, which cost more than silence.
//
class CMcpKnowledge
{
public:
	CMcpKnowledge();

	// Reads kb/ from the mounted pack, plus any loose kb/ directory, which
	// wins on a clash by id. Safe to call with a NULL or unopened pack: the
	// knowledge base is then simply empty, which the tools report plainly.
	void			Load(CMcpDocs* pDocs);
	int				GetCount() const		{ return m_Entries.count(); }
	QStringList		GetSources() const		{ return m_Sources; }

	// Free text over ids, titles, symptoms and diagnoses. Shipped content, so
	// this needs no authorisation.
	QJsonArray		Search(const QString& Query, int Limit) const;

	// One entry, whole. Empty object when the id is unknown.
	QJsonObject		Get(const QString& Id) const;

	// The part that needs the machine: score every entry's signals against
	// what was actually observed and return those that explain something,
	// best first. Observed codes are SBIE message numbers; Denied are the
	// resource paths from the trace; Settings is the box's configuration, for
	// the ini_present / ini_absent signals.
	QJsonArray		Match(const QList<int>& Codes, const QStringList& Denied,
						const QStringList& Settings, int Limit) const;

private:
	struct SEntry
	{
		QString		Id;
		QString		Title;
		QStringList	Symptoms;
		QString		Diagnosis;
		QString		Risk;
		QString		Verify;
		QString		Script;			// the .js that applies it, if any
		QStringList	IniChanges;		// "Setting=Value" lines
		QJsonArray	Signals;
		QString		Body;			// the prose after the front matter
		QString		Source;
	};

	static bool		Parse(const QString& Text, const QString& Source, SEntry* pOut);
	static QJsonObject ToJson(const SEntry& Entry, bool bFull);
	int				ScoreSignals(const SEntry& Entry, const QList<int>& Codes,
						const QStringList& Denied, const QStringList& Settings,
						QStringList* pWhy) const;

	QVector<SEntry>	m_Entries;
	QStringList		m_Sources;
};
