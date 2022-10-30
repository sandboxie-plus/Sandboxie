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
#include "./Wizards/SetupWizard.h"

CSettings* theConf = NULL;

QString g_PendingMessage;

int main(int argc, char *argv[])
{
	srand(QDateTime::currentDateTimeUtc().toSecsSinceEpoch());

	wchar_t szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath));
	*wcsrchr(szPath, L'\\') = L'\0';
	QString AppDir = QString::fromWCharArray(szPath);

	if (QFile::exists(AppDir + "\\Certificate.dat")) {
		CSettingsWindow::LoadCertificate(AppDir + "\\Certificate.dat");
		g_CertInfo.business = GetArguments(g_Certificate, L'\n', L':').value("TYPE").toUpper().contains("BUSINESS");
	}

	// use a shared setting location when used in a business environment for easier administration
	theConf = new CSettings(AppDir, "Sandboxie-Plus", g_CertInfo.business);

#ifndef _DEBUG
	InitMiniDumpWriter(QString("SandMan-v%1").arg(CSandMan::GetVersion()).toStdWString().c_str() , QString(theConf->GetConfigDir()).replace("/", "\\").toStdWString().c_str());
#endif

	// this must be done before we create QApplication
	int DPI = theConf->GetInt("Options/DPIScaling", 1);
	if (DPI == 1) {
		//SetProcessDPIAware();
		//SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		//SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		typedef DPI_AWARENESS_CONTEXT(WINAPI* P_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT dpiContext);
		P_SetThreadDpiAwarenessContext pSetThreadDpiAwarenessContext = (P_SetThreadDpiAwarenessContext)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetThreadDpiAwarenessContext");
		if(pSetThreadDpiAwarenessContext) // not present on windows 7
			pSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		else
			SetProcessDPIAware();
	}
	else if (DPI == 2) {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 
	}
	//else {
	//	QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
	//}


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
	
	int CmdPos = Args.indexOf("/OpenReg", Qt::CaseInsensitive);
	if (CmdPos != -1) {
		if (Args.count() > CmdPos + 2) {
			QProcess::startDetached(Args.at(CmdPos + 2));
			QThread::msleep(1000);
		}
		ShellOpenRegKey(Args.at(CmdPos + 1));
		return 0;
	}

	CmdPos = Args.indexOf("/ShellUninstall", Qt::CaseInsensitive);
	if (CmdPos != -1) {
		CSetupWizard::ShellUninstall();
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
	else {
		if (app.arguments().contains("-autorun") && app.isRunning())
			return 0;
		if (app.sendMessage("ShowWnd"))
			return 0;
	}

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();

	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)));

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}
