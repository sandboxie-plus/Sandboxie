#pragma once
#include <QString>
#include <QList>

//
// CMcpClientSetup - detecting MCP clients and writing their configuration.
//
// Writing into somebody else's configuration file is the part of this feature
// most able to do real damage, so the rules are narrow and the same for every
// client:
//
//   * merge, never overwrite - the file belongs to the user and almost always
//     has other servers in it
//   * back up before touching it, to <file>.sbiebak, so a mistake is
//     recoverable without us having to be right
//   * refuse rather than guess - if the file is not the JSON shape expected,
//     stop and let the user paste the snippet themselves
//   * write only where the location is unambiguous. A project-scoped config
//     lands inside the user's repository, and that must never be a side effect
//     of clicking a button in a sandbox manager
//
// Claude Code is deliberately *not* written to even though its path is known:
// it owns its own config through `claude mcp add`, and editing the file under a
// running CLI is how you get a confusing merge conflict. It gets the command.
//
class CMcpClientSetup
{
public:
	enum EKind
	{
		eWriteJson = 0,	// we can merge into a known JSON file
		eCommand,		// hand the user a command to run instead
		eManual,		// show the snippet; the location is the user's choice
	};

	struct SClient
	{
		QString	Id;				// stable slug, for settings and logs
		QString	Name;			// what the user calls it
		EKind	Kind = eManual;
		QString	ConfigPath;		// for eWriteJson
		QString	Command;		// for eCommand
		bool	bDetected = false;	// the config file or program was found
		QString	Note;			// why it is manual, when it is

		// The catch-all entry, which is never a find - it is always offerable,
		// so sorting it by bDetected would float a non-answer to the top of the
		// list of answers.
		bool	bFallback = false;

		// We can start this client ourselves, already pointed at us.
		bool	bCanLaunch = false;
	};

	// Every client we know how to help with, detected state filled in.
	static QList<SClient>	Detect();

	// What is actually installed first. Stable, so equally findable clients
	// keep Detect()'s order rather than shuffling between openings, and the
	// catch-all stays last however findable it claims to be.
	static void		SortFoundFirst(QList<SClient>& Clients);

	// The generic configuration an unknown client needs, as JSON the user can
	// paste. Also what the manual path shows.
	static QString	GetSnippet();
	static QString	GetShimPath();
	static bool		IsShimAvailable();

	// Merges our server into the client's config file. Returns false with
	// pError set and the file untouched. On success pBackupPath receives where
	// the previous contents were kept.
	static bool		WriteConfig(const SClient& Client, QString* pError, QString* pBackupPath);

	// Whether our entry is already present, so the dialog can say "configured"
	// rather than offering to do it again.
	static bool		IsConfigured(const SClient& Client);

	// Starts Claude Code in a folder of our own, configured to reach us.
	//
	// The folder sits beside Sandboxie-Plus.ini rather than anywhere of the
	// user's, which is what makes writing a project-scoped .mcp.json into it
	// acceptable: the rule against writing project configs is about not
	// dropping files into somebody's repository as a side effect of clicking
	// a button. A directory we created under our own configuration is not
	// somebody's repository.
	//
	// Returns false with pError set; pFolder receives where it was started.
	static bool		LaunchClaudeCode(QString* pError, QString* pFolder);

	// The half of that which touches the disk: creates the folder and merges
	// our server into its .mcp.json. Separate so it can be checked without
	// starting anything.
	static bool		PrepareAgentFolder(QString* pError);
	static QString	GetAgentFolder();

	// The key our entry lives under in a client config.
	static const char* const kServerName;
};
