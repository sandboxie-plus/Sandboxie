#include "stdafx.h"
#include "SbieObject.h"
#include "../SandMan.h"
#include "../Views/SbieView.h"
#include "../Windows/SettingsWindow.h"
#include "../QSbieAPI/SbieUtils.h"

CSbieObject::CSbieObject(QObject* parent) 
	: QObject(parent) 
{ 
	m_TraceStarted = false; 
}

CSbieObject::~CSbieObject()
{
	if(m_TraceStarted)
		theAPI->EnableMonitor(false);
}

QJSValue JSbieObject::getVersion() 
{
    return CSandMan::GetVersion();
}

CBoxObject* CSbieObject::GetBox(const QString& Name)
{
	if (!theAPI->IsConnected())
		return NULL;

	CSandBoxPtr pBox = theAPI->GetBoxByName(Name);
	if (!pBox) 
		return NULL;
	CBoxEngine* pEngine = qobject_cast<CBoxEngine*>(parent());
	if(pEngine) 
		emit pEngine->BoxUsed(pBox);
	if (CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent())) {
		QSharedPointer<CSbieIni> pShadow = pEngine->MakeShadow(pBox);
		return new CSBoxObject(pShadow, pBox->GetName(), parent());
	}
	return new CBoxObject(pBox, parent());
}

QStringList CSbieObject::ListBoxes()
{
	QStringList List;
	QMap<QString, CSandBoxPtr> Boxes = theAPI->GetAllBoxes();
	foreach(const CSandBoxPtr& pBox, Boxes)
		List.append(pBox->GetName());
	return List;
}

CBoxObject* CSbieObject::NewBox(const QString& Name)
{
	if (!theAPI->IsConnected())
		return NULL;

	QString BoxName = theAPI->MkNewName(Name);
	SB_STATUS Status = theAPI->CreateBox(BoxName, true);
	if (Status.IsError())
		return NULL;

	CSandBoxPtr pBox = theAPI->GetBoxByName(BoxName);
	if (!pBox) 
		return NULL;
	CBoxEngine* pEngine = qobject_cast<CBoxEngine*>(parent());
	if(pEngine) 
		emit pEngine->BoxUsed(pBox);
	if (CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent())) {
		pEngine->AddShadow(pBox);
		return new CSBoxObject(pBox, pBox->GetName(), parent());
	}
	return new CBoxObject(pBox, parent());
}

CIniObject* CSbieObject::GetTemplate(const QString& Name)
{
	if (!theAPI->IsConnected())
		return NULL;
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + Name, theAPI));
	if (Name.left(6) == "Local_") {
		if (CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent())) {
			QSharedPointer<CSbieIni> pShadow = pEngine->MakeShadow(pTemplate);
			return new CSTmplObject(pShadow, pTemplate->GetName(), parent());
		}
	}
	return new CIniObject(pTemplate, true, parent());
}

QStringList CSbieObject::ListTemplates()
{
	QStringList List;
	for (int index = 0; ; index++)
	{
		QString Name = theAPI->SbieIniGet2("", "", index, false, true, true);
		if (Name.isNull())
			break;

		if (Name.left(9).compare("Template_", Qt::CaseInsensitive) != 0)
			continue;

		List.append(Name.mid(9));
	}
	return List;
}

CIniObject* CSbieObject::NewTemplate(const QString& Name)
{
	if (!theAPI->IsConnected())
		return NULL;
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + Name, theAPI));
	pTemplate->SetText("Tmpl.Title", Name);
	pTemplate->SetText("Tmpl.Class", "Local");
	if (CWizardEngine* pEngine = qobject_cast<CWizardEngine*>(parent())) {
		pEngine->AddShadow(pTemplate);
		return new CSTmplObject(pTemplate, pTemplate->GetName(), parent());
	}
	return new CIniObject(pTemplate, parent());
}

//CIniObject* CSbieObject::GetSection(const QString& Name)
//{
//	if (!theAPI->IsConnected())
//		return NULL;
//	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni(Name, theAPI));
//	return pTemplate ? new CIniObject(pTemplate, parent()) : NULL;
//}

CIniObject* CSbieObject::GetGlobal()
{
	if (!theAPI->IsConnected())
		return NULL;
	return new CIniObject(theAPI->GetGlobalSettings(), parent());
}

CIniObject* CSbieObject::GetConf()
{
	return new CIniObject(NULL, parent());
}


bool CSbieObject::SetupTrace(const QVariantMap& Options)
{
	if (theAPI->IsMonitoring())
		return SB_OK; // already started
	m_TraceStarted = true;
	SB_STATUS Status = theAPI->EnableMonitor(true);
	return !Status.IsError();
}

QVariantList CSbieObject::ReadTrace(const QVariantMap& Options)
{
	int Start = Options["start"].toInt();
	int Count = Options["count"].toInt();

	quint32 FilterPid = Options["pid"].toUInt();
	quint32 FilterTid = Options["tid"].toUInt();
	//QList<quint32> FilterTypes;
	void* pCurrentBox = Options.contains("box") ? theAPI->GetBoxByName(Options["box"].toString()).data() : 0;

	QVariantList List;

	const QVector<CTraceEntryPtr> &ResourceLog = theAPI->GetTrace();
	for (int i = Start; i < ResourceLog.count() && (!Count || Count > ResourceLog.count()); i++)
	{
		const CTraceEntryPtr& pEntry = ResourceLog.at(i);

		if (pCurrentBox != NULL && pCurrentBox != pEntry->GetBoxPtr())
			continue;

		if (FilterPid != 0 && FilterPid != pEntry->GetProcessId())
			continue;

		if (FilterTid != 0 && FilterTid != pEntry->GetThreadId())
			continue;

		//if (!FilterTypes.isEmpty() && !FilterTypes.contains(pEntry->GetType()))
		//	continue;

		QVariantMap Entry;
		Entry["timeStamp"] = pEntry->GetTimeStamp();
		Entry["process"] = pEntry->GetProcessName();
		Entry["pid"] = pEntry->GetProcessId();
		Entry["tid"] = pEntry->GetThreadId();
		Entry["type"] = pEntry->GetTypeStr();
		Entry["status"] = pEntry->GetStautsStr();
		Entry["name"] = pEntry->GetName();
		Entry["message"] = pEntry->GetMessage();
		List.append(Entry);
	}

	return List;
}

void CSbieObject::CleanUp(const QVariantMap& Options)
{
	theAPI->UpdateProcesses(0, theGUI->ShowAllSessions());
}

void CSbieObject::ShellInstall(const QVariantMap& Options)
{
	CSettingsWindow::AddContextMenu(Options["legacy"].toBool());
	if (Options["runUnBoxed"].toBool()) {
		CSbieUtils::AddContextMenu2(QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe",
			tr("Run &Un-Sandboxed"),
			QApplication::applicationDirPath().replace("/", "\\") + "\\Start.exe");
	}
}

void CSbieObject::ShellRemove()
{
	CSettingsWindow::RemoveContextMenu();
	CSbieUtils::RemoveContextMenu2();
}

void CSbieObject::LogMessage(const QVariant& Message, bool bNotify)
{
	if (Message.type() == QVariant::Map) {
		QVariantMap Data = Message.toMap();
		theGUI->OnLogSbieMessage(Data["sbiemsg"].toInt(), Data["params"].toStringList(), 4);
	} else
		theGUI->OnLogMessage(Message.toString(), bNotify);
}

bool JSbieObject::testFeature(const QString& name)
{
	return theAPI->GetFeatureStr().contains(name, Qt::CaseInsensitive);
}