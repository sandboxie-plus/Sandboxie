#include "stdafx.h"
#include "BoxEngine.h"
#include "../../QSbieAPI/SbieUtils.h"

#include "BoxObject.h"
#include "SbieObject.h"
#include "IniObject.h"
#include "SysObject.h"
#include "WizardObject.h"

#include "ScriptManager.h"

#include "../MiscHelpers/Common/Common.h"

#include "../SandMan.h"
#include "../Views/SbieView.h"
#include "../Windows/OptionsWindow.h"
#include "../Windows/SettingsWindow.h"
#include "../Wizards/BoxAssistant.h"

#include <private/qv4engine_p.h>
#include <private/qv4script_p.h>
#include <private/qv4qobjectwrapper_p.h>

QSet<CBoxEngine*> CBoxEngine::m_Instances;

CBoxEngine::CBoxEngine(QObject* parent) : QThread(parent)
{
    m_Instances.insert(this);

    m_State = eUnknown;
    m_pEngine = NULL;
    m_pDebuggerBackend = NULL;

    //static QQmlDebuggingEnabler qQmlEnableDebuggingHelper(false);
    //QQmlDebuggingEnabler::startTcpDebugServer(1234, QQmlDebuggingEnabler::WaitForClient);
}

CBoxEngine::~CBoxEngine()
{
    m_Instances.remove(this);

    Stop();
}

void CBoxEngine::Stop()
{
    if (!isRunning())
        return;

    m_Mutex.lock();
    if (m_State == eQuery || m_State == eReady)
        Continue(true, eCanceled);
    else {
        if (m_State == eRunning || m_State == eRunningAsync)
            m_State = eCanceled;
        m_Mutex.unlock();
    }

    if (!wait(30 * 1000)) {
        qDebug() << "Failed to stop Box Engine";
        terminate();
        m_State = eCanceled;
    }
}

void CBoxEngine::StopAll()
{
    foreach(CBoxEngine* pEngine, m_Instances)
        pEngine->Stop();
}

QV4::ReturnedValue method_translate(const QV4::FunctionObject *b, const QV4::Value *v, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine *v4 = scope.engine;

    CBoxEngine* pEngine = qobject_cast<CBoxEngine*>(CJSEngineExt::getEngineByHandle(v4)->thread());
    CBoxAssistant* Wizard = qobject_cast<CBoxAssistant*>(pEngine->parent()); // todo make teranslation work also for system scripts

    QString Result;
    for (int i = 0; i < argc; i++) {
        if (i == 0) Result = theGUI->GetScripts()->Tr(argv[i].toQStringNoThrow());
        else Result = Result.arg(argv[i].toQStringNoThrow());
    }

    return QV4::Encode(scope.engine->newString(Result));
}

QV4::ReturnedValue method_wcmp(const QV4::FunctionObject *b, const QV4::Value *v, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine *v4 = scope.engine;

    if (argc < 2)
        return QV4::Encode::undefined();

    return QV4::Encode(CSbieUtils::WildCompare(argv[0].toQStringNoThrow(), argv[1].toQStringNoThrow()));
}

QV4::ReturnedValue method_invoke(const QV4::FunctionObject *b, const QV4::Value *v, const QV4::Value *argv, int argc)
{
    QV4::Scope scope(b);
    QV4::ExecutionEngine *v4 = b->engine();

    QString Name = argv[0].toQStringNoThrow();

    CBoxEngine* pEngine = qobject_cast<CBoxEngine*>(CJSEngineExt::getEngineByHandle(v4)->thread());
    CJSEngineExt* pJSEngine = pEngine->GetEngine();

    QString Script;
    QMetaObject::invokeMethod(theGUI->GetScripts(), "GetScript", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, Script), Q_ARG(QString, Name));
    if (!Script.isEmpty()) {
        QJSValue ret = pJSEngine->evaluateScript(Script, Name + ".js");
        if (ret.isError()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
			pJSEngine->throwError(ret.toString());
#else
            pJSEngine->throwError(ret);
#endif
            return QV4::Encode::undefined();
        } else {
            QV4::ScopedValue rv(scope, scope.engine->fromVariant(ret.toVariant()));
            QV4::Scoped<QV4::QObjectWrapper> qobjectWrapper(scope, rv);
            if (!!qobjectWrapper) {
                if (QObject *object = qobjectWrapper->object())
                    QQmlData::get(object, true)->setImplicitDestructible();
            }
            return rv->asReturnedValue();
        }
    }

    return QV4::Encode(false);
}

void CBoxEngine::init()
{
    connect(m_pEngine, SIGNAL(printTrace(const QString&)), this, SLOT(AppendLog(const QString&)));

    QV4::Scope scope(m_pEngine->handle());
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("tr"), method_translate);
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("wildCompare"), method_wcmp);
    scope.engine->globalObject->defineDefaultProperty(QStringLiteral("invoke"), method_invoke);

    m_pEngine->globalObject().setProperty("system", m_pEngine->newQObject(new JSysObject(this)));
    m_pEngine->globalObject().setProperty("sbie", m_pEngine->newQObject(new JSbieObject(new CSbieObject(this), this)));
}

bool CBoxEngine::RunScript(const QString& Script, const QString& Name, const QVariantMap& Params)
{
    if (isRunning())
        return false;

    m_Script = Script;
    m_Name = Name;
    m_Params = Params;
    m_State = eRunning;

    if(!m_pEngine) m_pEngine = new CJSEngineExt(); // the engine lives in its own thread
    //m_pEngine->installExtensions(QJSEngine::ConsoleExtension);

    init();

    m_pEngine->moveToThread(this);

    //////////////////////////////////////////////////////////////////////
    //
    //      !!! CAUTION Multi Threading !!!
    // 
    // Note: The engine runs in its own thread but the rest of SandMan 
    // is mostly single threaded, also QSbieAPI is not thread safe, so 
    // access to it must be synchronized. We solve this by executing 
    // all calls to theGUI and/or theAPI in the main thread through
    // the use of QT's slot system, we wrap all calls from the engine
    // in blocking QMetaObject::invokeMethod calls targeting to objects
    // which belong to the main thread, hence they need to be created in 
    // the main thread and passed to the caller from there.
    //
    //

    start();

    //return Wait();
    return true; // fully async operation
}

void CBoxEngine::run()
{
    //QElapsedTimer timer;
    //timer.start();
    
    for (auto I = m_Params.begin(); I != m_Params.end(); ++I)
        m_pEngine->globalObject().setProperty(I.key(), m_pEngine->toScriptValue(*I));

    //auto ret = m_pEngine->evaluateScript("(()=>{" + m_Script + "})()", m_Name);
    auto ret = m_pEngine->evaluateScript(m_Script, m_Name);

    //qDebug() << "CBoxEngine::run took" << timer.elapsed() << "ms";

    if (IsRunning()) {
        if (ret.isError()) {
            QString Error = tr("Uncaught exception at line %1: %2").arg(ret.property("lineNumber").toInt()).arg(ret.toString());
            AppendLog(Error);
            SetState(eError, Error);
        }
        else
            SetState(eCompleted);
    }

    m_Result = m_pEngine->globalObject().property("result").toVariant();
    
    delete m_pEngine;
    m_pEngine = NULL;
}

//bool CBoxEngine::Wait()
//{
//    while (m_State == eRunning)
//        QCoreApplication::processEvents(); // keep the main thread going
//    return true;
//}

bool CBoxEngine::SetResult(const QVariantMap& Result) 
{
    m_Mutex.lock();
    m_Data = Result;
    return Continue(true);
}

bool CBoxEngine::Continue(bool bLocked, EState State)
{
    Q_ASSERT(!IsRunning());

    if (!bLocked) m_Mutex.lock();

    // Note: we set the state directly and the engine thread emits set state from WaitLocked

    m_State = State; 
    m_Wait.wakeOne();
    m_Mutex.unlock();

    //return Wait();
    return true; // fully async operation
}

void CBoxEngine::SetState(EState state, const QString& Text)
{
    m_State = state;
    emit StateChanged(state, Text);
}

bool CBoxEngine::TestRunning() {

    // WARNING: call this function only from the engine thread itself !!!

    if (!IsRunning()) {
        m_pEngine->throwError(QString("Canceled"));
        return false;
    }
    return true;
}

bool CBoxEngine::WaitLocked() {

    // WARNING: call this function only from the engine thread itself !!!

    if (m_pDebuggerBackend) {
        while (!m_Wait.wait(&m_Mutex, 10))
            QCoreApplication::processEvents(); // for CV4DebugAgent::runJob()
    } else
        m_Wait.wait(&m_Mutex);

    emit StateChanged(m_State);

    return TestRunning();
}

void CBoxEngine::AppendLog(const QString& Line) 
{ 
    qDebug() << "BoxEngine Log:" << Line;

    emit LogMessage(Line);

    //QMutexLocker Locker(&m_Mutex);
    //m_Log += Line + "\n"; 
}

QObject* CBoxEngine::GetDebuggerBackend()
{
    if(!m_pEngine) 
        m_pEngine = new CJSEngineExt();
    if (!m_pDebuggerBackend) {
        m_pDebuggerBackend = newV4ScriptDebuggerBackendDynamic(m_pEngine);
        if (m_pDebuggerBackend) {
            QMetaObject::invokeMethod(m_pDebuggerBackend, "pause", Qt::DirectConnection);
            m_pDebuggerBackend->setParent(this);
        }
    }
    return m_pDebuggerBackend;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CWizardEngine
// 

int CWizardEngine::m_InstanceCount = 0;

CWizardEngine::CWizardEngine(QObject* parent) 
 : CBoxEngine(parent) 
{
    
    m_InstanceCount++;
}

CWizardEngine::~CWizardEngine()
{
    foreach(const SBoxShadow& pShadow, m_Shadows) {
        if (pShadow.pShadow) {
            if (pShadow.iApplyChanges == 2)
                continue; // this is a new added entry keep it
            CSandBoxPtr pBox = pShadow.pShadow.objectCast<CSandBox>();
            if(!pBox.isNull()) pBox->TerminateAll();
            pShadow.pShadow->RemoveSection();
        }
    }
    theAPI->ReloadBoxes(true);

    m_InstanceCount--;
}

bool CWizardEngine::ApplyShadowChanges()
{
    for (auto I = m_Shadows.begin(); I != m_Shadows.end(); ++I) {
        if (I->iApplyChanges != 1)
            continue;

        if (I->pOriginal.isNull()) {
            // This is a new box or template, not a copy, just clear shadow flag
            I->pShadow->DelValue("IsShadow", "y");
            I->iApplyChanges = 2;
            continue;
        }
        
        QList<QPair<QString, QString>> New = I->pShadow->GetIniSection();
        QList<QPair<QString, QString>> Old = I->pOriginal->GetIniSection();

        // discard unchanged
        for (auto I = New.begin(); I != New.end();) {
            auto II = I++;
            for (auto J = Old.begin(); J != Old.end();) {
                auto JJ = J++;
                if (II->first == JJ->first && II->second == JJ->second) {
                    I = New.erase(II);
                    J = Old.erase(JJ);
                    break;
                }
            }
        }

        // apply changed
        foreach(auto & O, Old)
            I->pOriginal->DelValue(O.first, O.second);
        foreach(auto & N, New) {
            if (N.first == "FileRootPath" || N.first == "IsShadow")
                continue; // skip
            if(N.first == "Template" && IsNoAppliedShadow("Template_" + N.second))
                continue; // don't copy non-applied shadow templates
            I->pOriginal->AppendText(N.first, N.second);
        }
    }

    theAPI->CommitIniChanges();

    return true;
}

void CWizardEngine::init()
{
    CBoxEngine::init();

    m_pEngine->globalObject().setProperty("wizard", m_pEngine->newQObject(new JWizardObject(this)));
}

void CWizardEngine::SetState(EState state, const QString& Text)
{
    if (!Text.isEmpty()) {
        if (state == eError)
            m_Report["Error"] = Text;
        else if (state == eFailed)
            m_Report["Failure"] = Text;
    }
    CBoxEngine::SetState(state, Text);
}

QSharedPointer<CSbieIni> CWizardEngine::MakeShadow(const QSharedPointer<CSbieIni>& pIni)
{
    SBoxShadow& pShadow = m_Shadows[pIni->GetName().toLower()];
    if (!pShadow.pShadow) {
        QString ShadowName = pIni->GetName();
        QString Suffix = "_Shadow"; // do not translate must be a valid sandbox name suffix
        ShadowName.truncate(32 - (Suffix.length() + 3)); // BOXNAME_COUNT
        ShadowName = theAPI->MkNewName(ShadowName.append(Suffix));

		QList<QPair<QString, QString>> Settings = pIni->GetIniSection();
		for (QList<QPair<QString, QString>>::iterator I = Settings.begin(); I != Settings.end(); ++I)
			theAPI->SbieIniSet(ShadowName, I->first, I->second, CSbieAPI::eIniInsert, false);

        CSandBoxPtr pBox = pIni.objectCast<CSandBox>();
        if(!pBox.isNull())
            theAPI->SbieIniSet(ShadowName, "FileRootPath", pBox->GetFileRoot(), CSbieAPI::eIniUpdate, false);
        theAPI->SbieIniSet(ShadowName, "IsShadow", "y", CSbieAPI::eIniUpdate, false);

		theAPI->CommitIniChanges();
		theAPI->ReloadBoxes(true);

        pShadow.pOriginal = pIni;
        if (pBox)
            pShadow.pShadow = theAPI->GetBoxByName(ShadowName);
        else
            pShadow.pShadow = QSharedPointer<CSbieIni>(new CSbieIni(ShadowName, theAPI));
    }
    return pShadow.pShadow;
}

void CWizardEngine::AddShadow(const QSharedPointer<CSbieIni>& pIni)
{
    SBoxShadow& pShadow = m_Shadows[pIni->GetName().toLower()];
    if (!pShadow.pShadow) {

        pIni->SetText("IsShadow", "y");

        pShadow.pShadow = pIni;
    }
}

void CWizardEngine::SetApplyShadow(const QString& OriginalName, bool bApply)
{
    auto I = m_Shadows.find(OriginalName.toLower());
    if(I !=  m_Shadows.end())
        I->iApplyChanges = bApply ? 1 : 0;
}

bool CWizardEngine::IsNoAppliedShadow(const QString& OriginalName)
{
    auto I = m_Shadows.find(OriginalName.toLower());
    if (I != m_Shadows.end())
        return I->iApplyChanges == 0;
    return false;
}

void CWizardEngine::OpenSettings(const QString& page)
{
    SetState(eReady);

    CSettingsWindow* pSettingsWnd = new CSettingsWindow();
	connect(pSettingsWnd, &CSettingsWindow::Closed, this, [=]() {
		Continue(false);
	});
    //theGUI->SafeExec(pSettingsWnd);
    pSettingsWnd->showTab(page);
}

void CWizardEngine::OpenOptions(const QString& box, const QString& page)
{
    SetState(eReady);

    CSandBoxPtr pBox = theAPI->GetBoxByName(box);
    if (pBox.isNull()) {
        Continue(false);
        return;
    }
    
    COptionsWindow* pOptionsWnd = new COptionsWindow(pBox, pBox->GetName());
	connect(pOptionsWnd, &COptionsWindow::Closed, this, [=]() {
		Continue(false);
	});
    //theGUI->SafeExec(pOptionsWnd);
    pOptionsWnd->showTab(page);
}