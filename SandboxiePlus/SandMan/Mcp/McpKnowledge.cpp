#include "stdafx.h"
#include "McpKnowledge.h"
#include "McpDocs.h"
// theConf, for the configuration directory
#include "../../MiscHelpers/Common/Settings.h"
#include <QDir>
#include <QRegularExpression>

CMcpKnowledge::CMcpKnowledge()
{
}

//
// A deliberately small front-matter reader rather than a YAML dependency.
//
// What the format actually uses is a flat map of scalars, a few inline lists,
// and one list of inline maps (signals). Pulling in a YAML parser to read that
// would be a new dependency, a new attack surface for content that arrives in
// an archive, and a licence question - for a grammar this narrow. The cost is
// that entries must stay within the subset, which is checked here and reported
// rather than guessed at.
//
static QString Unquote(QString Value)
{
	Value = Value.trimmed();
	if (Value.length() >= 2 && ((Value.startsWith('"') && Value.endsWith('"'))
		|| (Value.startsWith('\'') && Value.endsWith('\''))))
		Value = Value.mid(1, Value.length() - 2);
	return Value.trimmed();
}

static QStringList InlineList(const QString& Value)
{
	QString Inner = Value.trimmed();
	if (Inner.startsWith('[') && Inner.endsWith(']'))
		Inner = Inner.mid(1, Inner.length() - 2);

	QStringList Out;
	// Split on commas that are not inside quotes, so a symptom containing a
	// comma survives.
	QString Current;
	QChar Quote;
	foreach(const QChar& Char, Inner) {
		if (!Quote.isNull()) {
			if (Char == Quote) Quote = QChar();
			else Current += Char;
			continue;
		}
		if (Char == '"' || Char == '\'') { Quote = Char; continue; }
		if (Char == ',') { Out.append(Current.trimmed()); Current.clear(); continue; }
		Current += Char;
	}
	if (!Current.trimmed().isEmpty())
		Out.append(Current.trimmed());

	Out.removeAll(QString());
	return Out;
}

// { msg_code: 2205, status: STATUS_ACCESS_DENIED, name_match: '*\FrameServer*' }
static QJsonObject InlineMap(const QString& Value)
{
	QString Inner = Value.trimmed();
	if (Inner.startsWith('{') && Inner.endsWith('}'))
		Inner = Inner.mid(1, Inner.length() - 2);

	QJsonObject Out;
	foreach(const QString& Pair, InlineList(Inner)) {
		const int Colon = Pair.indexOf(':');
		if (Colon <= 0)
			continue;
		const QString Key = Pair.left(Colon).trimmed();
		const QString Raw = Unquote(Pair.mid(Colon + 1));
		bool bNumber = false;
		const int AsNumber = Raw.toInt(&bNumber);
		if (bNumber)
			Out[Key] = AsNumber;
		else
			Out[Key] = Raw;
	}
	return Out;
}

bool CMcpKnowledge::Parse(const QString& Text, const QString& Source, SEntry* pOut)
{
	// Front matter is the block between the first two --- lines. An entry
	// without one is prose, not knowledge, and is skipped rather than
	// half-read.
	const QStringList Lines = Text.split(QRegularExpression("\r?\n"));
	int Start = -1, End = -1;
	for (int i = 0; i < Lines.count(); i++) {
		if (Lines[i].trimmed() != "---")
			continue;
		if (Start < 0) Start = i;
		else { End = i; break; }
	}
	if (Start < 0 || End < 0)
		return false;

	pOut->Source = Source;
	bool bInSignals = false;

	for (int i = Start + 1; i < End; i++)
	{
		const QString Line = Lines[i];
		const QString Trimmed = Line.trimmed();
		if (Trimmed.isEmpty() || Trimmed.startsWith('#'))
			continue;

		// A "  - { ... }" line continues the signals list.
		if (bInSignals && Trimmed.startsWith("- ")) {
			const QJsonObject Signal = InlineMap(Trimmed.mid(2));
			if (!Signal.isEmpty())
				pOut->Signals.append(Signal);
			continue;
		}

		const int Colon = Trimmed.indexOf(':');
		if (Colon <= 0)
			continue;

		const QString Key = Trimmed.left(Colon).trimmed().toLower();
		const QString Value = Trimmed.mid(Colon + 1).trimmed();
		bInSignals = (Key == "signals");

		if (Key == "id")				pOut->Id = Unquote(Value);
		else if (Key == "title")		pOut->Title = Unquote(Value);
		else if (Key == "symptoms")		pOut->Symptoms = InlineList(Value);
		else if (Key == "diagnosis")	pOut->Diagnosis = Unquote(Value);
		else if (Key == "risk")			pOut->Risk = Unquote(Value);
		else if (Key == "verify")		pOut->Verify = Unquote(Value);
		else if (Key == "script")		pOut->Script = Unquote(Value);
		else if (Key == "ini_changes")	pOut->IniChanges = InlineList(Value);
		else if (Key == "signals" && !Value.isEmpty()) {
			// The inline form, signals: [ {...}, {...} ]
			foreach(const QString& One, InlineList(Value)) {
				const QJsonObject Signal = InlineMap(One);
				if (!Signal.isEmpty())
					pOut->Signals.append(Signal);
			}
			bInSignals = false;
		}
	}

	pOut->Body = Lines.mid(End + 1).join("\n").trimmed();

	// An entry with no id cannot be referenced, cited in a report, or
	// superseded later, so it is not usable knowledge.
	return !pOut->Id.isEmpty();
}

void CMcpKnowledge::Load(CMcpDocs* pDocs)
{
	m_Entries.clear();
	m_Sources.clear();

	QHash<QString, int> ById;

	// Loose directories first, then the pack, because first-wins below means
	// first-listed is the override. An entry dropped in a kb/ folder beats the
	// packed copy of the same id, which is how one gets written and tried
	// before it is packed.
	QStringList Dirs;
	Dirs << theConf->GetConfigDir() + "/kb";
	Dirs << QApplication::applicationDirPath() + "/kb";

	foreach(const QString& DirPath, Dirs) {
		QDir Dir(DirPath);
		if (!Dir.exists())
			continue;
		foreach(const QFileInfo& Info, Dir.entryInfoList(QStringList() << "*.md", QDir::Files, QDir::Name)) {
			QFile File(Info.absoluteFilePath());
			if (!File.open(QFile::ReadOnly | QFile::Text))
				continue;
			const QString Text = QString::fromUtf8(File.readAll());
			File.close();

			SEntry Entry;
			if (!Parse(Text, Info.fileName(), &Entry))
				continue;

			// First directory wins: the configuration copy overrides the
			// installed one, and neither is merged into the other.
			if (ById.contains(Entry.Id.toLower()))
				continue;
			ById.insert(Entry.Id.toLower(), m_Entries.count());
			m_Entries.append(Entry);
		}
		if (Dir.exists())
			m_Sources.append(DirPath);
	}

	// Then the pack, which is where entries normally live: they ship and
	// update through the same channel as the documentation and are reviewable
	// in a pull request rather than appearing on one machine.
	if (pDocs && pDocs->IsOpen()) {
		const QStringList Packed = pDocs->ListKnowledge();
		foreach(const QString& FileName, Packed) {
			const QString Text = pDocs->ReadKnowledge(FileName);
			if (Text.isEmpty())
				continue;
			SEntry Entry;
			if (!Parse(Text, QString("pack:%1").arg(FileName), &Entry))
				continue;
			if (ById.contains(Entry.Id.toLower()))
				continue;
			ById.insert(Entry.Id.toLower(), m_Entries.count());
			m_Entries.append(Entry);
		}
		if (!Packed.isEmpty())
			m_Sources.append(pDocs->GetPackPath());
	}
}

QJsonObject CMcpKnowledge::ToJson(const SEntry& Entry, bool bFull)
{
	QJsonObject Out;
	Out["id"] = Entry.Id;
	if (!Entry.Title.isEmpty())		Out["title"] = Entry.Title;
	if (!Entry.Diagnosis.isEmpty())	Out["diagnosis"] = Entry.Diagnosis;
	Out["symptoms"] = QJsonArray::fromStringList(Entry.Symptoms);

	if (!bFull)
		return Out;

	if (!Entry.Risk.isEmpty())		Out["risk"] = Entry.Risk;
	if (!Entry.Verify.isEmpty())	Out["verify"] = Entry.Verify;
	if (!Entry.Script.isEmpty())	Out["script"] = Entry.Script;
	if (!Entry.IniChanges.isEmpty())Out["iniChanges"] = QJsonArray::fromStringList(Entry.IniChanges);
	if (!Entry.Signals.isEmpty())	Out["signals"] = Entry.Signals;
	if (!Entry.Body.isEmpty())		Out["notes"] = Entry.Body;
	Out["source"] = Entry.Source;
	return Out;
}

QJsonArray CMcpKnowledge::Search(const QString& Query, int Limit) const
{
	QJsonArray Out;
	const QString Needle = Query.trimmed();
	if (Needle.isEmpty())
		return Out;	// an empty query returns nothing, not the whole corpus

	QVector<QPair<int, int>> Ranked;	// score, index
	for (int i = 0; i < m_Entries.count(); i++)
	{
		const SEntry& Entry = m_Entries[i];
		int Score = 0;
		if (Entry.Id.contains(Needle, Qt::CaseInsensitive))			Score += 10;
		if (Entry.Title.contains(Needle, Qt::CaseInsensitive))		Score += 8;
		foreach(const QString& Symptom, Entry.Symptoms)
			if (Symptom.contains(Needle, Qt::CaseInsensitive))		Score += 6;
		if (Entry.Diagnosis.contains(Needle, Qt::CaseInsensitive))	Score += 4;
		if (Entry.Body.contains(Needle, Qt::CaseInsensitive))		Score += 1;
		if (Score)
			Ranked.append(qMakePair(Score, i));
	}

	std::sort(Ranked.begin(), Ranked.end(), [](const QPair<int, int>& A, const QPair<int, int>& B) {
		return A.first > B.first;
	});

	foreach(const auto& Pair, Ranked) {
		if (Limit > 0 && Out.count() >= Limit)
			break;
		Out.append(ToJson(m_Entries[Pair.second], false));
	}
	return Out;
}

QJsonObject CMcpKnowledge::Get(const QString& Id) const
{
	foreach(const SEntry& Entry, m_Entries) {
		if (Entry.Id.compare(Id, Qt::CaseInsensitive) == 0)
			return ToJson(Entry, true);
	}
	return QJsonObject();
}

//
// Scoring. Every signal that matches adds; nothing subtracts.
//
// Penalising a signal that did not match would be the obvious thing and it is
// wrong here: the inputs are partial by nature. A trace captures what was
// logged, not everything that happened, and a user who started diagnosing late
// has no early messages at all. Under those conditions a missing signal is
// usually missing evidence rather than evidence of absence, and subtracting for
// it buries the right entry beneath a vaguer one.
//
int CMcpKnowledge::ScoreSignals(const SEntry& Entry, const QList<int>& Codes,
	const QStringList& Denied, const QStringList& Settings, QStringList* pWhy) const
{
	int Score = 0;
	foreach(const QJsonValue& Value, Entry.Signals)
	{
		const QJsonObject Signal = Value.toObject();

		if (Signal.contains("msg_code")) {
			const int Code = Signal.value("msg_code").toInt();
			if (Codes.contains(Code)) {
				Score += 10;
				if (pWhy) pWhy->append(QObject::tr("message SBIE%1 was seen").arg(Code, 4, 10, QChar('0')));
			}
		}

		if (Signal.contains("name_match")) {
			const QString Pattern = Signal.value("name_match").toString();
			if (!Pattern.isEmpty()) {
				const QRegularExpression Rx(QRegularExpression::wildcardToRegularExpression(Pattern),
					QRegularExpression::CaseInsensitiveOption);
				foreach(const QString& Path, Denied) {
					if (Rx.match(Path).hasMatch()) {
						Score += 8;
						if (pWhy) pWhy->append(QObject::tr("a blocked resource matched %1").arg(Pattern));
						break;
					}
				}
			}
		}

		// ini_absent is the signal that identifies a missing template, which is
		// the single most common real cause - so it is worth as much as a
		// message code rather than being treated as weak evidence.
		if (Signal.contains("ini_absent")) {
			const QString Want = Signal.value("ini_absent").toString();
			bool bFound = false;
			foreach(const QString& Line, Settings)
				if (Line.compare(Want, Qt::CaseInsensitive) == 0) { bFound = true; break; }
			if (!bFound) {
				Score += 10;
				if (pWhy) pWhy->append(QObject::tr("%1 is not set on the sandbox").arg(Want));
			}
		}

		if (Signal.contains("ini_present")) {
			const QString Want = Signal.value("ini_present").toString();
			foreach(const QString& Line, Settings) {
				if (Line.compare(Want, Qt::CaseInsensitive) == 0) {
					Score += 10;
					if (pWhy) pWhy->append(QObject::tr("%1 is set on the sandbox").arg(Want));
					break;
				}
			}
		}
	}
	return Score;
}

QJsonArray CMcpKnowledge::Match(const QList<int>& Codes, const QStringList& Denied,
	const QStringList& Settings, int Limit) const
{
	QVector<QPair<int, int>> Ranked;
	QVector<QStringList> Whys(m_Entries.count());

	for (int i = 0; i < m_Entries.count(); i++)
	{
		if (m_Entries[i].Signals.isEmpty())
			continue;	// nothing to match on; it can still be found by search
		QStringList Why;
		const int Score = ScoreSignals(m_Entries[i], Codes, Denied, Settings, &Why);
		if (Score <= 0)
			continue;	// explains nothing, so it is not a candidate
		Ranked.append(qMakePair(Score, i));
		Whys[i] = Why;
	}

	std::sort(Ranked.begin(), Ranked.end(), [](const QPair<int, int>& A, const QPair<int, int>& B) {
		return A.first > B.first;
	});

	QJsonArray Out;
	foreach(const auto& Pair, Ranked) {
		if (Limit > 0 && Out.count() >= Limit)
			break;
		QJsonObject Entry = ToJson(m_Entries[Pair.second], true);
		Entry["score"] = Pair.first;
		// What matched, so the agent can weigh the suggestion instead of
		// taking the ranking on faith - and so a wrong match is visibly wrong
		// rather than merely top of a list.
		Entry["matchedBecause"] = QJsonArray::fromStringList(Whys[Pair.second]);
		Out.append(Entry);
	}
	return Out;
}
