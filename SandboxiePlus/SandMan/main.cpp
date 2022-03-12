#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"
#include "../QtSingleApp/src/qtsingleapplication.h"
#include "../QSbieAPI/SbieUtils.h"
//#include "../MiscHelpers/Common/qRC4.h"
#include "../MiscHelpers/Common/Common.h"
#include <windows.h>
#include "./Windows/SettingsWindow.h"

CSettings* theConf = NULL;

QString g_PendingMessage;

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
	//SetProcessDPIAware();
#endif // Q_OS_WIN 
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 
	//QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

	QtSingleApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	//InitConsole(false);

	bool IsBoxed = GetModuleHandle(L"SbieDll.dll") != NULL;

	if (!IsBoxed) {
		SB_STATUS Status = CSbieUtils::DoAssist();
		if (Status.GetStatus()) {
			if (Status.GetStatus() != ERROR_OK)
				return Status.GetStatus();
			return 0;
		}
	}

	QStringList Args = QCoreApplication::arguments();
	
	int CmdPos = Args.indexOf("-open_reg", Qt::CaseInsensitive);
	if (CmdPos != -1) {
		if (Args.count() > CmdPos + 2) {
			QProcess::startDetached(Args.at(CmdPos + 2));
			QThread::msleep(1000);
		}
		ShellOpenRegKey(Args.at(CmdPos + 1));
		return 0;
	}

	CmdPos = Args.indexOf("-op", Qt::CaseInsensitive);
	if (CmdPos != -1) {
		QString Op;
		if (Args.count() > CmdPos)
			Op = Args.at(CmdPos + 1);
		g_PendingMessage = "Op:" + Op;
	}

	CmdPos = Args.indexOf("/box:__ask__", Qt::CaseInsensitive);
	if (CmdPos != -1) {
		// Note: a escaped command ending with \" will fail and unescape "
		//QString CommandLine;
		//for (int i = CmdPos + 1; i < Args.count(); i++)
		//	CommandLine += "\"" + Args[i] + "\" ";
		//g_PendingMessage = "Run:" + CommandLine.trimmed();
		LPWSTR ChildCmdLine = wcsstr(GetCommandLineW(), L"/box:__ask__") + 13;

		if (IsBoxed) {
			ShellExecute(NULL, L"open", ChildCmdLine, NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}

		g_PendingMessage = "Run:" + QString::fromWCharArray(ChildCmdLine);
		g_PendingMessage += "\nFrom:" + QDir::currentPath();
	}

	if (IsBoxed) {
		QMessageBox::critical(NULL, "Sandboxie-Plus", CSandMan::tr("Sandboxie Manager can not be run sandboxed!"));
		return -1;
	}

	if (!g_PendingMessage.isEmpty()) {
		if(app.sendMessage(g_PendingMessage))
			return 0;
		app.disableSingleApp(); // we start to do one job and exit, don't interfear with starting a regular instance
	}
	else if (app.sendMessage("ShowWnd"))
		return 0;


	if (QFile::exists(QCoreApplication::applicationDirPath() + "\\Certificate.dat")) {
		CSettingsWindow::LoadCertificate();
		g_CertInfo.business = GetArguments(g_Certificate, L'\n', L':').value("TYPE").toUpper().contains("BUSINESS");
	}

	// use a shared setting location when used in a business environment for easier administration
	theConf = new CSettings("Sandboxie-Plus", g_CertInfo.business);

#ifndef _DEBUG
	InitMiniDumpWriter(QString("SandMan-v%1").arg(CSandMan::GetVersion()).toStdWString().c_str() , QString(theConf->GetConfigDir()).replace("/", "\\").toStdWString().c_str());
#endif

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();

	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)));

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}

/*HANDLE hServerPipe = CreateFileW(L"\\\\.\\pipe\\qtsingleapp-sandma-ca4a-1", GENERIC_ALL, 0, NULL, OPEN_EXISTING, 0, NULL);
if (hServerPipe != INVALID_HANDLE_VALUE) {
	DWORD lenWritten;
    WriteFile(hServerPipe, "test", 4, &lenWritten, NULL)

    CloseHandle(hServerPipe);
}*/