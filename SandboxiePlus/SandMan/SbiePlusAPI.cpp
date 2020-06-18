#include "stdafx.h"
#include "SbiePlusAPI.h"


CSbiePlusAPI::CSbiePlusAPI(QObject* parent) : CSbieAPI(parent)
{

}

CSbiePlusAPI::~CSbiePlusAPI()
{
}

CSandBox* CSbiePlusAPI::NewSandBox(const QString& BoxName, class CSbieAPI* pAPI)
{
	return new CSandBoxPlus(BoxName, pAPI);
}

CBoxedProcess* CSbiePlusAPI::NewBoxedProcess(quint64 ProcessId, class CSandBox* pBox)
{
	return new CBoxedProcess(ProcessId, pBox);
}


///////////////////////////////////////////////////////////////////////////////
// CSandBox
//

CSandBoxPlus::CSandBoxPlus(const QString& BoxName, class CSbieAPI* pAPI) : CSandBox(BoxName, pAPI)
{
	m_bLogApiFound = false;
	m_bIsOpenBox = false;
}

CSandBoxPlus::~CSandBoxPlus()
{
}

void CSandBoxPlus::UpdateDetails()
{
	QStringList List = GetTextList("OpenPipePath");
	m_bLogApiFound = List.contains("\\Device\\NamedPipe\\LogAPI");

	m_bIsOpenBox = GetBool("OpenToken") || GetBool("UnrestrictedToken") || GetBool("UnfilteredToken") || GetBool("OriginalToken");

	CSandBox::UpdateDetails();
}