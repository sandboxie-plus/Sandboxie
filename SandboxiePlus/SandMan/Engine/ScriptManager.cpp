#include "stdafx.h"
#include "ScriptManager.h"
#include "../MiscHelpers/Common/Common.h"
#include "../MiscHelpers/Common/OtherFunctions.h"
#include "../SandMan.h"
#include "../OnlineUpdater.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "SysObject.h"

#include "../MiscHelpers/Common/OtherFunctions.h"

#include "../MiscHelpers/Archive/Archive.h"
#include "../MiscHelpers/Archive/ArchiveFS.h"

#include "../Wizards/BoxAssistant.h"

QList<StrPair> ReadCommentHeader(const QString& Header)
{
	QList<StrPair> HeaderFields;
	QStringList HeaderLines = Header.split("\n");
	foreach(QString Line, HeaderLines)
	{
		Line = Line.trimmed();
		if(Line.left(1) == "*")
		{
			Line.remove(0,1);
			Line = Line.trimmed();
		}
		
		StrPair Field = Split2(Line, ":");
		if(Field.first.isEmpty() || Field.first.contains(" "))
			continue;

		HeaderFields.append(Field);
	}
	return HeaderFields;
}

CScriptManager::CScriptManager(QObject* parent) 
	:QObject(parent) 
{
}

QString CScriptManager::GetScript(const QString& Name)
{
	C7zFileEngineHandler IssueFS("issue");
	QString Root = GetIssueDir(IssueFS);

	foreach(const QString &Path, ListDir(Root, QStringList() << "*.js")) {
		if (Path.right(Name.length() + 3) == Name + ".js") {
			return ReadFileAsString(Root + "/" + Path);
		}
	}
	return QString();
}

void CScriptManager::LoadIssues()
{
    //
    // Load Issues, when the user has an own troubleshooting folder, don't load the 7z or online
    //

    C7zFileEngineHandler IssueFS("issue");
    LoadIssues(GetIssueDir(IssueFS, &m_IssueDate));
    if (m_IssueDate.isValid()) {
        if (theConf->GetInt("Options/CheckForIssues", 2) == 1) {

            QVariantMap Data = theGUI->m_pUpdater->GetUpdateData();
            if (!Data.isEmpty() && Data.contains("issues") && theGUI->m_pUpdater->GetLastUpdateTime() > QDateTime::currentDateTime().addDays(-1)) 
                OnUpdateData(Data, QVariantMap());
            else
                theGUI->m_pUpdater->GetUpdates(this, SLOT(OnUpdateData(const QVariantMap&, const QVariantMap&)));
        }
    }
}

void CScriptManager::LoadIssues(const QString& IssueDir)
{
    QVariantMap Issues = QJsonDocument::fromJson(ReadFileAsString(IssueDir + "layout.json").toUtf8()).toVariant().toMap();

    QVariantList Entries = Issues.value("entries").toList();

    if (Entries.isEmpty()) {
        QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Fatal error, failed to load troubleshooting instructions!"));
        return;
    }

    m_GroupedIssues.clear();

    quint32 OsBuild = JSysObject::GetOSVersion()["build"].toUInt();

    //QDir Dir(IssueDir);
    //foreach(const QFileInfo & Info, Dir.entryInfoList(QStringList() << "*.js", QDir::Files)) {
    foreach(const QString & FileName, ListDir(IssueDir, QStringList() << "*.js")) {
        QFileInfo Info(IssueDir + FileName);
        QString Script = ReadFileAsString(Info.filePath());

	    int HeaderBegin = Script.indexOf("/*");
	    int HeaderEnd = Script.indexOf("*/");
	    if(HeaderBegin == -1 || HeaderEnd == -1)
		    continue; // Header is mandatory
        if(HeaderBegin != 0) {
            qDebug() << "Bad Header of" << Info.fileName();
            continue;
	    }

        QVariantMap Issue;
        Issue["id"] = Info.fileName().left(Info.fileName().length() - 3);
        Issue["type"] = "issue";
	    foreach(const StrPair& KeyValue, ReadCommentHeader(Script.mid(HeaderBegin + 2, HeaderEnd - (HeaderBegin + 2))))
            Issue[KeyValue.first] = KeyValue.second;

        if (Issue["group"] == "system" || Issue["group"] == "library")
            continue;

        Issue["script"] = Script;

        bool NotApplicable = false;
        if (Issue.contains("versions")) {
            NotApplicable = true;
            foreach(const QString & V, SplitStr(Issue["versions"].toString(), ",")) {
                StrPair VV = Split2(V, "-");
                if ((VV.second.isEmpty() && COnlineUpdater::VersionToInt(VV.first) == COnlineUpdater::CurrentVersion()) || // exact version match
                    (!VV.second.isEmpty() && (COnlineUpdater::VersionToInt(VV.first) <= COnlineUpdater::CurrentVersion() && COnlineUpdater::VersionToInt(VV.second) >= COnlineUpdater::CurrentVersion()))) { // inside version range
                    NotApplicable = false;
                    break;
                }
            }
        }
        if (!NotApplicable && Issue.contains("os_builds")) {
            NotApplicable = true;
            foreach(const QString & V, SplitStr(Issue["os_builds"].toString(), ",")) {
                StrPair VV = Split2(V, "-");
                if ((VV.second.isEmpty() && VV.first.toUInt() == OsBuild) || // exact version match
                    (!VV.second.isEmpty() && (VV.first.toUInt() <= OsBuild && VV.second.toUInt() >= OsBuild))) { // inside version range
                    NotApplicable = false;
                    break;
                }
            }
        }
        if (NotApplicable)
            continue;

        Entries.append(Issue);
    }

    foreach(const QVariant & vIssue, Entries) {
        QVariantMap Issue = vIssue.toMap();
        QList<QVariantMap>& Group = m_GroupedIssues[Issue["group"].toString()];
        // Note: This way we can define order in the layout json and have the issue scripts loaded at the right place
        QString ID = Issue["id"].toString();
        auto I = std::find_if(Group.begin(), Group.end(), [ID](const QVariantMap& cur)->int { return cur["id"] == ID; });
        if (I == Group.end())
            Group.append(Issue);
        else {
            if (I->contains("script")) {
                QMessageBox::warning(theGUI, "Sandboxie-Plus", tr("Error, troubleshooting instructions duplicated %1 (%2 <-> %3)!")
                    .arg(ID).arg(I->value("id").toString()).arg(Issue.value("id").toString()));
            }
            for(auto J = Issue.begin(); J != Issue.end(); ++J)
                I->insert(J.key(), J.value());
        }
    }

    //
    // Load Translations
    //

    QString Translation = ReadFileAsString(IssueDir + "lang_" + theGUI->m_Language + ".json");
    if (Translation.isEmpty()) {
        QString LangAux = theGUI->m_Language; // Short version as fallback
	    LangAux.truncate(LangAux.lastIndexOf('_'));
        Translation = ReadFileAsString(IssueDir + "lang_" + LangAux + ".json");
    }

    if(!Translation.isEmpty())
        m_Translation = QJsonDocument::fromJson(Translation.toUtf8()).toVariant().toMap();
}

QString CScriptManager::GetIssueDir(C7zFileEngineHandler& IssueFS, QDateTime* pDate)
{
    QString IssueDir = theConf->GetConfigDir() + "/troubleshooting/";
    if (!QFile::exists(IssueDir)) {

        QFileInfo Installed(QApplication::applicationDirPath() + "/troubleshooting.7z");
        QFileInfo Latest(theConf->GetConfigDir() + "/troubleshooting.7z");
        quint64 latest = Latest.lastModified().toSecsSinceEpoch();
        quint64 installed = Installed.lastModified().toSecsSinceEpoch();
        if (latest >= installed && IssueFS.Open(theConf->GetConfigDir() + "/troubleshooting.7z")) {
            IssueDir = IssueFS.Prefix() + "/";
            if(pDate) *pDate = Latest.lastModified();
        }
        else if (IssueFS.Open(QApplication::applicationDirPath() + "/troubleshooting.7z")) {
            IssueDir = IssueFS.Prefix() + "/";
            if(pDate) *pDate = Installed.lastModified();
        }
    }
    return IssueDir;
}

void CScriptManager::OnUpdateData(const QVariantMap& Data, const QVariantMap& Params)
{
    if (Data.isEmpty() || Data["error"].toBool())
        return;

    QVariantMap Issues = Data["issues"].toMap();

    quint64 Date = Issues["date"].toULongLong();
    if (Date >= m_IssueDate.toSecsSinceEpoch()) {

        QString Download = Issues["download"].toString();

        QVariantMap Params;
        Params["path"] = theConf->GetConfigDir() + "/troubleshooting.tmp";
        Params["setDate"] = QDateTime::fromSecsSinceEpoch(Date);
        Params["signature"] = Issues["signature"];
        theGUI->m_pUpdater->DownloadFile(Download, this, SLOT(OnDownload(const QString&, const QVariantMap&)), Params);
    }
}

extern "C" long VerifyFileSignatureImpl(const wchar_t* FilePath, void* Signature, unsigned long SignatureSize);

void CScriptManager::OnDownload(const QString& Path, const QVariantMap& Params)
{
	QByteArray Signature = QByteArray::fromBase64(Params["signature"].toByteArray());
	
	if (VerifyFileSignatureImpl(QString(Path).replace("/","\\").toStdWString().c_str(), Signature.data(), Signature.size()) < 0) { // !NT_SUCCESS
        QFile::remove(Path);
		return;
	}
    QString FinalPath = theConf->GetConfigDir() + "/troubleshooting.7z";
    QFile::remove(FinalPath);
    QFile::rename(Path, FinalPath);


	QString IssueDir;
    C7zFileEngineHandler IssueFS("issue");
    if (!IssueFS.Open(FinalPath)) {
        QMessageBox::critical(theGUI, "Sandboxie-Plus", tr("Downloaded troubleshooting instructions are corrupted!"));
        QFile::remove(Path);
        return;
    }
    
    LoadIssues(IssueFS.Prefix() + "/");

    emit IssuesUpdated();
}