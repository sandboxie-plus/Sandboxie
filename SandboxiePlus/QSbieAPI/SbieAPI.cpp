/*
 * 
 * Copyright (c) 2020, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include <QDebug>
#include "SbieAPI.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "SbieDefs.h"

#include "..\..\Sandboxie\common\win32_ntddk.h"

#include "..\..\Sandboxie\core\drv\api_defs.h"

#include "..\..\Sandboxie\core\svc\msgids.h"
#include "..\..\Sandboxie\core\svc\ProcessWire.h"
#include "..\..\Sandboxie\core\svc\sbieiniwire.h"

struct SSbieAPI
{
	SSbieAPI()
	{
		SbieApiHandle = INVALID_HANDLE_VALUE;

		PortHandle = NULL;
		MaxDataLen = 0;
		SizeofPortMsg = 0;
		CallSeqNumber = 0;

		if (!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId))
			sessionId = 0;

		lastMessageNum = 0;
		lastRecordNum = 0;

		SbieMsgDll = NULL;

		SvcLock = 0;
	}
	~SSbieAPI() {
	}

	NTSTATUS IoControl(ULONG64 *parms)
	{
		IO_STATUS_BLOCK IoStatusBlock;
		return NtDeviceIoControlFile(SbieApiHandle, NULL, NULL, NULL, &IoStatusBlock, API_SBIEDRV_CTLCODE, parms, sizeof(ULONG64) * API_NUM_ARGS, NULL, 0);
	}


	HANDLE SbieApiHandle;

	HANDLE PortHandle;
	ULONG MaxDataLen;
	ULONG SizeofPortMsg;
	ULONG CallSeqNumber;

	QString Password;

	ULONG sessionId;

	ULONG lastMessageNum;
	ULONG lastRecordNum;

	HMODULE SbieMsgDll;

	mutable volatile LONG   SvcLock;
	mutable void*			SvcReq;
	mutable void*			SvcRpl;
	mutable SB_STATUS		SvcStatus;
};

#define SVC_OP_STATE_IDLE	0
#define SVC_OP_STATE_PREP	1
#define SVC_OP_STATE_START	2
#define SVC_OP_STATE_EXEC	3
#define SVC_OP_STATE_DONE	4
#define SVC_OP_STATE_EVAL	5

quint64 FILETIME2ms(quint64 fileTime)
{
	if (fileTime < 116444736000000000ULL)
		return 0;
	return (fileTime - 116444736000000000ULL) / 10000ULL;
}

time_t FILETIME2time(quint64 fileTime)
{
	return FILETIME2ms(fileTime) / 1000ULL;
}


CSbieAPI::CSbieAPI(QObject* parent) : QThread(parent)
{
	m = new SSbieAPI();

	m_pGlobalSection = new CSbieIni("GlobalSettings", this, this);

	m_bReloadPending = false;

	connect(&m_IniWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(OnIniChanged(const QString&)));
}

CSbieAPI::~CSbieAPI()
{
	Disconnect();

	delete m;
}

bool CSbieAPI::IsSbieCtrlRunning()
{
	static const WCHAR *SbieCtrlMutexName = SANDBOXIE L"_SingleInstanceMutex_Control";

	HANDLE hSbieCtrlMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SbieCtrlMutexName);
	if (hSbieCtrlMutex) {
		CloseHandle(hSbieCtrlMutex);
		return true;
	}
	return false;
}

bool CSbieAPI::TerminateSbieCtrl()
{
	static const WCHAR *WindowClassName = L"SandboxieControlWndClass";

	HWND hwnd = FindWindow(WindowClassName, NULL);
	if (hwnd) {
		PostMessage(hwnd, WM_QUIT, 0, 0);
	}

	for (int i = 0; i < 10 && hwnd != NULL; i++) {
		QThread::msleep(100);
		hwnd = FindWindow(WindowClassName, NULL);
	}

	return hwnd == NULL;
}

CSandBox* CSbieAPI::NewSandBox(const QString& BoxName, class CSbieAPI* pAPI)
{
	return new CSandBox(BoxName, pAPI);
}

CBoxedProcess* CSbieAPI::NewBoxedProcess(quint64 ProcessId, class CSandBox* pBox)
{
	return new CBoxedProcess(ProcessId, pBox);
}

SB_STATUS CSbieAPI::Connect()
{
	if (IsConnected())
		return SB_OK;

	UNICODE_STRING uni;
	RtlInitUnicodeString(&uni, API_DEVICE_NAME);

	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS status = NtOpenFile(&m->SbieApiHandle, FILE_GENERIC_READ, &objattrs, &IoStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0);

	if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_NO_SUCH_DEVICE)
		status = STATUS_SERVER_DISABLED;

	if (status != STATUS_SUCCESS) {
		m->SbieApiHandle = INVALID_HANDLE_VALUE;
		return SB_ERR("Failed to connect to driver", status);
	}

	UpdateDriveLetters();

	m_SbiePath = GetSbieHome();
	m->SbieMsgDll = LoadLibraryEx((m_SbiePath.toStdWString() + L"\\" SBIEMSG_DLL).c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

	m->lastMessageNum = 0;
	m->lastRecordNum = 0;

	m_bTerminate = false;
	start();

	bool bHome = false;
	m_IniPath = GetIniPath(&bHome);
	qDebug() << "Config file:" << m_IniPath << (bHome ? "(home)" : "(system)");

	emit StatusChanged();
	return SB_OK;
}

SB_STATUS CSbieAPI::Disconnect()
{
	if (!IsConnected())
		return SB_OK;

	m_bTerminate = true;
	if (!wait(10 * 1000))
		terminate();

	if (m->SbieApiHandle != INVALID_HANDLE_VALUE) {
		NtClose(m->SbieApiHandle);
		m->SbieApiHandle = INVALID_HANDLE_VALUE;
	}

	if (m->PortHandle) {
		NtClose(m->PortHandle);
		m->PortHandle = NULL;
	}

	if (m->SbieMsgDll) {
		FreeLibrary(m->SbieMsgDll);
		m->SbieMsgDll = NULL;
	}

	m_SandBoxes.clear();
	m_BoxedProxesses.clear();

	emit StatusChanged();
	return SB_OK;
}

bool CSbieAPI::IsConnected() const
{
	return m->SbieApiHandle != INVALID_HANDLE_VALUE;
}

bool CSbieAPI__IsWow64()
{
	static bool IsWow64 = false;
#ifndef _WIN64
	static bool init = false;
	if (!init)
	{
		ULONG_PTR wow64;
		if (NT_SUCCESS(NtQueryInformationProcess(NtCurrentProcess(), ProcessWow64Information, &wow64, sizeof(ULONG_PTR), NULL))) {
			IsWow64 = !!wow64;
		}
		init = true;
	}
#endif
	return IsWow64;
}

SB_STATUS CSbieAPI__ConnectPort(SSbieAPI* m)
{
	if (m->PortHandle)
		return SB_OK;

	SECURITY_QUALITY_OF_SERVICE QoS;
	QoS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
	QoS.ImpersonationLevel = SecurityImpersonation;
	QoS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
	QoS.EffectiveOnly = TRUE;

	UNICODE_STRING PortName;
	RtlInitUnicodeString(&PortName, SBIESVC_PORT);
	NTSTATUS status = NtConnectPort(&m->PortHandle, &PortName, &QoS, NULL, NULL, &m->MaxDataLen, NULL, NULL);
	if (!NT_SUCCESS(status))
		return SB_ERR(status); // 2203

	// Function associate PortHandle with thread, and sends LPC_TERMINATION_MESSAGE to specified port immediatelly after call NtTerminateThread.
	//NtRegisterThreadTerminatePort(m->PortHandle);

	m->SizeofPortMsg = sizeof(PORT_MESSAGE);
	if (CSbieAPI__IsWow64())
		m->SizeofPortMsg += sizeof(ULONG) * 4;
	m->MaxDataLen -= m->SizeofPortMsg;

	return SB_OK;
}

SB_STATUS CSbieAPI__CallServer(SSbieAPI* m, MSG_HEADER* req, MSG_HEADER** prpl)
{
	if (!m->PortHandle) {
		SB_STATUS Status = CSbieAPI__ConnectPort(m);
		if (!Status)
			return Status;
	}

	UCHAR RequestBuff[MAX_PORTMSG_LENGTH];
	PORT_MESSAGE* ReqHeader = (PORT_MESSAGE*)RequestBuff;
	UCHAR* ReqData = RequestBuff + m->SizeofPortMsg;

	UCHAR ResponseBuff[MAX_PORTMSG_LENGTH];
	PORT_MESSAGE* ResHeader = (PORT_MESSAGE*)ResponseBuff;
	UCHAR* ResData = ResponseBuff + m->SizeofPortMsg;

	UCHAR CurSeqNumber = (UCHAR)m->CallSeqNumber++;

	// Send the request in chunks
	UCHAR* Buffer = (UCHAR*)req;
	ULONG BuffLen = req->length;
	while (BuffLen)
	{
		ULONG send_len = BuffLen > m->MaxDataLen ? m->MaxDataLen : BuffLen;

		// set header
		memset(ReqHeader, 0, m->SizeofPortMsg);
		ReqHeader->u1.s1.DataLength = (USHORT)send_len;
		ReqHeader->u1.s1.TotalLength = (USHORT)(m->SizeofPortMsg + send_len);

		// set req data
		memcpy(ReqData, Buffer, send_len);

		// use highest byte of the length as sequence field
		if (Buffer == (UCHAR*)req)
			ReqData[3] = CurSeqNumber;

		// advance position
		Buffer += send_len;
		BuffLen -= send_len;

		NTSTATUS status = NtRequestWaitReplyPort(m->PortHandle, (PORT_MESSAGE*)RequestBuff, (PORT_MESSAGE*)ResponseBuff);

		if (!NT_SUCCESS(status))
		{
			NtClose(m->PortHandle);
			m->PortHandle = NULL;
			return SB_ERR(CSbieAPI::tr("request %1").arg(status, 8, 16), status); // 2203
		}

		if (BuffLen && ResHeader->u1.s1.DataLength)
			return SB_ERR(CSbieAPI::tr("early reply")); // 2203
	}

	// the last call to NtRequestWaitReplyPort should return the first chunk of the reply
	if (ResHeader->u1.s1.DataLength >= sizeof(MSG_HEADER))
	{
		if (ResData[3] != CurSeqNumber)
			return SB_ERR(CSbieAPI::tr("mismatched reply")); // 2203

		// clear highest byte of the size filed
		ResData[3] = 0;
		BuffLen = ((MSG_HEADER*)ResData)->length;
	}
	else
		BuffLen = 0;
	if (BuffLen == 0)
		return SB_ERR(CSbieAPI::tr("null reply (msg %1 len %2)").arg(req->msgid, 8, 16).arg(req->length)); // 2203

	// read remining chunks
	MSG_HEADER*& rpl = *prpl;
	rpl = (MSG_HEADER*)malloc(BuffLen);
	Buffer = (UCHAR*)rpl;
	for (;;)
	{
		NTSTATUS status;
		if (ResHeader->u1.s1.DataLength > BuffLen)
			status = STATUS_PORT_MESSAGE_TOO_LONG;
		else
		{
			// get data
			memcpy(Buffer, ResData, ResHeader->u1.s1.DataLength);

			// adcance position
			Buffer += ResHeader->u1.s1.DataLength;
			BuffLen -= ResHeader->u1.s1.DataLength;

			// are we done yet?
			if (!BuffLen)
				break;

			// set header
			memset(ReqHeader, 0, m->SizeofPortMsg);
			ReqHeader->u1.s1.TotalLength = (USHORT)m->SizeofPortMsg;

			status = NtRequestWaitReplyPort(m->PortHandle, (PORT_MESSAGE*)RequestBuff, (PORT_MESSAGE*)ResponseBuff);
		}

		if (!NT_SUCCESS(status))
		{
			free(rpl);
			rpl = NULL;

			NtClose(m->PortHandle);
			m->PortHandle = NULL;
			return SB_ERR(CSbieAPI::tr("reply %1").arg(status, 8, 16), status); // 2203
		}
	}

	return SB_OK;
}

SB_STATUS CSbieAPI::CallServer(void* req, void* rpl) const
{
	//
	// Note: Once we open a port to the server from a threat the service will remember it we can't reconnect after disconnection
	//			So for every new connection we need a new threat, we achive this by letting our monitor threat issue all requests
	//

	while (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_PREP, SVC_OP_STATE_IDLE) != SVC_OP_STATE_IDLE)
		QThread::msleep(1);

	m->SvcReq = req;
	m->SvcRpl = rpl;
	m->SvcStatus = SB_OK;

	InterlockedExchange(&m->SvcLock, SVC_OP_STATE_START);

	// Wake threat imminetly
	m_ThreadMutex.lock();
	m_ThreadWait.wakeAll();
	m_ThreadMutex.unlock();

	// worker: SVC_OP_STATE_START -> SVC_OP_STATE_EXEC -> SVC_OP_STATE_DONE

	while (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_EVAL, SVC_OP_STATE_DONE) != SVC_OP_STATE_DONE)
		QThread::usleep(100);

	m->SvcReq = NULL;
	m->SvcRpl = NULL;
	SB_STATUS Status = m->SvcStatus;

	InterlockedExchange(&m->SvcLock, SVC_OP_STATE_IDLE);

	return Status;

	//return CSbieAPI__CallServer(m, (MSG_HEADER*)req, (MSG_HEADER**)rpl);
}

void CSbieAPI::run()
{
	int Idle = 0;

	while (!m_bTerminate)
	{
		int Done = 0;

		if (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_EXEC, SVC_OP_STATE_START) == SVC_OP_STATE_START)
		{
			m->SvcStatus = CSbieAPI__CallServer(m, (MSG_HEADER*)m->SvcReq, (MSG_HEADER**)m->SvcRpl);

			InterlockedExchange(&m->SvcLock, SVC_OP_STATE_DONE);

			Done++;
		}

		if (GetLog()) // this emits sbie message events if there are any
			Done++;

		if (GetMonitor()) {
			Done++;
			//QMetaObject::invokeMethod(this, "OnMonitorEntry", Qt::AutoConnection, Q_ARG(quint64, ProcessId), Q_ARG(quint32, Type), Q_ARG(const QString&, Value));
		}

		if (Done != 0)
			Idle = 0;
		else
		{
			if(Idle < 5)
				Idle++;

			m_ThreadMutex.lock();
			m_ThreadWait.wait(&m_ThreadMutex, 10 * Idle);
			m_ThreadMutex.unlock();
		}
	}
}

/*void CSbieAPI::OnMonitorEntry(quint64 ProcessId, quint32 Type, const QString& Value)
{
	QMap<quint64, CBoxedProcessPtr>::iterator I = m_BoxedProxesses.find(ProcessId);
	if (I == m_BoxedProxesses.end())
	{
		UpdateProcesses(true);
		I = m_BoxedProxesses.find(ProcessId);
	}
	if (I == m_BoxedProxesses.end())
		return;

	I.value()->AddResourceEntry(Type, Value);
}*/

QString CSbieAPI::GetVersion()
{
	WCHAR out_version[16];

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_GET_VERSION_ARGS *args = (API_GET_VERSION_ARGS*)parms;

    memset(parms, 0, sizeof(parms));
    args->func_code = API_GET_VERSION;
    args->string.val = out_version;

    if (! NT_SUCCESS(m->IoControl(parms)))
        wcscpy(out_version, L"unknown");

	return QString::fromWCharArray(out_version);
}

SB_STATUS CSbieAPI::TakeOver()
{
	__declspec(align(8)) ULONG64 ResultValue;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_SESSION_LEADER_ARGS *args = (API_SESSION_LEADER_ARGS *)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_SESSION_LEADER;
	args->token_handle.val64 = 0;
	args->process_id.val64 = 0;
	
	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::WatchIni(bool bEnable)
{
	if (bEnable)
		m_IniWatcher.addPath(m_IniPath);
	else
		m_IniWatcher.removePath(m_IniPath);
	return SB_OK;
}

void CSbieAPI::OnIniChanged(const QString &path)
{
	if (!m_bReloadPending)
	{
		m_bReloadPending = true;
		QTimer::singleShot(500, this, SLOT(OnReloadConfig()));
	}
}

void CSbieAPI::OnReloadConfig()
{
	m_bReloadPending = false;
	ReloadConfig();
}

void CSbieAPI::UpdateDriveLetters()
{
	m_DriveLetters.clear();

	// \Device\HarddiskVolume10
	// \Device\HarddiskVolume1

	// \Device\LanmanRedirector\server\share\file.txt
	// \Device\LanmanRedirector\;Q:0000000000001234\server\share

	wchar_t lpTargetPath [MAX_PATH];
	for (wchar_t ltr = L'A'; ltr <= L'Z'; ltr++)
	{
		wchar_t drv[] = { ltr, L':', '\0' };
		uint size = QueryDosDevice(drv, lpTargetPath, MAX_PATH);
		if (size > 0)
		{
			QString Key = QString::fromWCharArray(lpTargetPath);
			QStringList Chunks = Key.split("\\");
			if (Chunks.count() >= 5 && Chunks[2].compare("LanmanRedirector", Qt::CaseInsensitive) == 0) {
				Chunks.removeAt(3);
				Key = Chunks.join("\\");
			}
			Key.append("\\");
			m_DriveLetters.insert(Key, QString::fromWCharArray(drv) + "\\");
		}
	}
}

QString CSbieAPI::Nt2DosPath(QString NtPath) const
{
	for (QMap<QString, QString>::const_iterator I = m_DriveLetters.begin(); I != m_DriveLetters.end(); ++I)
	{
		const QString& Key = I.key();
		if (Key.compare(NtPath.left(Key.length()), Qt::CaseInsensitive) == 0)
			return NtPath.replace(0, Key.length(), I.value());
	}
	return NtPath;
}

QString CSbieAPI::GetSbieHome() const
{
	WCHAR DosPath[MAX_PATH];
	ULONG DosPathMaxLen = MAX_PATH;

	__declspec(align(8)) UNICODE_STRING64 dos_path_uni = { 0, (USHORT)(DosPathMaxLen * sizeof(WCHAR)), (ULONG64)DosPath };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_GET_HOME_PATH_ARGS *args = (API_GET_HOME_PATH_ARGS *)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_GET_HOME_PATH;
	args->nt_path.val64 = NULL;
	if (DosPath)
		args->dos_path.val64 = (ULONG64)(ULONG_PTR)&dos_path_uni;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return QString();
	return QString::fromWCharArray(DosPath);
}

QString CSbieAPI::GetIniPath(bool* IsHome) const
{
	QString IniPath;

	SBIE_INI_GET_PATH_REQ req;
	req.h.msgid = MSGID_SBIE_INI_GET_PATH;
	req.h.length = sizeof(SBIE_INI_GET_PATH_REQ);

	SBIE_INI_GET_PATH_RPL *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return QString();
	if (rpl->h.status == 0) {
		IniPath = QString::fromWCharArray(rpl->path);
		if (IsHome)
			*IsHome = rpl->is_home_path;
	}
	free(rpl);
	
	return IniPath;
}

SB_STATUS CSbieAPI::RunStart(const QString& BoxName, const QString& Command, QProcess* pProcess)
{
	if (m_SbiePath.isEmpty())
		return SB_ERR(tr("Can't find Sandboxie instal path."));

	QStringList Arguments;
	Arguments.append("/box:" + BoxName);
	Arguments.append(Command);
	if (pProcess)
		pProcess->start(m_SbiePath + "//" + QString::fromWCharArray(SBIESTART_EXE), Arguments);
	else
		QProcess::startDetached(m_SbiePath + "//" + QString::fromWCharArray(SBIESTART_EXE), Arguments);
	return SB_OK;
}

SB_STATUS CSbieAPI::ReloadBoxes()
{
	QMap<QString, CSandBoxPtr> OldSandBoxes = m_SandBoxes;

	for (int i = 0;;i++)
	{
		QString BoxName = SbieIniGet(QString(), QString(), (i | CONF_GET_NO_EXPAND));
		if (BoxName.isNull())
			break;
		if (!IsBoxEnabled(BoxName))
			continue;

		CSandBoxPtr pBox = OldSandBoxes.take(BoxName.toLower());
		if (!pBox)
		{
			pBox = CSandBoxPtr(NewSandBox(BoxName, this));
			m_SandBoxes.insert(BoxName.toLower(), pBox);
		}
			
		UpdateBoxPaths(pBox);

		pBox->UpdateDetails();
	}

	foreach(const QString& BoxName, OldSandBoxes.keys())
		m_SandBoxes.remove(BoxName);

	return SB_OK;
}

SB_STATUS CSbieAPI::SbieIniSet(void *RequestBuf, void *pPasswordWithinRequestBuf, const QString& SectionName, const QString& SettingName)
{
retry:
	m->Password.toWCharArray((WCHAR*)pPasswordWithinRequestBuf); // fix-me: potential overflow
	((WCHAR*)pPasswordWithinRequestBuf)[m->Password.length()] = L'\0';

	MSG_HEADER *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer((MSG_HEADER *)RequestBuf, &rpl);
	SecureZeroMemory(pPasswordWithinRequestBuf, sizeof(WCHAR) * 64);
	if (!Status || !rpl)
		return Status;
	ULONG status = rpl->status;
	free(rpl);
	if (status == 0)
		return SB_OK;
	if (status == STATUS_LOGON_NOT_GRANTED || status == STATUS_WRONG_PASSWORD) 
	{
		if (((MSG_HEADER *)RequestBuf)->msgid != MSGID_SBIE_INI_TEST_PASSWORD)
		{
			bool bRetry = false;
			emit NotAuthorized(status == STATUS_WRONG_PASSWORD, bRetry);
			if (bRetry)
				goto retry;
		}
		return SB_ERR(CSbieAPI::tr("You are not authorized to update configuration in section '%1'").arg(SectionName), status);
	}
	return SB_ERR(CSbieAPI::tr("Failed to set configuration setting %1 in section %2: %3").arg(SettingName).arg(SectionName).arg(status, 8, 16), status);
}

SB_STATUS CSbieAPI::SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode)
{
	if (Section.isEmpty())
		return SB_ERR();

	ULONG msgid = 0;
	switch (Mode)
	{
	case eIniUpdate:	msgid = MSGID_SBIE_INI_SET_SETTING; break;
	case eIniAppend:	msgid = MSGID_SBIE_INI_ADD_SETTING; break;
	case eIniInsert:	msgid = MSGID_SBIE_INI_INS_SETTING; break;
	case eIniDelete:	msgid = MSGID_SBIE_INI_DEL_SETTING; break;
	default:
		return SB_ERR();
	}

	SBIE_INI_SETTING_REQ *req = (SBIE_INI_SETTING_REQ *)malloc(REQUEST_LEN);

	Section.toWCharArray(req->section); // fix-me: potential overflow
	req->section[Section.length()] = L'\0';
	Setting.toWCharArray(req->setting); // fix-me: potential overflow
	req->setting[Setting.length()] = L'\0';
	Value.toWCharArray(req->value); // fix-me: potential overflow
	req->value[Value.length()] = L'\0';
	req->value_len = Value.length();
	req->h.msgid = msgid;
	req->h.length = sizeof(SBIE_INI_SETTING_REQ) + req->value_len * sizeof(WCHAR);

	SB_STATUS Status = SbieIniSet(req, req->password, Section, Setting);
	if (!Status)
		emit LogMessage(tr("Failed to communicate with Sandboxie Service: %1").arg(Status.GetText()));
	free(req);
	return Status;
}

QString CSbieAPI::SbieIniGet(const QString& Section, const QString& Setting, quint32 Index, qint32* ErrCode)
{
	wstring section = Section.toStdWString();
	wstring setting = Setting.toStdWString();

	WCHAR out_buffer[CONF_LINE_LEN] = { 0 };

	__declspec(align(8)) UNICODE_STRING64 Output = { 0, CONF_LINE_LEN - 4 , (ULONG64)out_buffer };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

	memset(parms, 0, sizeof(parms));
	parms[0] = API_QUERY_CONF;
	parms[1] = (ULONG64)section.c_str();
	parms[2] = (ULONG64)setting.c_str();
	parms[3] = (ULONG64)&Index;
	parms[4] = (ULONG64)&Output;

	NTSTATUS status = m->IoControl(parms);
	if (ErrCode)
		*ErrCode = status;
	if (!NT_SUCCESS(status))
		return QString();
	return QString::fromWCharArray(out_buffer);
}

SB_STATUS CSbieAPI::CreateBox(const QString& BoxName)
{
	return SbieIniSet(BoxName, "Enabled", "y");
}

SB_STATUS CSbieAPI::UpdateProcesses(bool bKeep)
{
	foreach(const CSandBoxPtr& pBox, m_SandBoxes)
		UpdateProcesses(bKeep, pBox);
	return SB_OK;
}

SB_STATUS CSbieAPI::UpdateProcesses(bool bKeep, const CSandBoxPtr& pBox)
{
	wstring box_name = pBox->GetName().toStdWString(); // WCHAR [34]
	BOOLEAN all_sessions = TRUE;
	ULONG which_session = 0; // -1 for current session
	ULONG boxed_pids[512]; // ULONG [512]

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

	memset(parms, 0, sizeof(parms));
	parms[0] = API_ENUM_PROCESSES;
	parms[1] = (ULONG64)boxed_pids;
	parms[2] = (ULONG64)box_name.c_str();
	parms[3] = (ULONG64)all_sessions;
	parms[4] = (ULONG64)which_session;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);

	QMap<quint64, CBoxedProcessPtr>	OldProcessList = pBox->m_ProcessList;

	for (int i=1; i < boxed_pids[0] + 1; i++)
	{
		quint64 ProcessId = boxed_pids[i];

		CBoxedProcessPtr pProcess = OldProcessList.take(ProcessId);
		if (!pProcess)
		{
			pProcess = CBoxedProcessPtr(NewBoxedProcess(ProcessId, pBox.data()));
			//pProcess->m_pBox = pBox;
			pBox->m_ProcessList.insert(ProcessId, pProcess);
			m_BoxedProxesses.insert(ProcessId, pProcess);

			UpdateProcessInfo(pProcess);
			pProcess->InitProcessInfo();
		}

		// todo:
	}


	foreach(const CBoxedProcessPtr& pProcess, OldProcessList) {
		if (!bKeep) {
			pBox->m_ProcessList.remove(pProcess->m_ProcessId);
			m_BoxedProxesses.remove(pProcess->m_ProcessId);
		}
		else if (!pProcess->IsTerminated()) {
			pProcess->SetTerminated();
			m_BoxedProxesses.remove(pProcess->m_ProcessId);
		}
	}

	return SB_OK;
}

SB_STATUS CSbieAPI__QueryBoxPath(SSbieAPI* m, const WCHAR *box_name, WCHAR *out_file_path, WCHAR *out_key_path, WCHAR *out_ipc_path,
									ULONG *inout_file_path_len, ULONG *inout_key_path_len, ULONG *inout_ipc_path_len)
{
	__declspec(align(8)) UNICODE_STRING64 FilePath = { 0, (USHORT)*inout_file_path_len, (ULONG64)out_file_path };
	__declspec(align(8)) UNICODE_STRING64 KeyPath = { 0, (USHORT)*inout_key_path_len, (ULONG64)out_key_path };
	__declspec(align(8)) UNICODE_STRING64 IpcPath = { 0, (USHORT)*inout_ipc_path_len, (ULONG64)out_ipc_path };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_BOX_PATH_ARGS *args = (API_QUERY_BOX_PATH_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_QUERY_BOX_PATH;
	args->box_name.val64 = (ULONG64)(ULONG_PTR)box_name;

	args->file_path_len.val = inout_file_path_len;
	if (out_file_path) 
		args->file_path.val64 = (ULONG64)(ULONG_PTR)&FilePath;

	args->key_path_len.val = inout_key_path_len;
	if (out_key_path)
		args->key_path.val64 = (ULONG64)(ULONG_PTR)&KeyPath;

	args->ipc_path_len.val = inout_ipc_path_len;
	if (out_ipc_path)
		args->ipc_path.val64 = (ULONG64)(ULONG_PTR)&IpcPath;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::UpdateBoxPaths(const CSandBoxPtr& pSandBox)
{
	wstring boxName = pSandBox->GetName().toStdWString();

	ULONG filePathLength = 0;
	ULONG keyPathLength = 0;
	ULONG ipcPathLength = 0;
	SB_STATUS Status = CSbieAPI__QueryBoxPath(m, boxName.c_str(), NULL, NULL, NULL, &filePathLength, &keyPathLength, &ipcPathLength);
	if (!Status)
		return Status;

	wstring FileRoot(filePathLength / 2 + 1, '\0');
	wstring KeyRoot(keyPathLength / 2 + 1, '\0');
	wstring IpcRoot(ipcPathLength / 2 + 1, '\0');
	Status = CSbieAPI__QueryBoxPath(m, boxName.c_str(), (WCHAR*)FileRoot.data(), (WCHAR*)KeyRoot.data(), (WCHAR*)IpcRoot.data(), &filePathLength, &keyPathLength, &ipcPathLength);
	if (!Status)
		return Status;

	pSandBox->m_FilePath = Nt2DosPath(QString::fromStdWString(FileRoot));
	pSandBox->m_RegPath = QString::fromStdWString(KeyRoot);
	pSandBox->m_IpcPath = QString::fromStdWString(IpcRoot);
	return SB_OK;
}

SB_STATUS CSbieAPI::UpdateProcessInfo(const CBoxedProcessPtr& pProcess)
{
	//WCHAR box_name_wchar34[34] = { 0 };
	WCHAR image_name[MAX_PATH];
	//WCHAR sid[96];
	ULONG session_id;
	ULONG64 create_time;

	//__declspec(align(8)) UNICODE_STRING64 BoxName = { 0, sizeof(box_name_wchar34) , (ULONG64)box_name_wchar34 };
	__declspec(align(8)) UNICODE_STRING64 ImageName = { 0, sizeof(image_name), (ULONG64)image_name };
	//__declspec(align(8)) UNICODE_STRING64 SidString = { 0, sizeof(sid), (ULONG64)sid };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_PROCESS_ARGS *args = (API_QUERY_PROCESS_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_QUERY_PROCESS;
	args->process_id.val64 = (ULONG64)pProcess->m_ProcessId;
	//args->box_name.val64 = (ULONG64)&BoxName;
	args->image_name.val64 = (ULONG64)&ImageName;
	//args->sid_string.val64 = (ULONG64)&SidString;
	args->session_id.val64 = (ULONG64)&session_id;
	args->create_time.val64 = (ULONG64)&create_time;

	NTSTATUS status = m->IoControl(parms);
	if(!NT_SUCCESS(status))
		return SB_ERR(status);

	pProcess->m_ImageName = QString::fromWCharArray(image_name, ImageName.Length/sizeof(WCHAR));
	pProcess->m_SessionId = session_id;
	pProcess->m_StartTime = QDateTime::fromMSecsSinceEpoch(FILETIME2ms(create_time));
	// sid todo

	return SB_OK;
}

CSandBoxPtr CSbieAPI::GetBoxByProcessId(quint64 ProcessId) const
{
	CBoxedProcessPtr pProcess = m_BoxedProxesses.value(ProcessId);
	if (!pProcess)
		return CSandBoxPtr();
	return m_SandBoxes.value(pProcess->GetBoxName().toLower());
}

SB_STATUS CSbieAPI::TerminateAll(const QString& BoxName)
{
	PROCESS_KILL_ALL_REQ req;
	req.h.length = sizeof(PROCESS_KILL_ALL_REQ);
	req.h.msgid = MSGID_PROCESS_KILL_ALL;
	req.session_id = -1;
	BoxName.toWCharArray(req.boxname); // fix-me: potential overflow
	req.boxname[BoxName.size()] = L'\0';

	MSG_HEADER *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer(&req.h, &rpl);
	if (!Status)
		emit LogMessage(tr("Failed to communicate with Sandboxie Service: %1").arg(Status.GetText()));
	if (!Status || !rpl)
		return Status;
	if(rpl->status != 0) 
		Status = SB_ERR(rpl->status);
	free(rpl);
	return Status;
}

SB_STATUS CSbieAPI::TerminateAll()
{
	SB_STATUS Status = SB_OK;
	foreach(const CSandBoxPtr& pBox, m_SandBoxes) {
		if (!pBox->TerminateAll())
			Status = SB_ERR(tr("Failed to terminate all processes"));
	}
	return Status;
}

SB_STATUS CSbieAPI::Terminate(quint64 ProcessId)
{
	PROCESS_KILL_ONE_REQ req;
	req.h.length = sizeof(PROCESS_KILL_ONE_REQ);
	req.h.msgid = MSGID_PROCESS_KILL_ONE;
	req.pid = ProcessId;

	MSG_HEADER *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer(&req.h, &rpl);
	if (!Status)
		emit LogMessage(tr("Failed to communicate with Sandboxie Service: %1").arg(Status.GetText()));
	if (!Status || !rpl)
		return Status;
	if (rpl->status != 0)
		Status = SB_ERR(rpl->status);
	free(rpl);
	return Status;
}

LONG CSbieAPI__OpenDeviceMap(SSbieAPI* m, HANDLE *DirectoryHandle)
{
	__declspec(align(8)) ULONG64 ResultHandle;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_OPEN_DEVICE_MAP_ARGS *args = (API_OPEN_DEVICE_MAP_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_OPEN_DEVICE_MAP;
	args->handle.val64 = (ULONG64)&ResultHandle;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		ResultHandle = 0;
	if (DirectoryHandle)
		*DirectoryHandle = (HANDLE*)ResultHandle;
	return status;
}

QString CSbieAPI::GetDeviceMap()
{
	UNICODE_STRING objname;
	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
	RtlInitUnicodeString(&objname, L"\\??");

	HANDLE handle;
	NTSTATUS status = NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);
	if (status == STATUS_ACCESS_DENIED) // if we are missign admin rights, lets the driver do it
		status = CSbieAPI__OpenDeviceMap(m, &handle);

	if (!NT_SUCCESS(status)) {
		emit LogMessage(tr("Cannot open device map: %1").arg(status, 8, 16));
	}
	else 
	{
		WCHAR dirname[128];
		ULONG length = sizeof(dirname);
		status = NtQueryObject(handle, ObjectNameInformation, dirname, length, &length);

		NtClose(handle);

		if (!NT_SUCCESS(status)) {
			emit LogMessage(tr("Cannot query device map: %1").arg(status, 8, 16));
		}
		else 
		{
			UNICODE_STRING* ustr = &((OBJECT_NAME_INFORMATION*)dirname)->Name;
			length = ustr->Length / sizeof(WCHAR);
			return QString::fromWCharArray(ustr->Buffer, length);
		}
	}
	return QString();
}

QByteArray CSbieAPI::MakeEnvironment(bool AddDeviceMap)
{
	QStringList EnvList;
	
	//if(AllUsers.length() > 0)
	//	EnvList.push_back("00000000_SBIE_ALL_USERS=" + AllUsers);
	//if(CurrentUser.length() > 0)
	//	EnvList.push_back("00000000_SBIE_CURRENT_USER=" + CurrentUser);
	//if(PublicUser.length() > 0)
	//	EnvList.push_back("00000000_SBIE_PUBLIC_USER=" + PublicUser);
	QString DeviceMap = AddDeviceMap ? GetDeviceMap() : QString();
	if (DeviceMap.length() > 0)
		EnvList.push_back("00000000_SBIE_DEVICE_MAP=" + DeviceMap);

	ULONG ExtraLength = 0;
	foreach(const QString& Entry, EnvList)
		ExtraLength += Entry.length() + 1;

	WCHAR *Environment = GetEnvironmentStrings();
	ULONG EnvLength = 0;
	for(WCHAR* envPtr = (WCHAR*)Environment; *envPtr;)
	{
		ULONG len = wcslen(envPtr) + 1;
		envPtr += len;
		EnvLength += len;
	}

	QByteArray env;
	env.resize((ExtraLength + EnvLength + 1) * sizeof(WCHAR)); // key1=value1\0key2=value2\0...keyN=valueN\0\0
	WCHAR* envPtr = (WCHAR*)env.data();
	foreach(const QString& Entry, EnvList)
	{
		Entry.toWCharArray(envPtr);
		envPtr += Entry.length();
		*envPtr++ = L'\0';
	}

	wmemcpy(envPtr, Environment, EnvLength);
	envPtr += EnvLength;

	*envPtr = L'\0';

	return env;
}

SB_STATUS CSbieAPI::RunSandboxed(const QString& BoxName, const QString& Command, QString WrkDir, quint32 Flags)
{
	DWORD wShowWindow = SW_SHOWNORMAL;

	if (Command.isEmpty())
		return SB_ERR(ERROR_INVALID_PARAMETER);

	if (WrkDir.isEmpty())
		WrkDir = QDir::currentPath();

	ULONG cmd_len = Command.length();
	ULONG dir_len = WrkDir.length();
	QByteArray env = MakeEnvironment(true);
	ULONG env_len = (env.size() - 1) / sizeof(WCHAR);

	ULONG req_len = sizeof(PROCESS_RUN_SANDBOXED_REQ) + (cmd_len + dir_len + env_len + 8) * sizeof(WCHAR);
	PROCESS_RUN_SANDBOXED_REQ* req = (PROCESS_RUN_SANDBOXED_REQ*)malloc(req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_PROCESS_RUN_SANDBOXED;
	BoxName.toWCharArray(req->boxname); // fix-me: potential overflow
	req->boxname[BoxName.length()] = L'\0';
	req->si_flags = STARTF_FORCEOFFFEEDBACK;
	req->si_show_window = wShowWindow;
	if (req->si_show_window != SW_SHOWNORMAL)
		req->si_flags |= STARTF_USESHOWWINDOW;
	req->creation_flags = Flags;

	WCHAR* ptr = (WCHAR*)((ULONG_PTR)req + sizeof(PROCESS_RUN_SANDBOXED_REQ));

	req->cmd_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
	req->cmd_len = cmd_len;
	if (cmd_len) {
		Command.toWCharArray(ptr);
		ptr += cmd_len;
	}
	*ptr++ = L'\0';

	req->dir_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
	req->dir_len = dir_len;
	if (dir_len) {
		WrkDir.toWCharArray(ptr);
		ptr += dir_len;
	}
	*ptr++ = L'\0';

	req->env_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req);
	req->env_len = env_len;
	if (env_len) {
		wmemcpy(ptr, (WCHAR*)env.data(), env_len);
		ptr += env_len;
	}
	*ptr++ = L'\0';

	PROCESS_RUN_SANDBOXED_RPL *rpl;
	SB_STATUS Status = CSbieAPI::CallServer(&req->h, &rpl);
	free(req);
	if (!Status)
		emit LogMessage(tr("Failed to communicate with Sandboxie Service: %1").arg(Status.GetText()));
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	

	if (rpl->h.status != 0)
		Status = SB_ERR(rpl->h.status);

	PROCESS_INFORMATION pi;
	pi.hProcess = (HANDLE)rpl->hProcess;
	pi.hThread = (HANDLE)rpl->hThread;
	pi.dwProcessId = rpl->dwProcessId;
	pi.dwThreadId = rpl->dwThreadId;

	free(rpl);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	return Status;
}


///////////////////////////////////////////////////////////////////////////////
// Conf
//

SB_STATUS CSbieAPI::ReloadConfig(quint32 SessionId)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

	memset(parms, 0, sizeof(parms));
	parms[0] = API_RELOAD_CONF;
	parms[1] = SessionId;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);

	emit LogMessage("Sandboxie config has been reloaded.", false);

	ReloadBoxes();

	return SB_OK;
}

bool CSbieAPI::IsBoxEnabled(const QString& BoxName)
{
	wstring box_name = BoxName.toStdWString();

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_IS_BOX_ENABLED_ARGS *args = (API_IS_BOX_ENABLED_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_IS_BOX_ENABLED;
	args->box_name.val = (WCHAR*)box_name.c_str();

	return NT_SUCCESS(m->IoControl(parms));
}

bool CSbieAPI::IsConfigLocked()
{
	return m->Password.isEmpty() && !SbieIniGet("GlobalSettings", "EditPassword", 0).isEmpty(); 
}

SB_STATUS CSbieAPI::UnlockConfig(const QString& Password)
{
	SBIE_INI_PASSWORD_REQ *req = (SBIE_INI_PASSWORD_REQ *)malloc(REQUEST_LEN);
	req->h.msgid = MSGID_SBIE_INI_TEST_PASSWORD;
	req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);
	m->Password = Password;
	SB_STATUS Status = SbieIniSet(req, req->old_password, "GlobalSettings", "*");
	if (Status.IsError())
		m->Password.clear();
	free(req);
	return Status;
}

SB_STATUS CSbieAPI::LockConfig(const QString& NewPassword)
{
	SBIE_INI_PASSWORD_REQ *req = (SBIE_INI_PASSWORD_REQ *)malloc(REQUEST_LEN);
	req->h.msgid = MSGID_SBIE_INI_SET_PASSWORD;
	req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);
	m->Password.toWCharArray(req->new_password); // fix-me: potential overflow
	req->new_password[m->Password.length()] = L'\0';
	SB_STATUS Status = SbieIniSet(req, req->old_password, "GlobalSettings", "*");
	if (!Status.IsError())
		m->Password = NewPassword;
	free(req);
	return Status;
}

void CSbieAPI::ClearPassword()
{
	m->Password.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Log
//

QString CSbieAPI__FormatSbieMsg(SSbieAPI* m, ULONG code, const WCHAR *ins1, const WCHAR *ins2)
{
	ULONG FormatFlags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	ULONG Lang = 1033; // English

	ULONG ret_count = 0;
	WCHAR* ret_str = NULL;

	if (m->SbieMsgDll)
	{
		const WCHAR *ins[6] = { 0 , ins1, ins2, 0, 0, 0 };
		ret_count = FormatMessage(FormatFlags, m->SbieMsgDll, code, Lang, (LPWSTR)&ret_str, 4, (va_list *)ins);
	}

	if (ret_count == 0)
	{
		ret_str = (WCHAR*)LocalAlloc(LMEM_FIXED, 128 * sizeof(WCHAR));
		swprintf(ret_str, L"SBIE%04i: %s; %s", code & 0x0000FFFF, ins1 ? ins1 : L"", ins2 ? ins2 : L"");
	}

	QString qStr = QString::fromWCharArray(ret_str);
	LocalFree(ret_str);
	return qStr.trimmed(); // note messages may have \r\n at the end
}

bool CSbieAPI::GetLog()
{
	wchar_t* Buffer[4*1024];
	ULONG Length = ARRAYSIZE(Buffer);

	ULONG MsgCode = 0;
	ULONG ProcessId = 0;
	ULONG MessageNum = m->lastMessageNum;

	__declspec(align(8)) UNICODE_STRING64 msgtext = { 0, (USHORT)Length, (ULONG64)Buffer };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_GET_MESSAGE_ARGS *args = (API_GET_MESSAGE_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_GET_MESSAGE;
	args->msg_num.val = &MessageNum;
	args->session_id.val = m->sessionId;
	args->msgid.val = &MsgCode;
	args->msgtext.val = &msgtext;
	args->process_id.val = &ProcessId;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return false; // error or no more entries

	//if (MessageNum != m->lastMessageNum + 1)
	//	we missed something
	m->lastMessageNum = MessageNum;

	if (MsgCode == 0)
		return true; // empty dummy message for maintaining sequence consistency

    WCHAR *str1 = (WCHAR*)msgtext.Buffer;
    ULONG str1_len = wcslen(str1);
    WCHAR *str2 = str1 + str1_len + 1;
    ULONG str2_len = wcslen(str2);

	//
	//	0xTFFFMMMM
	//
	//	T = ttcr
	//		tt = 00 - Ok
	//		tt = 01 - Info
	//		tt = 10 - Warning
	//		tt = 11 - Error
	//		c  = unused
	//		r  = reserved
	//
	//	FFF = 0x000 UIstr
	//	FFF = 0x101 POPUP
	//	FFF = 0x102 EVENT
	//
	//	MMMM = Message Code
	//
	quint8  Severity = MsgCode >> 30;
	quint16 Facility = (MsgCode >> 16) & 0x0FFF;
	quint16 MessageId= MsgCode & 0xFFFF;

	if (MessageId == 2199) // Auto Recovery notification
	{
		QString TempStr = QString::fromWCharArray(str1);
		int TempPos = TempStr.indexOf(" ");
		FileToRecover(TempStr.left(TempPos), Nt2DosPath(TempStr.mid(TempPos + 1)));
		return true;
	}

	QString Message = CSbieAPI__FormatSbieMsg(m, MsgCode, str1, str2);
	if(ProcessId != 4) // if its not from the driver add the pid
		Message += tr(" by process: %1").arg(ProcessId);
	emit LogMessage(Message);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Forced Processes
//

SB_STATUS CSbieAPI::DisableForceProcess(bool Set)
{
	//m_pGlobalSection->SetNum("ForceDisableSeconds", Seconds);

	ULONG uEnable = Set ? TRUE : FALSE;

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_DISABLE_FORCE_PROCESS_ARGS* args = (API_DISABLE_FORCE_PROCESS_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_DISABLE_FORCE_PROCESS;
	args->set_flag.val = &uEnable; // NewState
	args->get_flag.val = NULL; // OldState

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

bool CSbieAPI::AreForceProcessDisabled()
{
	ULONG uEnabled = FALSE;

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_DISABLE_FORCE_PROCESS_ARGS* args = (API_DISABLE_FORCE_PROCESS_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_DISABLE_FORCE_PROCESS;
	args->set_flag.val = NULL; // NewState
	args->get_flag.val = &uEnabled; // OldState

	return NT_SUCCESS(m->IoControl(parms)) && uEnabled;
}

///////////////////////////////////////////////////////////////////////////////
// Monitor
//

SB_STATUS CSbieAPI::EnableMonitor(bool Enable)
{
	ULONG uEnable = Enable ? TRUE : FALSE;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_CONTROL_ARGS* args	= (API_MONITOR_CONTROL_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code = API_MONITOR_CONTROL;
    args->set_flag.val = &uEnable; // NewState
    args->get_flag.val = NULL; // OldState
    
	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

bool CSbieAPI::IsMonitoring()
{
	ULONG uEnabled = FALSE;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_CONTROL_ARGS* args	= (API_MONITOR_CONTROL_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code = API_MONITOR_CONTROL;
    args->set_flag.val = NULL; // NewState
    args->get_flag.val = &uEnabled; // OldState
    
	return NT_SUCCESS(m->IoControl(parms)) && uEnabled;
}

bool CSbieAPI::GetMonitor()
{
	USHORT type;
	ULONG64 pid;
	WCHAR name[256 + 1] = { 0 };

	ULONG RecordNum = m->lastRecordNum;

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_GET_EX_ARGS* args	= (API_MONITOR_GET_EX_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code	= API_MONITOR_GET_EX;
	args->name_seq.val = &RecordNum;
    args->name_type.val = &type;
	args->name_pid.val = &pid;
    args->name_len.val = 256 * sizeof(WCHAR);
    args->name_ptr.val = name;
    
	if (!NT_SUCCESS(m->IoControl(parms)))
		return false;

	//if (RecordNum != m->lastRecordNum + 1)
	//	we missed something
	m->lastRecordNum = RecordNum;

	if (type == 0)
		return false;

	CResLogEntryPtr LogEntry = CResLogEntryPtr(new CResLogEntry(pid, type, QString::fromWCharArray(name)));

	QWriteLocker Lock(&m_ResLogMutex); 
	m_ResLogList.append(LogEntry);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Other 
//

QString CSbieAPI::GetSbieMessage(int MessageId, const QString& arg1, const QString& arg2) const
{
	return CSbieAPI__FormatSbieMsg(m, MessageId, arg1.toStdWString().c_str(), arg2.toStdWString().c_str());
}

///////////////////////////////////////////////////////////////////////////////
// 
//

CResLogEntry::CResLogEntry(quint64 ProcessId, quint32 Type, const QString& Value)
{
	m_ProcessId = ProcessId;
	m_Name = Value;
	switch (Type & 0x0FFF)
	{
	case MONITOR_PIPE:			m_Type = "Pipe"; break;
	case MONITOR_IPC:			m_Type = "Ipc"; break;
	case MONITOR_WINCLASS:		m_Type = "WinClass"; break;
	case MONITOR_DRIVE:			m_Type = "Drive"; break;
	case MONITOR_COMCLASS:		m_Type = "ComClass"; break;
	case MONITOR_IGNORE:		m_Type = "Ignore"; break;
	case MONITOR_IMAGE:			m_Type = "Image"; break;
	case MONITOR_FILE_OR_KEY:	m_Type = "File"; break;
	case MONITOR_OTHER:			m_Type = "Other"; break;
	default:					m_Type = "Unknown: " + QString::number(Type);
	}
	m_Open = (Type & MONITOR_OPEN) != 0;
	m_Deny = (Type & MONITOR_DENY) != 0;
	//m_Verbose = (Type & MONITOR_VERBOSE) != 0;
	//m_User = (Type & MONITOR_USER) != 0;
	m_TimeStamp = QDateTime::currentDateTime(); // ms resolution

	static atomic<quint64> uid = 0;
	m_uid = uid.fetch_add(1);
}

///////////////////////////////////////////////////////////////////////////////
// 
//

QString GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return QString();

	char* messageBuffer = NULL;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	QString message(messageBuffer);
	LocalFree(messageBuffer);
	return message;
}

