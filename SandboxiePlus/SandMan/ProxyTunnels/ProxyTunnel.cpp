// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#ifdef PROXY_TUNNELS_STANDALONE
#include <QtCore>
#else
#include "stdafx.h"
#endif
#include "ProxyTunnel.h"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QHostInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkProxy>
#include <QSet>
#include <QSslSocket>
#include <QUuid>
#include <QJsonDocument>
#include <memory>
#ifdef Q_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <netioapi.h>
#endif

CProxyTunnel::CProxyTunnel(const SProxyProfile& Profile, const QString& Directory, QObject* Parent)
	: QThread(Parent), m_Profile(Profile), m_Directory(Directory) {}

CProxyTunnel::~CProxyTunnel() { RequestStop(); wait(); }

QString CProxyTunnel::StateText(int State)
{
	switch (State) {
	case Starting: return tr("Starting");
	case Running: return tr("Running (TCP checked)");
	case Checking: return tr("Checking tunnel");
	case Stopping: return tr("Stopping");
	case Failed: return tr("Failed / offline");
	default: return tr("Stopped");
	}
}

void CProxyTunnel::run()
{
	emit StateChanged(m_Profile.Id, Starting, tr("Preparing the managed tunnel."));
	const QString Error = RunWindows();
	if (!Error.isEmpty() && !m_Stop.load()) emit StateChanged(m_Profile.Id, Failed, Error);
	else emit StateChanged(m_Profile.Id, Stopped, tr("Tunnel stopped. Sandbox bindings were retained."));
}

#if defined(Q_OS_WIN) && defined(SBIE_PROXY_TUNNELS_LAB)
namespace
{
	QString Message(const char* Text) { return QCoreApplication::translate("ProxyTunnel", Text); }
	QString WinError(const char* Stage, DWORD Code = GetLastError())
	{
		// Never include a command line, proxy URL, exception text, or child output.
		return Message("%1 (Windows error %2).").arg(Message(Stage)).arg(Code);
	}

	struct SHandle
	{
		HANDLE Value = nullptr;
		~SHandle() { if (Value && Value != INVALID_HANDLE_VALUE) CloseHandle(Value); }
		SHandle() = default;
		SHandle(const SHandle&) = delete;
		SHandle& operator=(const SHandle&) = delete;
	};

	struct SChild
	{
		SHandle Job;
		SHandle Process;
		DWORD Pid = 0;
		~SChild()
		{
			// Process is exclusively a handle returned by our own CreateProcessW call.
			if (Process.Value) {
				TerminateProcess(Process.Value, ERROR_CANCELLED);
				WaitForSingleObject(Process.Value, 5000);
			}
		}
		bool Alive() const { return Process.Value && WaitForSingleObject(Process.Value, 0) == WAIT_TIMEOUT; }
	};

	bool VerifyAndLock(const QString& Path, const QByteArray& Expected, SHandle& File)
	{
		const std::wstring Native = QDir::toNativeSeparators(Path).toStdWString();
		// Refuse replacement/deletion while the verified file is in use.
		File.Value = CreateFileW(Native.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (File.Value == INVALID_HANDLE_VALUE) return false;
		QCryptographicHash Hash(QCryptographicHash::Sha256);
		char Buffer[65536];
		DWORD Read = 0;
		qint64 Total = 0;
		for (;;) {
			if (!ReadFile(File.Value, Buffer, sizeof(Buffer), &Read, nullptr)) return false;
			if (Read == 0) break;
			Total += Read;
			if (Total > 64 * 1024 * 1024) return false;
			Hash.addData(Buffer, int(Read));
		}
		return Hash.result().toHex() == Expected;
	}

	bool Launch(const QString& Exe, const QString& Pipe, SChild& Child, QString& Error)
	{
		Child.Job.Value = CreateJobObjectW(nullptr, nullptr);
		if (!Child.Job.Value) { Error = WinError("Cannot create the tunnel job"); return false; }
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION Limit = {};
		Limit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		if (!SetInformationJobObject(Child.Job.Value, JobObjectExtendedLimitInformation, &Limit, sizeof(Limit))) {
			Error = WinError("Cannot configure the tunnel job"); return false;
		}
		SECURITY_ATTRIBUTES Attributes = {sizeof(Attributes), nullptr, TRUE};
		SHandle Null;
		Null.Value = CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			&Attributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (Null.Value == INVALID_HANDLE_VALUE) { Error = WinError("Cannot isolate tunnel output"); return false; }
		SIZE_T Size = 0;
		InitializeProcThreadAttributeList(nullptr, 1, 0, &Size);
		QByteArray Storage(int(Size), 0);
		auto List = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(Storage.data());
		if (!InitializeProcThreadAttributeList(List, 1, 0, &Size)) { Error = WinError("Cannot create the process attribute list"); return false; }
		struct SAttributes { LPPROC_THREAD_ATTRIBUTE_LIST Value; ~SAttributes() { DeleteProcThreadAttributeList(Value); } } AttributeGuard{List};
		HANDLE Inherited[] = {Null.Value};
		if (!UpdateProcThreadAttribute(List, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, Inherited, sizeof(Inherited), nullptr, nullptr)) {
			Error = WinError("Cannot restrict inherited handles"); return false;
		}
		STARTUPINFOEXW Info = {};
		Info.StartupInfo.cb = sizeof(Info);
		Info.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
		Info.StartupInfo.hStdInput = Info.StartupInfo.hStdOutput = Info.StartupInfo.hStdError = Null.Value;
		Info.lpAttributeList = List;
		const std::wstring Executable = QDir::toNativeSeparators(Exe).toStdWString();
		// Paths cannot contain quotes on Windows. Pipe names are internally generated UUIDs.
		std::wstring Command = L"\"" + Executable + L"\" -loglevel silent -config \"" + Pipe.toStdWString() + L"\"";
		wchar_t SystemDirectory[MAX_PATH] = {};
		if (!GetSystemDirectoryW(SystemDirectory, MAX_PATH)) { Error = WinError("Cannot locate the system directory"); return false; }
		PROCESS_INFORMATION Process = {};
		if (!CreateProcessW(Executable.c_str(), &Command[0], nullptr, nullptr, TRUE,
			CREATE_SUSPENDED | CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
			nullptr, SystemDirectory, &Info.StartupInfo, &Process)) {
			Error = WinError("Cannot start tun2socks"); return false;
		}
		Child.Process.Value = Process.hProcess;
		Child.Pid = Process.dwProcessId;
		SHandle Thread;
		Thread.Value = Process.hThread;
		if (!AssignProcessToJobObject(Child.Job.Value, Child.Process.Value)) {
			Error = WinError("Cannot attach the tunnel to its lifetime job"); return false;
		}
		if (ResumeThread(Thread.Value) == DWORD(-1)) { Error = WinError("Cannot resume the tunnel process"); return false; }
		return true;
	}

	bool BestHostRoute(NET_LUID& Luid)
	{
		SOCKADDR_INET Destination = {};
		Destination.Ipv4.sin_family = AF_INET;
		InetPtonW(AF_INET, L"1.1.1.1", &Destination.Ipv4.sin_addr);
		MIB_IPFORWARD_ROW2 Route = {};
		SOCKADDR_INET Source = {};
		if (GetBestRoute2(nullptr, 0, nullptr, &Destination, 0, &Route, &Source) != NO_ERROR) return false;
		Luid = Route.InterfaceLuid;
		return true;
	}

	struct SNetwork
	{
		NET_LUID Luid = {};
		GUID Guid = {};
		MIB_UNICASTIPADDRESS_ROW Address = {};
		MIB_IPFORWARD_ROW2 Route = {};
		bool AddedAddress = false;
		bool AddedRoute = false;
		~SNetwork()
		{
			MIB_IF_ROW2 Current = {};
			Current.InterfaceLuid = Luid;
			// Never clean up by a reusable numeric interface index or an alias alone.
			if (GetIfEntry2(&Current) != NO_ERROR || !IsEqualGUID(Guid, Current.InterfaceGuid)) return;
			if (AddedRoute) DeleteIpForwardEntry2(&Route);
			if (AddedAddress) DeleteUnicastIpAddressEntry(&Address);
		}
		bool Ready() const
		{
			MIB_IF_ROW2 Current = {};
			Current.InterfaceLuid = Luid;
			if (GetIfEntry2(&Current) != NO_ERROR || !IsEqualGUID(Guid, Current.InterfaceGuid)
				|| Current.OperStatus != IfOperStatusUp) return false;
			MIB_UNICASTIPADDRESS_ROW Check = Address;
			if (GetUnicastIpAddressEntry(&Check) != NO_ERROR || Check.DadState != IpDadStatePreferred) return false;
			PMIB_UNICASTIPADDRESS_TABLE Table = nullptr;
			if (GetUnicastIpAddressTable(AF_INET6, &Table) != NO_ERROR) return false;
			bool Valid = true;
			for (ULONG i = 0; i < Table->NumEntries; ++i) {
				const auto& Row = Table->Table[i];
				if (Row.InterfaceLuid.Value == Luid.Value && !IN6_IS_ADDR_LINKLOCAL(&Row.Address.Ipv6.sin6_addr)) Valid = false;
			}
			FreeMibTable(Table);
			return Valid;
		}
	};

	bool Configure(SNetwork& Network, QString& Error)
	{
		MIB_IF_ROW2 Adapter = {};
		Adapter.InterfaceLuid = Network.Luid;
		DWORD Code = GetIfEntry2(&Adapter);
		if (Code) { Error = WinError("Cannot identify the new tunnel adapter", Code); return false; }
		Network.Guid = Adapter.InterfaceGuid;
		MIB_IPINTERFACE_ROW Interface = {};
		InitializeIpInterfaceEntry(&Interface);
		Interface.Family = AF_INET;
		Interface.InterfaceLuid = Network.Luid;
		Code = GetIpInterfaceEntry(&Interface);
		if (Code) { Error = WinError("Cannot read the tunnel interface", Code); return false; }
		Interface.WeakHostSend = FALSE;
		Interface.WeakHostReceive = FALSE;
		Interface.UseAutomaticMetric = FALSE;
		Interface.Metric = 9999;
		Interface.SitePrefixLength = 0;
		Code = SetIpInterfaceEntry(&Interface);
		if (Code) { Error = WinError("Cannot configure strong-host mode", Code); return false; }
		PMIB_UNICASTIPADDRESS_TABLE Table = nullptr;
		Code = GetUnicastIpAddressTable(AF_INET, &Table);
		if (Code) { Error = WinError("Cannot allocate a tunnel address", Code); return false; }
		QSet<quint32> Used;
		for (ULONG i = 0; i < Table->NumEntries; ++i) Used.insert(Table->Table[i].Address.Ipv4.sin_addr.S_un.S_addr);
		FreeMibTable(Table);
		InitializeUnicastIpAddressEntry(&Network.Address);
		Network.Address.InterfaceLuid = Network.Luid;
		Network.Address.Address.Ipv4.sin_family = AF_INET;
		Network.Address.OnLinkPrefixLength = 32;
		Network.Address.PrefixOrigin = IpPrefixOriginManual;
		Network.Address.SuffixOrigin = IpSuffixOriginManual;
		Network.Address.SkipAsSource = TRUE;
		Network.Address.ValidLifetime = Network.Address.PreferredLifetime = 0xffffffff;
		// /32 avoids installing an overlapping connected /15 route on the host.
		for (quint32 Index = 1; Index < 65535; ++Index) {
			const quint32 Ip = htonl(0xc6120000u + Index); // 198.18.0.0/16, benchmarking range
			if (Used.contains(Ip)) continue;
			Network.Address.Address.Ipv4.sin_addr.S_un.S_addr = Ip;
			Code = CreateUnicastIpAddressEntry(&Network.Address);
			if (Code == ERROR_OBJECT_ALREADY_EXISTS) continue;
			if (Code) { Error = WinError("Cannot assign the tunnel address", Code); return false; }
			Network.AddedAddress = true;
			break;
		}
		if (!Network.AddedAddress) { Error = Message("No free tunnel address is available."); return false; }
		InitializeIpForwardEntry(&Network.Route);
		Network.Route.InterfaceLuid = Network.Luid;
		Network.Route.DestinationPrefix.Prefix.Ipv4.sin_family = AF_INET;
		Network.Route.DestinationPrefix.PrefixLength = 0;
		Network.Route.NextHop.Ipv4.sin_family = AF_INET;
		Network.Route.Metric = 9999;
		Network.Route.Protocol = MIB_IPPROTO_NETMGMT;
		Code = CreateIpForwardEntry2(&Network.Route);
		if (Code) { Error = WinError("Cannot add the interface-scoped tunnel route", Code); return false; }
		Network.AddedRoute = true;
		return true;
	}

	QString CheckEgress(const SNetwork& Network, const std::atomic<bool>& Stop, QString& Address)
	{
		QSslSocket Socket;
		Socket.setProxy(QNetworkProxy::NoProxy);
		const quint32 Source = ntohl(Network.Address.Address.Ipv4.sin_addr.S_un.S_addr);
		if (!Socket.bind(QHostAddress(Source))) return Message("Cannot bind the egress check to the tunnel address.");
		NET_IFINDEX Index = 0;
		if (ConvertInterfaceLuidToIndex(&Network.Luid, &Index) != NO_ERROR) return Message("The tunnel interface disappeared.");
		DWORD NativeIndex = htonl(Index);
		if (setsockopt(SOCKET(Socket.socketDescriptor()), IPPROTO_IP, IP_UNICAST_IF,
			reinterpret_cast<const char*>(&NativeIndex), sizeof(NativeIndex)) != 0)
			return Message("Cannot pin the egress check to the tunnel interface.");
		// Numeric TLS endpoint: no host DNS lookup, no inherited system proxy, no insecure TLS fallback.
		Socket.connectToHostEncrypted(QStringLiteral("1.1.1.1"), 443);
		if (!Socket.waitForEncrypted(7000) || Stop.load()) return Message("The tunnel TCP/TLS check failed. Check the proxy and its credentials.");
		const QByteArray Request("GET /cdn-cgi/trace HTTP/1.1\r\nHost: 1.1.1.1\r\nConnection: close\r\n\r\n");
		if (Socket.write(Request) != Request.size() || (Socket.bytesToWrite() && !Socket.waitForBytesWritten(3000))) return Message("The tunnel check could not send its request.");
		QByteArray Reply;
		QElapsedTimer Deadline;
		Deadline.start();
		while (!Stop.load() && Deadline.elapsed() < 7000 && Reply.size() <= 16384) {
			Reply += Socket.readAll();
			if (Socket.state() == QAbstractSocket::UnconnectedState) break;
			Socket.waitForReadyRead(100);
		}
		Reply += Socket.readAll();
		if (Stop.load() || Reply.size() > 16384 || !Reply.startsWith("HTTP/1.1 200 ") || !Reply.contains("\r\n\r\n"))
			return Message("The tunnel check returned no valid HTTPS response.");
		const QByteArray Body = Reply.mid(Reply.indexOf("\r\n\r\n") + 4);
		for (const QByteArray& Line : Body.split('\n')) {
			if (!Line.startsWith("ip=")) continue;
			QHostAddress Ip(QString::fromLatin1(Line.mid(3).trimmed()));
			if (Ip.protocol() != QAbstractSocket::IPv4Protocol || Ip.isNull() || Ip.isMulticast() || Ip.isLoopback()) break;
			Address = Ip.toString();
			return QString();
		}
		return Message("The tunnel check returned no valid IPv4 address.");
	}
}
#endif

QString CProxyTunnel::RunWindows()
{
#if !defined(Q_OS_WIN)
	return tr("Tunnel execution is available only on Windows.");
#elif !defined(SBIE_PROXY_TUNNELS_LAB)
	return tr("Tunnel activation is disabled pending Windows routing and failure qualification.");
#else
	if (!QSslSocket::supportsSsl()) return tr("TLS support is unavailable. No tunnel was started.");
	const QString Exe = QDir(m_Directory).absoluteFilePath(QStringLiteral("tun2socks.exe"));
	const QString Dll = QDir(m_Directory).absoluteFilePath(QStringLiteral("wintun.dll"));
#if defined(_M_ARM64) || defined(__aarch64__)
	const QByteArray ExeHash("559eb3ee0935f628a6ab9952976b30ca7c3706dfe5fd9f1095fbe5692f3a13fa");
	const QByteArray DllHash("f7ba89005544be9d85231a9e0d5f23b2d15b3311667e2dad0debd344918a3f80");
#elif defined(_WIN64)
	const QByteArray ExeHash("076b3c3d6a372bae3f49f2b415a4105f70c30a3ed3caaed7979390e649892559");
	const QByteArray DllHash("e5da8447dc2c320edc0fc52fa01885c103de8c118481f683643cacc3220dafce");
#else
	const QByteArray ExeHash("bd964c932ca9969026f3ff47790d614f10186536c2e1330dc5cb4352f589cfa5");
	const QByteArray DllHash("d694fa46ab4cfebcb2632d094c7aa97278eef2f8052438621766d863ae98a931");
#endif
	SHandle ExeFile, DllFile;
	if (!VerifyAndLock(Exe, ExeHash, ExeFile) || !VerifyAndLock(Dll, DllHash, DllFile))
		return tr("The official, pinned tunnel dependencies are missing, changed, or in use. Install the ProxyTunnels add-on.");
	SProxyInput Input;
	QString Error;
	if (!ProxyProfiles::Reveal(m_Profile, Input, Error)) return Error;
	QHostAddress Endpoint(Input.Host);
	if (Endpoint.protocol() != QAbstractSocket::IPv4Protocol) {
		// Lookup is for the proxy endpoint, not a sandbox application's DNS request.
		QEventLoop Loop;
		QTimer Timeout, Cancel;
		Timeout.setSingleShot(true);
		connect(&Timeout, &QTimer::timeout, &Loop, &QEventLoop::quit);
		connect(&Cancel, &QTimer::timeout, &Loop, [&]() { if (m_Stop.load()) Loop.quit(); });
		const int Lookup = QHostInfo::lookupHost(Input.Host, &Loop, [&](const QHostInfo& Info) {
			for (const auto& Candidate : Info.addresses()) {
				if (Candidate.protocol() == QAbstractSocket::IPv4Protocol) { Endpoint = Candidate; break; }
			}
			Loop.quit();
		});
		Timeout.start(5000);
		Cancel.start(100);
		Loop.exec();
		QHostInfo::abortHostLookup(Lookup);
	}
	if (m_Stop.load()) return QString();
	if (Endpoint.protocol() != QAbstractSocket::IPv4Protocol) return tr("This prototype requires an IPv4-reachable proxy endpoint.");
	Input.Host = Endpoint.toString();
	NET_LUID OriginalHostRoute = {};
	if (!BestHostRoute(OriginalHostRoute)) return tr("No usable host route exists. The tunnel will not become the host default.");
	SOCKADDR_INET Destination = {};
	Destination.Ipv4.sin_family = AF_INET;
	Destination.Ipv4.sin_addr.S_un.S_addr = htonl(Endpoint.toIPv4Address());
	MIB_IPFORWARD_ROW2 UpstreamRoute = {};
	SOCKADDR_INET UpstreamSource = {};
	DWORD Code = GetBestRoute2(nullptr, 0, nullptr, &Destination, 0, &UpstreamRoute, &UpstreamSource);
	if (Code) return WinError("No route to the proxy endpoint", Code);
	wchar_t UpstreamAlias[IF_MAX_STRING_SIZE + 1] = {};
	Code = ConvertInterfaceLuidToAlias(&UpstreamRoute.InterfaceLuid, UpstreamAlias, IF_MAX_STRING_SIZE + 1);
	if (Code) return WinError("Cannot identify the upstream interface", Code);
	if (QString::fromWCharArray(UpstreamAlias).startsWith(QStringLiteral("SbieProxy-"), Qt::CaseInsensitive))
		return tr("A managed tunnel cannot be used as the upstream interface of another tunnel.");
	const std::wstring Alias = m_Profile.AdapterName().toStdWString();
	NET_LUID Existing = {};
	if (ConvertInterfaceAliasToLuid(Alias.c_str(), &Existing) == NO_ERROR)
		return tr("An adapter already has this profile's name. It was not adopted, modified, or removed. Resolve the stale adapter before retrying.");
	QLocalServer Server;
	Server.setSocketOptions(QLocalServer::UserAccessOption);
	const QString PipeName = QStringLiteral("SbieProxyConfig-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
	if (!Server.listen(PipeName)) return tr("Cannot create the private configuration pipe.");
	SChild Child;
	if (!Launch(Exe, Server.fullServerName(), Child, Error)) return Error;
	QElapsedTimer Deadline;
	Deadline.start();
	while (!m_Stop.load() && Child.Alive() && Deadline.elapsed() < 10000 && !Server.hasPendingConnections()) Server.waitForNewConnection(100);
	std::unique_ptr<QLocalSocket> Socket(Server.nextPendingConnection());
	if (!Socket || m_Stop.load()) return tr("tun2socks did not open its private configuration pipe.");
	ULONG ClientPid = 0;
	if (!GetNamedPipeClientProcessId(HANDLE(Socket->socketDescriptor()), &ClientPid) || ClientPid != Child.Pid)
		return tr("The configuration pipe peer is not the owned tunnel process. No credentials were sent.");
	QJsonObject Config{{"device", m_Profile.AdapterName()}, {"proxy", ProxyProfiles::ProxyUrl(Input)},
		{"interface", QString::fromWCharArray(UpstreamAlias)}, {"loglevel", "silent"}, {"mtu", 1500}};
	QByteArray Bytes = QJsonDocument(Config).toJson(QJsonDocument::Compact); // JSON is valid YAML.
	Input.Password.fill(QChar(0));
	Input.Password.clear();
	Input.User.clear();
	Config = QJsonObject();
	bool Sent = Socket->write(Bytes) == Bytes.size() && (!Socket->bytesToWrite() || Socket->waitForBytesWritten(3000));
	Bytes.detach();
	SecureZeroMemory(Bytes.data(), size_t(Bytes.size()));
	Bytes.clear();
	if (!Sent) return tr("Cannot deliver the tunnel configuration.");
	Socket->disconnectFromServer();
	if (Socket->state() != QLocalSocket::UnconnectedState) Socket->waitForDisconnected(3000);
	Server.close();
	SNetwork Network;
	Deadline.restart();
	while (!m_Stop.load() && Child.Alive() && Deadline.elapsed() < 15000) {
		if (ConvertInterfaceAliasToLuid(Alias.c_str(), &Network.Luid) == NO_ERROR) break;
		msleep(100);
	}
	if (m_Stop.load()) return QString();
	if (!Child.Alive() || Network.Luid.Value == 0) return tr("The tunnel process exited or did not create its adapter. Child output was discarded to protect credentials.");
	if (!Configure(Network, Error)) return Error;
	NET_LUID HostRoute = {};
	if (!BestHostRoute(HostRoute) || HostRoute.Value != OriginalHostRoute.Value)
		return tr("The host's selected route changed. The owned tunnel route was rolled back; host routes were not rewritten.");
	Deadline.restart();
	while (!m_Stop.load() && Child.Alive() && !Network.Ready() && Deadline.elapsed() < 5000) msleep(100);
	QElapsedTimer LastCheck;
	LastCheck.start();
	m_Check.store(true);
	while (!m_Stop.load()) {
		if (!Child.Alive()) return tr("The owned tunnel process exited unexpectedly. Sandbox bindings remain in place.");
		if (!Network.Ready()) return tr("The tunnel adapter or its address disappeared, or unsupported IPv6 configuration was detected.");
		if (!BestHostRoute(HostRoute) || HostRoute.Value != OriginalHostRoute.Value)
			return tr("The host route changed. The tunnel was stopped rather than changing the host's preferred route.");
		if (m_Check.exchange(false) || LastCheck.elapsed() >= 60000) {
			emit StateChanged(m_Profile.Id, Checking, tr("Checking IPv4 TCP over this adapter; this is not a leak-proofness test."));
			QString PublicAddress;
			Error = CheckEgress(Network, m_Stop, PublicAddress);
			if (!Error.isEmpty()) return Error;
			emit EgressChecked(m_Profile.Id, PublicAddress);
			emit StateChanged(m_Profile.Id, Running, tr("The adapter and owned process are active; the last IPv4 TCP check passed."));
			LastCheck.restart();
		}
		msleep(100);
	}
	emit StateChanged(m_Profile.Id, Stopping, tr("Stopping the owned process and rolling back only this tunnel's IP configuration."));
	return QString();
#endif
}
