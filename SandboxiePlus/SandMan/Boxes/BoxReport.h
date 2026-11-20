#pragma once
#include <QNetworkReply>
#include <functional>
#include <QFileInfo>
#include "../../QSbieAPI/SbieAPI.h"

//
// CBoxReport - diagnostic capture and issue submission.
//
// These four were already static members of CBoxAssistant, touching nothing but
// theAPI, theGUI and CSandBoxPlus - a QWizard subclass was simply where they
// happened to live. Using them from anywhere else meant including
// BoxAssistant.h and with it the whole wizard UI, which is why they are here
// instead: an MCP session composes a report without instantiating a wizard.
//
// Not a QObject - a class only so tr() has a context.
//

class CBoxReport
{
	Q_DECLARE_TR_FUNCTIONS(CBoxReport)

public:
	// The one-shot form: capture a box's diagnostics and submit them.
	static QNetworkReply* SubmitReport(const QString& BoxName);

	// The machine-captured half of a report. AssembleDiagData does NOT consume
	// the events it reads - the wizard, the MCP server and a second report all
	// look at the same per-box list, and whoever read it first used to empty it
	// for everyone else.
	static QString		AssembleDiagData(const QString& BoxName);
	static QString		AssembleDiagInfo();

	static QNetworkReply* SubmitReport(const QString& ReportText, int AttachLog, bool AttachIni, const QList<QFileInfo>& CrashDumps = QList<QFileInfo>(), const QString& Mail = QString(), const CSbieProgressPtr& pUploadProgress = NULL);

	// What one submission consists of, so that adding a field does not mean
	// changing every caller's argument list.
	struct SSubmission
	{
		QString				Text;
		// 0 nothing, 1 the message log, 2 the message log and the resource
		// trace. AttachLogFrom turns the tristate checkbox into it.
		int					AttachLog = 0;
		bool				bAttachIni = false;
		QList<QFileInfo>	CrashDumps;
		QString				Mail;
	};

	// Everything that happens after the user presses the button: the upload,
	// its progress in the main window, the tracker's {"success"} answer, and
	// telling the user which way it went.
	//
	// This used to live inside the troubleshooting wizard's submit page, and
	// the agent session's own submission window needs all of it. A second copy
	// would have drifted from this one, and it would have drifted in the error
	// path - the half nobody exercises until it matters.
	//
	// Returns false only when the upload could not be started at all.
	// Otherwise the outcome reaches OnDone(bOk) once the tracker has answered.
	static bool			Submit(QWidget* pParent, const SSubmission& Request,
						const std::function<void(bool)>& OnDone = std::function<void(bool)>());

	// The tristate convention in one place: checked means the trace as well,
	// partially checked means the message log only.
	static int			AttachLogFrom(Qt::CheckState State);

	// Keeps an address the user typed, and only then - an empty field must not
	// erase one they set in the settings a year ago.
	static void			RememberMail(const QString& Mail);
};
