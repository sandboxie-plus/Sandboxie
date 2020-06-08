#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"
#include "../QtSingleApp/src/qtsingleapplication.h"
#include "../QSbieAPI/SbieUtils.h"

CSettings* theConf = NULL;

int main(int argc, char *argv[])
{
	QtSingleApplication app(argc, argv);

	SB_STATUS Status = CSbieUtils::DoAssist();
	if (Status.GetStatus()) {
		app.sendMessage("Status:" + Status.GetText());
		return 0;
	}
	
	if (app.sendMessage("ShowWnd"))
		return 0;

	theConf = new CSettings("Sandboxie-Plus");

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();
	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)));

	pWnd->show();

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}
