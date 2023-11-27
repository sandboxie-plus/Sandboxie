#include "stdafx.h"
#include "SbieProcess.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\Sandboxie\common\win32_ntddk.h"

#include <winnt.h>

CSbieProcess::CSbieProcess(quint32 ProcessId, class CSandBox* pBox) 
	: CBoxedProcess(ProcessId, pBox) 
{
}

QString CSbieProcess::ImageTypeToStr(quint32 type)
{
	enum {
		UNSPECIFIED = 0,
		SANDBOXIE_RPCSS,
		SANDBOXIE_DCOMLAUNCH,
		SANDBOXIE_CRYPTO,
		SANDBOXIE_WUAU,
		SANDBOXIE_BITS,
		SANDBOXIE_SBIESVC,
		MSI_INSTALLER,
		TRUSTED_INSTALLER,
		WUAUCLT,
		SHELL_EXPLORER,
		INTERNET_EXPLORER,
		MOZILLA_FIREFOX,
		WINDOWS_MEDIA_PLAYER,
		NULLSOFT_WINAMP,
		PANDORA_KMPLAYER,
		WINDOWS_LIVE_MAIL,
		SERVICE_MODEL_REG,
		RUNDLL32,
		DLLHOST,
		DLLHOST_WININET_CACHE,
		WISPTIS,
		GOOGLE_CHROME,
		GOOGLE_UPDATE,
		ACROBAT_READER,
		OFFICE_OUTLOOK,
		OFFICE_EXCEL,
		FLASH_PLAYER_SANDBOX,
		PLUGIN_CONTAINER,
		OTHER_WEB_BROWSER,
		OTHER_MAIL_CLIENT,
		DLL_IMAGE_MOZILLA_THUNDERBIRD
	};

	switch (type)
	{
		case UNSPECIFIED: return tr("");
		case SANDBOXIE_RPCSS: return tr("Sbie RpcSs");
		case SANDBOXIE_DCOMLAUNCH: return tr("Sbie DcomLaunch");
		case SANDBOXIE_CRYPTO: return tr("Sbie Crypto");
		case SANDBOXIE_WUAU: return tr("Sbie WuauServ");
		case SANDBOXIE_BITS: return tr("Sbie BITS");
		case SANDBOXIE_SBIESVC: return tr("Sbie Svc");
		case MSI_INSTALLER: return tr("MSI Installer");
		case TRUSTED_INSTALLER: return tr("Trusted Installer");
		case WUAUCLT: return tr("Windows Update");
		case SHELL_EXPLORER: return tr("Windows Explorer");
		case INTERNET_EXPLORER: return tr("Internet Explorer");
		case MOZILLA_FIREFOX: return tr("Firefox");
		case WINDOWS_MEDIA_PLAYER: return tr("Windows Media Player");
		case NULLSOFT_WINAMP: return tr("Winamp");
		case PANDORA_KMPLAYER: return tr("KMPlayer");
		case WINDOWS_LIVE_MAIL: return tr("Windows Live Mail");
		case SERVICE_MODEL_REG: return tr("Service Model Reg");
		case RUNDLL32: return tr("RunDll32");
		case DLLHOST: return tr("DllHost");
		case DLLHOST_WININET_CACHE: return tr("DllHost");
		case WISPTIS: return tr("Windows Ink Services");
		case GOOGLE_CHROME: return tr("Chromium Based");
		case GOOGLE_UPDATE: return tr("Google Updater");
		case ACROBAT_READER: return tr("Acrobat Reader");
		case OFFICE_OUTLOOK: return tr("MS Outlook");
		case OFFICE_EXCEL: return tr("MS Excel");
		case FLASH_PLAYER_SANDBOX: return tr("Flash Player");
		case PLUGIN_CONTAINER: return tr("Firefox Plugin Container");
		case OTHER_WEB_BROWSER: return tr("Generic Web Browser");
		case OTHER_MAIL_CLIENT: return tr("Generic Mail Client");
		case DLL_IMAGE_MOZILLA_THUNDERBIRD: return tr("Thunderbird");
		default: return tr("");
	}
}

QString CSbieProcess::GetStatusStr() const
{
	QString Status;

	if (m_uTerminated != 0)
		Status = tr("Terminated");
	else if (m_bSuspended)
		Status = tr("Suspended");
	else {
		Status = tr("Running");
		if ((m_ProcessFlags & 0x00000002) != 0) // SBIE_FLAG_FORCED_PROCESS
			Status.prepend(tr("Forced "));
	}

	if (m_ProcessInfo.IsElevated)
		Status += tr(" Elevated");
	if (m_ProcessInfo.IsSystem)
		Status += tr(" as System");

	if(m_SessionId != theAPI->GetSessionID() && m_SessionId != -1)
		Status += tr(" in session %1").arg(m_SessionId);

	quint32 ImageType = GetImageType();
	if (ImageType != -1) {
		QString Type = ImageTypeToStr(ImageType);
		if(!Type.isEmpty())
			Status += tr(" (%1)").arg(Type);
	}

	return Status;
}

//void CSbieProcess::InitProcessInfoImpl(void* ProcessHandle)
//{
//	CBoxedProcess::InitProcessInfoImpl(ProcessHandle);
//}