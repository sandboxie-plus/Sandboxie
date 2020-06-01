#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"

CSettings* theConf = NULL;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	theConf = new CSettings("Sandboxie-Plus");

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();
	pWnd->show();

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}
