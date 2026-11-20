#pragma once
class CHttpSocket;
#include "../../mischelpers_global.h"
#include "../Common.h"      // GetTime
#include "HttpHelper.h"
#include "HttpTransport.h"
#include "HttpServer.h"

class MISCHELPERS_EXPORT CHttpSocket: public QIODevice
{
	 Q_OBJECT

public:
	// pSocket is a QTcpSocket or a QLocalSocket; see HttpTransport.h.
	CHttpSocket(QIODevice* pSocket, quint32 KeepAlive, QObject* parent = NULL);
	~CHttpSocket();
	void			Reset();

	enum ERequestType
	{
		eNone = 0,
		eGET,
		ePUT,
		ePOST,
		eMultiPOST,
		ePlainPOST,
		eRawPOST,
		eOPTIONS,
		eHEAD,
		eDELETE,
	};

	enum EState
	{
		eWaiting = 0, 
		eReading,	// reading request
		eHandling,	// passed controll to handler 
		eFailing,
		eWriting,	// writign resposne
	};

	QString			GetPath()													{return m_RequestPath;}
	QString			GetQuery()													{return m_RequestQuery;}
	ERequestType	GetType()													{return m_RequestType;}
	bool			IsGet()														{return m_RequestType == eGET;}
	bool			IsPost()													{return m_RequestType == ePOST || m_RequestType == eMultiPOST || m_RequestType == ePlainPOST || m_RequestType == eRawPOST;}
	bool			IsPut()														{return m_RequestType == ePUT;}

	QHostAddress	GetAddress()												{return HttpTransport::GetPeerAddress(m_pSocket);} // null on a local socket
	quint16			GetLocalPort()												{return HttpTransport::GetLocalPort(m_pSocket);}   // 0 on a local socket
	QString			GetPeerName()												{return HttpTransport::GetPeerName(m_pSocket);}    // for logs only

	// The connecting process, or 0 when it cannot be established. 0 means
	// "unknown" and never "benign" - a caller gating access must refuse on it.
	quint32			GetPeerProcessId()											{return HttpTransport::GetPeerProcessId(m_pSocket);}
	bool			IsLocal()													{return HttpTransport::IsLocal(m_pSocket);}

	void			SetRawHandling()											{m_RawData = true; m_TransactionState = eHandling;}
	bool			HasRawData()												{return m_RawData;}

	EState			GetState()													{return m_TransactionState;}

	void			SetRequestBuffer(QIODevice* FileBuffer)						{ASSERT(m_TransactionState == eReading); m_FileBuffer = FileBuffer;} // for put and Rawpost
	void			SetResponseBuffer(QIODevice* FileBuffer, int Code = 0)		{ASSERT(m_TransactionState == eHandling); m_FileBuffer = FileBuffer; m_ResponseCode = Code;}

	void			SendResponse(int Code = 0, quint64 UploadSize = -1);

	void			RespondWithFile(QString FilePath);
	void			RespondWithError(int Code, QString Response = "");

	bool			KeepAlive()													{return (m_LastRequest + m_KeepAlive) >= GetTime();}

	QString			GetHeader(const QString &Name);
	const THttpArgs& GetHeader()												{return m_RequestHeader;}

	QStringList		GetPostKeys();
	QString			GetPostValue(const QString &Name);
	QIODevice*		GetPostFile(const QString &Name);

	bool			HeaderSet(const QString &Name)								{return m_ResponseHeader.contains(Name);}
	void			SetHeader(const QString &Name, const QString &Value)		{m_ResponseHeader.insert(Name,Value);}	
	void			SetCaching(time_t uMaxAge);

	bool			RequestCompleted() const									{return m_DownloadedSize <= m_Downloaded;}

	// disconnect(this) unhooked nothing: the readyRead/disconnected slots hang
	// off the CHttpServer, not off this. Closing is what CHttpServer::Process
	// wanted, and it takes OnDisconnect's cleanup with it.
	void			Drop()														{HttpTransport::Disconnect(m_pSocket);}
	QIODevice*		SetPostBuffer(QIODevice* FileBuffer = NULL);

	virtual qint64	bytesAvailable() const										{return m_DownloadedSize != -1 ? m_DownloadedSize : 0;}
	virtual qint64	bytesToWrite() const										{return m_UploadSize != -1 ? m_UploadSize : 0;}
	virtual bool	isSequential() const										{return false;}
	//virtual bool	atEnd() const												{return m_RequestBuffer.isEmpty() && RequestCompleted();}
	virtual qint64	size() const												{return m_DownloadedSize;}

signals:
	void			FilePosted(QString Name, QString File, QString Type);

protected:
	friend class CHttpServer;

	void			TryReadSocket();
	bool			TrySendBuffer();

	int				HandleHeader(QStringList &RequestHeader);
	void			HandleData();

	CHttpServer*	GetServer()													{return qobject_cast<CHttpServer*>(parent());}

	virtual qint64	readData(char *data, qint64 maxlen);
    virtual qint64	writeData(const char *data, qint64 len);

	QIODevice*		m_pSocket;
	EState			m_TransactionState;

	// Request
	ERequestType	m_RequestType;
	bool			m_RawData;
	QString			m_RequestPath;
	QString			m_RequestQuery;
	THttpArgs		m_RequestHeader;
	TPostList		m_PostedData;
	QByteArray		m_Boundary;
	QIODevice*		m_FileBuffer;
	QByteArray		m_RequestBuffer;
	quint32			m_KeepAlive;
	time_t			m_LastRequest;

	quint64			m_DownloadedSize;
	quint64			m_Downloaded;

	// Response
	int				m_ResponseCode;
	THttpArgs		m_ResponseHeader;
	QByteArray		m_ResponseBuffer;

	quint64			m_UploadSize;
	quint64			m_Uploaded;
};
