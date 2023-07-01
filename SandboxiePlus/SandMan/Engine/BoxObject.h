#pragma once
#include "IniObject.h"

class CBoxObject : public CIniObject
{
    Q_OBJECT
public:
    CBoxObject(const QSharedPointer<CSbieIni>& pBox, QObject* parent) 
        : CIniObject(pBox, parent) {}

public slots:

    QString         GetFileRoot() { return m_pIni.objectCast<CSandBox>()->GetFileRoot(); }
    QString         GetRegRoot() { return m_pIni.objectCast<CSandBox>()->GetRegRoot(); }

    quint32         StartTask(const QString& Command, const QVariantMap& Options);
    QVariantList    ListTasks();
    bool            StopTask(quint32 pid);
    bool            Terminate();
    QVariantMap     TaskInfo(quint32 pid);

    bool            DeleteContent();

    bool            RemoveSandbox();


    bool            MakeShortcut(const QString& Target, const QVariantMap& Options);

protected:
    QVariantMap     TaskInfo(const CBoxedProcessPtr& pProcess);

};


class CSBoxObject : public CBoxObject
{
    Q_OBJECT
public:
    CSBoxObject(const QSharedPointer<CSbieIni>& pBox, const QString& OriginalName, QObject* parent)
        : CBoxObject(pBox, parent), m_OriginalName(OriginalName) { }

public slots:

    void            ApplyChanges(bool bApply);

protected:
    QString         m_OriginalName;
};

//////////////////////////////////////////////////////////////////////////////////////////
// JBoxObject
// 

class JBoxObject : public JIniObject
{
    Q_OBJECT
public:
    JBoxObject(CBoxObject* pObject, CBoxEngine* pEngine)
        : JIniObject(pObject, pEngine) {}
    ~JBoxObject() { QMetaObject::invokeMethod(m_pObject, "deleteLater"); }

    Q_INVOKABLE QJSValue getFileRoot(){
        QString Text;
        QMetaObject::invokeMethod(m_pObject, "GetFileRoot", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, Text));
        return Text;
    }
    Q_INVOKABLE QJSValue getRegRoot(){
        QString Text;
        QMetaObject::invokeMethod(m_pObject, "GetRegRoot", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, Text));
        return Text;
    }

    Q_INVOKABLE QJSValue startTask(const QString& Command, const QVariantMap& Options = QVariantMap()){
        quint32 pid;
        QMetaObject::invokeMethod(m_pObject, "StartTask", Qt::BlockingQueuedConnection, Q_RETURN_ARG(quint32, pid), Q_ARG(QString, Command), Q_ARG(const QVariantMap&, Options));
        return pid;
    }
    Q_INVOKABLE QJSValue listTasks(){
        QVariantList List;
        QMetaObject::invokeMethod(m_pObject, "ListTasks", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariantList, List));
        return m_pEngine->m_pEngine->toScriptValue(List);
    }
    Q_INVOKABLE QJSValue stopTask(quint32 pid){
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "StopTask", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_RETURN_ARG(quint32, pid));
        return Success;
    }
    Q_INVOKABLE QJSValue terminate(){
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "Terminate", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success));
        return Success;
    }
    Q_INVOKABLE QJSValue taskInfo(quint32 pid) {
        QVariantMap Info;
        QMetaObject::invokeMethod(m_pObject, "TaskInfo", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariantMap, Info));
        return m_pEngine->m_pEngine->toScriptValue(Info);
    }

    Q_INVOKABLE QJSValue deleteContent() {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "DeleteContent", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success));
        return Success;
    }

    Q_INVOKABLE QJSValue removeSandbox() {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "RemoveSandbox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success));
        return Success;
    }


    Q_INVOKABLE QJSValue makeShortcut(const QString& Target, const QVariantMap& Options = QVariantMap()) {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "MakeShortcut", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(QString, Target), Q_ARG(const QVariantMap&, Options));
        return Success;
    }

};

//////////////////////////////////////////////////////////////////////////////////////////
// JSBoxObject - Box Shadow Object
// 

class JSBoxObject : public JBoxObject
{
    Q_OBJECT
public:
    JSBoxObject(CBoxObject* pObject, CBoxEngine* pEngine)
        : JBoxObject(pObject, pEngine) {}

    Q_INVOKABLE void applyChanges(bool bApply = true) {
        QMetaObject::invokeMethod(m_pObject, "ApplyChanges", Qt::BlockingQueuedConnection, Q_ARG(bool, bApply));
    }
};
