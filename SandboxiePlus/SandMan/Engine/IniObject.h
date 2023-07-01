#pragma once
#include "BoxEngine.h"
#include "../SbiePlusAPI.h"
#include "../../MiscHelpers/Common/Settings.h"

class CIniObject : public QObject
{
    Q_OBJECT
public:
    CIniObject(const QSharedPointer<CSbieIni>& pIni, bool WithTemplate, QObject* parent) : QObject(parent), m_pIni(pIni), m_WithTemplate(WithTemplate) {}
    CIniObject(const QSharedPointer<CSbieIni>& pIni, QObject* parent) : CIniObject(pIni, false, parent) {}

public slots:

    virtual QString getName() const;

    virtual QVariantMap getIniSection(const QVariantMap& Options = QVariantMap());

    virtual bool setIniValue(const QString& Setting, const QVariant& Value);
    virtual bool insertIniValue(const QString& Setting, const QString& Value);
    virtual bool appendIniValue(const QString& Setting, const QString& Value);
    virtual bool delIniValue(const QString& Setting, const QString& Value = "");
    virtual QStringList getIniValues(const QString& Setting, const QVariantMap& Options = QVariantMap());
    virtual QVariant getIniValue(const QString& Setting, const QVariantMap& Options = QVariantMap());

    virtual bool removeSection();

protected:
    QSharedPointer<CSbieIni> m_pIni;
    bool m_WithTemplate;
};


class CSTmplObject : public CIniObject
{
    Q_OBJECT
public:
    CSTmplObject(const QSharedPointer<CSbieIni>& pIni, const QString& OriginalName, QObject* parent)
        : CIniObject(pIni, parent), m_OriginalName(OriginalName) { }

public slots:

    void            ApplyChanges(bool bApply);

protected:
    QString         m_OriginalName;
};

//////////////////////////////////////////////////////////////////////////////////////////
// JIniObject
// 

class JIniObject : public QObject
{
    Q_OBJECT
public:
    JIniObject(CIniObject* pObject, CBoxEngine* pEngine) 
        : m_pObject(pObject), m_pEngine(pEngine) {}
    ~JIniObject() { QMetaObject::invokeMethod(m_pObject, "deleteLater"); }

    Q_INVOKABLE virtual QJSValue getName() const {
        QString Name;
        QMetaObject::invokeMethod(m_pObject, "getName", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, Name));
        return Name;
    }

    Q_INVOKABLE virtual QJSValue getIniSection(const QVariantMap& Options = QVariantMap()) {
        QVariantMap Data;
        QMetaObject::invokeMethod(m_pObject, "getIniSection", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariantMap, Data), Q_ARG(const QVariantMap&, Options));
        return m_pEngine->m_pEngine->toScriptValue(Data);
    }

    Q_INVOKABLE virtual bool setIniValue(const QString& Setting, const QVariant& Value) {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "setIniValue", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(QString, Setting), Q_ARG(QVariant, Value));
        return Success;
    }
    Q_INVOKABLE virtual bool insertIniValue(const QString& Setting, const QString& Value) {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "insertIniValue", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(QString, Setting), Q_ARG(QString, Value));
        return Success;
    }
    Q_INVOKABLE virtual bool appendIniValue(const QString& Setting, const QString& Value) {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "appendIniValue", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(QString, Setting), Q_ARG(QString, Value));
        return Success;
    }
    Q_INVOKABLE virtual bool delIniValue(const QString& Setting, const QString& Value = "") {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "delIniValue", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success), Q_ARG(QString, Setting), Q_ARG(QString, Value));
        return Success;
    }
    Q_INVOKABLE virtual QJSValue getIniValues(const QString& Setting, const QVariantMap& Options = QVariantMap()) {
        QStringList List;
        QMetaObject::invokeMethod(m_pObject, "getIniValues", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, List), Q_ARG(QString, Setting), Q_ARG(const QVariantMap&, Options));
        return m_pEngine->m_pEngine->toScriptValue(List);
    }
    Q_INVOKABLE virtual QJSValue getIniValue(const QString& Setting, const QVariantMap& Options = QVariantMap()) {
        QVariant Value;
        QMetaObject::invokeMethod(m_pObject, "getIniValue", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariant, Value), Q_ARG(QString, Setting), Q_ARG(const QVariantMap&, Options));
        return m_pEngine->m_pEngine->toScriptValue(Value);
    }

    Q_INVOKABLE virtual bool removeSection() {
        bool Success;
        QMetaObject::invokeMethod(m_pObject, "removeSection", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, Success));
        return Success;
    }

protected:
    CBoxEngine* m_pEngine;
    CIniObject* m_pObject;
};

//////////////////////////////////////////////////////////////////////////////////////////
// JTmplObject
// 

class JTmplObject : public JIniObject
{
    Q_OBJECT
public:
    JTmplObject(CIniObject* pObject, CBoxEngine* pEngine)
        : JIniObject(pObject, pEngine) {}

    Q_INVOKABLE virtual QJSValue getName() const {
        QString Name;
        QMetaObject::invokeMethod(m_pObject, "GetName", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, Name));
        return Name.mid(9);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// JSTmplObject
// 

class JSTmplObject : public JTmplObject
{
    Q_OBJECT
public:
    JSTmplObject(CIniObject* pObject, CBoxEngine* pEngine)
        : JTmplObject(pObject, pEngine) {}

    Q_INVOKABLE void applyChanges(bool bApply = true) {
        QMetaObject::invokeMethod(m_pObject, "ApplyChanges", Qt::BlockingQueuedConnection, Q_ARG(bool, bApply));
    }
};