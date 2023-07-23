#pragma once
#include "BoxEngine.h"
#include "BoxObject.h"

class CSbieObject : public QObject
{
    Q_OBJECT
public:
    CSbieObject(QObject* parent);
    ~CSbieObject();

public slots:
    CBoxObject*     GetBox(const QString& Name);
    QStringList     ListBoxes();
    CBoxObject*     NewBox(const QString& Name);

    CIniObject*     GetTemplate(const QString& Name);
    QStringList     ListTemplates();
    CIniObject*     NewTemplate(const QString& Name);

    //CIniObject*     GetSection(const QString& Name);

    CIniObject*     GetGlobal();
    CIniObject*     GetConf();

    bool            SetupTrace(const QVariantMap& Options);
    QVariantList    ReadTrace(const QVariantMap& Options);

    void            CleanUp(const QVariantMap& Options);

    void            ShellInstall(const QVariantMap& Options);
    void            ShellRemove();

    void            LogMessage(const QVariant& Message, bool bNotify);
protected:
    bool            m_TraceStarted;
};

//////////////////////////////////////////////////////////////////////////////////////////
// JSbieObject
// 

class JSbieObject : public QObject
{
    Q_OBJECT
public:
    JSbieObject(CSbieObject* pObject, CBoxEngine* pEngine)
        : m_pObject(pObject), m_pEngine(pEngine) {}
    ~JSbieObject() { QMetaObject::invokeMethod(m_pObject, "deleteLater"); }

    Q_INVOKABLE QJSValue getVersion();

    static QJSValue makeShadow(QJSValueList args) {
        QString result = "Hello, ";

        // Access arguments passed from JavaScript
        if (args.length() > 0 && args[0].isString()) {
            QString name = args[0].toString();
            result += name;
        }

        return QJSValue(result);
    }

    Q_INVOKABLE QJSValue getBox(const QString& Name) {
	    CBoxObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "GetBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CBoxObject*, pObj), Q_ARG(QString, Name));
        if(!pObj)
            return QJSValue(QJSValue::NullValue);
        if(m_pEngine->inherits("CWizardEngine"))
            return m_pEngine->m_pEngine->newQObject(new JSBoxObject(pObj, m_pEngine));
        return m_pEngine->m_pEngine->newQObject(new JBoxObject(pObj, m_pEngine));

    }
    Q_INVOKABLE QJSValue listBoxes() {
        QStringList List;
        QMetaObject::invokeMethod(m_pObject, "ListBoxes", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, List));
        return m_pEngine->m_pEngine->toScriptValue(List);
    }
    Q_INVOKABLE QJSValue newBox(const QString& Name) {
        CBoxObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "NewBox", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CBoxObject*, pObj), Q_ARG(QString, Name));
	    return pObj ? m_pEngine->m_pEngine->newQObject(new JBoxObject(pObj, m_pEngine)) : QJSValue(QJSValue::NullValue);
    }

    Q_INVOKABLE QJSValue getTemplate(const QString& Name) {
        if (Name.left(6) != "Local_") {
            // shortcut for non user defined templates
            QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + Name, theAPI));
            return m_pEngine->m_pEngine->newQObject(new CIniObject(pTemplate, true, m_pEngine->m_pEngine));
        }
	    CIniObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "GetTemplate", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CIniObject*, pObj), Q_ARG(QString, Name));
        if(!pObj)
            return QJSValue::NullValue;
        if(m_pEngine->inherits("CWizardEngine"))
            return m_pEngine->m_pEngine->newQObject(new JSTmplObject(pObj, m_pEngine));
        return m_pEngine->m_pEngine->newQObject(new JTmplObject(pObj, m_pEngine));
    }
    Q_INVOKABLE QJSValue listTemplates() {
        QStringList List;
        QMetaObject::invokeMethod(m_pObject, "ListTemplates", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, List));
        return m_pEngine->m_pEngine->toScriptValue(List);
    }
    Q_INVOKABLE QJSValue newTemplate(const QString& Name) {
        CIniObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "NewTemplate", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CIniObject*, pObj), Q_ARG(QString, Name));
	    return pObj ? m_pEngine->m_pEngine->newQObject(new JTmplObject(pObj, m_pEngine)) : QJSValue(QJSValue::NullValue);
    }

    //Q_INVOKABLE QJSValue GetSection(const QString& Name) {
    //    CIniObject* pObj = NULL;
	   // QMetaObject::invokeMethod(m_pObject, "getSection", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CIniObject*, pObj), Q_ARG(QString, Name));
	   // return pObj ? m_pEngine->m_pEngine->newQObject(new JIniObject(pObj, m_pEngine)) : QJSValue(QJSValue::NullValue);
    //}

    Q_INVOKABLE QJSValue getGlobal() {
	    CIniObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "GetGlobal", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CIniObject*, pObj));
	    return pObj ? m_pEngine->m_pEngine->newQObject(new JIniObject(pObj, m_pEngine)) : QJSValue(QJSValue::NullValue);
    }
    Q_INVOKABLE QJSValue getConf() {
	    CIniObject* pObj = NULL;
	    QMetaObject::invokeMethod(m_pObject, "GetConf", Qt::BlockingQueuedConnection, Q_RETURN_ARG(CIniObject*, pObj));
	    return pObj ? m_pEngine->m_pEngine->newQObject(new JIniObject(pObj, m_pEngine)) : QJSValue(QJSValue::NullValue);
    }


    Q_INVOKABLE QJSValue setupTrace(const QVariantMap& Options = QVariantMap()) {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "SetupTrace", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(const QVariantMap&, Options));
        return Success;
    }
    Q_INVOKABLE QJSValue readTrace(const QVariantMap& Options = QVariantMap()) {
        QVariantList List;
        QMetaObject::invokeMethod(m_pObject, "ReadTrace", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariantList, List), Q_ARG(const QVariantMap&, Options));
        return m_pEngine->m_pEngine->toScriptValue(List);
    }


    Q_INVOKABLE void cleanUp(const QVariantMap& Options = QVariantMap()) {
        QMetaObject::invokeMethod(m_pObject, "CleanUp", Qt::BlockingQueuedConnection, Q_ARG(const QVariantMap&, Options));
    }


    Q_INVOKABLE void shellInstall(const QVariantMap& Options = QVariantMap()) {
        QMetaObject::invokeMethod(m_pObject, "ShellInstall", Qt::BlockingQueuedConnection, Q_ARG(const QVariantMap&, Options));
    }

    Q_INVOKABLE void shellRemove() {
        QMetaObject::invokeMethod(m_pObject, "ShellRemove", Qt::BlockingQueuedConnection);
    }


    Q_INVOKABLE void logMessage(const QVariant& Message, bool bNotify = false) {
        QMetaObject::invokeMethod(m_pObject, "LogMessage", Qt::BlockingQueuedConnection, Q_ARG(const QVariant&, Message), Q_ARG(bool, bNotify));
    }

    Q_INVOKABLE bool testFeature(const QString& name);
    
protected:
    CBoxEngine* m_pEngine;
    CSbieObject* m_pObject;
};