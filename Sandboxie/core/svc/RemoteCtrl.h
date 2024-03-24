#pragma once
struct RemoteConfigChanging
{
	char* m_Name;
	char* setting;
	char* value;
	bool bDelete;
	bool bSuccess;
}*PRemoteConfigChanging;
class CRemoteCtrlUtil
{
protected:
	CRemoteCtrlServer* Servers;
public:
	CRemoteCtrlUtil(CRemoteCtrlServer[]);
	~CRemoteCtrlUtil();
	static bool CheckControlList(CRemoteCtrlServer* server, int);
	bool GetChangingRequest(RemoteConfigChanging* conf);
	bool GetOperationRequest();
	bool GetEncryptKey(char* ID, char* BoxName, char* QueryServer, char* outKey);
};

class CRemoteCtrlServer
{
protected:
	char* ServerName, ServerAddress;
	unsigned int pPort;
	int pPriority;
public:
	CRemoteCtrlServer(char* lpName, char* lpIP4, unsigned int pPort = 0, int pPriority = 1);
	const char* GetServerName();
	const char* GetServerAddress();
	int GetPriority();
	~CRemoteCtrlServer();
};


class CRemoteCtrlSocket {
protected:
	CRemoteCtrlServer Server;
public:
	CRemoteCtrlSocket(CRemoteCtrlServer server);
	~CRemoteCtrlSocket();
	virtual int ConnectTo();
	virtual int DisConnect();
	virtual int AbortConnect();
	virtual int send(const char* buf);
	virtual int recv(char** buf);
	virtual int SwitchServer();
};
