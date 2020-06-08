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

#include <qtsingleapplication.h>
#include <QFile>
#include <QMainWindow>
#include <QPrinter>
#include <QPainter>
#include <QTextEdit>
#include <QMdiArea>
#include <QTextStream>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();

public slots:
    void handleMessage(const QString& message);

signals:
    void needToShow();

private:
    QMdiArea *workspace;
};

MainWindow::MainWindow()
{
    workspace = new QMdiArea(this);

    setCentralWidget(workspace);
}

void MainWindow::handleMessage(const QString& message)
{
    enum Action {
	Nothing,
	Open,
	Print
    } action;

    action = Nothing;
    QString filename = message;
    if (message.toLower().startsWith("/print ")) {
	filename = filename.mid(7);
	action = Print;
    } else if (!message.isEmpty()) {
	action = Open;
    }
    if (action == Nothing) {
        emit needToShow();
	return;
    }

    QFile file(filename);
    QString contents;
    if (file.open(QIODevice::ReadOnly))
        contents = file.readAll();
    else
        contents = "[[Error: Could not load file " + filename + "]]";

    QTextEdit *view = new QTextEdit;
    view->setPlainText(contents);

    switch(action) {
    case Print:
	{
	    QPrinter printer;
            view->print(&printer);
            delete view;
        }
	break;

    case Open:
	{
	    workspace->addSubWindow(view);
	    view->setWindowTitle(message);
	    view->show();
            emit needToShow();
	}
	break;
    default:
	break;
    };
}

#include "main.moc"

int main(int argc, char **argv)
{
    QtSingleApplication instance("File loader QtSingleApplication example", argc, argv);
    QString message;
    for (int a = 1; a < argc; ++a) {
	message += argv[a];
	if (a < argc-1)
	    message += " ";
    }

    if (instance.sendMessage(message))
	return 0;

    MainWindow mw;
    mw.handleMessage(message);
    mw.show();

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
		     &mw, SLOT(handleMessage(const QString&)));

    instance.setActivationWindow(&mw, false);
    QObject::connect(&mw, SIGNAL(needToShow()), &instance, SLOT(activateWindow()));

    return instance.exec();
}
