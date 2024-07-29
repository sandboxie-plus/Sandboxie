#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"
#include "../QtSingleApp/src/qtsingleapplication.h"
#include "../QSbieAPI/SbieUtils.h"
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

	if (QFile::exists(AppDir + "\\Certificate.dat"))
		CSettingsWindow::LoadCertificate(AppDir + "\\Certificate.dat");

	// use AppFolder/PlusData when present, else fallback to AppFolder
	QString ConfDir = AppDir + "\\PlusData";
	if(!QFile::exists(ConfDir))
		ConfDir = AppDir;
	// use a shared setting location when used in a business environment for easier administration
	theConf = new CSettings(ConfDir, "Sandboxie-Plus");

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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

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

	// Context Menu invocations
	int BoxPos = -1;
	for (int i = 0; i < Args.size(); i++) {
		if (Args[i].left(5).compare("/box:", Qt::CaseInsensitive) == 0) {
			BoxPos = i;
			break;
		}
	}
	int DfpPos = Args.indexOf("/disable_force", Qt::CaseInsensitive);
	int AfpPos = Args.indexOf("/add_force", Qt::CaseInsensitive);
	int AOPos = Args.indexOf("/add_open", Qt::CaseInsensitive);

	//Add_Force has the highest priority.
	if (AfpPos != -1) {
		DfpPos = -1;
		BoxPos = -1;
	}
	else if (AOPos != -1)
	{
		DfpPos = -1;
		BoxPos = -1;
	}
		// the first argument wins
	else if (BoxPos != -1 && DfpPos != -1) {
		if (BoxPos < DfpPos) DfpPos = -1;
		else				 BoxPos = -1;
	}
	// run sandboxed
	if (BoxPos != -1) 
	{
		// Note: a escaped command ending with \" will fail and unescape "
		//QString CommandLine;
		//for (int i = BoxPos + 1; i < Args.count(); i++)
		//	CommandLine += "\"" + Args[i] + "\" ";
		//g_PendingMessage = "Run:" + CommandLine.trimmed();
		LPWSTR cmdLine0 = wcsstr(GetCommandLineW(), L"/box:");
		if (!cmdLine0) return -1;
		LPWSTR cmdLine = wcschr(cmdLine0 + 5, L' ');
		if (!cmdLine) return -2;

		if (IsBoxed) {
			ShellExecute(NULL, L"open", cmdLine + 1, NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}

		g_PendingMessage = "Run:" + QString::fromWCharArray(cmdLine + 1);
		g_PendingMessage += "\nFrom:" + QDir::currentPath();

		QString BoxName = QString::fromWCharArray(cmdLine0 + 5, cmdLine - (cmdLine0 + 5));
		if(BoxName != "__ask__") 
			g_PendingMessage += "\nIn:" + BoxName;
	}
	// run un sandboxed
	if (DfpPos != -1)
	{
		LPWSTR cmdLine0 = wcsstr(GetCommandLineW(), L"/disable_force");
		if (!cmdLine0) return -1;
		LPWSTR cmdLine = cmdLine0 + 14;

		if (IsBoxed) {
			ShellExecute(NULL, L"open", cmdLine + 1, NULL, NULL, SW_SHOWNORMAL);
			return 0;
		}

		g_PendingMessage = "Run:" + QString::fromWCharArray(cmdLine + 1);
		g_PendingMessage += "\nFrom:" + QDir::currentPath();

		g_PendingMessage += "\nIn:*DFP*";
	}
	if (AfpPos != -1) {
		LPWSTR cmdLine0 = wcsstr(GetCommandLineW(), L"/add_force");
		if (!cmdLine0) return -1;
		LPWSTR cmdLine = cmdLine0 + 10;
		g_PendingMessage = "AddForce:" + QString::fromWCharArray(cmdLine + 1);
	}
	if (AOPos != -1) {
		LPWSTR cmdLine0 = wcsstr(GetCommandLineW(), L"/add_open");
		if (!cmdLine0) return -1;
		LPWSTR cmdLine = cmdLine0 + 9;
		g_PendingMessage = "AddOpen:" + QString::fromWCharArray(cmdLine + 1);
	}
	
	if (IsBoxed) {
		QMessageBox::critical(NULL, "Sandboxie-Plus", CSandMan::tr("Sandboxie Manager can not be run sandboxed!"));
		return -1;
	}

	if (!g_PendingMessage.isEmpty()) {
		if(app.sendMessage(g_PendingMessage))
			return 0;
		app.disableSingleApp(); // we start to do one job and exit, don't interfere with starting a regular instance
	}
	else {
		if (app.arguments().contains("-autorun") && app.isRunning())
			return 0;
		if (app.sendMessage("ShowWnd"))
			return 0;
	}

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();

	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)), Qt::QueuedConnection);

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}
