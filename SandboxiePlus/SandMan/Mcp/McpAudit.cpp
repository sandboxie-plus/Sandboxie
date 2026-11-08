#include "stdafx.h"
#include "McpAudit.h"
#include "../SandMan.h"
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

// Big enough to hold a long session, small enough that nobody has to care it
// exists. One generation is kept, so the worst case on disk is twice this.
static const qint64 kMaxBytes = 2 * 1024 * 1024;

CMcpAudit::CMcpAudit()
{
}

CMcpAudit::~CMcpAudit()
{
	Close();
}

void CMcpAudit::Rotate(const QString& Path)
{
	const QString Previous = Path + ".1";
	QFile::remove(Previous);
	QFile::rename(Path, Previous);
}

bool CMcpAudit::Open()
{
	Close();

	const QString Path = theConf->GetConfigDir() + "/SbieMcpAudit.log";
	if (QFileInfo(Path).size() >= kMaxBytes)
		Rotate(Path);

	m_File.setFileName(Path);
	if (!m_File.open(QFile::Append | QFile::Text))
		return false;

	Record("audit", "log opened");
	return true;
}

void CMcpAudit::Close()
{
	if (m_File.isOpen()) {
		Record("audit", "log closed");
		m_File.close();
	}
}

void CMcpAudit::Record(const QString& Category, const QString& Message, const QJsonObject& Data)
{
	if (!m_File.isOpen())
		return;

	QJsonObject Entry;
	// ISO 8601 with the offset, because a support log read in another timezone
	// is otherwise a guessing game.
	Entry["at"] = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
	Entry["category"] = Category;
	Entry["message"] = Message;
	if (!Data.isEmpty())
		Entry["data"] = Data;

	// Compact and one per line: JSON Lines, so an interrupted write costs the
	// last line rather than the file's structure.
	m_File.write(QJsonDocument(Entry).toJson(QJsonDocument::Compact));
	m_File.write("\n");
	// Flushed every time on purpose. The interesting entries are the ones
	// written just before something went wrong.
	m_File.flush();
}
