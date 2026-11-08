#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include "../../MiscHelpers/Archive/ArchiveFS.h"

//
// CMcpDocs - the offline documentation, read out of docs.7z.
//
// The pack is mounted rather than extracted, through the same
// C7zFileEngineHandler that troubleshooting.7z already uses, so a page is read
// with an ordinary QFile against a path inside the archive. That is the whole
// reason the pack is built non-solid: a solid archive would have to be decoded
// from the start of the stream to reach any one page, and the access pattern
// here is "read two pages out of six hundred".
//
// index.json travels inside the pack and is read once. It carries, per page,
// the title, the heading outline, and the settings the page documents - plus
// two reverse maps built at pack time: settingPage, the one page that is a
// setting's reference, and settingIndex, every page that mentions it. An agent
// almost always arrives holding a setting name, so that mapping is the point.
//
// Coverage is partial and the tools must say so: of the settings Sandboxie
// actually has, only about two in five have a documentation page. "No page" is
// therefore not evidence that a setting does not exist, and an agent told
// otherwise will talk a user out of settings that work.
//
class CMcpDocs
{
public:
	CMcpDocs();
	~CMcpDocs();

	// Mounts the newest available pack. Prefers a copy in the configuration
	// directory over the installed one, so an updated pack can be dropped in
	// without reinstalling - the same precedence CScriptManager uses.
	bool			Open();
	// A specific pack, bypassing the search above. Also what the tests use.
	bool			Open(const QString& Path);
	void			Close();
	bool			IsOpen() const		{ return m_bOpen; }

	QString			GetPackPath() const	{ return m_PackPath; }
	int				GetPageCount() const;

	// Page text, or empty with pError set. Only paths present in the index are
	// served: the index is the allow-list, which keeps a crafted path from
	// reaching an arbitrary archive member.
	QString			ReadPage(const QString& Path, QString* pError) const;

	// Free-text search over titles, paths, outlines and setting names. Ranked,
	// capped, and never the whole corpus.
	QJsonArray		Search(const QString& Query, int Limit) const;

	// What documents this setting: the reference page first, then any page that
	// merely mentions it. Empty when the setting has no documentation at all,
	// which the caller must report as distinct from "no such setting".
	QJsonObject		ForSetting(const QString& Setting) const;

	bool			HasPage(const QString& Path) const;

	// The knowledge-base tree, which is a different thing from the pages and
	// is not in index.json - that index describes documentation. These are
	// restricted by prefix instead: only kb/*.md, and the name may carry no
	// separator, so there is nothing for a crafted entry name to traverse.
	QStringList		ListKnowledge() const;
	QString			ReadKnowledge(const QString& FileName) const;

private:
	QJsonObject		PageEntry(const QString& Path) const;

	C7zFileEngineHandler*	m_pFS;
	QString					m_PackPath;
	bool					m_bOpen;

	QJsonObject				m_Index;
	// path -> index entry, for lookup without scanning the pages array.
	QHash<QString, QJsonObject>	m_ByPath;
};
