#include "stdafx.h"
#include "SandMan.h"
#include <QtWidgets/QApplication>
#include "../QSbieAPI/SbieAPI.h"
#include "../QtSingleApp/src/qtsingleapplication.h"
#include "../QSbieAPI/SbieUtils.h"
#include "../MiscHelpers/Common/qRC4.h"
#include "../MiscHelpers/Common/Common.h"
#include <windows.h>

CSettings* theConf = NULL;

void PackDriver();
void UnPackDrivers();

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
	SetProcessDPIAware();
#endif // Q_OS_WIN 

	//QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 
	//QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

	QtSingleApplication app(argc, argv);

	//InitConsole(false);
	if (app.arguments().contains("-rc4"))
	{
		PackDriver();
		return 0;
	}

	SB_STATUS Status = CSbieUtils::DoAssist();
	if (Status.GetStatus()) {
		app.sendMessage("Status:" + Status.GetText());
		return 0;
	}
	
	if (app.sendMessage("ShowWnd"))
		return 0;

	theConf = new CSettings("Sandboxie-Plus");

	UnPackDrivers();

	//QThreadPool::globalInstance()->setMaxThreadCount(theConf->GetInt("Options/MaxThreadPool", 10));

	CSandMan* pWnd = new CSandMan();
	QObject::connect(&app, SIGNAL(messageReceived(const QString&)), pWnd, SLOT(OnMessage(const QString&)));

	int ret =  app.exec();

	delete pWnd;

	delete theConf;
	theConf = NULL;

	return ret;
}

bool TransformFile(const QString& InName, const QString& OutName, const QString& Key = "default_key")
{
	QFile InFile(InName);
	QFile OutFile(OutName);
	if (InFile.open(QIODevice::ReadOnly))
	{
		if (OutFile.open(QIODevice::WriteOnly))
		{
			rc4_sbox_s sbox;
			rc4_init(&sbox, Key.toLatin1());
			OutFile.write(rc4_transform(&sbox, InFile.readAll()));
			OutFile.flush();
			return true;
		}
	}
	return false;
}

bool TestFile(const QString& OutName)
{
	QThread::sleep(3);
	return QFile::exists(OutName);
}

void PackDriver()
{
	int pos = QCoreApplication::arguments().indexOf("-rc4");
	QString InName = QCoreApplication::arguments().at(pos + 1);
	QString OutName = InName;
	if (OutName.right(4) == ".rc4")
		OutName.truncate(OutName.length() - 4);
	else
		OutName.append(".rc4");

	if (TransformFile(InName, OutName))
		printf("success\r\n");
	else
		printf("failed\r\n");
}

bool NotifyCert()
{
	QString Caption = QObject::tr(
		"<h3>Important Notie</h3>"
	);
	QString Text = QObject::tr(
		"<p>Sandboxie requires a driver to operate (SbieDrv.sys), Windows however denies loading a driver file that has not been digitally signed using an expensive <b>EV-Code Signing Certificate</b>. "
		"Such certificates cost upwards of 1000$ per year!</p>"
		"<p>Luckily brave hackers have Leaked a few of these Certificates over the years, one of them was found by the author of this software and put to good use.</p>"
		"<p>Unfortunately, such certificates have been abused by malware authors resulting in many Anti Malware Fools being Lazy and flagging Everything signed with them Wrongfully as Malware. "
		"This Prejudice is Damaging the Open Source Ecosystem as most nonprofit developers can't afford to waste this kind of money every year only to pay that <u>Windows Kernel \"Tax\"</u>.</p>"
		"<p>Therefore, the required driver is provided in an obfuscated form and before use must be unpacked. "
		"<font color='red'>When doing so said said Anti Viruses will complain and attempt to destroy the freshly created file.</font> "
		"Please be aware that this is a <u>false positive</u> caused by the necessary use of a compromitted certificate. "
		"If this happens you will be notified and offered the option to repeat the unpacking operation, for the operation to succeed you will need to <u>configure an aproproate exemption</u>.</p>"

		"<p></p>"
		"<p><b>If you want the project to obtain a genuine EV-Code Signing Certificate please visit the <a href=\"https://xanasoft.com/\">Homepage</a> <font color='red'>and please donate</font></b>.</p>"
		"<p></p>"

		"<p>If you want to proceed with the unpacking of the driver pres YES.</p>"
	);
	QMessageBox *msgBox = new QMessageBox(NULL);
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setWindowTitle("Sandboxie-Plus");
	msgBox->setText(Caption);
	msgBox->setInformativeText(Text);
	msgBox->setStandardButtons(QMessageBox::Yes);
	msgBox->addButton(QMessageBox::No);
	msgBox->setDefaultButton(QMessageBox::Yes);

	QIcon ico(QLatin1String(":/SandMan.png"));
	msgBox->setIconPixmap(ico.pixmap(64, 64));

	return msgBox->exec() == QMessageBox::Yes;
}

void UnPackDrivers()
{
	bool notifyNotOk = false;
	QDir appDir(QApplication::applicationDirPath());
	foreach(const QString& FileName, appDir.entryList(QStringList("*.sys.rc4"), QDir::Files))
	{
		QString InName = QApplication::applicationDirPath() + "/" + FileName;
		QString OutName = InName.mid(0, InName.length() - 4);

		QFileInfo InInfo(InName);
		QFileInfo OutInfo(OutName);
		if (InInfo.size() != OutInfo.size() || InInfo.lastModified() > OutInfo.lastModified())
		{
			if (theConf->GetBool("Options/NotifyUnPack", true)) {
				if (!NotifyCert()) {
					notifyNotOk = true;
					break;
				}
				theConf->SetValue("Options/NotifyUnPack", false);
			}

		retry:
			if (!TransformFile(InName, OutName))
				QMessageBox::warning(NULL, "Sandboxie-Plus", QObject::tr("Failed to decrypt %1 ensure app directory is writable.").arg(FileName));
			else if (!TestFile(OutName))
			{
				if (QMessageBox("Sandboxie-Plus",
					QObject::tr("The decrypted file %1 seam to have been removed. Retry file extraction?").arg(FileName),
					QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::Cancel, QMessageBox::NoButton).exec() == QMessageBox::Yes)
					goto retry;
				notifyNotOk = true;
			}
		}
	}
	if (notifyNotOk)
		QMessageBox::warning(NULL, "Sandboxie-Plus", QObject::tr("Without the Driver Sandboxie-Plus wont be able to run properly."));
}