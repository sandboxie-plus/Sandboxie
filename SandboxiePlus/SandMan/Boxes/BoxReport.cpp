#include "stdafx.h"
#include "BoxReport.h"
#include "../SandMan.h"
#include "../SbiePlusAPI.h"
#include "../Views/TraceView.h"
#include "../OnlineUpdater.h"
#include "../../MiscHelpers/Archive/Archive.h"
#include "../../MiscHelpers/Common/NetworkAccessManager.h"
#include <QHttpMultiPart>
#include <QMessageBox>
#include <QJsonDocument>

QNetworkReply* CBoxReport::SubmitReport(const QString& BoxName)
{
    QString Report = AssembleDiagData(BoxName);
    Report += AssembleDiagInfo();

    int AttachLog = 0; // 1 // 2 // todo: xxxx read from ini
    bool AttachIni = true; 
    return CBoxReport::SubmitReport(Report, AttachLog, AttachIni);
}

QString CBoxReport::AssembleDiagData(const QString& BoxName)
{
#ifndef SBIE_HAS_DIAG_EVENTS
    // Nothing records diagnostic events in this build, so there is no section
    // to write rather than an empty heading to explain. See SBIE_HAS_DIAG_EVENTS.
    Q_UNUSED(BoxName);
    return QString();
#else
    auto pBoxEx = theAPI->GetBoxByName(BoxName).objectCast<CSandBoxPlus>();
    if (!pBoxEx) return QString();

    QString Report = "Collected Diagnostic Data:\n";
    foreach(const auto & pEntry, pBoxEx->GetDiagEvents()) {
        if(pEntry.ProcessName.isEmpty())
            Report += QString("%1:").arg(pEntry.EventId);
        else
            Report += QString("%1 (%2):").arg(pEntry.EventId).arg(pEntry.ProcessName);
        if(pEntry.EventData.size() == 3 && pEntry.EventData[2].isEmpty())
            Report += pEntry.EventData[1] + "\n";
        else
            Report += pEntry.EventData.mid(1).join("; ") + "\n";
    }

    // Deliberately not cleared here. Reading is not consuming: the wizard, the
    // MCP server and a second report all look at the same per-box list, and
    // whoever read it first used to empty it for everyone else. The events also
    // reach the central message log now, so a clear here is not even a loss of
    // the last copy - it is just surprising. Clear it where the decision
    // belongs, after a submission is known to have succeeded.

    return Report;
#endif
}

QString CBoxReport::AssembleDiagInfo()
{
    QString Report;
    Report += "\n\nFurther information:\n";
    Report += "Sandboxie-Plus Version: " + theGUI->GetVersion() + "\n";
    Report += "Operating System Version: " + QSysInfo::kernelVersion() + "-" + QSysInfo::currentCpuArchitecture() + "\n";
    return Report;
}

QNetworkReply* CBoxReport::SubmitReport(const QString& ReportText, int AttachLog, bool AttachIni, const QList<QFileInfo>& CrashDumps, const QString& Mail, const CSbieProgressPtr& pUploadProgress)
{
    QBuffer* pTraceLog = NULL;
    if (AttachLog == 2) {
        pTraceLog = new QBuffer();
        pTraceLog->open(QIODevice::ReadWrite);
        if (!CTraceView::SaveToFile(pTraceLog)) {
            delete pTraceLog;
            return NULL;
        }
    }

    CNetworkAccessManager* pRequestManager = new CNetworkAccessManager(30 * 1000);

    QHttpMultiPart* pMultiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart Report;
    Report.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"report\""));
    Report.setBody(ReportText.toUtf8());
    pMultiPart->append(Report);

    if (AttachIni) {

        QFile* pSbieIni = new QFile(theAPI->GetIniPath(), pMultiPart);

        if (pSbieIni->open(QIODevice::ReadOnly)) {

            QHttpPart SbieIni;
            SbieIni.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
            SbieIni.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieIni\"; filename=\"Sandboxie.ini\""));

            SbieIni.setBodyDevice(pSbieIni);
            pMultiPart->append(SbieIni);
        }
    }

    if (AttachLog != 0) {

        QBuffer* pSbieLogs = new QBuffer(pMultiPart);
        CArchive Archive("SbieTrace.7z", pSbieLogs);

        QMap<int, QIODevice*> Files;

        QBuffer* pMessageLog = new QBuffer();
        pMessageLog->open(QIODevice::ReadWrite);
        theGUI->SaveMessageLog(pMessageLog);
        int ArcIndex = Archive.AddFile("SbieMsg.log");
        pMessageLog->seek(0);
	    Files.insert(ArcIndex, pMessageLog);

        if (pTraceLog) {
            ArcIndex = Archive.AddFile("SbieTrace.log");
            pTraceLog->seek(0);
            Files.insert(ArcIndex, pTraceLog);
        }

        if(pUploadProgress) pUploadProgress->ShowMessage(tr("Compressing Logs"));
        SCompressParams Params;
        Params.iLevel = 9;
        Archive.Update(&Files, true, &Params);
        
        if (pSbieLogs->open(QIODevice::ReadOnly)) {

            QHttpPart SbieLogs;
            SbieLogs.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-strea"));
            SbieLogs.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieLogs\"; filename=\"SbieLogs.7z\""));

            SbieLogs.setBodyDevice(pSbieLogs);
            pMultiPart->append(SbieLogs);
        }
    }

    if (!CrashDumps.isEmpty()) {

        QBuffer* pSbieDumps = new QBuffer(pMultiPart);
        CArchive Archive("SbieDumps.7z", pSbieDumps);

        QMap<int, QIODevice*> Files;
        foreach(const QFileInfo & Info, CrashDumps) {
            QFile* pCrashDump = new QFile(Info.filePath());
            int ArcIndex = Archive.AddFile(Info.fileName());
            Files.insert(ArcIndex, pCrashDump);
        }

        if(pUploadProgress) pUploadProgress->ShowMessage(tr("Compressing Dumps"));
        SCompressParams Params;
        Params.iLevel = 9;
        Archive.Update(&Files, true, &Params);

        if (pSbieDumps->open(QIODevice::ReadOnly)) {

            QHttpPart SbieDumps;
            SbieDumps.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-strea"));
            SbieDumps.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sbieDumps\"; filename=\"SbieDumps.7z\""));

            SbieDumps.setBodyDevice(pSbieDumps);
            pMultiPart->append(SbieDumps);
        }
    }

    if (!Mail.isEmpty()) {
        QHttpPart eMail;
        eMail.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
        eMail.setBody(Mail.toUtf8());
        pMultiPart->append(eMail);
    }

    quint64 RandID = COnlineUpdater::GetRandID();
    QHttpPart randId;
    randId.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"randId\""));
    randId.setBody(QString::number(RandID, 16).rightJustified(16, '0').toUpper().toUtf8());
    pMultiPart->append(randId);

    QUrl Url("https://sandboxie-plus.com/issues/submit.php");
    QNetworkRequest Request(Url);
	//Request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	Request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	//Request.setRawHeader("Accept-Encoding", "gzip");
    QNetworkReply* pReply = pRequestManager->post(Request, pMultiPart);
    pMultiPart->setParent(pReply);

	if(pUploadProgress) pUploadProgress->ShowMessage(tr("Submitting report..."));

    QObject::connect(pReply, &QNetworkReply::finished, pReply, [pReply, pRequestManager]() {
        pRequestManager->deleteLater();
        pReply->deleteLater();
    });

    return pReply;
}

int CBoxReport::AttachLogFrom(Qt::CheckState State)
{
	// Tristate rather than a plain checkbox because the two logs are not
	// comparable: the resource trace lists everything every sandboxed program
	// touched, so "the message log only" has to be an answer of its own.
	if (State == Qt::Checked)
		return 2;
	if (State == Qt::PartiallyChecked)
		return 1;
	return 0;
}

void CBoxReport::RememberMail(const QString& Mail)
{
	if (!Mail.isEmpty())
		theConf->SetValue("User/eMail", Mail);
}

bool CBoxReport::Submit(QWidget* pParent, const SSubmission& Request,
		const std::function<void(bool)>& OnDone)
{
	CSbieProgressPtr pProgress = CSbieProgressPtr(new CSbieProgress());

	// Built first, registered after - and that ordering is the whole fix for a
	// deadlock that hung SandMan at "Saving TraceLog...".
	//
	// With the trace attached, SubmitReport calls CTraceView::SaveToFile, which
	// registers a progress operation of its own and then waits on it MODALLY.
	// That wait ends when CSandMan has no pending operations left - so an upload
	// operation registered up front, which cannot finish because the upload has
	// not been assembled yet, holds the modal loop open for ever. The trace was
	// written; the dialog simply could never close.
	//
	// So nothing of ours is pending while SubmitReport runs. The cost is the
	// "Compressing Logs" message going nowhere, which is a line of text against
	// a hang that needs Task Manager to escape.
	QNetworkReply* pReply = SubmitReport(Request.Text, Request.AttachLog, Request.bAttachIni,
		Request.CrashDumps, Request.Mail, pProgress);
	if (!pReply)
		return false;

	theGUI->AddAsyncOp(pProgress, false, QString(), pParent);

	QObject::connect(pReply, &QNetworkReply::finished, pParent, [pParent, pReply, pProgress, OnDone]() mutable {
		const QByteArray Answer = pReply->readAll();
		pProgress->Finish(SB_OK);
		pProgress.clear();

		// The tracker answers {"success": bool, "error": int} with HTTP 200
		// either way, so the body is the only thing that says what happened.
		const QVariantMap Result = QJsonDocument::fromJson(Answer).toVariant().toMap();
		const bool bOk = Result["success"].toBool();
		if (!bOk)
			QMessageBox::critical(pParent, "Sandboxie-Plus", tr("Failed to submit report, error %1\nTry submitting without the log attached.").arg(Result["error"].toInt()));
		else
			QMessageBox::information(pParent, "Sandboxie-Plus", tr("Your issue report has been successfully submitted, thank you."));

		if (OnDone)
			OnDone(bOk);
	});

	QObject::connect(pReply, &QNetworkReply::uploadProgress, pParent, [pProgress](qint64 Bytes, qint64 Total) {
		if (Total != 0 && !pProgress.isNull())
			pProgress->Progress(100 * Bytes / Total);
	});

	return true;
}

