#include "stdafx.h"
#include "McpRedact.h"
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

namespace McpRedact
{

struct SRule
{
	QString Pattern;	// literal, matched case-insensitively
	QString Token;
};

//
// Order matters, and it is the reason this is a list rather than a map: the
// profile path contains the user name, so it has to be substituted first or the
// path turns into "C:/Users/%UserName%" and stops being recognisable as a
// single thing.
//
static QList<SRule> LiteralRules()
{
	QList<SRule> Rules;

	const QString Profile = QDir::fromNativeSeparators(QDir::homePath());
	const QString User = qEnvironmentVariable("USERNAME");
	const QString Computer = qEnvironmentVariable("COMPUTERNAME");
	const QString Temp = QDir::fromNativeSeparators(QDir::tempPath());
	const QString AppData = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	const QString Local = QDir::fromNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation));

	// Longest first within each family, so the more specific path wins.
	if (!Temp.isEmpty())		Rules.append({ Temp, "%Temp%" });
	if (!Local.isEmpty())		Rules.append({ Local, "%LocalAppData%" });
	if (!AppData.isEmpty())		Rules.append({ AppData, "%AppData%" });
	if (!Profile.isEmpty())		Rules.append({ Profile, "%User%" });
	if (!User.isEmpty())		Rules.append({ User, "%UserName%" });
	if (!Computer.isEmpty())	Rules.append({ Computer, "%Computer%" });

	return Rules;
}

QString Apply(const QString& Text)
{
	if (Text.isEmpty())
		return Text;

	QString Out = Text;

	// Both separator conventions: a trace carries native paths, a config carries
	// whatever the user typed.
	foreach(const SRule& Rule, LiteralRules()) {
		if (Rule.Pattern.isEmpty())
			continue;
		Out.replace(Rule.Pattern, Rule.Token, Qt::CaseInsensitive);
		Out.replace(QDir::toNativeSeparators(Rule.Pattern), Rule.Token, Qt::CaseInsensitive);
	}

	// A SID identifies an account across machines, so it goes whatever form it
	// is in. The domain part is kept: S-1-5-18 and friends are well-known
	// accounts and saying which one it was is diagnostically useful.
	static const QRegularExpression Sid("\\bS-1-5-21-[0-9-]+\\b");
	Out.replace(Sid, "%SID%");

	// Session and object GUIDs - the kind Windows mints per logon or per volume.
	static const QRegularExpression Guid(
		"\\{?[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}\\}?");
	Out.replace(Guid, "%GUID%");

	return Out;
}

QStringList Apply(const QStringList& Lines)
{
	QStringList Out;
	Out.reserve(Lines.count());
	foreach(const QString& Line, Lines)
		Out.append(Apply(Line));
	return Out;
}

QStringList Describe()
{
	// The tokens, without echoing what they replaced: a legend that spells out
	// the user name would defeat the point of having removed it.
	return QStringList()
		<< "%Temp%, %LocalAppData%, %AppData%, %User% - per-user paths"
		<< "%UserName% - the account name"
		<< "%Computer% - the machine name"
		<< "%SID% - account security identifiers"
		<< "%GUID% - session and object identifiers";
}

}
