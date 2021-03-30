#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"
#include "../QtSingleApp/src/qtsingleapplication.h"
#include "../QSbieAPI/SbieUtils.h"
//#include "../MiscHelpers/Common/qRC4.h"
#include "../MiscHelpers/Common/Common.h"
#include <windows.h>

CSettings* theConf = NULL;

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
	SetProcessDPIAware();
#endif // Q_OS_WIN 

	//QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 
	//QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

	QtSingleApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	//InitConsole(false);

	SB_STATUS Status = CSbieUtils::DoAssist();
	if (Status.GetStatus()) {
		if(Status.GetStatus() == ERROR_OK) app.sendMessage("Status:OK");
		else app.sendMessage("Status:" + CSandMan::FormatError(Status)); // todo: localization
		return 0;
	}
	
	if (app.sendMessage("ShowWnd"))
		return 0;

	theConf = new CSettings("Sandboxie-Plus");

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();
	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)));

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}
