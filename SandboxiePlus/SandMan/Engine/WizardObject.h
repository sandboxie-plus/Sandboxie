#pragma once
#include "BoxEngine.h"

class JWizardObject : public QObject
{
    Q_OBJECT
public:
    JWizardObject(CWizardEngine* pEngine) : m_pEngine(pEngine) {}

    /*Q_INVOKABLE QJSValue showMessageBox(const QString& Type, const QString& Text){

        //Type = "info_yes_NO_cancel";

        return "cancel";
    }*/

    Q_INVOKABLE QJSValue showForm(const QJSValue& form, const QString& Text = ""){

        QMutexLocker Locker(&m_pEngine->m_Mutex);
        if (!m_pEngine->TestRunning()) return QJSValue();

        QVariantMap Data;
        Data["type"] = "form";
        Data["form"] = form.toVariant().toList();
        Data["text"] = Text;
        m_pEngine->m_Data = Data;

        m_pEngine->SetState(CBoxEngine::eQuery);
        if (!m_pEngine->WaitLocked()) return QJSValue();

        return m_pEngine->m_pEngine->toScriptValue(m_pEngine->m_Data);
    }

    Q_INVOKABLE void showStatus(const QString& Text, bool bWait = false){

        QMutexLocker Locker(&m_pEngine->m_Mutex);
        if (!m_pEngine->TestRunning()) return;
        
        m_pEngine->SetState(bWait ? CBoxEngine::eReady : CBoxEngine::eRunningAsync, Text);
        if(bWait) m_pEngine->WaitLocked();
    }

    Q_INVOKABLE void setResult(bool bSuccess, const QString& Text = "") {

        QMutexLocker Locker(&m_pEngine->m_Mutex);
        if (!m_pEngine->TestRunning()) return;

        if(bSuccess) m_pEngine->SetState(CBoxEngine::eSuccess);
        else m_pEngine->SetState(CBoxEngine::eFailed, Text);
    }

    Q_INVOKABLE void reportAdd(const QString& name, const QString& value) {
        m_pEngine->m_Report[name] = value;
    }

    Q_INVOKABLE void openSettings(const QString& page) {

        QMutexLocker Locker(&m_pEngine->m_Mutex);
        if (!m_pEngine->TestRunning()) return;

        QMetaObject::invokeMethod(m_pEngine, "OpenSettings", Qt::QueuedConnection, Q_ARG(QString, page));

        m_pEngine->WaitLocked();
    }

    Q_INVOKABLE void openOptions(const QString& box, const QString& page) {

        QMutexLocker Locker(&m_pEngine->m_Mutex);
        if (!m_pEngine->TestRunning()) return;

        QMetaObject::invokeMethod(m_pEngine, "OpenOptions", Qt::QueuedConnection, Q_ARG(QString, box), Q_ARG(QString, page));

        m_pEngine->WaitLocked();
    }

protected:
    CWizardEngine* m_pEngine;
};
