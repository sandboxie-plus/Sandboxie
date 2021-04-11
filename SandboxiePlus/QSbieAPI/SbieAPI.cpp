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
#include <QStandardPaths>
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
#include "..\..\Sandboxie\core\svc\QueueWire.h"
#include "..\..\Sandboxie\core\svc\InteractiveWire.h"

int _SB_STATUS_type = qRegisterMetaType<SB_STATUS>("SB_STATUS");

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

		wsprintf(QueueName, L"*%s_%08X", INTERACTIVE_QUEUE_NAME, sessionId);

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

	WCHAR QueueName[64];

	QString Password;

	ULONG sessionId;

	bool clearingBuffers;
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

	m_pUserSection = NULL;

	m_bReloadPending = false;

	connect(&m_IniWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(OnIniChanged(const QString&)));
	connect(this, SIGNAL(ProcessBoxed(quint32, const QString&, const QString&, quint32)), this, SLOT(OnProcessBoxed(quint32, const QString&, const QString&, quint32)));
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

CBoxedProcess* CSbieAPI::NewBoxedProcess(quint32 ProcessId, class CSandBox* pBox)
{
	return new CBoxedProcess(ProcessId, pBox);
}

QString CSbieAPI__GetRegValue(HANDLE hKey, const WCHAR* pName)
{
	char buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_PATH];
	KEY_VALUE_PARTIAL_INFORMATION *value = (KEY_VALUE_PARTIAL_INFORMATION *)buf;
	ULONG len;

	UNICODE_STRING uni;
	RtlInitUnicodeString(&uni, pName);
	if (NT_SUCCESS(NtQueryValueKey(hKey, &uni, KeyValuePartialInformation, value, sizeof(buf) - 16, &len)))
	{
		WCHAR expand[MAX_PATH + 8];
		len = ExpandEnvironmentStrings((WCHAR*)value->Data, expand, MAX_PATH + 4);

		return QString::fromWCharArray(expand);
	}
	return QString();
}

bool CSbieAPI__GetCurrentSidString(UNICODE_STRING *SidString)
{
	HANDLE token;
	NTSTATUS status = NtOpenProcessToken(NtCurrentProcess(), TOKEN_QUERY, &token);
	if (NT_SUCCESS(status)) 
	{
		ULONG len;
		union {
			TOKEN_USER userToken;
			WCHAR info_space[64];
		} data;
		status = NtQueryInformationToken(token, TokenUser, &data, sizeof(data), &len);
		if (NT_SUCCESS(status)) 
			status = RtlConvertSidToUnicodeString(SidString, data.userToken.User.Sid, TRUE);
		NtClose(token);
	}
	return NT_SUCCESS(status);
}

void CSbieAPI::GetUserPaths()
{
	const WCHAR* ProfileList = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList";

	UNICODE_STRING objname;
	RtlInitUnicodeString(&objname, ProfileList);

	OBJECT_ATTRIBUTES objattrs;
	InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE hProfileKey;
	if (NT_SUCCESS(NtOpenKey(&hProfileKey, KEY_READ, &objattrs)))
	{
		//m_ProfilesDir = CSbieAPI__GetRegValue(hProfileKey, L"ProfilesDirectory");
		m_ProgramDataDir = CSbieAPI__GetRegValue(hProfileKey, L"ProgramData"); 
		//if(ProgramDataDir.isEmpty()) // for Win XP
		//	ProgramDataDir = CSbieAPI__GetRegValue(hProfileKey, L"AllUsersProfile");
		m_PublicDir = CSbieAPI__GetRegValue(hProfileKey, L"Public");

		if (CSbieAPI__GetCurrentSidString(&objname))
		{
			InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, hProfileKey, NULL);

			HANDLE hSidKey;
			if (NT_SUCCESS(NtOpenKey(&hSidKey, KEY_READ, &objattrs)))
			{
				m_UserDir = CSbieAPI__GetRegValue(hSidKey, L"ProfileImagePath");

				NtClose(hSidKey);
			}
		}

		NtClose(hProfileKey);
	}
}

SB_STATUS CSbieAPI::Connect(bool withQueue)
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
		return SB_ERR(SB_DriverFail, status);
	}

	UpdateDriveLetters();

	m_SbiePath = GetSbieHome();
	m->SbieMsgDll = LoadLibraryEx((m_SbiePath.toStdWString() + L"\\" SBIEMSG_DLL).c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

	m->lastMessageNum = 0;
	m->lastRecordNum = 0;

	// Note: this lib is not using all functions hence it can be compatible with multiple driver ABI revisions
	QStringList CompatVersions = QStringList () << "5.49.0";
	QString CurVersion = GetVersion();
	if (!CompatVersions.contains(CurVersion))
	{
		NtClose(m->SbieApiHandle);
		m->SbieApiHandle = INVALID_HANDLE_VALUE;	
		return SB_ERR(SB_Incompatible, QVariantList() << CurVersion << CompatVersions.join(", "));
	}

	m_bWithQueue = withQueue;
	m_bTerminate = false;
	m->clearingBuffers = true;
	start();

	bool bHome = false;
	m_IniPath = GetIniPath(&bHome);
	qDebug() << "Config file:" << m_IniPath << (bHome ? "(home)" : "(system)");

	if (m_pUserSection == NULL)
	{
		QString UserSection = GetUserSection(&m_UserName);
		if(!UserSection.isEmpty())
			m_pUserSection = new CSbieIni(UserSection, this, this);

		GetUserPaths();
	}

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
			return SB_ERR(SB_ServiceFail, QVariantList() << QString("request %1").arg(status, 8, 16), status); // 2203
		}

		if (BuffLen && ResHeader->u1.s1.DataLength)
			return SB_ERR(SB_ServiceFail, QVariantList() << QString("early reply")); // 2203
	}

	// the last call to NtRequestWaitReplyPort should return the first chunk of the reply
	if (ResHeader->u1.s1.DataLength >= sizeof(MSG_HEADER))
	{
		if (ResData[3] != CurSeqNumber)
			return SB_ERR(SB_ServiceFail, QVariantList() << QString("mismatched reply")); // 2203

		// clear highest byte of the size filed
		ResData[3] = 0;
		BuffLen = ((MSG_HEADER*)ResData)->length;
	}
	else
		BuffLen = 0;
	if (BuffLen == 0)
		return SB_ERR(SB_ServiceFail, QVariantList() << QString("null reply (msg %1 len %2)").arg(req->msgid, 8, 16).arg(req->length)); // 2203

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
			return SB_ERR(SB_ServiceFail, QVariantList() << QString("reply %1").arg(status, 8, 16), status); // 2203
		}
	}

	return SB_OK;
}

SB_STATUS CSbieAPI__QueueCreate(SSbieAPI* m, const WCHAR* QueueName, HANDLE *out_EventHandle)
{
	QUEUE_CREATE_REQ req;
	req.h.length = sizeof(QUEUE_CREATE_REQ);
	req.h.msgid = MSGID_QUEUE_CREATE;
	wcscpy(req.queue_name, QueueName);
	req.event_handle = (ULONG64)(ULONG_PTR)CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!req.event_handle)
		return SB_ERR(STATUS_UNSUCCESSFUL);

	NTSTATUS status = STATUS_SERVER_DISABLED;
	QUEUE_CREATE_RPL *rpl = NULL;
	CSbieAPI__CallServer(m, &req.h, (MSG_HEADER**)&rpl);
	if (rpl) 
	{
		status = rpl->h.status;
		free(rpl);
	}

	if (!NT_SUCCESS(status)) {
		CloseHandle((HANDLE)(ULONG_PTR)req.event_handle);
		return SB_ERR(status);
	}

	*out_EventHandle = (HANDLE)(ULONG_PTR)req.event_handle;

	return SB_OK;
}

bool CSbieAPI::GetQueue()
{
	QUEUE_GETREQ_REQ req;
	req.h.length = sizeof(QUEUE_GETREQ_REQ);
	req.h.msgid = MSGID_QUEUE_GETREQ;
	wcscpy(req.queue_name, m->QueueName);

	NTSTATUS status = STATUS_SERVER_DISABLED;
	QUEUE_GETREQ_RPL *rpl = NULL;
	CSbieAPI__CallServer(m, &req.h, (MSG_HEADER**)&rpl);
	if (rpl)
	{
		status = rpl->h.status;
		if (NT_SUCCESS(status) && rpl->data_len >= 4)
		{
			QVariantMap Data;
			switch (*(ULONG*)rpl->data)
			{
				case MAN_FILE_MIGRATION:
				{
					MAN_FILE_MIGRATION_REQ *req = (MAN_FILE_MIGRATION_REQ *)rpl->data;

					Data["id"] = (int)eFileMigration;
					Data["fileSize"] = req->file_size;
					Data["fileName"] = Nt2DosPath(QString::fromWCharArray(req->file_path));
					break;
				}
				case MAN_INET_BLOCKADE:
					Data["id"] = (int)eInetBlockade;
					break;
			}

			emit QueuedRequest(rpl->client_pid, rpl->client_tid, rpl->req_id, Data);
		}

		free(rpl);
	}

	//if(status == STATUS_END_OF_FILE) // there are no more requests in the queue at this time
	//	return false; // nothign more to do

	if (NT_SUCCESS(status))
		return true; // we did something
	return false;
}

void CSbieAPI::SendReplyData(quint32 RequestId, const QVariantMap& Result)
{
	QByteArray Data;

	switch(Result["id"].toInt())
	{
		case CSbieAPI::eFileMigration:
		{
			MAN_FILE_MIGRATION_RPL rpl;
			rpl.status = STATUS_SUCCESS;
			rpl.retval = Result["retval"].toUInt();

			Data = QByteArray((char*)&rpl, sizeof(MAN_FILE_MIGRATION_RPL));
			break;
		}
		case CSbieAPI::eInetBlockade:
		{
			MAN_INET_BLOCKADE_RPL rpl;
			rpl.status = STATUS_SUCCESS;
			rpl.retval = Result["retval"].toUInt();

			Data = QByteArray((char*)&rpl, sizeof(MAN_INET_BLOCKADE_RPL));
			break;
		}
	}

	ULONG req_len = sizeof(QUEUE_PUTRPL_REQ) + Data.length();

	QUEUE_PUTRPL_REQ *req;
	req = (QUEUE_PUTRPL_REQ*)malloc(req_len);
	req->h.length = req_len;
	req->h.msgid = MSGID_QUEUE_PUTRPL;
	wcscpy(req->queue_name, m->QueueName);
	req->req_id = RequestId;
	req->data_len = Data.length();
	memcpy(req->data, Data.constData(), req->data_len);

	NTSTATUS status = STATUS_SERVER_DISABLED;
	QUEUE_PUTRPL_RPL *rpl = NULL;
	CallServer(&req->h, (MSG_HEADER**)&rpl);
	if (rpl)
	{
		status = rpl->h.status;
		free(rpl);
	}

	free(req);
}

void CSbieAPI::run()
{
	int Idle = 0;

	HANDLE EventHandle = NULL;
	if(m_bWithQueue)
		CSbieAPI__QueueCreate(m, m->QueueName, &EventHandle);

	while (!m_bTerminate)
	{
		int Done = 0;

		if (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_EXEC, SVC_OP_STATE_START) == SVC_OP_STATE_START)
		{
			m->SvcStatus = CSbieAPI__CallServer(m, (MSG_HEADER*)m->SvcReq, (MSG_HEADER**)m->SvcRpl);

			InterlockedExchange(&m->SvcLock, SVC_OP_STATE_DONE);

			Done++;
		}

		if (EventHandle != NULL && WaitForSingleObject(EventHandle, 0) == 0)
		{
			while(GetQueue())
				Done++;
		}

		while (GetLog()) // this emits sbie message events if there are any
			Done++;

		while (GetMonitor())
			Done++;

		if (Done != 0)
			Idle = 0;
		else
		{
			if(m->clearingBuffers)
				m->clearingBuffers = false;

			if(Idle < 5)
				Idle++;

			m_ThreadMutex.lock();
			m_ThreadWait.wait(&m_ThreadMutex, 10 * Idle);
			m_ThreadMutex.unlock();
		}
	}

	if (EventHandle != NULL)
		CloseHandle(EventHandle);
}

SB_STATUS CSbieAPI::CallServer(void* req, void* rpl) const
{
	//
	// Note: Once we open a port to the server from a threat the service will remember it we can't reconnect after disconnection
	//			So for every new connection we need a new threat, we achive this by letting our monitor threat issue all requests
	//

	while (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_PREP, SVC_OP_STATE_IDLE) != SVC_OP_STATE_IDLE)
		QThread::usleep(100);

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
}

/*void CSbieAPI::OnMonitorEntry(quint32 ProcessId, quint32 Type, const QString& Value)
{
	QMap<quint32, CBoxedProcessPtr>::iterator I = m_BoxedProxesses.find(ProcessId);
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
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_SESSION_LEADER_ARGS *args = (API_SESSION_LEADER_ARGS *)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_SESSION_LEADER;
	args->token_handle.val64 = 0; // (ULONG64)(ULONG_PTR)GetCurrentProcessToken();
	args->process_id.val64   = 0; // (ULONG64)(ULONG_PTR)&ResultValue;
	
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
	QWriteLocker Lock(&m_DriveLettersMutex);

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
	QReadLocker Lock(&m_DriveLettersMutex);

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

QString CSbieAPI::GetUserSection(QString* pUserName, bool* pIsAdmin) const
{
	QString UserSection;

	SBIE_INI_GET_USER_REQ req;
	req.h.msgid = MSGID_SBIE_INI_GET_USER;
	req.h.length = sizeof(SBIE_INI_GET_USER_REQ);

	SBIE_INI_GET_USER_RPL *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return QString();

	if (rpl->h.status == 0) {
		if (pIsAdmin) *pIsAdmin = rpl->admin;
		UserSection = QString::fromWCharArray(rpl->section);
		if (pUserName) *pUserName = QString::fromWCharArray(rpl->name);
	}
	free(rpl);

	return UserSection;
}

SB_STATUS CSbieAPI::RunStart(const QString& BoxName, const QString& Command, QProcess* pProcess, bool Elevated)
{
	if (m_SbiePath.isEmpty())
		return SB_ERR(SB_PathFail);

	QString StartCmd = "\"" + GetStartPath() + "\"" + (Elevated ? " /elevated" : "" ) + " /box:" + BoxName + " " + Command;
	if (pProcess)
		pProcess->start(StartCmd);
	else
		QProcess::startDetached(StartCmd);
	return SB_OK;
}

QString CSbieAPI::GetStartPath() const
{
	return m_SbiePath + "//" + QString::fromWCharArray(SBIESTART_EXE);
}

SB_STATUS CSbieAPI::ReloadBoxes(bool bFullUpdate)
{
	QMap<QString, CSandBoxPtr> OldSandBoxes = m_SandBoxes;

	for (int i = 0;;i++)
	{
		QString BoxName = SbieIniGet(QString(), QString(), (i | CONF_GET_NO_EXPAND));
		if (BoxName.isNull())
			break;

		bool bIsEnabled;
		if (!IsBox(BoxName, bIsEnabled))
			continue;

		CSandBoxPtr pBox = OldSandBoxes.take(BoxName.toLower());
		if (!pBox)
		{
			pBox = CSandBoxPtr(NewSandBox(BoxName, this));
			m_SandBoxes.insert(BoxName.toLower(), pBox);
			UpdateBoxPaths(pBox);
		}
		else if(bFullUpdate)
			UpdateBoxPaths(pBox);

		pBox->m_IsEnabled = bIsEnabled;

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
		return SB_ERR(SB_NotAuthorized, QVariantList() << SectionName, status);
	}
	return SB_ERR(SB_ConfigFailed, QVariantList() << SettingName << SectionName << (quint32)status, status);
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
	//if (!Status) 
	//	emit LogSbieMessage(0xC1020000 | 2203, QStringList() << "" << Status.GetText() << "", GetCurrentProcessId());
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

SB_STATUS CSbieAPI::ValidateName(const QString& BoxName)
{
	if (BoxName.length() > 32)
		return SB_ERR(SB_NameLenLimit);

	QStringList DeviceNames = QStringList() <<
		"aux" << "clock$" << "con" << "nul" << "prn" <<
		"com1" << "com2" << "com3" << "com4" << "com5" << "com6" << "com7" << "com8" << "com9" << "com0" <<
		"lpt1" << "lpt2" << "lpt3" << "lpt4" << "lpt5" << "lpt6" << "lpt7" << "lpt8" << "lpt9" << "lpt0";
	if (DeviceNames.contains(BoxName, Qt::CaseInsensitive))
		return SB_ERR(SB_BadNameDev);

	if(BoxName.compare("GlobalSettings", Qt::CaseInsensitive) == 0)
		return SB_ERR(SB_BadNameDev);
	if(BoxName.left(13).compare("UserSettings_", Qt::CaseInsensitive) == 0)
		return SB_ERR(SB_BadNameDev);

	if (BoxName.contains(QRegExp("[^A-Za-z0-9_]")))
		return SB_ERR(SB_BadNameChar);

	return SB_OK;
}

SB_STATUS CSbieAPI::CreateBox(const QString& BoxName)
{
	SB_STATUS Status = ValidateName(BoxName);
	if(Status.IsError())
		return Status;

	if(m_SandBoxes.contains(BoxName.toLower()))
		return SB_ERR(SB_NameExists);

	Status = SbieIniSet(BoxName, "Enabled", "y");
	if (Status.IsError()) 
		return Status;

	ReloadConfig();
	ReloadBoxes();
	return Status;
}

SB_STATUS CSbieAPI::UpdateProcesses(bool bKeep)
{
	foreach(const CSandBoxPtr& pBox, m_SandBoxes)
		UpdateProcesses(bKeep, pBox);
	return SB_OK;
}

SB_STATUS CSbieAPI__GetProcessPIDs(SSbieAPI* m, const QString& BoxName, ULONG* pids, ULONG* count)
{
	WCHAR box_name[34];
	BoxName.toWCharArray(box_name); // fix-me: potential overflow
	box_name[BoxName.size()] = L'\0';
	BOOLEAN all_sessions = TRUE;
	ULONG which_session = 0; // -1 for current session

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

	memset(parms, 0, sizeof(parms));
	parms[0] = API_ENUM_PROCESSES;
	parms[1] = (ULONG64)pids;
	parms[2] = (ULONG64)box_name;
	parms[3] = (ULONG64)all_sessions;
	parms[4] = (ULONG64)which_session;
	parms[5] = (ULONG64)count;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::UpdateProcesses(bool bKeep, const CSandBoxPtr& pBox)
{
	ULONG count = 1024;
	ULONG boxed_pids[1024]; // ULONG [512]
	SB_STATUS Status = CSbieAPI__GetProcessPIDs(m, pBox->GetName(), boxed_pids, &count);
	if (Status.IsError())
		return Status;

	QMap<quint32, CBoxedProcessPtr>	OldProcessList = pBox->m_ProcessList;

	for (int i=0; i < count; i++)
	{
		quint32 ProcessId = boxed_pids[i];

		CBoxedProcessPtr pProcess = OldProcessList.take(ProcessId);
		if (!pProcess)
		{
			pProcess = CBoxedProcessPtr(NewBoxedProcess(ProcessId, pBox.data()));
			pBox->m_ProcessList.insert(ProcessId, pProcess);
			m_BoxedProxesses.insert(ProcessId, pProcess);

			UpdateProcessInfo(pProcess);
			pProcess->InitProcessInfo();
		}

		// todo:
	}

	foreach(const CBoxedProcessPtr& pProcess, OldProcessList) 
	{
		if (!pProcess->IsTerminated())
			pProcess->SetTerminated();
		else if (!bKeep && pProcess->IsTerminated(1500)) { // keep for at least 1.5 seconds
			pBox->m_ProcessList.remove(pProcess->m_ProcessId);
			m_BoxedProxesses.remove(pProcess->m_ProcessId);
		}
	}

	bool WasBoxClosed = pBox->m_ActiveProcessCount > 0 && count == 0;
	pBox->m_ActiveProcessCount = count;
	if (WasBoxClosed) {
		pBox->CloseBox();
		emit BoxClosed(pBox->GetName());
	}

	return SB_OK;
}

bool CSbieAPI::HasProcesses(const QString& BoxName)
{
	ULONG count;
	return CSbieAPI__GetProcessPIDs(m, BoxName, NULL, &count) && (count > 0);
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

	pSandBox->m_FilePath = Nt2DosPath(QString::fromWCharArray(FileRoot.c_str(), wcslen(FileRoot.c_str())));
	pSandBox->m_RegPath = QString::fromWCharArray(KeyRoot.c_str(), wcslen(KeyRoot.c_str()));
	pSandBox->m_IpcPath = QString::fromWCharArray(IpcRoot.c_str(), wcslen(IpcRoot.c_str()));
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

CSandBoxPtr CSbieAPI::GetBoxByProcessId(quint32 ProcessId) const
{
	CBoxedProcessPtr pProcess = m_BoxedProxesses.value(ProcessId);
	if (!pProcess)
		return CSandBoxPtr();
	return m_SandBoxes.value(pProcess->GetBoxName().toLower());
}

CBoxedProcessPtr CSbieAPI::GetProcessById(quint32 ProcessId) const
{
	return m_BoxedProxesses.value(ProcessId);
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
			Status = SB_ERR(SB_FailedKillAll);
	}
	return Status;
}

SB_STATUS CSbieAPI::Terminate(quint32 ProcessId)
{
	PROCESS_KILL_ONE_REQ req;
	req.h.length = sizeof(PROCESS_KILL_ONE_REQ);
	req.h.msgid = MSGID_PROCESS_KILL_ONE;
	req.pid = ProcessId;

	MSG_HEADER *rpl = NULL;
	SB_STATUS Status = CSbieAPI::CallServer(&req.h, &rpl);
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
		emit LogSbieMessage(0xC1020000 | 2321, QStringList() << "" << QString("%1").arg(status, 8, 16) << "", GetCurrentProcessId());
	}
	else 
	{
		WCHAR dirname[128];
		ULONG length = sizeof(dirname);
		status = NtQueryObject(handle, ObjectNameInformation, dirname, length, &length);

		NtClose(handle);

		if (!NT_SUCCESS(status)) {
			emit LogSbieMessage(0xC1020000 | 2321, QStringList() << "" << QString("%1").arg(status, 8, 16) << "", GetCurrentProcessId());
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

SB_STATUS CSbieAPI__ProceccssExemptionControl(SSbieAPI* m, quint32 process_id, quint32 action_id, ULONG *NewState, ULONG *OldState)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_PROCESS_EXEMPTION_CONTROL_ARGS *args = (API_PROCESS_EXEMPTION_CONTROL_ARGS *)parms;

	memzero(parms, sizeof(parms));
	args->func_code = API_PROCESS_EXEMPTION_CONTROL;
	args->process_id.val64 = process_id;
	args->action_id.val = action_id;
	args->set_flag.val = NewState;
	args->get_flag.val = OldState;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::SetProcessExemption(quint32 process_id, quint32 action_id, bool NewState)
{
	ULONG uNewState = NewState ? TRUE : FALSE;
	return CSbieAPI__ProceccssExemptionControl(m, process_id, action_id, &uNewState, NULL);
}

bool CSbieAPI::GetProcessExemption(quint32 process_id, quint32 action_id)
{
	ULONG uOldState = FALSE;
	CSbieAPI__ProceccssExemptionControl(m, process_id, action_id, NULL, &uOldState);
	return uOldState != FALSE;
}

QString CSbieAPI::GetBoxedPath(const QString& BoxName, const QString& Path)
{
	CSandBoxPtr pBox = GetBoxByName(BoxName);
	if (!pBox)
		return QString();
	return GetBoxedPath(pBox, Path);
}

QString CSbieAPI::GetBoxedPath(const CSandBoxPtr& pBox, const QString& Path)
{
	QString BoxRoot = pBox->m_FilePath;

	if (Path.indexOf("\\device\\mup", 0, Qt::CaseInsensitive) == 0)
		return BoxRoot + "\\share" + Path.mid(11);

	if (pBox->GetBool("SeparateUserFolders", true))
	{
		if (Path.indexOf(m_UserDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\current" + Path.mid(m_UserDir.length());
		else if (Path.indexOf(m_ProgramDataDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\all" + Path.mid(m_ProgramDataDir.length());
		else if (Path.indexOf(m_PublicDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\public" + Path.mid(m_PublicDir.length());
	}

	if (Path.length() < 3 || Path.at(1) != ':')
		return QString();
	
	return BoxRoot + "\\drive\\" + Path.at(0) + Path.mid(2);
}

QString CSbieAPI::GetRealPath(const CSandBoxPtr& pBox, const QString& Path)
{
	QString RealPath;
	QString BoxRoot = pBox->m_FilePath;
	if (BoxRoot.right(1) == "\\") BoxRoot.truncate(BoxRoot.length() - 1);

	if (Path.length() < BoxRoot.length())
		return RealPath;

	RealPath = Path.mid(BoxRoot.length());

	if (RealPath.left(6) == "\\share")
		RealPath = "\\device\\mup" + RealPath.mid(6);

	if (RealPath.left(5) == "\\user")
	{
		if (RealPath.mid(5, 8) == "\\current")
			RealPath = m_UserDir + RealPath.mid(5 + 8);
		else if (RealPath.mid(5, 4) == "\\all")
			RealPath = m_ProgramDataDir + RealPath.mid(5 + 4);
		else if (RealPath.mid(5, 7) == "\\public")
			RealPath = m_PublicDir + RealPath.mid(5 + 7);
	}

	if (RealPath.left(6) == "\\drive")
		RealPath = RealPath.mid(7, 1) + ":" + RealPath.mid(8);

	return RealPath;
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

	emit ConfigReloaded();

	//emit LogMessage("Sandboxie config has been reloaded.", false);
	emit LogSbieMessage(0, QStringList() << "Sandboxie config has been reloaded" << "" << "", 4);

	ReloadBoxes(true);

	return SB_OK;
}

bool CSbieAPI::IsBox(const QString& BoxName, bool& bIsEnabled)
{
	wstring box_name = BoxName.toStdWString();

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_IS_BOX_ENABLED_ARGS *args = (API_IS_BOX_ENABLED_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_IS_BOX_ENABLED;
	args->box_name.val = (WCHAR*)box_name.c_str();

	NTSTATUS status = m->IoControl(parms);
	bIsEnabled = NT_SUCCESS(status);
	return bIsEnabled || status == STATUS_ACCOUNT_RESTRICTION;
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

/*QString CSbieAPI__FormatSbieMsg(SSbieAPI* m, ULONG code, const WCHAR *ins1, const WCHAR *ins2)
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
}*/

//QString CSbieAPI__GetTag(const QString& data, const QString& tag)
//{
//	int pos = data.indexOf(tag + "=");
//	if(pos == -1)
//		return QString();
//	pos += tag.length() + 1;
//
//	int end = data.indexOf(" ", pos);
//	QString value =  data.mid(pos, end == -1 ? -1 : end - pos);
//
//	return value;
//}

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

	m->lastMessageNum = MessageNum;

	if (m->clearingBuffers)
		return true; 

	QStringList MsgData;
	MsgData.append("");
	for (size_t pos = 0; pos < msgtext.Length; ) {
		size_t len = wcslen((WCHAR*)(msgtext.Buffer + pos));
		if (len == 0)
			break;
		MsgData.append(QString::fromWCharArray((WCHAR*)(msgtext.Buffer + pos), len));
		pos += (len + 1) * sizeof(WCHAR);
	}
	while (MsgData.length() < 3)
		MsgData.append("");

	/*
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
	quint16 MessageId= MsgCode & 0xFFFF;*/

	if ((MsgCode & 0xFFFF) == 1399) // Process Start Notification
	{
		emit ProcessBoxed(ProcessId, Nt2DosPath(MsgData[1]), MsgData[2], MsgData.length() < 4 ? 0 : MsgData[3].toUInt());
		return true;
	}
	
	if ((MsgCode & 0xFFFF) == 2199) // Auto Recovery notification
	{
		emit FileToRecover(MsgData[1], Nt2DosPath(MsgData[2]), ProcessId);
		return true;
	}

	/*
	QString Message = CSbieAPI__FormatSbieMsg(m, MsgCode, str1, str2);
	if (ProcessId != 4) // if its not from the driver add the pid
		Message += tr(", by process: %1").arg(ProcessId);

	emit LogMessage(Message, MessageId);
	*/

	emit LogSbieMessage(MsgCode, MsgData, ProcessId);
	return true;
}

CBoxedProcessPtr CSbieAPI::OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId)
{
	CBoxedProcessPtr pProcess = m_BoxedProxesses.value(ProcessId);
	if (!pProcess)
	{
		CSandBoxPtr pBox = GetBoxByName(Box);
		if (!pBox)
			return CBoxedProcessPtr();
		
		pProcess = CBoxedProcessPtr(NewBoxedProcess(ProcessId, pBox.data()));
		pBox->m_ProcessList.insert(ProcessId, pProcess);
		m_BoxedProxesses.insert(ProcessId, pProcess);

		UpdateProcessInfo(pProcess);
		pProcess->InitProcessInfo();
	}

	if (pProcess->m_ParendPID == 0){
		pProcess->m_ParendPID = ParentId;
		pProcess->m_ImagePath = Path;
	}
	if(pProcess->m_ImageName.isEmpty())
		pProcess->m_ImageName = Path.mid(Path.lastIndexOf("\\") + 1);

	return pProcess;
}

///////////////////////////////////////////////////////////////////////////////
// Forced Processes
//

SB_STATUS CSbieAPI__ForceProcessControl(SSbieAPI* m, ULONG *NewState, ULONG *OldState)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_DISABLE_FORCE_PROCESS_ARGS* args = (API_DISABLE_FORCE_PROCESS_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_DISABLE_FORCE_PROCESS;
	args->set_flag.val = NewState;
	args->get_flag.val = OldState;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::DisableForceProcess(bool Set, int Seconds)
{
	if(Seconds > 0)
		m_pGlobalSection->SetNum("ForceDisableSeconds", Seconds);

	ULONG uNewState = Set ? TRUE : FALSE;
	return CSbieAPI__ForceProcessControl(m, &uNewState, NULL);
}

bool CSbieAPI::AreForceProcessDisabled()
{
	ULONG uOldState = FALSE;
	CSbieAPI__ForceProcessControl(m, NULL, &uOldState);
	return uOldState != FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// Monitor
//

SB_STATUS CSbieAPI__MonitorControl(SSbieAPI* m, ULONG *NewState, ULONG *OldState)
{
	//ULONG Used = 0;

    __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_CONTROL_ARGS* args	= (API_MONITOR_CONTROL_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code = API_MONITOR_CONTROL;
    args->set_flag.val = NewState;
    args->get_flag.val = OldState;
	//args->get_used.val = &Used;
    
	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);

	//qDebug() << "used bytes" << (quint32)Used;

	return SB_OK;
}

SB_STATUS CSbieAPI::EnableMonitor(bool Enable)
{
	ULONG uNewState = Enable ? TRUE : FALSE;
	return CSbieAPI__MonitorControl(m, &uNewState, NULL);
}

bool CSbieAPI::IsMonitoring()
{
	ULONG uOldState = FALSE;
	CSbieAPI__MonitorControl(m, NULL, &uOldState);
	return uOldState != FALSE;
}

bool CSbieAPI::GetMonitor()
{
	ULONG type;
	ULONG pid = 0;
	ULONG tid = 0;
	wchar_t* Buffer[4 * 1024];
	ULONG Length = ARRAYSIZE(Buffer);

	ULONG RecordNum = m->lastRecordNum;

	__declspec(align(8)) UNICODE_STRING64 log_buffer = { 0, (USHORT)Length, (ULONG64)Buffer };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_GET_EX_ARGS* args	= (API_MONITOR_GET_EX_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code	= API_MONITOR_GET_EX;
	args->log_seq.val = &RecordNum; // set this to NULL for record clearing
    args->log_type.val = &type;
	args->log_pid.val = &pid;
	args->log_tid.val = &tid;
	args->log_data.val = &log_buffer;
    
	if (!NT_SUCCESS(m->IoControl(parms)))
		return false; // error or no more entries

	if (type == 0) // versions prioir to 5.45.0 return success and type 0 when there are no more entries
		return false;

	if (RecordNum != m->lastRecordNum + 1 && !m->clearingBuffers)
		emit LogSbieMessage(0xC1020000 | 1242, QStringList() << "" << "" << "", GetCurrentProcessId()); // Monitor buffer overflow
	m->lastRecordNum = RecordNum;

	if (m->clearingBuffers)
		return true; 

	QString Data = QString::fromWCharArray((wchar_t*)log_buffer.Buffer, log_buffer.Length / sizeof(wchar_t));
	if (Data.length() == Length - 1) // if we got exactly the max length assume data were truncated and indicate accordingly...
		Data += "..."; 

	// cleanup debug output strings and drop empty once.
	if (type == (MONITOR_OTHER | MONITOR_TRACE)) {
		Data = Data.trimmed();
		if (Data.isEmpty())
			return true;
	}

	CTraceEntryPtr LogEntry = CTraceEntryPtr(new CTraceEntry(pid, tid, type, Data));
	AddTraceEntry(LogEntry, true);

	return true;
}

void CSbieAPI::AddTraceEntry(const CTraceEntryPtr& LogEntry, bool bCanMerge)
{
	QWriteLocker Lock(&m_TraceMutex);

	if (bCanMerge && !m_TraceList.isEmpty() && m_TraceList.last()->Equals(LogEntry)) {
		m_TraceList.last()->Merge(LogEntry);
		return;
	}

	m_TraceList.append(LogEntry);
}

///////////////////////////////////////////////////////////////////////////////
// Other 
//

QString CSbieAPI::GetSbieMsgStr(quint32 code, quint32 Lang)
{
	ULONG FormatFlags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	WCHAR* ret_str = NULL;
	if (!m->SbieMsgDll || (FormatMessage(FormatFlags, m->SbieMsgDll, code, Lang, (LPWSTR)&ret_str, 4, NULL) == 0
						&& FormatMessage(FormatFlags, m->SbieMsgDll, code, 1033, (LPWSTR)&ret_str, 4, NULL) == 0))
		return QString("SBIE%0: %1; %2").arg(code, 4, 10);
	QString qStr = QString::fromWCharArray(ret_str);
	LocalFree(ret_str);
	return qStr.trimmed(); // note messages may have \r\n at the end
}
