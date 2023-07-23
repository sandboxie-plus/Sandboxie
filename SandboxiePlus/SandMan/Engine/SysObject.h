#pragma once
#include "BoxEngine.h"

class JSysObject : public QObject
{
    Q_OBJECT
public:
    JSysObject(CBoxEngine* pEngine);

    Q_INVOKABLE void log(const QString& line);
    Q_INVOKABLE void sleep(qint64 ms);

    // FS
    Q_INVOKABLE QJSValue listDir(const QString& Path, const QStringList& filter = QStringList(), bool bSubDirs = false);
    Q_INVOKABLE QJSValue mkDir(const QString& Path);
    Q_INVOKABLE QJSValue remove(const QString& Path);
    Q_INVOKABLE QJSValue exists(const QString& Path);
    Q_INVOKABLE QJSValue readFile(const QString& Path, quint64 pos = 0, quint64 length = -1);
    Q_INVOKABLE QJSValue writeFile(const QString& Path, const QByteArray& Data, quint64 pos = -1);
    Q_INVOKABLE QJSValue getFileInfo(const QString& Path);

    
    // REG
    Q_INVOKABLE QJSValue listRegKey(const QString& Key);
    Q_INVOKABLE QJSValue setRegValue(const QString& Key, const QString& Name, const QVariant& Value, const QString& Type = QString());
    Q_INVOKABLE QJSValue getRegValue(const QString& Key, const QString& Name);
    Q_INVOKABLE QJSValue removeRegKey(const QString& Key);
    Q_INVOKABLE QJSValue removeRegValue(const QString& Key, const QString& Name);
   
    // SYS
    Q_INVOKABLE QJSValue execute(const QString& Path, const QVariant& Arguments, const QVariantMap& Options = QVariantMap());
    Q_INVOKABLE QJSValue expand(const QString& name);

    // OS
    static QVariantMap GetOSVersion();
    Q_INVOKABLE QJSValue version();
    Q_INVOKABLE QJSValue language();

    // 
    Q_INVOKABLE void resetData();

    Q_INVOKABLE QJSValue enumClasses();
    Q_INVOKABLE QJSValue enumServices();
    Q_INVOKABLE QJSValue enumProducts();
    Q_INVOKABLE QJSValue enumObjects();
    Q_INVOKABLE QJSValue enumUpdates(); // this can take quite a while

    Q_INVOKABLE QJSValue expandPath(const QString& path);
    Q_INVOKABLE QJSValue checkFile(const QString& value);
    Q_INVOKABLE QJSValue checkRegKey(const QString& value);
    Q_INVOKABLE QJSValue checkClasses(const QString& value);
    Q_INVOKABLE QJSValue checkServices(const QString& value);
    Q_INVOKABLE QJSValue checkProducts(const QString& value);
    Q_INVOKABLE QJSValue checkObjects(const QString& value);
    Q_INVOKABLE QJSValue checkUpdates(const QString& value);


protected:

    CBoxEngine* m_pEngine;
};
