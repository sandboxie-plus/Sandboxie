/* 
 * Copyright (c) 2020-2023, David Xanatos
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
#include "..\..\Sandboxie\common\defines.h"

#include "..\..\Sandboxie\core\drv\api_defs.h"
#include "..\..\Sandboxie\core\drv\api_flags.h"

#include "..\..\Sandboxie\core\svc\msgids.h"
#include "..\..\Sandboxie\core\svc\ProcessWire.h"
#include "..\..\Sandboxie\core\svc\GuiWire.h"
#include "..\..\Sandboxie\core\svc\sbieiniwire.h"
#include "..\..\Sandboxie\core\svc\QueueWire.h"
#include "..\..\Sandboxie\core\svc\InteractiveWire.h"
#include "..\..\Sandboxie\core\svc\MountManagerWire.h"

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

		wsprintfW(QueueName, L"*%s_%08X", INTERACTIVE_QUEUE_NAME, sessionId);

		lastMessageNum = 0;
		//lastRecordNum = 0;
		traceBuffer = NULL;
		traceBufferLen = 0;

		SbieMsgDll = NULL;

		SvcLock = 0;
	}
	~SSbieAPI() {
		if (traceBuffer) 
			free(traceBuffer);
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
	//ULONG lastRecordNum;
	UCHAR* traceBuffer;
	ULONG traceBufferLen;

	HMODULE SbieMsgDll;

	mutable volatile LONG   SvcLock;
	mutable MSG_HEADER*		SvcReq;
	mutable CSbieAPI::SScopedVoid* SvcRpl;
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

	m_pGlobalSection = QSharedPointer<CSbieIni>(new CSbieIni("GlobalSettings", this));
	m_pUserSection = QSharedPointer<CSbieIni>(new CSbieIni("UserSettings", this)); // dummy

	m_IniReLoad = false;
	m_bReloadPending = false;
	m_bBoxesDirty = false;

	connect(&m_IniWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(OnIniChanged(const QString&)));
	connect(this, SIGNAL(ProcessBoxed(quint32, const QString&, const QString&, quint32, const QString&)), this, SLOT(OnProcessBoxed(quint32, const QString&, const QString&, quint32, const QString&)));
}

CSbieAPI::~CSbieAPI()
{
	Disconnect();

	delete m;
}

bool CSbieAPI::IsSbieCtrlRunning()
{
	static const WCHAR *SbieCtrlMutexName = SANDBOXIE L"_SingleInstanceMutex_Control";

	HANDLE hSbieCtrlMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, SbieCtrlMutexName);
	if (hSbieCtrlMutex) {
		CloseHandle(hSbieCtrlMutex);
		return true;
	}
	return false;
}

bool CSbieAPI::TerminateSbieCtrl()
{
	static const WCHAR *WindowClassName = L"SandboxieControlWndClass";

	HWND hwnd = FindWindowW(WindowClassName, NULL);
	if (hwnd) {
		PostMessage(hwnd, WM_QUIT, 0, 0);
	}

	for (int i = 0; i < 10 && hwnd != NULL; i++) {
		QThread::msleep(100);
		hwnd = FindWindowW(WindowClassName, NULL);
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
		len = ExpandEnvironmentStringsW((WCHAR*)value->Data, expand, MAX_PATH + 4);

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
			m_UserSid = QString::fromWCharArray(objname.Buffer);

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

SB_STATUS CSbieAPI::Connect(bool takeOver, bool withQueue)
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
	m->SbieMsgDll = LoadLibraryExW((m_SbiePath.toStdWString() + L"\\" SBIEMSG_DLL).c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);

	m->lastMessageNum = 0;
	//m->lastRecordNum = 0;

	// Note: this lib is not using all functions, hence it can be compatible with multiple driver ABI revisions
	//QStringList CompatVersions = QStringList () << "5.55.0";
	//QString CurVersion = GetVersion();
	//if (!CompatVersions.contains(CurVersion))
	//{
	//	NtClose(m->SbieApiHandle);
	//	m->SbieApiHandle = INVALID_HANDLE_VALUE;	
	//	return SB_ERR(SB_Incompatible, QVariantList() << CurVersion << CompatVersions.join(", "));
	//}

	SB_STATUS Status = SB_OK;
	if (takeOver) {
		Status = TakeOver();
		if (!Status) // only the session leader manages the interactive queue
			withQueue = false;
	}

	m_bWithQueue = withQueue;
	m_bTerminate = false;
	m->clearingBuffers = true;
	start();

	bool bHome = false;
	m_IniPath = GetIniPath(&bHome);
	qDebug() << "Config file:" << m_IniPath << (bHome ? "(home)" : "(system)");

	if (m_UserName.isEmpty()) {
		QString UserSection = GetUserSection(&m_UserName);
		if(!UserSection.isEmpty())
			m_pUserSection = QSharedPointer<CSbieIni>(new CSbieIni(UserSection, this));
	}

	if (m_UserDir.isEmpty()) {
		GetUserPaths();
	}

	emit StatusChanged();
	return Status;
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

	// ensure that the client thread is entirely terminated to avoid the service keeping the reference
	// see SbieSvc PortDisconnectByCreateTime
	QThread::msleep(250); 
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
	m_bBoxesDirty = true;

	emit StatusChanged();
	return SB_OK;
}

bool CSbieAPI::IsConnected() const
{
	return m->SbieApiHandle != INVALID_HANDLE_VALUE;
}

bool CSbieAPI::IsWow64()
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

	// Function associate PortHandle with thread, and sends LPC_TERMINATION_MESSAGE to specified port immediately after call NtTerminateThread.
	//NtRegisterThreadTerminatePort(m->PortHandle);

	m->SizeofPortMsg = sizeof(PORT_MESSAGE);
	if (CSbieAPI::IsWow64())
		m->SizeofPortMsg += sizeof(ULONG) * 4;
	m->MaxDataLen -= m->SizeofPortMsg;

	return SB_OK;
}

SB_STATUS CSbieAPI__CallServer(SSbieAPI* m, MSG_HEADER* req, CSbieAPI::SScopedVoid* prpl)
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

		// clear highest byte of the size field
		ResData[3] = 0;
		BuffLen = ((MSG_HEADER*)ResData)->length;
	}
	else
		BuffLen = 0;
	if (BuffLen == 0)
		return SB_ERR(SB_ServiceFail, QVariantList() << QString("null reply (msg %1 len %2)").arg(req->msgid, 8, 16).arg(req->length)); // 2203

	// read remaining chunks
	MSG_HEADER* rpl = (MSG_HEADER*)malloc(BuffLen);
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
	prpl->Assign(rpl, Buffer - (UCHAR*)rpl);

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
	CSbieAPI::SScoped<QUEUE_CREATE_RPL> rpl;
	CSbieAPI__CallServer(m, &req.h, &rpl);
	if (rpl) 
		status = rpl->h.status;

	if (!NT_SUCCESS(status)) {
		CloseHandle((HANDLE)(ULONG_PTR)req.event_handle);
		return SB_ERR(status);
	}

	*out_EventHandle = (HANDLE)(ULONG_PTR)req.event_handle;

	return SB_OK;
}

bool CSbieAPI::GetQueueReq()
{
	QUEUE_GETREQ_REQ req;
	req.h.length = sizeof(QUEUE_GETREQ_REQ);
	req.h.msgid = MSGID_QUEUE_GETREQ;
	wcscpy(req.queue_name, m->QueueName);

	NTSTATUS status = STATUS_SERVER_DISABLED;
	SScoped<QUEUE_GETREQ_RPL> rpl;
	CSbieAPI__CallServer(m, &req.h, &rpl);
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
				case -1:
					Data["id"] = 0;
					break;
			}

			emit QueuedRequest(rpl->client_pid, rpl->client_tid, rpl->req_id, Data);
		}
	}

	//if(status == STATUS_END_OF_FILE) // there are no more requests in the queue at this time
	//	return false; // nothing more to do

	if (NT_SUCCESS(status))
		return true; // we did something
	return false;
}

void CSbieAPI::SendQueueRpl(quint32 RequestId, const QVariantMap& Result)
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
		default:
		{
			ULONG rpl[2];
			rpl[0] = STATUS_SUCCESS;
			rpl[1] = Result["retval"].toUInt();
			Data = QByteArray((char*)&rpl, sizeof(rpl));
		}
	}

	ULONG req_len = sizeof(QUEUE_PUTRPL_REQ) + Data.length();
	SScoped<QUEUE_PUTRPL_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_QUEUE_PUTRPL;
	wcscpy(req->queue_name, m->QueueName);
	req->req_id = RequestId;
	req->data_len = Data.length();
	memcpy(req->data, Data.constData(), req->data_len);

	NTSTATUS status = STATUS_SERVER_DISABLED;
	SScoped<QUEUE_PUTRPL_RPL> rpl;
	CallServer(&req->h, &rpl);
	if (rpl)
		status = rpl->h.status;
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
			m->SvcStatus = CSbieAPI__CallServer(m, m->SvcReq, m->SvcRpl);

			InterlockedExchange(&m->SvcLock, SVC_OP_STATE_DONE);

			Done++;
		}

		if (EventHandle != NULL && WaitForSingleObject(EventHandle, 0) == 0)
		{
			while(GetQueueReq())
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

SB_STATUS CSbieAPI::CallServer(void* req, SScopedVoid* prpl) const
{
	//
	// Note: Once we open a port to the server from a threat the service will remember it we can't reconnect after disconnection
	//			So for every new connection we need a new threat, we achieve this by letting our monitor threat issue all requests
	// 
	//		 As of Sbie build 5.50.5 the SbieCvc properly handles reconnection attempts so this mechanism is no longer necessary
	// 	     However for the queue mechanism we need the communication to be still handled by the helper thread
	//

	while (InterlockedCompareExchange(&m->SvcLock, SVC_OP_STATE_PREP, SVC_OP_STATE_IDLE) != SVC_OP_STATE_IDLE)
		QThread::usleep(100);

	m->SvcReq = (MSG_HEADER*)req;
	m->SvcRpl = prpl;
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
	
	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::WatchIni(bool bEnable, bool bReLoad)
{
	if (bEnable) {
		m_IniWatcher.addPath(m_IniPath);
		m_IniReLoad = bReLoad;
	}
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
	m_bBoxesDirty = true;
	if (m_IniReLoad) 
		ReloadConfig(true);
}

extern "C" {
	NTSTATUS NTAPI RtlSetThreadErrorMode(IN ULONG NewMode, OUT PULONG OldMode);
}

quint32 CSbieAPI::GetVolumeSN(const wchar_t* path, std::wstring* pLabel)
{
    ULONG sn = 0;
    HANDLE handle;
    IO_STATUS_BLOCK iosb;

    UNICODE_STRING objname;
	size_t path_len = wcslen(path);
	objname.Buffer = new wchar_t[path_len + 2];
    wmemcpy(objname.Buffer, path, path_len);
    objname.Buffer[path_len    ] = L'\\';
    objname.Buffer[path_len + 1] = L'\0';
    
    objname.Length = (USHORT)(path_len + 1) * sizeof(WCHAR);
    objname.MaximumLength = objname.Length + sizeof(WCHAR);

    OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(
        &objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	ULONG OldMode;
	RtlSetThreadErrorMode(0x10u, &OldMode);
    NTSTATUS status = NtCreateFile(
        &handle, GENERIC_READ | SYNCHRONIZE, &objattrs,
        &iosb, NULL, 0, FILE_SHARE_VALID_FLAGS,
        FILE_OPEN,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);
	RtlSetThreadErrorMode(OldMode, 0i64);

    delete [] objname.Buffer;

    if (NT_SUCCESS(status))
    {
        union {
            FILE_FS_VOLUME_INFORMATION volumeInfo;
            BYTE volumeInfoBuff[64];
        } u;
		if (NT_SUCCESS(NtQueryVolumeInformationFile(handle, &iosb, &u.volumeInfo, sizeof(u), FileFsVolumeInformation))) {
			sn = u.volumeInfo.VolumeSerialNumber;
			if (pLabel) *pLabel = std::wstring(u.volumeInfo.VolumeLabel, u.volumeInfo.VolumeLabelLength/sizeof(wchar_t));
		}

        NtClose(handle);
    }

    return sn;
}

QString CSbieAPI::ResolveAbsolutePath(const QString& Path)
{
	std::wstring path = Path.toStdWString();
	UNICODE_STRING uni;
    RtlInitUnicodeString(&uni, path.c_str());
	OBJECT_ATTRIBUTES objattrs;
    InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE FileHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS status = NtCreateFile(&FileHandle, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &objattrs, &IoStatusBlock, NULL, 0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,NULL, 0);

    if (NT_SUCCESS(status)) {

		const ULONG NameLen = 4096;
		WCHAR NameBuf[NameLen];

		__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
		API_GET_FILE_NAME_ARGS *args = (API_GET_FILE_NAME_ARGS *)parms;

		memset(parms, 0, sizeof(parms));
		args->func_code               = API_GET_FILE_NAME;
		args->handle.val64            = (ULONG64)(ULONG_PTR)FileHandle;
		args->name_len.val64          = (ULONG64)(ULONG_PTR)(NameLen * sizeof(WCHAR));
		args->name_buf.val64          = (ULONG64)(ULONG_PTR)NameBuf;
		status = m->IoControl(parms);

        NtClose(FileHandle);

		if (NT_SUCCESS(status))
			return QString::fromWCharArray(NameBuf);
    }

	return Path;
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
		uint size = QueryDosDeviceW(drv, lpTargetPath, MAX_PATH);
		if (size > 0)
		{
			SDrive Drive;
			QString Key = QString::fromWCharArray(lpTargetPath);
			QStringList Chunks = Key.split("\\");
			if (Chunks.count() >= 5 && Chunks[2].compare("LanmanRedirector", Qt::CaseInsensitive) == 0) {
				Drive.Type = SDrive::EShare;
				Chunks.removeAt(3);
				Key = Chunks.join("\\");
				Drive.Aux = Chunks.mid(3).join("\\");
			}
			else {
				Drive.Type = SDrive::EVolume;
				if (quint32 sn = GetVolumeSN(lpTargetPath))
					Drive.Aux = QString("%1-%2").arg((ushort)HIWORD(sn), 4, 16, QChar('0')).arg((ushort)LOWORD(sn), 4, 16, QChar('0')).toUpper();
			}
			Key.append("\\");
			Drive.Letter = QString::fromWCharArray(drv) + "\\";
			Drive.NtPath = Key;

			m_DriveLetters.insert(Drive.Letter, Drive);
		}
	}
}

QString CSbieAPI::Nt2DosPath(QString NtPath, bool* pOk) const
{
	if (NtPath.left(11) == "\\??\\Volume{") {
		wchar_t DeviceName [MAX_PATH];
		std::wstring VolumeName = NtPath.mid(4, 44).toStdWString();
		if (QueryDosDeviceW(VolumeName.c_str(), DeviceName, ARRAYSIZE(DeviceName)))
			NtPath.replace(0, 48, QString::fromWCharArray(DeviceName));
	}

	if (NtPath.indexOf("\\device\\mup", 0, Qt::CaseInsensitive) == 0)
		NtPath = "\\Device\\LanmanRedirector" + NtPath.mid(11);

	QReadLocker Lock(&m_DriveLettersMutex);

	for (QMap<QString, SDrive>::const_iterator I = m_DriveLetters.begin(); I != m_DriveLetters.end(); ++I)
	{
		const SDrive& Drive = I.value();
		if (Drive.NtPath.compare(NtPath.left(Drive.NtPath.length()), Qt::CaseInsensitive) == 0) {
			if(pOk) *pOk = true;
			return NtPath.replace(0, Drive.NtPath.length(), Drive.Letter);
		}
	}
	if(pOk) *pOk = false;
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

	SScoped<SBIE_INI_GET_PATH_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return QString();
	if (rpl->h.status == 0) {
		IniPath = QString::fromWCharArray(rpl->path);
		if (IsHome)
			*IsHome = rpl->is_home_path;
	}
	
	return IniPath;
}

QString CSbieAPI::GetUserSection(QString* pUserName, bool* pIsAdmin) const
{
	QString UserSection;

	SBIE_INI_GET_USER_REQ req;
	req.h.msgid = MSGID_SBIE_INI_GET_USER;
	req.h.length = sizeof(SBIE_INI_GET_USER_REQ);

	SScoped<SBIE_INI_GET_USER_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return QString();

	if (rpl->h.status == 0) {
		if (pIsAdmin) *pIsAdmin = rpl->admin;
		UserSection = QString::fromWCharArray(rpl->section);
		if (pUserName) *pUserName = QString::fromWCharArray(rpl->name);
	}

	return UserSection;
}

SB_RESULT(quint32) CSbieAPI::RunStart(const QString& BoxName, const QString& Command, EStartFlags Flags, const QString& WorkingDir, QProcess* pProcess)
{
	if (m_SbiePath.isEmpty())
		return SB_ERR(SB_PathFail);

	QString StartArgs;
	if (Flags & eStartElevated)
		StartArgs += "/elevated ";
	if (Flags & eStartFCP)
		StartArgs += "/fcp ";

	if (!BoxName.isEmpty())
		StartArgs += "/box:" + BoxName + " ";
	else
		StartArgs += "/disable_force ";

	StartArgs += Command;

	qint64 pid = 0;
	//wchar_t sysPath[MAX_PATH];
	//GetSystemDirectoryW(sysPath, MAX_PATH);
	if (pProcess) {
		//pProcess->setWorkingDirectory(QString::fromWCharArray(sysPath));
		if (!WorkingDir.isEmpty())
			pProcess->setWorkingDirectory(WorkingDir);
		pProcess->setProgram(GetStartPath());
		pProcess->setNativeArguments(StartArgs);
		pProcess->start();
		pid = pProcess->processId();
	}
	else {
		QProcess process;
		//process.setWorkingDirectory(QString::fromWCharArray(sysPath));
		if (!WorkingDir.isEmpty())
			process.setWorkingDirectory(WorkingDir);
		process.setProgram(GetStartPath());
		process.setNativeArguments(StartArgs);
		process.startDetached(&pid);
	}

	/*
	QString CommandLine = "\"" + GetStartPath() + "\" " + StartArgs;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcessW( NULL,   // No module name (use command line)
        (wchar_t*)CommandLine.toStdWString().c_str(),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
		return SB_ERR();
    }

    // Wait until child process exits.
    //WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	*/

	if(pid == 0)
		return SB_ERR();
	return CSbieResult<quint32>((quint32)pid);
}

QString CSbieAPI::GetStartPath() const
{
	return m_SbiePath + "\\" + QString::fromWCharArray(SBIESTART_EXE);
}

quint32 CSbieAPI::GetSessionID() const
{
	return m->sessionId;
}

SB_STATUS CSbieAPI::ReloadBoxes(bool bForceUpdate)
{
	if (!bForceUpdate && !m_bBoxesDirty && !m_IniWatcher.files().isEmpty())
		return SB_OK;
	m_bBoxesDirty = false;

	QMap<QString, CSandBoxPtr> OldSandBoxes = m_SandBoxes;

	for (int i = 0;;i++)
	{
		QString BoxName = SbieIniGet(QString(), QString(), (i | CONF_GET_NO_EXPAND | CONF_GET_NO_TEMPLS));
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
			emit BoxAdded(pBox);
		}
		UpdateBoxPaths(pBox.data());

		pBox->m_IsEnabled = bIsEnabled;

		pBox->UpdateDetails();
	}

	foreach(const QString & BoxName, OldSandBoxes.keys()) {
		CSandBoxPtr pBox = m_SandBoxes.take(BoxName);
		pBox->SetBoxPaths(QString(), QString(), QString());
		emit BoxRemoved(pBox);
	}

	return SB_OK;
}

QString CSbieAPI__FormatNtStatus(long nsCode) 
{
	static HMODULE hNtDll = NULL;
	if(!hNtDll)
		hNtDll = GetModuleHandleW(L"ntdll.dll");
	if (hNtDll == NULL)
		return QString();

	WCHAR* ret_str = NULL;
    DWORD dwRes = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
        hNtDll, nsCode >= 0 ? nsCode : RtlNtStatusToDosError(nsCode), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&ret_str, 0, NULL);

	QString qStr = dwRes > 0 ? QString::fromWCharArray(ret_str) : QString();
    LocalFree(ret_str);
	return qStr;
}

SB_STATUS CSbieAPI::SbieIniSet(void *RequestBuf, void *pPasswordWithinRequestBuf, const QString& SectionName, const QString& SettingName)
{
retry:
	m->Password.toWCharArray((WCHAR*)pPasswordWithinRequestBuf); // fix-me: potential overflow
	((WCHAR*)pPasswordWithinRequestBuf)[m->Password.length()] = L'\0';

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer((MSG_HEADER *)RequestBuf, &rpl);
	SecureZeroMemory(pPasswordWithinRequestBuf, sizeof(WCHAR) * 64);
	if (!Status || !rpl)
		return Status;
	ULONG status = rpl->status;
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
	return SB_ERR(SB_ConfigFailed, QVariantList() << SettingName << SectionName << CSbieAPI__FormatNtStatus(status), status);
}

SB_STATUS CSbieAPI::SbieIniSet(const QString& Section, const QString& Setting, const QString& Value, ESetMode Mode, bool bRefresh)
{
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

	ULONG req_len = sizeof(SBIE_INI_SETTING_REQ) + Value.length() * sizeof(WCHAR);
	SScoped<SBIE_INI_SETTING_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->refresh = bRefresh ? TRUE : FALSE;

	Section.toWCharArray(req->section); // fix-me: potential overflow
	req->section[Section.length()] = L'\0';
	Setting.toWCharArray(req->setting); // fix-me: potential overflow
	req->setting[Setting.length()] = L'\0';
	Value.toWCharArray(req->value); // fix-me: potential overflow
	req->value[Value.length()] = L'\0';
	req->value_len = Value.length();
	req->h.msgid = msgid;
	req->h.length = sizeof(SBIE_INI_SETTING_REQ) + req->value_len * sizeof(WCHAR);

	SB_STATUS Status = SbieIniSet(&req->h, req->password, Section, Setting);
	//if (!Status) 
	//	emit LogSbieMessage(0xC1020000 | 2203, QStringList() << "" << Status.GetText() << "", GetCurrentProcessId());
	return Status;
}

void CSbieAPI::CommitIniChanges()
{
	bool bRemoved = m_IniWatcher.removePath(m_IniPath);
			
	SbieIniSet("", "", ""); // commit and refresh

	if (bRemoved) m_IniWatcher.addPath(m_IniPath);

	m_bBoxesDirty = true;
}

QString CSbieAPI::SbieIniGetEx(const QString& Section, const QString& Setting)
{
	QString Value;

	ULONG req_len = sizeof(SBIE_INI_SETTING_REQ);
	SScoped<SBIE_INI_SETTING_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	Section.toWCharArray(req->section); // fix-me: potential overflow
	req->section[Section.length()] = L'\0';
	Setting.toWCharArray(req->setting); // fix-me: potential overflow
	req->setting[Setting.length()] = L'\0';
	req->h.msgid = MSGID_SBIE_INI_GET_SETTING;
	req->h.length = sizeof(SBIE_INI_SETTING_REQ);

	SScoped<SBIE_INI_SETTING_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	if (!Status || !rpl)
		return QString();
	if (rpl->h.status == 0) {
		Value = QString::fromWCharArray(rpl->value, rpl->value_len - 1);
	}
	
	return Value;
}

QString CSbieAPI::SbieIniGet(const QString& Section, const QString& Setting, quint32 Index, qint32* ErrCode)
{
	std::wstring section = Section.toStdWString();
	std::wstring setting = Setting.toStdWString();

	WCHAR out_buffer[CONF_LINE_LEN] = { 0 };

	__declspec(align(8)) UNICODE_STRING64 Output = { 0, sizeof(out_buffer) - 4 , (ULONG64)out_buffer };
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

QString CSbieAPI::SbieIniGet2(const QString& Section, const QString& Setting, quint32 Index, bool bWithGlobal, bool bNoExpand, bool withTemplates)
{
	int flags = (bWithGlobal ? 0 : CONF_GET_NO_GLOBAL);
	if (!withTemplates)
		flags |= CONF_GET_NO_TEMPLS;
	if (bNoExpand)
		flags |= CONF_GET_NO_EXPAND;

	return SbieIniGet(Section, Setting, Index | flags);
}

SB_STATUS CSbieAPI::ValidateName(const QString& BoxName)
{
	if (BoxName.length() > (BOXNAME_COUNT - 2) || BoxName.isEmpty())
		return SB_ERR(SB_NameLenLimit);

	/* invalid file name characters on windows
	  < (less than)
	  > (greater than)
	  : (colon - sometimes works, but is actually NTFS Alternate Data Streams)
	  " (double quote)
	  / (forward slash)
	  \ (backslash)
	  | (vertical bar or pipe)
	  ? (question mark)
	  * (asterisk)
	*/

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

	if (BoxName.contains(QRegularExpression("[^A-Za-z0-9_]")))
		return SB_ERR(SB_BadNameChar);

	return SB_OK;
}

QString CSbieAPI::MkNewName(QString Name)
{
	Name.replace(QRegularExpression("[<>:\"/\\\\|?*\\[\\]]"), "-").replace(" ", "_");
	for (int i=0;; i++) {
		QString NewName = Name;
		if (i > 0) NewName.append("_" + QString::number(i));
		if (m_SandBoxes.contains(NewName.toLower()))
			continue;
		return NewName;
	}
}

SB_STATUS CSbieAPI::CreateBox(const QString& BoxName, bool bReLoad)
{
	SB_STATUS Status = ValidateName(BoxName);
	if(Status.IsError())
		return Status;

	if(m_SandBoxes.contains(BoxName.toLower()))
		return SB_ERR(SB_NameExists);

	Status = SbieIniSet(BoxName, "Enabled", "y");
	if (Status.IsError()) 
		return Status;

	if (bReLoad) {
		ReloadConfig();
		ReloadBoxes();
	}
	return Status;
}

SB_STATUS CSbieAPI__GetProcessPIDs(SSbieAPI* m, const QString& BoxName, bool bAllSessions, ULONG* pids, ULONG* count)
{
	WCHAR box_name[BOXNAME_COUNT];
	BoxName.toWCharArray(box_name); // fix-me: potential overflow
	box_name[BoxName.size()] = L'\0';
	BOOLEAN all_sessions = bAllSessions ? TRUE : false;
	ULONG which_session = -1; // -1 means current session, ignoreewd when all_sessions == true

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

SB_STATUS CSbieAPI::UpdateProcesses(int iKeep, bool bAllSessions)
{
	ULONG count = 0;
	SB_STATUS Status = CSbieAPI__GetProcessPIDs(m, "", bAllSessions, NULL, &count); // query count
	if (Status.IsError()) 
		return Status;

	count += 128; // add some extra space
	ULONG* boxed_pids = new ULONG[count]; 

	Status = CSbieAPI__GetProcessPIDs(m, "", bAllSessions, boxed_pids, &count); // query pids
	if (Status.IsError()) {
		delete[] boxed_pids;
		return Status;
	}

	QMap<quint32, CBoxedProcessPtr>	OldProcessList;
	foreach(const CSandBoxPtr& pBox, m_SandBoxes)
		OldProcessList.insert(pBox->m_ProcessList);

	for (int i=0; i < count; i++)
	{
		quint32 ProcessId = (quint32)boxed_pids[i];

		CBoxedProcessPtr pProcess = OldProcessList.take(ProcessId);
		if (!pProcess)
		{
			pProcess = CBoxedProcessPtr(NewBoxedProcess(ProcessId, NULL));
			UpdateProcessInfo(pProcess);
			
			CSandBoxPtr pBox = GetBoxByName(pProcess->GetBoxName());
			if (pBox.isNull())
				continue;

			if (pBox->m_ActiveProcessCount == 0) {
				pBox->m_ActiveProcessCount = 1;
				pBox->OpenBox();
				m_bBoxesDirty = true;
				emit BoxOpened(pBox);
			}

			pProcess->m_pBox = pBox.data();
			pBox->m_ProcessList.insert(ProcessId, pProcess);
			m_BoxedProxesses.insert(ProcessId, pProcess);
			pBox->m_ActiveProcessDirty = true;

			pProcess->InitProcessInfo();
		}

		pProcess->UpdateProcessInfo();
	}

	foreach(const CBoxedProcessPtr& pProcess, OldProcessList) 
	{
		if (!pProcess->IsTerminated()) {
			pProcess->SetTerminated();
			pProcess->m_pBox->m_ActiveProcessDirty = true;
		}
		else if (iKeep != -1 && pProcess->IsTerminated(iKeep)) {
			pProcess->m_pBox->m_ProcessList.remove(pProcess->m_ProcessId);
			m_BoxedProxesses.remove(pProcess->m_ProcessId);
		}
	}

	foreach(const CSandBoxPtr & pBox, m_SandBoxes)
	{
		if (pBox->m_ActiveProcessDirty) 
		{
			pBox->m_ActiveProcessDirty = false;
			int ActiveProcessCount = 0;
			foreach(const CBoxedProcessPtr & pProcess, pBox->GetProcessList()) {
				if (!pProcess->IsTerminated())
					ActiveProcessCount++;
			}
			bool WasBoxClosed = pBox->m_ActiveProcessCount > 0 && ActiveProcessCount == 0;
			pBox->m_ActiveProcessCount = ActiveProcessCount;
			if (WasBoxClosed) {
				pBox->CloseBox();
				m_bBoxesDirty = true;
				emit BoxClosed(pBox);
			}
		}
	}

	delete[] boxed_pids;
	return SB_OK;
}

bool CSbieAPI::HasProcesses(const QString& BoxName)
{
	ULONG count;
	return CSbieAPI__GetProcessPIDs(m, BoxName, false, NULL, &count) && (count > 0);
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

SB_STATUS CSbieAPI::UpdateBoxPaths(CSandBox* pSandBox)
{
	std::wstring boxName = pSandBox->GetName().toStdWString();

	ULONG filePathLength = 0;
	ULONG keyPathLength = 0;
	ULONG ipcPathLength = 0;
	SB_STATUS Status = CSbieAPI__QueryBoxPath(m, boxName.c_str(), NULL, NULL, NULL, &filePathLength, &keyPathLength, &ipcPathLength);
	if (!Status)
		return Status;

	std::wstring FileRoot(filePathLength / 2 + 1, '\0');
	std::wstring KeyRoot(keyPathLength / 2 + 1, '\0');
	std::wstring IpcRoot(ipcPathLength / 2 + 1, '\0');
	Status = CSbieAPI__QueryBoxPath(m, boxName.c_str(), (WCHAR*)FileRoot.data(), (WCHAR*)KeyRoot.data(), (WCHAR*)IpcRoot.data(), &filePathLength, &keyPathLength, &ipcPathLength);
	if (!Status)
		return Status;

	QString FilePath = QString::fromWCharArray(FileRoot.c_str());
	QString RegPath = QString::fromWCharArray(KeyRoot.c_str());
	QString IpcPath = QString::fromWCharArray(IpcRoot.c_str());

	pSandBox->SetBoxPaths(Nt2DosPath(FilePath), RegPath, IpcPath);
	return SB_OK;
}

SB_STATUS CSbieAPI::UpdateProcessInfo(const CBoxedProcessPtr& pProcess)
{
	WCHAR box_name[BOXNAME_COUNT] = { 0 };
	WCHAR image_name[MAX_PATH];
	//WCHAR sid[96];
	ULONG session_id;
	ULONG64 create_time;

	__declspec(align(8)) UNICODE_STRING64 BoxName = { 0, sizeof(box_name) , (ULONG64)box_name };
	__declspec(align(8)) UNICODE_STRING64 ImageName = { 0, sizeof(image_name), (ULONG64)image_name };
	//__declspec(align(8)) UNICODE_STRING64 SidString = { 0, sizeof(sid), (ULONG64)sid };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_PROCESS_ARGS *args = (API_QUERY_PROCESS_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_QUERY_PROCESS;
	args->process_id.val64 = (ULONG64)pProcess->m_ProcessId;
	args->box_name.val64 = (ULONG64)&BoxName;
	args->image_name.val64 = (ULONG64)&ImageName;
	//args->sid_string.val64 = (ULONG64)&SidString;
	args->session_id.val64 = (ULONG64)&session_id;
	args->create_time.val64 = (ULONG64)&create_time;

	NTSTATUS status = m->IoControl(parms);
	if(!NT_SUCCESS(status))
		return SB_ERR(status);

	pProcess->m_BoxName = QString::fromWCharArray(box_name);
	pProcess->m_ImageName = QString::fromWCharArray(image_name, ImageName.Length/sizeof(WCHAR));
	pProcess->m_SessionId = session_id;
	pProcess->m_StartTime = QDateTime::fromMSecsSinceEpoch(FILETIME2ms(create_time));
	// sid todo

	return SB_OK;
}

SB_STATUS CSbieAPI::GetProcessInfo(quint32 ProcessId, quint32* pParentId, quint32* pInfo, bool* pSuspended, QString* pImagePath, QString* pCommandLine, QString* pWorkingDir)
{
	PROCESS_GET_INFO_REQ req;
	req.h.length = sizeof(PROCESS_GET_INFO_REQ);
	req.h.msgid = MSGID_PROCESS_GET_INFO;
	req.dwProcessId = ProcessId;
	req.dwInfoClasses = 0;

	if(pParentId || pInfo)
		req.dwInfoClasses |= SBIE_PROCESS_BASIC_INFO;
	if(pSuspended)
		req.dwInfoClasses |= SBIE_PROCESS_EXEC_INFO;
	if(pImagePath || pCommandLine || pWorkingDir)
		req.dwInfoClasses |= SBIE_PROCESS_PATHS_INFO;

	SScoped<PROCESS_INFO_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return Status;
	
	if (!NT_SUCCESS(rpl->h.status))
		return SB_ERR(rpl->h.status);

	if (pParentId)		*pParentId	= rpl->dwParentId;
	if (pInfo)			*pInfo		= rpl->dwInfo;
	if (pSuspended)		*pSuspended = rpl->bSuspended;

	if (pImagePath && rpl->app_ofs && rpl->app_len > 0)		*pImagePath   = QString::fromWCharArray((WCHAR*)((ULONG_PTR)rpl.Value() + rpl->app_ofs), rpl->app_len);
	if (pCommandLine && rpl->cmd_ofs && rpl->cmd_len > 0)	*pCommandLine = QString::fromWCharArray((WCHAR*)((ULONG_PTR)rpl.Value() + rpl->cmd_ofs), rpl->cmd_len);
	if (pWorkingDir && rpl->dir_ofs && rpl->dir_len > 0)	*pWorkingDir  = QString::fromWCharArray((WCHAR*)((ULONG_PTR)rpl.Value() + rpl->dir_ofs), rpl->dir_len);

	return SB_OK;
}

CSandBoxPtr CSbieAPI::GetBoxByProcessId(quint32 ProcessId) const
{
	CBoxedProcessPtr pProcess = m_BoxedProxesses.value(ProcessId);
	if (!pProcess || pProcess->IsTerminated())
		return CSandBoxPtr();
	return GetBoxByName(pProcess->GetBoxName());
}

CBoxedProcessPtr CSbieAPI::GetProcessById(quint32 ProcessId) const
{
	return m_BoxedProxesses.value(ProcessId);
}

quint64 CSbieAPI::QueryProcessInfo(quint32 ProcessId, quint32 InfoClass)
{
	__declspec(align(8)) ULONG64 ResultValue;
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_PROCESS_INFO_ARGS *args = (API_QUERY_PROCESS_INFO_ARGS *)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code             = API_QUERY_PROCESS_INFO;

	args->process_id.val64      = (ULONG64)(ULONG_PTR)ProcessId;
	args->info_type.val64       = (ULONG64)(ULONG_PTR)InfoClass;
	args->info_data.val64       = (ULONG64)(ULONG_PTR)&ResultValue;
	args->ext_data.val64        = (ULONG64)(ULONG_PTR)0;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return NULL;
	return ResultValue;
}

SB_STATUS CSbieAPI::TerminateAll(const QString& BoxName)
{
	PROCESS_KILL_ALL_REQ req;
	req.h.length = sizeof(PROCESS_KILL_ALL_REQ);
	req.h.msgid = MSGID_PROCESS_KILL_ALL;
	req.session_id = -1;
	BoxName.toWCharArray(req.boxname); // fix-me: potential overflow
	req.boxname[BoxName.size()] = L'\0';

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return Status;
	if(rpl->status != 0) 
		Status = SB_ERR(rpl->status);
	return Status;
}

SB_STATUS CSbieAPI::TerminateAll(bool bNoExceptions)
{
#ifdef _DEBUG
	qDebug() << "TerminateAll" << bNoExceptions;
#endif
	bool bFailed = false;
	foreach(const CSandBoxPtr& pBox, m_SandBoxes) {
		if (!bNoExceptions && pBox->GetBool("ExcludeFromTerminateAll", false))
			continue;
		if (!pBox->TerminateAll())
			bFailed = true;
	}
	return bFailed ? SB_ERR(SB_FailedKillAll) : SB_OK;
}

SB_STATUS CSbieAPI::Terminate(quint32 ProcessId)
{
	PROCESS_KILL_ONE_REQ req;
	req.h.length = sizeof(PROCESS_KILL_ONE_REQ);
	req.h.msgid = MSGID_PROCESS_KILL_ONE;
	req.pid = ProcessId;

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return Status;
	if (rpl->status != 0)
		Status = SB_ERR(rpl->status);
	return Status;
}

SB_STATUS CSbieAPI::SetSuspendedAll(const QString& BoxName, bool bSuspended)
{
	PROCESS_SUSPEND_RESUME_ALL_REQ req;
	req.h.length = sizeof(PROCESS_SUSPEND_RESUME_ALL_REQ);
	req.h.msgid = MSGID_PROCESS_SUSPEND_RESUME_ALL;
	req.session_id = -1;
	BoxName.toWCharArray(req.boxname); // fix-me: potential overflow
	req.boxname[BoxName.size()] = L'\0';
	req.suspend = bSuspended ? TRUE : FALSE;

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return Status;
	if(rpl->status != 0) 
		Status = SB_ERR(rpl->status);
	return Status;
}

SB_STATUS CSbieAPI::SetSuspended(quint32 ProcessId, bool bSuspended)
{
	PROCESS_SUSPEND_RESUME_ONE_REQ req;
	req.h.length = sizeof(PROCESS_SUSPEND_RESUME_ONE_REQ);
	req.h.msgid = MSGID_PROCESS_SUSPEND_RESUME_ONE;
	req.pid = ProcessId;
	req.suspend = bSuspended ? TRUE : FALSE;

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status || !rpl)
		return Status;
	if (rpl->status != 0)
		Status = SB_ERR(rpl->status);
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
	if (status == STATUS_ACCESS_DENIED) // if we are missing admin rights, lets the driver do it
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

	WCHAR *Environment = GetEnvironmentStringsW();
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
		WrkDir = QDir::currentPath().replace("/","\\");

	ULONG cmd_len = Command.length();
	ULONG dir_len = WrkDir.length();
	QByteArray env = MakeEnvironment(true);
	ULONG env_len = (env.size() - 1) / sizeof(WCHAR);

	ULONG req_len = sizeof(PROCESS_RUN_SANDBOXED_REQ) + (cmd_len + dir_len + env_len + 8) * sizeof(WCHAR);
	SScoped<PROCESS_RUN_SANDBOXED_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_PROCESS_RUN_SANDBOXED;
	BoxName.toWCharArray(req->boxname); // fix-me: potential overflow
	req->boxname[BoxName.length()] = L'\0';
	req->si_flags = STARTF_FORCEOFFFEEDBACK;
/*#ifdef _DEBUG
	if ((QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) != 0)
		req->si_flags |= 0x80000000;
#endif*/
	req->si_show_window = wShowWindow;
	if (req->si_show_window != SW_SHOWNORMAL)
		req->si_flags |= STARTF_USESHOWWINDOW;
	req->creation_flags = Flags;

	WCHAR* ptr = (WCHAR*)((ULONG_PTR)req.Value() + sizeof(PROCESS_RUN_SANDBOXED_REQ));

	req->cmd_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req.Value());
	req->cmd_len = cmd_len;
	if (cmd_len) {
		Command.toWCharArray(ptr);
		ptr += cmd_len;
	}
	*ptr++ = L'\0';

	req->dir_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req.Value());
	req->dir_len = dir_len;
	if (dir_len) {
		WrkDir.toWCharArray(ptr);
		ptr += dir_len;
	}
	*ptr++ = L'\0';

	req->env_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req.Value());
	req->env_len = env_len;
	if (env_len) {
		wmemcpy(ptr, (WCHAR*)env.data(), env_len);
		ptr += env_len;
	}
	*ptr++ = L'\0';

	SScoped<PROCESS_RUN_SANDBOXED_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	

	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);

	CloseHandle((HANDLE)rpl->hProcess);
	CloseHandle((HANDLE)rpl->hThread);
	
	return SB_OK;
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
	return GetBoxedPath(pBox.data(), Path);
}

//#pragma comment(lib, "mpr.lib")

QString CSbieAPI::GetBoxedPath(CSandBox* pBox, const QString& Path, const QString& Snapshot)
{
	QString BoxRoot = pBox->m_FilePath;

	if (!Snapshot.isEmpty())
		BoxRoot += QString("\\snapshot-%1").arg(Snapshot);

    //WCHAR Buffer[4096];
    //DWORD dwBufferLength = sizeof(Buffer)/sizeof(WCHAR );
    //UNIVERSAL_NAME_INFO * unameinfo = (UNIVERSAL_NAME_INFO *) &Buffer;
	//if (WNetGetUniversalName(Path.toStdWString().c_str(), UNIVERSAL_NAME_INFO_LEVEL, (LPVOID)unameinfo, &dwBufferLength) == NO_ERROR)
	//	return BoxRoot + "\\share" + QString::fromWCharArray(unameinfo->lpUniversalName).mid(1);

	//if (Path.indexOf("\\device\\mup", 0, Qt::CaseInsensitive) == 0)
	//	return QStringList(BoxRoot + "\\share" + Path.mid(11));

	if (pBox->GetBool("SeparateUserFolders", true, true))
	{
		if (Path.indexOf(m_UserDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\current" + Path.mid(m_UserDir.length());
		else if (Path.indexOf(m_ProgramDataDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\all" + Path.mid(m_ProgramDataDir.length());
		else if (Path.indexOf(m_PublicDir, 0, Qt::CaseInsensitive) == 0)
			return BoxRoot + "\\user\\public" + Path.mid(m_PublicDir.length());
	}

	if (Path.length() >= 3 && Path.left(2) == "\\\\")
		return BoxRoot + "\\share\\" + Path.mid(2);

	if (Path.length() < 3 || Path.at(1) != ':')
		return QString();
	
	QReadLocker Lock(&m_DriveLettersMutex);
	QMap<QString, SDrive>::const_iterator I = m_DriveLetters.find(Path.left(3).toUpper());
	if (I != m_DriveLetters.end())
	{
		if (I->Type == SDrive::EShare)
			return BoxRoot + "\\share\\" + I->Aux + Path.mid(2);
		else if (pBox->GetBool("UseVolumeSerialNumbers", false) && !I->Aux.isEmpty())
			return BoxRoot + "\\drive\\" + Path.at(0) + "~" + I->Aux + Path.mid(2);
	}

	return BoxRoot + "\\drive\\" + Path.at(0) + Path.mid(2);

	/*QStringList Paths;

	// todo: include snapshot locations

	if (pBox->GetBool("UseVolumeSerialNumbers", false))
	{
		QDir Dir(BoxRoot + "\\drive\\");
		foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
		{
			if (Info.fileName().left(1).compare(Path.at(0), Qt::CaseInsensitive) == 0)
				Paths.append(BoxRoot + "\\drive\\" + Info.fileName() + Path.mid(2));
		}
	}

	if(Paths.isEmpty())
		Paths = QStringList(BoxRoot + "\\drive\\" + Path.at(0) + Path.mid(2));
	return Paths;*/
}

QString CSbieAPI::GetRealPath(CSandBox* pBox, const QString& Path)
{
	QString RealPath;
	QString BoxRoot = pBox->m_FilePath;
	if (BoxRoot.right(1) == "\\") BoxRoot.truncate(BoxRoot.length() - 1);

	if (Path.length() < BoxRoot.length())
		return QString();

	RealPath = Path.mid(BoxRoot.length());
	if (RealPath.left(10).compare("\\snapshot-", Qt::CaseInsensitive) == 0) {
		int pos = RealPath.indexOf("\\", 1);
		if (pos != -1)
			RealPath.remove(0, pos);
	}

	if (RealPath.left(6) == "\\share") 
	{
		QString NtFolder = RealPath.mid(6);
		bool bBs = false;
		if ((bBs = (NtFolder.count("\\") < 3))) NtFolder += "\\";
		bool bOk;
		QString Folder = Nt2DosPath("\\Device\\LanmanRedirector" + NtFolder, &bOk);
		if (!bOk) return "\\" + NtFolder;
		if (bBs) Folder.truncate(Folder.length() - 1);
		return Folder;
	}

	if (RealPath.left(5) == "\\user")
	{
		if (RealPath.mid(5, 8) == "\\current")
			return m_UserDir + RealPath.mid(5 + 8);
		else if (RealPath.mid(5, 4) == "\\all")
			return m_ProgramDataDir + RealPath.mid(5 + 4);
		else if (RealPath.mid(5, 7) == "\\public")
			return m_PublicDir + RealPath.mid(5 + 7);
	}

	if (RealPath.left(6) == "\\drive") 
	{
		int pos = RealPath.indexOf("\\", 7);
		return RealPath.mid(7, 1) + ":" + (pos != -1 ? RealPath.mid(pos) : "");
	}

	return QString();
}

///////////////////////////////////////////////////////////////////////////////
// Conf
//

SB_STATUS CSbieAPI::ReloadConfig(bool ReconfigureDrv)
{
	return ReloadConf(ReconfigureDrv ? SBIE_CONF_FLAG_RECONFIGURE : 0);
}

SB_STATUS CSbieAPI::ReloadCert()
{
	return ReloadConf(SBIE_CONF_FLAG_RELOAD_CERT);
}

SB_STATUS CSbieAPI::ReloadConf(quint32 flags, quint32 SessionId)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];

	memset(parms, 0, sizeof(parms));
	parms[0] = API_RELOAD_CONF;
	parms[1] = SessionId;
	parms[2] = flags;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);

	emit ConfigReloaded();

	m_bBoxesDirty = true;

	return SB_OK;
}

bool CSbieAPI::IsBox(const QString& BoxName, bool& bIsEnabled)
{
	std::wstring box_name = BoxName.toStdWString();

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
	SScoped<SBIE_INI_PASSWORD_REQ> req(malloc(REQUEST_LEN));
	memset(req, 0, REQUEST_LEN);

	req->h.msgid = MSGID_SBIE_INI_TEST_PASSWORD;
	req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);
	m->Password = Password;
	SB_STATUS Status = SbieIniSet(&req->h, req->old_password, "GlobalSettings", "*");
	if (Status.IsError())
		m->Password.clear();
	return Status;
}

SB_STATUS CSbieAPI::LockConfig(const QString& NewPassword)
{
	if(NewPassword.length() > 64)
		return SB_ERR(SB_PasswordBad, STATUS_INVALID_PARAMETER);

	SScoped<SBIE_INI_PASSWORD_REQ> req(malloc(REQUEST_LEN));
	memset(req, 0, REQUEST_LEN);

	req->h.msgid = MSGID_SBIE_INI_SET_PASSWORD;
	req->h.length = sizeof(SBIE_INI_PASSWORD_REQ);
	NewPassword.toWCharArray(req->new_password); // fix-me: potential overflow
	req->new_password[NewPassword.length()] = L'\0';
	SB_STATUS Status = SbieIniSet(&req->h, req->old_password, "GlobalSettings", "*");
	if (!Status.IsError())
		m->Password = NewPassword;
	return Status;
}

void CSbieAPI::ClearPassword()
{
	m->Password.clear();
}

SB_STATUS CSbieAPI::SetDatFile(const QString& FileName, const QByteArray& Data)
{
	ULONG req_len = sizeof(SBIE_INI_SETTING_REQ) + Data.length();
	SScoped<SBIE_INI_SETTING_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.msgid = MSGID_SBIE_INI_SET_DAT;

	req->section[0] = L'\0'; // unused
	FileName.toWCharArray(req->setting); // fix-me: potential overflow
	req->setting[FileName.length()] = L'\0';
	memcpy(req->value, Data, Data.length());
	req->value_len = Data.length();
	req->h.length = sizeof(SBIE_INI_SETTING_REQ) + req->value_len * sizeof(WCHAR);

	SScoped<MSG_HEADER> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	return Status;
}

//SB_RESULT(QByteArray) CSbieAPI::GetDatFile(const QString& FileName)
//{
//}

SB_RESULT(QByteArray) CSbieAPI::RC4Crypt(const QByteArray& Data)
{
	ULONG req_len = sizeof(SBIE_INI_RC4_CRYPT_REQ) + Data.size();
	SScoped<SBIE_INI_RC4_CRYPT_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_SBIE_INI_RC4_CRYPT;
	req->value_len = Data.size();
	memcpy(req->value, Data.constData(), req->value_len);
	
	SScoped<SBIE_INI_RC4_CRYPT_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);
	return CSbieResult<QByteArray>(QByteArray((char*)rpl->value, rpl->value_len));
}

bool CSbieAPI::GetDriverInfo(quint32 InfoClass, void* pBuffer, size_t Size)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_QUERY_DRIVER_INFO_ARGS *args = (API_QUERY_DRIVER_INFO_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
	args->func_code = API_QUERY_DRIVER_INFO;
	args->info_class.val = InfoClass;
	args->info_data.val = pBuffer;
	args->info_len.val = Size;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status)) {
		memset(pBuffer, 0, Size);
		return false;
	}
	return true;
}

quint32 CSbieAPI::GetFeatureFlags()
{
	quint32 flags = 0;
	GetDriverInfo(0, &flags, sizeof(flags));
	return flags;
}

QString CSbieAPI::GetFeatureStr()
{
	quint32 flags = GetFeatureFlags();

	QStringList str;
	if (flags & SBIE_FEATURE_FLAG_WFP)
		str.append("WFP");		// Windows Filtering Platform
	if (flags & SBIE_FEATURE_FLAG_OB_CALLBACKS)
		str.append("ObCB");		// Object Callbacks
	if (flags & SBIE_FEATURE_FLAG_SBIE_LOGIN)
		str.append("SbL");		// Sandboxie Login
	if (flags & SBIE_FEATURE_FLAG_SECURITY_MODE)
		str.append("SMod");		// Security Mode
	if (flags & SBIE_FEATURE_FLAG_PRIVACY_MODE)
		str.append("PMod");		// Privacy Mode
	if (flags & SBIE_FEATURE_FLAG_COMPARTMENTS)
		str.append("AppC");		// Application Compartment
	if (flags & SBIE_FEATURE_FLAG_WIN32K_HOOK)
		str.append("W32k");		// Win32 Hooks
	if (flags & SBIE_FEATURE_FLAG_ENCRYPTION)
		str.append("EBox");		// Encrypted Boxes
	if (flags & SBIE_FEATURE_FLAG_NET_PROXY)
		str.append("NetI");		// Network Interception
	if (flags & SBIE_FEATURE_FLAG_NO_SIG)
		str.append("DEV");		// Developer

	return str.join(",");
}

int CSbieAPI::IsDyndataActive()
{
	quint32 flags = GetFeatureFlags();

	if (flags & SBIE_FEATURE_FLAG_DYNDATA_EXP)
		return -1;
	if (flags & SBIE_FEATURE_FLAG_DYNDATA_OK)
		return 1;
	return 0;
}

SB_STATUS CSbieAPI::SetSecureParam(const QString& Name, const void* data, size_t size)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_SECURE_PARAM_ARGS *args = (API_SECURE_PARAM_ARGS*)parms;

	std::wstring name = Name.toStdWString();

	memset(parms, 0, sizeof(parms));
	args->func_code = API_SET_SECURE_PARAM;
	args->param_name.val = (WCHAR*)name.c_str();
	args->param_data.val = (void*)data;
	args->param_size.val = size;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

SB_STATUS CSbieAPI::GetSecureParam(const QString& Name, void* data, size_t size, quint32* size_out, bool bVerify)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	API_SECURE_PARAM_ARGS *args = (API_SECURE_PARAM_ARGS*)parms;

	std::wstring name = Name.toStdWString();

	memset(parms, 0, sizeof(parms));
	args->func_code = API_GET_SECURE_PARAM;
	args->param_name.val = (WCHAR*)name.c_str();
	args->param_data.val = (void*)data;
	args->param_size.val = size;
	args->param_size_out.val = (ULONG*)size_out;
	args->param_verify.val = bVerify;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return SB_ERR(status);
	return SB_OK;
}

bool CSbieAPI::TestSignature(const QByteArray& Data, const QByteArray& Signature)
{
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
	parms[0] = API_VERIFY;
	parms[1] = (ULONG_PTR)Data.constData();
	parms[2] = Data.size();
	parms[3] = (ULONG_PTR)Signature.constData();
	parms[4] = Signature.size();

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return false;
	return true;
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
		emit ProcessBoxed(ProcessId, Nt2DosPath(MsgData[1]), MsgData[2], MsgData.length() < 4 ? 0 : MsgData[3].toUInt(), MsgData.length() < 5 ? QString() : MsgData[4]);
		return true;
	}
	
	if ((MsgCode & 0xFFFF) == 2199) // Auto Recovery notification
	{
		QString FilePath = Nt2DosPath(MsgData[2]);
		QString BoxPath = MsgData.length() >= 4 ? Nt2DosPath(MsgData[3]) : QString();
		emit FileToRecover(MsgData[1], FilePath, BoxPath, ProcessId);
		return true;
	}

	/*
	QString Message = CSbieAPI__FormatSbieMsg(m, MsgCode, str1, str2);
	if (ProcessId != 4) // if it is not from the driver, add the pid
		Message += tr(", by process: %1").arg(ProcessId);

	emit LogMessage(Message, MessageId);
	*/

	emit LogSbieMessage(MsgCode, MsgData, ProcessId);
	return true;
}

CBoxedProcessPtr CSbieAPI::OnProcessBoxed(quint32 ProcessId, const QString& Path, const QString& Box, quint32 ParentId, const QString& CmdLine)
{
	CBoxedProcessPtr pProcess = m_BoxedProxesses.value(ProcessId);
	if (!pProcess)
	{
		CSandBoxPtr pBox = GetBoxByName(Box);
		if (!pBox)
			return CBoxedProcessPtr();
		
		if (pBox->m_ActiveProcessCount == 0) {
			pBox->m_ActiveProcessCount = 1;
			pBox->OpenBox();
			m_bBoxesDirty = true;
			emit BoxOpened(pBox);
		}

		pProcess = CBoxedProcessPtr(NewBoxedProcess(ProcessId, pBox.data()));
		pBox->m_ProcessList.insert(ProcessId, pProcess);
		m_BoxedProxesses.insert(ProcessId, pProcess);
		pBox->m_ActiveProcessDirty = true;

		UpdateProcessInfo(pProcess);
		pProcess->InitProcessInfo();
	}

	if(pProcess->m_ImageName.isEmpty())
		pProcess->m_ImageName = Path.mid(Path.lastIndexOf("\\") + 1);
	if (pProcess->m_ParendPID == 0)
		pProcess->m_ParendPID = ParentId;
	if (pProcess->m_ImagePath.isEmpty())
		pProcess->m_ImagePath = Path;
	if (pProcess->m_CommandLine.isEmpty() && !CmdLine.isEmpty())
		pProcess->m_CommandLine = CmdLine;

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
// Mount Manager
//

SB_STATUS CSbieAPI::ImBoxCreate(CSandBox* pBox, quint64 uSizeKb, const QString& Password)
{
	std::wstring file_root = L"\\??\\" + pBox->GetFileRoot().toStdWString();
	std::wstring password = Password.toStdWString();
	if(password.length() > 128)
		return SB_ERR(ERROR_INVALID_PARAMETER);

	ULONG req_len = sizeof(IMBOX_CREATE_REQ);
	req_len += file_root.length() * sizeof(wchar_t);
	SScoped<IMBOX_CREATE_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_IMBOX_CREATE;
	req->image_size = uSizeKb;
	wcscpy(req->password, password.c_str());
	wcscpy(req->file_root, file_root.c_str());

	SScoped<IMBOX_CREATE_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	m_bBoxesDirty = true;
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);
	return SB_OK;
}

SB_STATUS CSbieAPI::ImBoxMount(CSandBox* pBox, const QString& Password, bool bProtect, bool bAutoUnmount)
{
	std::wstring root = pBox->GetRegRoot().toStdWString();
	if(root.length() >= MAX_REG_ROOT_LEN)
		return SB_ERR(ERROR_INVALID_PARAMETER);
	std::wstring file_root = L"\\??\\" + pBox->GetFileRoot().toStdWString();
	std::wstring password = Password.toStdWString();
	if(password.length() > 128)
		return SB_ERR(ERROR_INVALID_PARAMETER);

	ULONG req_len = sizeof(IMBOX_MOUNT_REQ);
	req_len += file_root.length() * sizeof(wchar_t);
	SScoped<IMBOX_MOUNT_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_IMBOX_MOUNT;
	wcscpy(req->password, password.c_str());
	req->protect_root = bProtect;
	req->auto_unmount = bAutoUnmount;
	wcscpy(req->reg_root, root.c_str());
	wcscpy(req->file_root, file_root.c_str());

	SScoped<IMBOX_MOUNT_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	m_bBoxesDirty = true;
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);
	return SB_OK;
}

SB_STATUS CSbieAPI::ImBoxUnmount(CSandBox* pBox)
{
	std::wstring root = pBox->GetRegRoot().toStdWString();

	IMBOX_UNMOUNT_REQ req;
	req.h.length = sizeof(IMBOX_UNMOUNT_REQ);
	req.h.msgid = MSGID_IMBOX_UNMOUNT;
	wcscpy(req.reg_root, root.c_str());

	SScoped<IMBOX_UNMOUNT_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	m_bBoxesDirty = true;
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);
	return SB_OK;
}

SB_RESULT(QStringList) CSbieAPI::ImBoxEnum()
{
	IMBOX_ENUM_REQ req;
	req.h.length = sizeof(IMBOX_ENUM_REQ);
	req.h.msgid = MSGID_IMBOX_ENUM;

	SScoped<IMBOX_ENUM_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);

	QStringList Roots;

	wchar_t* reg_roots = rpl->reg_roots;
	while (*reg_roots) {
		size_t len = wcslen(reg_roots);
		Roots.append(QString::fromWCharArray(reg_roots, len));
		reg_roots += len + 1;
	}

	return CSbieResult<QStringList>(Roots);
}

SB_RESULT(QVariantMap) CSbieAPI::ImBoxQuery(const QString& Root)
{
	std::wstring root = Root.toStdWString();

	IMBOX_QUERY_REQ req;
	req.h.length = sizeof(IMBOX_QUERY_REQ);
	req.h.msgid = MSGID_IMBOX_QUERY;
	wcscpy(req.reg_root, root.c_str());

	SScoped<IMBOX_QUERY_RPL> rpl;
	SB_STATUS Status = CallServer(&req.h, &rpl);
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);

	QVariantMap Info;

	Info["DiskSize"] = rpl->disk_size;
	Info["UsedSize"] = rpl->used_size;
	Info["DiskRoot"] = QString::fromWCharArray(rpl->disk_root);

	return CSbieResult<QVariantMap>(Info);
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
#if 0
	ULONG type;
	ULONG pid = 0;
	ULONG tid = 0;
	wchar_t* Buffer[4 * 1024];

	//ULONG RecordNum = m->lastRecordNum;

	__declspec(align(8)) UNICODE_STRING64 log_buffer = { 0, (USHORT)ARRAYSIZE(Buffer), (ULONG64)Buffer };
	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_GET_EX_ARGS* args	= (API_MONITOR_GET_EX_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code	= API_MONITOR_GET_EX;
	//args->log_seq.val = &RecordNum; // set this to NULL for record clearing
    args->log_type.val = &type;
	args->log_pid.val = &pid;
	args->log_tid.val = &tid;
	args->log_data.val = &log_buffer;
    
	if (!NT_SUCCESS(m->IoControl(parms)))
		return false; // error or no more entries

	if (type == 0) // versions prior to 5.45.0 return success and type 0 when there are no more entries
		return false;

	//if (RecordNum != m->lastRecordNum + 1 && !m->clearingBuffers)
	//	emit LogSbieMessage(0xC1020000 | 1242, QStringList() << "" << "" << "", GetCurrentProcessId()); // Monitor buffer overflow
	//m->lastRecordNum = RecordNum;

	if (m->clearingBuffers)
		return true; 

	QStringList LogData;
	for (size_t pos = 0; pos < log_buffer.Length; ) {
		size_t len = wcslen((WCHAR*)(log_buffer.Buffer + pos));
		LogData.append(QString::fromWCharArray((WCHAR*)(log_buffer.Buffer + pos), len));
		pos += (len + 1) * sizeof(WCHAR);
	}

	CTraceEntryPtr LogEntry = CTraceEntryPtr(new CTraceEntry(0, pid, tid, type, LogData));

	QMutexLocker Lock(&m_TraceMutex);
	m_TraceCache.append(LogEntry);

	return true;

#else // bulk retrieval starting with build 1.6.6

	if (m->traceBuffer == NULL) {
		m->traceBufferLen = 256 * PAGE_SIZE;
		m->traceBuffer = (UCHAR*)malloc(m->traceBufferLen);
	}

	ULONG buffer_len = m->traceBufferLen;
	UCHAR* buffer = m->traceBuffer;

	__declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
    API_MONITOR_GET2_ARGS* args	= (API_MONITOR_GET2_ARGS*)parms;

	memset(parms, 0, sizeof(parms));
    args->func_code	= API_MONITOR_GET2;
	args->buffer_ptr.val = (WCHAR*)buffer;
	args->buffer_len.val = &buffer_len;

	NTSTATUS status = m->IoControl(parms);
	if (!NT_SUCCESS(status))
		return false; // error or no more entries

	if (m->clearingBuffers)
		return true; 

	for (UCHAR* ptr = buffer; *(ULONG*)ptr > 0; ) {

		ULONG uSize = *(ULONG*)ptr;
		ptr += sizeof(ULONG);

		LONGLONG uTimestamp = *(LONGLONG*)ptr;
		ptr += sizeof(LONGLONG);
		uSize -= sizeof(LONGLONG);

		ULONG uType = *(ULONG*)ptr;
		ptr += sizeof(ULONG);
		uSize -= sizeof(ULONG);

		ULONG uPid = *(ULONG*)ptr;
		ptr += sizeof(ULONG);
		uSize -= sizeof(ULONG);

		ULONG uTid = *(ULONG*)ptr;
		ptr += sizeof(ULONG);
		uSize -= sizeof(ULONG);

		QStringList LogData;
		for (; uSize > 0;) {
			if (*(WCHAR*)ptr == 0xFFFF) { // end of strings marker
				ptr += sizeof(WCHAR);
				uSize -= sizeof(WCHAR);
				break;
			}
			size_t len = wcslen((WCHAR*)ptr);
			QString str = QString::fromWCharArray((WCHAR*)ptr, len);
			ptr += (len + 1) * sizeof(WCHAR);
			uSize -= (len + 1) * sizeof(WCHAR);
			LogData.append(str);
		}

		QVector<quint64> Stack;

		for (; uSize > 0;) {

			ULONG uTagID = *(ULONG*)ptr;
			ptr += sizeof(ULONG);
			uSize -= sizeof(ULONG);

			ULONG uTagLen = *(ULONG*)ptr;
			ptr += sizeof(ULONG);
			uSize -= sizeof(ULONG);

			switch (uTagID) {
			case 'STCK':
				int PtrSize = sizeof(PVOID);
#ifndef _WIN64
				if (IsWow64())
					PtrSize = sizeof(__int64);
#endif
				int Frames = uTagLen / PtrSize;
				Stack.reserve(Frames);
				for (int i = 0; i < Frames; i++) {
					quint64 Address;
#ifndef _WIN64
					if (PtrSize == sizeof(quint32))
						Address = ((quint32*)ptr)[i];
					else
#endif
						Address = ((quint64*)ptr)[i];

					if ((Address & 0x8000000000000000ull) == 0) // skip kernel addresses
						Stack.append(Address);
				}
			}

			ptr += uTagLen;
			uSize -= uTagLen;
		}

		CTraceEntryPtr LogEntry = CTraceEntryPtr(new CTraceEntry(FILETIME2ms(uTimestamp), uPid, uTid, uType, LogData, Stack));

		QMutexLocker Lock(&m_TraceMutex);
		m_TraceCache.append(LogEntry);
	}

	return status == STATUS_MORE_ENTRIES;
#endif
}

const QVector<CTraceEntryPtr>& CSbieAPI::GetTrace()
{ 
	QMutexLocker Lock(&m_TraceMutex);

	for (int i = 0; i < m_TraceCache.count(); i++) 
	{
		CTraceEntryPtr& pEntry = m_TraceCache[i];

#ifdef USE_MERGE_TRACE
		if (!m_TraceList.isEmpty() && m_TraceList.last()->Equals(pEntry)) {
			m_TraceList.last()->Merge(pEntry);
			continue;
		}
#endif

		if (CBoxedProcessPtr proc = m_BoxedProxesses.value(pEntry->GetProcessId())) {
			pEntry->SetProcessName(proc->GetProcessName());
			pEntry->SetBoxPtr(proc->GetBoxPtr());
			QVector<quint64> Stack = pEntry->GetStack();
			if(!Stack.isEmpty())
				proc->ResolveSymbols(Stack);
		}

		m_TraceList.append(pEntry);
	}
	m_TraceCache.clear();

	return m_TraceList; 
}

///////////////////////////////////////////////////////////////////////////////
// Other 
//

QString CSbieAPI::GetSbieMsgStr(quint32 code, quint32 Lang)
{
	ULONG FormatFlags = FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER;
	WCHAR* ret_str = NULL;
	if (!m->SbieMsgDll || (FormatMessageW(FormatFlags, m->SbieMsgDll, code, Lang, (LPWSTR)&ret_str, 4, NULL) == 0
						&& FormatMessageW(FormatFlags, m->SbieMsgDll, code, 1033, (LPWSTR)&ret_str, 4, NULL) == 0))
		return QString("SBIE%0: %1; %2").arg(code & 0xFFFF, 4, 10);
	QString qStr = QString::fromWCharArray(ret_str);
	LocalFree(ret_str);
	return qStr.trimmed(); // note messages may have \r\n at the end
}

void CSbieAPI::LoadEventLog()
{
	QByteArray buff;
	buff.resize(8 * 1024);

    HANDLE hEventLog = OpenEventLogW(NULL, L"System");

    while (hEventLog) 
	{
        ULONG bytesRead, bytesNeeded;
        if(!ReadEventLogW(hEventLog, EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ, 0, buff.data(), buff.size(), &bytesRead, &bytesNeeded))
            break;

        EVENTLOGRECORD *rec = (EVENTLOGRECORD *)buff.data();
        while (bytesRead > 0) 
		{
			time_t recently = time(NULL) - 15 * 60;

            WCHAR *source = (WCHAR *) ((UCHAR *)rec + sizeof(EVENTLOGRECORD));
            if ((time_t)rec->TimeGenerated >= recently && (wcscmp(source, SBIEDRV) == 0 || wcscmp(source, SBIESVC) == 0)) 
			{
				ULONG MsgCode = rec->EventID;
				QStringList MsgData = QStringList() << "" << "" << "";

				WCHAR *str0 = (WCHAR *)((UCHAR *)rec + rec->StringOffset);
                if (rec->NumStrings >= 2) 
				{
                    WCHAR *str1 = str0 + wcslen(str0) + 1;
					MsgData[1] = QString::fromWCharArray(str1);
					if (rec->NumStrings >= 3)
					{
						WCHAR *str2 = str1 + wcslen(str1) + 1;
						MsgData[2] = QString::fromWCharArray(str2);
					}
                }

                emit LogSbieMessage(MsgCode & 0xFFFF, MsgData, 0);
            }

            bytesRead -= rec->Length;
            rec = (EVENTLOGRECORD *)((UCHAR *)rec + rec->Length);
        }
    }

    if (hEventLog)
        CloseEventLog(hEventLog);
}

///////////////////////////////////////////////////////////////////////////////
// Updater 
//

SB_RESULT(int) CSbieAPI::RunUpdateUtility(const QStringList& Params, quint32 Elevate, bool Wait)
{
	if (Params.isEmpty())
		return SB_ERR(ERROR_INVALID_PARAMETER);

	QString Command;
	foreach(const QString & Param, Params) {
		if (!Command.isEmpty()) Command += " ";
		Command += "\"" + Param + "\"";
	}

	ULONG cmd_len = Command.length();

	ULONG req_len = sizeof(PROCESS_RUN_UPDATER_REQ) + (cmd_len + 8) * sizeof(WCHAR);
	SScoped<PROCESS_RUN_UPDATER_REQ> req(malloc(req_len));
	memset(req, 0, req_len);

	req->h.length = req_len;
	req->h.msgid = MSGID_PROCESS_RUN_UPDATER;
	req->elevate = Elevate;

	WCHAR* ptr = (WCHAR*)((ULONG_PTR)req.Value() + sizeof(PROCESS_RUN_UPDATER_REQ));

	req->cmd_ofs = (ULONG)((ULONG_PTR)ptr - (ULONG_PTR)req.Value());
	req->cmd_len = cmd_len;
	if (cmd_len) {
		Command.toWCharArray(ptr);
		ptr += cmd_len;
	}
	*ptr++ = L'\0';

	SScoped<PROCESS_RUN_UPDATER_RPL> rpl;
	SB_STATUS Status = CallServer(&req->h, &rpl);
	if (!Status)
		return Status;
	if (!rpl) 
		return SB_ERR(ERROR_SERVER_DISABLED);
	
	if (rpl->h.status != 0)
		return SB_ERR(rpl->h.status);

	DWORD ExitCode = 0;
	if (Wait) {
		WaitForSingleObject((HANDLE)rpl->hProcess, INFINITE);
		GetExitCodeProcess((HANDLE)rpl->hProcess, &ExitCode);
	}

	CloseHandle((HANDLE)rpl->hProcess);
	CloseHandle((HANDLE)rpl->hThread);

	return CSbieResult<int>((int)ExitCode);
}
