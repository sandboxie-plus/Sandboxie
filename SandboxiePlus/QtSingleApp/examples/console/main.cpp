/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtsinglecoreapplication.h"
#include <QDebug>


void report(const QString& msg)
{
    qDebug("[%i] %s", (int)QCoreApplication::applicationPid(), qPrintable(msg));
}

class MainClass : public QObject
{
    Q_OBJECT
public:
    MainClass()
        : QObject()
        {}

public slots:
    void handleMessage(const QString& message)
        {
            report( "Message received: \"" + message + "\"");
        }
};

int main(int argc, char **argv)
{
    report("Starting up");

    QtSingleCoreApplication app(argc, argv);

    if (app.isRunning()) {
        QString msg(QString("Hi master, I am %1.").arg(QCoreApplication::applicationPid()));
        bool sentok = app.sendMessage(msg, 2000);
        QString rep("Another instance is running, so I will exit.");
        rep += sentok ? " Message sent ok." : " Message sending failed; the other instance may be frozen.";
        report(rep);
        return 0;
    } else {
        report("No other instance is running; so I will.");
        MainClass mainObj;
        QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                         &mainObj, SLOT(handleMessage(const QString&)));
        return app.exec();
    }
}


#include "main.moc"
