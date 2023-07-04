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

#ifndef CJSENGINEEXT_H
#define CJSENGINEEXT_H

#include <QObject>
#include <QVariant>

#include "V4ScriptDebuggerApi.h"

class CJSEngineExt : public QJSEngine, public CV4EngineItf
{
    Q_OBJECT
public:
    CJSEngineExt(QObject* parent = NULL);
    ~CJSEngineExt();

    QJSEngine* self() { return this; }

    Q_INVOKABLE QJSValue evaluateScript(const QString& program, const QString& fileName, int lineNumber = 1);

    int getScriptCount() const { return m_Scripts.count(); }
    QString getScriptName(qint64 scriptId) const { if (scriptId < m_Scripts.size()) return m_Scripts[scriptId].Name; return QString(); }
    QString getScriptSource(qint64 scriptId) const { if (scriptId < m_Scripts.size()) return m_Scripts[scriptId].Source; return QString(); }
    int getScriptLineNumber(qint64 scriptId) const { if (scriptId < m_Scripts.size()) return m_Scripts[scriptId].LineNumber; return -1; }
    qint64 getScriptId(const QString& fileName) const { return m_ScriptIDs.value(fileName.toLower()); }

    QString trackScript(const QString& program, const QString& fileName, int lineNumber = 1);

    static CJSEngineExt* getEngineByHandle(void* handle);

signals:
    void evaluateFinished(const QJSValue& ret);
    void printTrace(const QString& Message);
    void invokeDebugger();

protected:
    struct SScript
    {
        QString Name;
        int LineNumber = 0;
        QString Source;
    };
    QList<SScript> m_Scripts;
    QMap<QString, qint64> m_ScriptIDs;

private:
    QJSValue evaluate(const QString& program, const QString& fileName = QString(), int lineNumber = 1) { return QJSValue(); } // don't use this, use evaluateScript instead
};

#endif