#include "stdafx.h"
#include "BoxObject.h"
#include "../SandMan.h"
#include "../Views/SbieView.h"
#include "../Windows/SettingsWindow.h"
#include "../QSbieAPI/SbieUtils.h"


quint32 CBoxObject::StartTask(const QString& Command, const QVariantMap& Options) 
{
    SB_RESULT(quint32) result = theGUI->RunStart(getName(), Command, Options["elevalted"].toBool(), Options["directory"].toString());
    return result.IsError() ? -1 : result.GetValue();
}

QVariantList CBoxObject::ListTasks() 
{
    QVariantList List;
    foreach(const CBoxedProcessPtr& pProcess, m_pIni.objectCast<CSandBox>()->GetProcessList())
        List.append(TaskInfo(pProcess));
    return List;
}

bool CBoxObject::StopTask(quint32 pid)
{
    CBoxedProcessPtr pProcess = m_pIni.objectCast<CSandBox>()->GetProcessList().value(pid);
    if (!pProcess)
        return false;
    SB_STATUS Status = pProcess->Terminate();
    return !Status.IsError();
}

bool CBoxObject::Terminate()
{
    SB_STATUS Status = m_pIni.objectCast<CSandBox>()->TerminateAll();
    return !Status.IsError();
}

QVariantMap CBoxObject::TaskInfo(quint32 pid) 
{
    return TaskInfo(m_pIni.objectCast<CSandBox>()->GetProcessList().value(pid));
}

QVariantMap CBoxObject::TaskInfo(const CBoxedProcessPtr& pProcess)
{
    QVariantMap Info;
    Info["pid"] = pProcess->GetProcessId();
    Info["parentId"] = pProcess->GetParendPID();
    Info["name"] = pProcess->GetProcessName();
    Info["commandline"] = pProcess->GetCommandLine();
    Info["fileName"] = pProcess->GetFileName();
    Info["timeStamp"] = pProcess->GetTimeStamp();
    Info["flags"] = pProcess->GetProcessFlags();
    Info["type"] = pProcess->GetImageType();
    if (pProcess->IsTerminated()) {
        Info["isRunning"] = false;
        Info["returnCode"] = pProcess->GetReturnCode();
    } else
        Info["isRunning"] = true;
    return Info;
}

bool CBoxObject::DeleteContent()
{
    SB_STATUS Status = theGUI->DeleteBoxContent(m_pIni.objectCast<CSandBox>(), CSandMan::eCleanUp);
    return !Status.IsError();
}

bool CBoxObject::RemoveSandbox() 
{
    SB_STATUS Status = theGUI->DeleteBoxContent(m_pIni.objectCast<CSandBox>(), CSandMan::eForDelete);
    if (Status.GetMsgCode() == SB_Canceled)
        return false;
    Status = m_pIni.objectCast<CSandBox>()->RemoveBox();
    return !Status.IsError();
}

bool CBoxObject::MakeShortcut(const QString& Target, const QVariantMap& Options)
{
    QString Location = Options["location"].toString();
    QStandardPaths::StandardLocation location = (QStandardPaths::StandardLocation)-1;
    if (Location.compare("desktop", Qt::CaseInsensitive) == 0)
        location = QStandardPaths::DesktopLocation;
    else if (Location.compare("startmenu", Qt::CaseInsensitive) == 0)
        location = QStandardPaths::ApplicationsLocation;
    else if (Location.compare("documents", Qt::CaseInsensitive) == 0)
        location = QStandardPaths::DocumentsLocation;
    QString Name = Options["name"].toString();
    if (location != -1) {
        QString Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation).replace("/", "\\");
        if (Target == "default_browser")
            Path += "\\" + CSettingsWindow::tr("Sandboxed Web Browser") + ".lnk";
        else if (!Name.isEmpty())
            Path += "\\" + Name + ".lnk";
        else
            return false;
        QString StartExe = theAPI->GetSbiePath() + "\\SandMan.exe";
        return CSbieUtils::CreateShortcut(StartExe, Path, "", getName(), Target);
    }
    return CSbieView::CreateShortcutEx(Target, getName(), Name, Options["iconPath"].toString(), Options["iconIndex"].toInt(), Options["workDir"].toString());
}

void CSBoxObject::ApplyChanges(bool bApply)
{
    CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent());
    pEngine->SetApplyShadow(m_OriginalName, bApply);
}