#include "stdafx.h"
#include <windows.h>
#include <string>
// theConf, for the configuration directory
#include "../../MiscHelpers/Common/Settings.h"
#include "McpClientSetup.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

const char* const CMcpClientSetup::kServerName = "sandboxie";

QString CMcpClientSetup::GetShimPath()
{
	return QDir::toNativeSeparators(QApplication::applicationDirPath() + "/SbieMcp.exe");
}

bool CMcpClientSetup::IsShimAvailable()
{
	return QFile::exists(GetShimPath());
}

void CMcpClientSetup::SortFoundFirst(QList<SClient>& Clients)
{
	std::stable_sort(Clients.begin(), Clients.end(),
		[](const SClient& A, const SClient& B) {
			// The catch-all is always "available", so sorting it by bDetected
			// would float a non-answer above the real ones.
			if (A.bFallback != B.bFallback)
				return B.bFallback;
			return A.bDetected && !B.bDetected;
		});
}

QString CMcpClientSetup::GetSnippet()
{
	// Built through QJsonDocument so the path is escaped correctly - a Windows
	// path pasted raw into JSON is the classic way this goes wrong.
	QJsonObject Server;
	Server["command"] = GetShimPath();

	QJsonObject Servers;
	Servers[kServerName] = Server;

	QJsonObject Root;
	Root["mcpServers"] = Servers;

	return QString::fromUtf8(QJsonDocument(Root).toJson(QJsonDocument::Indented));
}

QList<CMcpClientSetup::SClient> CMcpClientSetup::Detect()
{
	QList<SClient> Clients;
	const QString AppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	// AppDataLocation already ends in the application name on Windows, so step
	// up to Roaming itself.
	const QString Roaming = QFileInfo(AppData).dir().absolutePath();

	{
		// Claude Desktop keeps one file in a fixed place, which is the only
		// case where writing is unambiguous enough to offer.
		SClient Client;
		Client.Id = "claude-desktop";
		Client.Name = QObject::tr("Claude Desktop");
		Client.Kind = eWriteJson;
		Client.ConfigPath = QDir::toNativeSeparators(Roaming + "/Claude/claude_desktop_config.json");
		// The directory, not the file: the file does not exist until a server
		// is configured, so requiring it would hide the client from anyone who
		// has never added one.
		Client.bDetected = QFileInfo(QFileInfo(Client.ConfigPath).dir().absolutePath()).exists();
		Clients.append(Client);
	}
	{
		SClient Client;
		Client.Id = "claude-code";
		Client.Name = QObject::tr("Claude Code");
		Client.Kind = eCommand;
		Client.Command = QString("claude mcp add %1 -- \"%2\"").arg(kServerName).arg(GetShimPath());
		Client.Note = QObject::tr("Claude Code manages its own configuration; run this command instead.");
		Client.bDetected = !QStandardPaths::findExecutable("claude").isEmpty();
		// Only worth offering when we can actually start it.
		Client.bCanLaunch = Client.bDetected;
		Clients.append(Client);
	}
	{
		SClient Client;
		Client.Id = "vscode";
		Client.Name = QObject::tr("Visual Studio Code");
		Client.Kind = eManual;
		Client.Note = QObject::tr("Add it per workspace or to your user settings - we do not write "
			"into a workspace, because that file lives in your repository.");
		Client.bDetected = !QStandardPaths::findExecutable("code").isEmpty();
		Clients.append(Client);
	}
	{
		SClient Client;
		Client.Id = "other";
		Client.Name = QObject::tr("Another client");
		Client.Kind = eManual;
		Client.Note = QObject::tr("Most clients take a command to run. Use the path or the snippet below.");
		Client.bDetected = true;
		Client.bFallback = true;
		Clients.append(Client);
	}

	return Clients;
}

static bool ReadJsonObject(const QString& Path, QJsonObject* pOut, QString* pError)
{
	QFile File(Path);
	if (!File.exists()) {
		*pOut = QJsonObject();	// a file that is not there yet is simply empty
		return true;
	}
	if (!File.open(QFile::ReadOnly)) {
		*pError = QObject::tr("%1 could not be opened for reading.").arg(Path);
		return false;
	}
	const QByteArray Raw = File.readAll();
	File.close();

	if (Raw.trimmed().isEmpty()) {
		*pOut = QJsonObject();
		return true;
	}

	QJsonParseError Parse;
	const QJsonDocument Doc = QJsonDocument::fromJson(Raw, &Parse);
	if (Parse.error != QJsonParseError::NoError || !Doc.isObject()) {
		// Refuse rather than replace. A file we cannot parse may still be
		// precious to the user, and overwriting it to "fix" the problem would
		// be the worst possible response.
		*pError = QObject::tr("%1 is not valid JSON (%2), so it was left untouched. "
			"Add the configuration by hand instead.").arg(Path).arg(Parse.errorString());
		return false;
	}

	*pOut = Doc.object();
	return true;
}

bool CMcpClientSetup::IsConfigured(const SClient& Client)
{
	if (Client.Kind != eWriteJson)
		return false;

	QJsonObject Root;
	QString Error;
	if (!ReadJsonObject(Client.ConfigPath, &Root, &Error))
		return false;

	const QJsonObject Servers = Root.value("mcpServers").toObject();
	if (!Servers.contains(kServerName))
		return false;

	// Present, but is it pointing at this installation? A stale entry from an
	// old install is worse than none, because it looks configured and fails.
	return Servers.value(kServerName).toObject().value("command").toString()
		.compare(GetShimPath(), Qt::CaseInsensitive) == 0;
}

bool CMcpClientSetup::WriteConfig(const SClient& Client, QString* pError, QString* pBackupPath)
{
	if (Client.Kind != eWriteJson) {
		*pError = QObject::tr("This client has to be configured by hand.");
		return false;
	}
	if (!IsShimAvailable()) {
		*pError = QObject::tr("SbieMcp.exe was not found next to SandMan.");
		return false;
	}

	QJsonObject Root;
	if (!ReadJsonObject(Client.ConfigPath, &Root, pError))
		return false;

	// Back up first, and only when there is something to lose.
	const QFileInfo Info(Client.ConfigPath);
	if (Info.exists()) {
		const QString Backup = Client.ConfigPath + ".sbiebak";
		QFile::remove(Backup);
		if (!QFile::copy(Client.ConfigPath, Backup)) {
			*pError = QObject::tr("%1 could not be backed up, so it was left untouched.").arg(Client.ConfigPath);
			return false;
		}
		if (pBackupPath) *pBackupPath = Backup;
	}
	else if (!QDir().mkpath(Info.dir().absolutePath())) {
		*pError = QObject::tr("%1 could not be created.").arg(Info.dir().absolutePath());
		return false;
	}

	// Merge: keep every other server exactly as it was, and replace only ours.
	QJsonObject Servers = Root.value("mcpServers").toObject();
	QJsonObject Server;
	Server["command"] = GetShimPath();
	Servers[kServerName] = Server;
	Root["mcpServers"] = Servers;

	// Write through a temporary and rename, so an interrupted write cannot
	// leave the user with half a config file.
	const QString TempPath = Client.ConfigPath + ".sbietmp";
	QFile::remove(TempPath);
	QFile Temp(TempPath);
	if (!Temp.open(QFile::WriteOnly | QFile::Truncate)) {
		*pError = QObject::tr("%1 could not be written.").arg(TempPath);
		return false;
	}
	Temp.write(QJsonDocument(Root).toJson(QJsonDocument::Indented));
	Temp.close();

	QFile::remove(Client.ConfigPath);
	if (!QFile::rename(TempPath, Client.ConfigPath)) {
		*pError = QObject::tr("%1 could not be replaced; the new configuration is in %2.")
			.arg(Client.ConfigPath).arg(TempPath);
		return false;
	}

	return true;
}

QString CMcpClientSetup::GetAgentFolder()
{
	// cleanPath, because GetConfigDir may or may not end in a separator and
	// the doubled one shows up in the dialog text.
	return QDir::toNativeSeparators(QDir::cleanPath(theConf->GetConfigDir() + "/agent"));
}

//
// Start Claude Code already pointed at us.
//
// The point is not saving the user a command - it is that the command has
// to be run somewhere, and "somewhere" is a decision a newcomer should not
// have to make. A folder of our own answers it: the configuration is
// project-scoped, so it reaches this one session and nothing else the user
// does with Claude Code, and it can be thrown away by deleting a directory.
//
bool CMcpClientSetup::PrepareAgentFolder(QString* pError)
{
	const QString Folder = GetAgentFolder();

	if (!IsShimAvailable()) {
		if (pError) *pError = QObject::tr("SbieMcp.exe was not found next to SandMan.");
		return false;
	}

	if (!QDir().mkpath(Folder)) {
		if (pError) *pError = QObject::tr("This folder could not be created:\n%1").arg(Folder);
		return false;
	}

	// Merged rather than written over, and backed up first, for the same
	// reason as any other client config: the user may have put something here
	// themselves between one launch and the next.
	const QString ConfigPath = Folder + QDir::separator() + ".mcp.json";
	QJsonObject Root;
	QFile Existing(ConfigPath);
	if (Existing.exists()) {
		if (!Existing.open(QFile::ReadOnly)) {
			if (pError) *pError = QObject::tr("This file could not be read:\n%1").arg(ConfigPath);
			return false;
		}
		const QByteArray Raw = Existing.readAll();
		Existing.close();

		QJsonParseError Error;
		const QJsonDocument Doc = QJsonDocument::fromJson(Raw, &Error);
		if (!Raw.trimmed().isEmpty() && (Error.error != QJsonParseError::NoError || !Doc.isObject())) {
			// Refuse rather than guess - a file we do not understand may still
			// be precious.
			if (pError) *pError = QObject::tr("This file is not the JSON this expects, so it was "
				"left alone:\n%1").arg(ConfigPath);
			return false;
		}
		Root = Doc.object();

		QFile::remove(ConfigPath + ".sbiebak");
		QFile::copy(ConfigPath, ConfigPath + ".sbiebak");
	}

	QJsonObject Server;
	Server["command"] = GetShimPath();
	QJsonObject Servers = Root.value("mcpServers").toObject();
	Servers[kServerName] = Server;
	Root["mcpServers"] = Servers;

	QFile Out(ConfigPath);
	if (!Out.open(QFile::WriteOnly | QFile::Truncate)) {
		if (pError) *pError = QObject::tr("This file could not be written:\n%1").arg(ConfigPath);
		return false;
	}
	Out.write(QJsonDocument(Root).toJson(QJsonDocument::Indented));
	Out.close();
	return true;
}

bool CMcpClientSetup::LaunchClaudeCode(QString* pError, QString* pFolder)
{
	const QString Folder = GetAgentFolder();
	if (pFolder) *pFolder = Folder;

	if (QStandardPaths::findExecutable("claude").isEmpty()) {
		if (pError) *pError = QObject::tr("Claude Code was not found. Install it, or open a "
			"terminal yourself and run the command shown above.");
		return false;
	}

	if (!PrepareAgentFolder(pError))
		return false;

	// CreateProcess rather than QProcess, and the reason is the whole bug this
	// replaced: QProcess adds CREATE_NO_WINDOW on Windows, because it assumes
	// a child process is something you want to read the output of rather than
	// talk to. Claude Code is a terminal program, so that produced a running
	// claude.exe with no console attached and no way to type at it - started,
	// by every measure except the one that matters.
	//
	// CREATE_NEW_CONSOLE asks for the opposite explicitly. On Windows 11 that
	// still opens in whatever the user set as their default terminal, so this
	// does not force anyone back to the old console host.
	//
	// cmd /K rather than claude directly, so the window stays after it exits
	// and a startup error can be read instead of flashing past.
	STARTUPINFOW StartupInfo;
	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	PROCESS_INFORMATION ProcessInfo;
	memset(&ProcessInfo, 0, sizeof(ProcessInfo));

	// CreateProcessW may modify the command line buffer, so it cannot be a
	// temporary or a literal.
	std::wstring CommandLine = QString("cmd.exe /K claude").toStdWString();
	const std::wstring WorkingDir = QDir::toNativeSeparators(Folder).toStdWString();

	if (!CreateProcessW(NULL, &CommandLine[0], NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE, NULL, WorkingDir.c_str(), &StartupInfo, &ProcessInfo)) {
		if (pError) *pError = QObject::tr("Claude Code could not be started (error %1).")
			.arg((quint32)GetLastError());
		return false;
	}
	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);
	return true;
}

