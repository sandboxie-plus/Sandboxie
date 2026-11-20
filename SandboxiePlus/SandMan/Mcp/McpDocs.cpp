#include "stdafx.h"
#include "McpDocs.h"
#include <QDir>
#include "../SandMan.h"
#include <QJsonDocument>
#include <QFileInfo>

// A page is text; this is a generous ceiling that still bounds a single reply.
static const int kMaxPageBytes = 256 * 1024;

CMcpDocs::CMcpDocs()
{
	m_pFS = NULL;
	m_bOpen = false;
}

CMcpDocs::~CMcpDocs()
{
	Close();
}

void CMcpDocs::Close()
{
	if (m_pFS) {
		m_pFS->Close();
		delete m_pFS;
		m_pFS = NULL;
	}
	m_Index = QJsonObject();
	m_ByPath.clear();
	m_PackPath.clear();
	m_bOpen = false;
}

bool CMcpDocs::Open()
{
	Close();

	// Same precedence as CScriptManager::GetIssueDir: a pack in the
	// configuration directory wins when it is at least as new as the installed
	// one, so an update can arrive without reinstalling.
	const QString Installed = QApplication::applicationDirPath() + "/docs.7z";
	const QString Updated = theConf->GetConfigDir() + "/docs.7z";

	const QFileInfo InstalledInfo(Installed);
	const QFileInfo UpdatedInfo(Updated);

	if (UpdatedInfo.exists() && (!InstalledInfo.exists() || UpdatedInfo.lastModified() >= InstalledInfo.lastModified())) {
		if (Open(Updated))
			return true;
	}
	if (InstalledInfo.exists())
		return Open(Installed);
	return false;
}

bool CMcpDocs::Open(const QString& Path)
{
	Close();

	// The scheme has to be unique among mounted archives, or two handlers fight
	// over the same prefix.
	m_pFS = new C7zFileEngineHandler("sbiedocs");
	if (!m_pFS->Open(Path)) {
		delete m_pFS;
		m_pFS = NULL;
		return false;
	}

	QFile IndexFile(m_pFS->Prefix() + "/index.json");
	if (!IndexFile.open(QFile::ReadOnly)) {
		// A pack without its index is not usable as a reference: every lookup
		// here is index-driven, and scanning 600 pages per query is not an
		// answer. Refuse rather than degrade silently.
		m_pFS->Close();
		delete m_pFS;
		m_pFS = NULL;
		return false;
	}

	const QByteArray Raw = IndexFile.readAll();
	IndexFile.close();

	QJsonParseError Error;
	const QJsonDocument Doc = QJsonDocument::fromJson(Raw, &Error);
	if (Error.error != QJsonParseError::NoError || !Doc.isObject()) {
		m_pFS->Close();
		delete m_pFS;
		m_pFS = NULL;
		return false;
	}

	m_Index = Doc.object();
	foreach(const QJsonValue& Value, m_Index.value("pages").toArray()) {
		const QJsonObject Page = Value.toObject();
		m_ByPath.insert(Page.value("path").toString(), Page);
	}

	m_PackPath = Path;
	m_bOpen = true;
	return true;
}

int CMcpDocs::GetPageCount() const
{
	return m_ByPath.count();
}

bool CMcpDocs::HasPage(const QString& Path) const
{
	return m_ByPath.contains(Path);
}

QJsonObject CMcpDocs::PageEntry(const QString& Path) const
{
	return m_ByPath.value(Path);
}

QStringList CMcpDocs::ListKnowledge() const
{
	if (!m_bOpen)
		return QStringList();
	QDir Dir(m_pFS->Prefix() + "/kb");
	if (!Dir.exists())
		return QStringList();
	return Dir.entryList(QStringList() << "*.md", QDir::Files, QDir::Name);
}

QString CMcpDocs::ReadKnowledge(const QString& FileName) const
{
	if (!m_bOpen)
		return QString();

	// A bare file name, nothing else. Rejecting separators outright is
	// cheaper to be sure of than sanitising them.
	if (FileName.contains(QChar('/')) || FileName.contains(QChar('\\'))
		|| FileName.contains(QString("..")) || !FileName.endsWith(".md", Qt::CaseInsensitive))
		return QString();

	QFile File(m_pFS->Prefix() + "/kb/" + FileName);
	if (!File.open(QFile::ReadOnly))
		return QString();
	const QByteArray Data = File.read(kMaxPageBytes);
	File.close();
	return QString::fromUtf8(Data);
}

QString CMcpDocs::ReadPage(const QString& Path, QString* pError) const
{
	if (!m_bOpen) {
		if (pError) *pError = QObject::tr("the documentation pack is not available");
		return QString();
	}

	// The index is the allow-list. Checking membership rather than sanitising
	// the string means a path with traversal in it simply is not a page, and no
	// cleverness about separators can turn it into one.
	if (!m_ByPath.contains(Path)) {
		if (pError) *pError = QObject::tr("'%1' is not a page in the documentation").arg(Path);
		return QString();
	}

	QFile File(m_pFS->Prefix() + "/" + Path);
	if (!File.open(QFile::ReadOnly)) {
		if (pError) *pError = QObject::tr("'%1' could not be read from the pack").arg(Path);
		return QString();
	}
	const QByteArray Data = File.read(kMaxPageBytes);
	File.close();

	return QString::fromUtf8(Data);
}

//
// Ranked rather than filtered: a title hit is worth more than a mention in an
// outline, and an exact setting match is worth most of all, because that is
// what the caller usually has.
//
QJsonArray CMcpDocs::Search(const QString& Query, int Limit) const
{
	QJsonArray Results;
	if (!m_bOpen || Query.trimmed().isEmpty())
		return Results;

	const QString Needle = Query.trimmed();
	QList<QPair<int, QJsonObject> > Scored;

	foreach(const QJsonObject& Page, m_ByPath) {
		int Score = 0;
		const QString Title = Page.value("title").toString();
		const QString Path = Page.value("path").toString();
		const QString Documents = Page.value("documents").toString();

		if (!Documents.isEmpty() && Documents.compare(Needle, Qt::CaseInsensitive) == 0)
			Score += 100;	// this page *is* the reference for what was asked
		if (Title.compare(Needle, Qt::CaseInsensitive) == 0)
			Score += 80;
		if (Title.contains(Needle, Qt::CaseInsensitive))
			Score += 30;
		if (Path.contains(Needle, Qt::CaseInsensitive))
			Score += 20;

		foreach(const QJsonValue& S, Page.value("settings").toArray()) {
			if (S.toString().compare(Needle, Qt::CaseInsensitive) == 0) { Score += 40; break; }
		}
		foreach(const QJsonValue& H, Page.value("outline").toArray()) {
			if (H.toString().contains(Needle, Qt::CaseInsensitive)) { Score += 10; break; }
		}

		if (Score > 0)
			Scored.append(qMakePair(Score, Page));
	}

	std::sort(Scored.begin(), Scored.end(),
		[](const QPair<int, QJsonObject>& A, const QPair<int, QJsonObject>& B) {
			if (A.first != B.first)
				return A.first > B.first;
			// Stable, and alphabetical reads better than hash order.
			return A.second.value("path").toString() < B.second.value("path").toString();
		});

	for (int i = 0; i < Scored.count() && i < Limit; i++) {
		const QJsonObject& Page = Scored.at(i).second;
		QJsonObject Hit;
		Hit["path"] = Page.value("path");
		Hit["title"] = Page.value("title");
		if (Page.contains("documents"))
			Hit["documents"] = Page.value("documents");
		if (Page.contains("outline"))
			Hit["outline"] = Page.value("outline");
		Results.append(Hit);
	}
	return Results;
}

QJsonObject CMcpDocs::ForSetting(const QString& Setting) const
{
	QJsonObject Out;
	Out["setting"] = Setting;

	if (!m_bOpen)
		return Out;

	// settingPage and settingIndex are both built at pack time against the
	// section names of SbieSettings.ini, so a hit here means the docs really do
	// write about this setting rather than merely containing the word.
	const QJsonObject Pages = m_Index.value("settingPage").toObject();
	const QJsonObject Mentions = m_Index.value("settingIndex").toObject();

	// Case-insensitive, because an agent will not match the docs' capitalisation
	// reliably and the setting names themselves are case-insensitive to the
	// driver.
	QString Key;
	foreach(const QString& Candidate, Mentions.keys()) {
		if (Candidate.compare(Setting, Qt::CaseInsensitive) == 0) { Key = Candidate; break; }
	}
	if (Key.isEmpty()) {
		foreach(const QString& Candidate, Pages.keys()) {
			if (Candidate.compare(Setting, Qt::CaseInsensitive) == 0) { Key = Candidate; break; }
		}
	}
	if (Key.isEmpty())
		return Out;

	Out["setting"] = Key;
	if (Pages.contains(Key)) {
		const QString Path = Pages.value(Key).toString();
		Out["referencePage"] = Path;
		Out["title"] = PageEntry(Path).value("title");
	}

	QJsonArray Also;
	foreach(const QJsonValue& Value, Mentions.value(Key).toArray()) {
		if (Value.toString() != Out.value("referencePage").toString())
			Also.append(Value);
	}
	if (!Also.isEmpty())
		Out["alsoMentionedIn"] = Also;

	return Out;
}
