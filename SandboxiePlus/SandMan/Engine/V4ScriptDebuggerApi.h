/****************************************************************************
**
** Copyright (C) 2023 David Xanatos (xanasoft.com) All rights reserved.
** Contact: XanatosDavid@gmil.com
**
**
** To use the V4ScriptTools in a commercial project, you must obtain
** an appropriate business use license.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
****************************************************************************/

#ifndef CV4SCRIPTDEBUGGERAPI_H
#define CV4SCRIPTDEBUGGERAPI_H

#include <QJSEngine>
#include <QMainWindow>
#include <QLibrary>

class CV4EngineItf
{
public:
    virtual QJSEngine* self() = 0;

    virtual int getScriptCount() const = 0;
    virtual QString getScriptName(qint64 scriptId) const = 0;
    virtual QString getScriptSource(qint64 scriptId) const = 0;
    virtual int getScriptLineNumber(qint64 scriptId) const = 0;
    virtual qint64 getScriptId(const QString& fileName) const = 0;

    //
    // Note: the implementation of this interface must be derived from 
    //  QObject and include the following signals and slots:
    //

//slots:
//    QJSValue evaluateScript(const QString& program, const QString& fileName, int lineNumber = 1);
// 
//signals:
//    void evaluateFinished(const QJSValue& ret);
//    void printTrace(const QString& Message);
//    void invokeDebugger();
};

typedef QObject* (*pNewV4ScriptDebuggerBackend)(CV4EngineItf* engine);
typedef QObject* (*pNewJSScriptDebuggerFrontend)();
typedef QMainWindow* (*pNewJSScriptDebugger)(QObject* frontend, QWidget *parent, Qt::WindowFlags flags);

__forceinline QObject* newV4ScriptDebuggerBackendDynamic(CV4EngineItf* engine)
{
    pNewV4ScriptDebuggerBackend myNewV4ScriptDebuggerBackend = (pNewV4ScriptDebuggerBackend)QLibrary::resolve("V4ScriptDebugger", "newV4ScriptDebuggerBackend");
    if (myNewV4ScriptDebuggerBackend)
        return myNewV4ScriptDebuggerBackend(engine);
    return NULL;
}

__forceinline QObject* newJSScriptDebuggerFrontendDynamic()
{
    pNewJSScriptDebuggerFrontend myNewJSScriptDebuggerFrontend = (pNewJSScriptDebuggerFrontend)QLibrary::resolve("V4ScriptDebugger", "newJSScriptDebuggerFrontend");
    if (myNewJSScriptDebuggerFrontend)
        return myNewJSScriptDebuggerFrontend();
    return NULL;
}

__forceinline QMainWindow* newJSScriptDebuggerDynamic(QObject* frontend, QWidget *parent = nullptr, Qt::WindowFlags flags = {})
{
    pNewJSScriptDebugger myNewJSScriptDebugger = (pNewJSScriptDebugger)QLibrary::resolve("V4ScriptDebugger", "newJSScriptDebugger");
    if (myNewJSScriptDebugger)
        return myNewJSScriptDebugger(frontend, parent, flags);
    return NULL;
}

#endif