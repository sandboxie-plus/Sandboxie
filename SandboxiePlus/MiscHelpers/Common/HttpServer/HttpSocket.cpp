#include "stdafx.h"
#include "HttpSocket.h"
#include "HttpServer.h"
#include "../DebugHelpers.h"
#if 0 // response gzip is pointless over a local pipe and qzlib is not part of
      // MiscHelpers; kept as code so re-enabling it is one line plus the port.
#include "../qzlib.h"
#endif
#include <QLocale>

CHttpSocket::CHttpSocket(QIODevice* pSocket, quint32 KeepAlive, QObject* parent)
: QIODevice(parent)
{
	m_pSocket = pSocket;
	m_KeepAlive = KeepAlive;
	// the window starts now, not at the first request: a client that has
	// connected and not finished its header yet is not an expired one
	m_LastRequest = GetTime();
	Reset();
}

CHttpSocket::~CHttpSocket()
{
	foreach(SHttpPost* Entry, m_PostedData)
		delete Entry;
}

void CHttpSocket::Reset()
{
	close();

	m_TransactionState = eWaiting;

	// Request
	m_RequestType = eNone;
	m_RawData = false;
	m_RequestPath.clear();
	m_RequestQuery.clear();
	m_RequestHeader.clear();
	foreach(SHttpPost* Entry, m_PostedData)
		delete Entry;
	m_PostedData.clear();
	m_Boundary.clear();
	m_FileBuffer = NULL;
	m_RequestBuffer.clear();

	m_DownloadedSize = -1;
	m_Downloaded = 0;

	// Response
	m_ResponseCode = 0;
	m_ResponseHeader.clear();
	m_ResponseBuffer.clear();

	m_UploadSize = -1;
	m_Uploaded = 0;

	open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

void CHttpSocket::TryReadSocket()
{
	if(m_RequestBuffer.size() > (1024 * 1024))
		return;

	QByteArray Data = m_pSocket->readAll();
	m_RequestBuffer.append(Data);
	m_Downloaded += Data.size();
	if(!m_RequestBuffer.isEmpty())
		emit readyRead();
}

int CHttpSocket::HandleHeader(QStringList &RequestHeader)
{
	QStringList Request = QString(RequestHeader.takeFirst()).split(" ");
	if(Request.count() != 3)
	{
		GetServer()->LogLine(CHttpServer::eLogDebug, tr("recived invalid HTTP request"));
		return 400;
	}

	if(Request[0] == "GET")
		m_RequestType = CHttpSocket::eGET;
	else if(Request[0] == "POST")
		m_RequestType = CHttpSocket::ePOST;
	else if(Request[0] == "PUT")
	{
		m_RequestType = CHttpSocket::ePUT;
		m_RawData = true;
	}
	else if(Request[0] == "OPTIONS")
		m_RequestType = CHttpSocket::eOPTIONS;
	else if(Request[0] == "HEAD")
		m_RequestType = CHttpSocket::eHEAD;
	else if(Request[0] == "DELETE")
		m_RequestType = CHttpSocket::eDELETE;
	else
	{
		GetServer()->LogLine(CHttpServer::eLogDebug, tr("recived unsupported HTTP request %1 from %2").arg(Request[0]).arg(GetPeerName()));
		return 501;
	}

	int Mark = Request[1].indexOf("?");
	m_RequestPath = QUrl::fromPercentEncoding(((Mark != -1) ? Request[1].left(Mark) : Request[1]).toLatin1());
	if(!EscalatePath(m_RequestPath))
		return 401;
	m_RequestQuery = (Mark != -1) ? Request[1].mid(Mark) : "";
	
	foreach(const QString &Entry, RequestHeader)
	{
		int Sep = Entry.indexOf(":");
		if(Sep != -1)
			m_RequestHeader.insert(Entry.left(Sep),Entry.mid(Sep+1).trimmed());
	}

	switch(m_RequestType)
	{
		case ePOST: // eMultiPOST, ePlainPOST, eRawPOST
		{
			THttpArgs PostType = GetHttpArguments(GetHeader("Content-Type"));
			if(PostType.value("").compare("multipart/form-data", Qt::CaseInsensitive) == 0)
			{
				m_RequestType = CHttpSocket::eMultiPOST;
				m_Boundary = PostType.value("boundary").toUtf8();
				if(m_Boundary.isEmpty())
				{
					GetServer()->LogLine(CHttpServer::eLogDebug, tr("recived invalid POST request %1").arg(GetHeader("Content-Type")));
					return 400;
				}
			}
			else if(PostType.value("").compare("text/plain", Qt::CaseInsensitive) == 0)
				m_RequestType = CHttpSocket::ePlainPOST;
			else if(PostType.value("").compare("application/x-www-form-urlencoded", Qt::CaseInsensitive) != 0)
			{
				m_RequestType = CHttpSocket::eRawPOST;
				m_RawData = true;
			}
		}
		case ePUT:
		{
			QString sLength = GetHeader("Content-Length");
			if(!sLength.isEmpty())
				m_DownloadedSize = sLength.toULongLong();
			else //if(m_RequestType != CHttpSocket::eMultiPOST) // Note: wqith Boundries we can find the end relaiably
			{
				GetServer()->LogLine(CHttpServer::eLogWarning, tr("recived POST/PUT request without Content-Length !!!"));
				return 411;
			}
			break;
		}
		case eGET:
		case eOPTIONS:
		case eHEAD:
		case eDELETE:
		{
			m_DownloadedSize = 0;
			break;
		}
	}

	if(GetHeader("Connection").compare("close", Qt::CaseInsensitive) == 0)
		m_KeepAlive = 0;
	else if(!GetHeader("Keep-Alive").isEmpty()) // GetHeader("Connection").compare("keep-alive", Qt::CaseInsensitive)) == 0
		m_KeepAlive = GetHeader("Keep-Alive").toUInt();
	m_LastRequest = GetTime();

	return 0;
}

void CHttpSocket::HandleData()
{
	switch(m_RequestType)
	{
		case CHttpSocket::eGET:
			if(!m_RequestBuffer.isEmpty())
				GetServer()->LogLine(CHttpServer::eLogError, tr("recived GET request with attached data ?!"));
			break;
		case CHttpSocket::eRawPOST: // bulk data, same as put
		case CHttpSocket::ePUT:
		{
			if (m_FileBuffer)
			{
				m_FileBuffer->write(m_RequestBuffer);
				m_RequestBuffer.clear();
			}
			break;
		}
		case CHttpSocket::ePOST:
		case CHttpSocket::ePlainPOST:
		{
			//if(!RequestCompleted() && (m_DownloadedSize != -1 || m_RequestBuffer.indexOf("\r\n") == -1))
			//	break;

			if(!RequestCompleted())
				break;
			
			THttpArgs Post = GetHttpArguments(m_RequestBuffer, u'&');
			foreach(const QString &Name, Post.keys())
			{
				QString Value = Post.value(Name);
				Value.replace("+", " ");
				if(m_RequestType == CHttpSocket::ePOST)
					m_PostedData.append(new SHttpPost(Name, QUrl::fromPercentEncoding(Value.toUtf8())));
				else
					m_PostedData.append(new SHttpPost(Name, Value));
			}
			m_RequestBuffer.clear();

			//if(m_DownloadedSize == -1)
			//	m_DownloadedSize = m_Downloaded;
			break;
		}
		case CHttpSocket::eMultiPOST:
		{
			int BoundarySize = m_Boundary.size()+4;

			SHttpPost* Post = m_PostedData.isEmpty() ? NULL : m_PostedData.last();
			if(Post && Post->Buffer)
			{
				if(m_RequestBuffer.size() > BoundarySize)
				{
					int End = m_RequestBuffer.indexOf("\r\n--" + m_Boundary);
					if(End != -1)
					{
						Post->Buffer->write(m_RequestBuffer.data(), End);
						m_RequestBuffer.remove(0,End+2);
						Post->CloseBuffer();
					}
					else
					{
						// hold back a whole boundary marker, so one split across two
						// reads is never written into the file. Without the brackets
						// this held back boundary-4, which goes negative for a short
						// boundary and reads past the buffer.
						quint64 uToGo = m_RequestBuffer.size() - (m_Boundary.size()+4);
						Post->Buffer->write(m_RequestBuffer.data(), uToGo);
						m_RequestBuffer.remove(0,uToGo);
					}
				}
			}

			if(Post && !Post->IsBufferClosed())
				return;

			while(m_RequestBuffer.size() > BoundarySize)
			{
				if(m_RequestBuffer.left(BoundarySize) == "--" + m_Boundary + "\r\n")
				{
					int End = m_RequestBuffer.indexOf("\r\n\r\n");
					if(End == -1) // hreader not yet complee
						break;

					QStringList PartHeader = QString(m_RequestBuffer.mid(BoundarySize, End-BoundarySize)).split('\n');
					m_RequestBuffer.remove(0,End+4);
					
					THttpArgs PostHeader;
					foreach(const QString &Entry, PartHeader)
					{
						int Sep = Entry.indexOf(":");
						if(Sep != -1)
							PostHeader.insert(Entry.left(Sep),Entry.mid(Sep+1).trimmed());
					}

					Post = new SHttpPost();
					m_PostedData.append(Post);
					THttpArgs Disposition = GetHttpArguments(PostHeader.value("Content-Disposition"));
					Post->Name = Disposition.value("name");
					Post->Type = PostHeader.value("Content-Type");
					if(Disposition.contains("filename"))
					{
						Post->Value = Disposition.value("filename");
						Post->Buff = SHttpPost::eBuffWaiting;
						emit FilePosted(Post->Name, Post->Value, Post->Type);
						return;
					}
				}
				else if(m_RequestBuffer.left(BoundarySize+2) == "--" + m_Boundary + "--\r\n")
				{
					if(m_DownloadedSize == -1)
						m_DownloadedSize = m_Downloaded;
					break;
				}
				else 
				{
					int End = m_RequestBuffer.indexOf("\r\n--" + m_Boundary);
					if(End == -1) // paylaod not yet complee
						break;

					if(Post) // no part header came first, so there is nothing to attach
						Post->Value = m_RequestBuffer.left(End);
					else    // this to. Drop it and resync on the boundary.
						GetServer()->LogLine(CHttpServer::eLogDebug, tr("recived a POST payload before any part header"));
					m_RequestBuffer.remove(0,End+2);
				}
			}
		}
	}

	if(RequestCompleted())
	{
		ASSERT(m_TransactionState == eReading);
		m_FileBuffer = NULL;
		m_TransactionState = eHandling;
		//TRACE(L"Socket %i emiting readChannelFinished", (int)this);
		emit readChannelFinished();
	}
}

QIODevice* CHttpSocket::SetPostBuffer(QIODevice* FileBuffer)
{
	ASSERT(!m_PostedData.isEmpty());

	SHttpPost* Post = m_PostedData.last();
	ASSERT(Post->Buff == SHttpPost::eBuffWaiting);
	if(FileBuffer)
	{
		Post->Buff = SHttpPost::eBuffExtern;
		Post->Buffer = FileBuffer; 
	}
	else
	{
		Post->Buff = SHttpPost::eBuffIntern;
		FileBuffer = Post->Buffer = new QBuffer(this); 
		Post->Buffer->open(QBuffer::ReadWrite);
	}
	HandleData();
	return FileBuffer;
}

void CHttpSocket::SendResponse(int Code, quint64 UploadSize)
{
	if(!m_ResponseCode && !Code)
		m_ResponseCode = 200; // ok
	else if(Code)
		m_ResponseCode = Code;

	ASSERT(m_ResponseBuffer.isEmpty() || m_FileBuffer == NULL); // Eider or, not booth

	if(UploadSize != -1)
	{
		ASSERT(UploadSize >= (quint64)m_ResponseBuffer.size());
		m_UploadSize = UploadSize;
	}
	else if(m_FileBuffer)
	{
		if(!m_FileBuffer->isSequential())
			m_UploadSize = m_FileBuffer->size();
	}
	else
	{
#if 0 // see the qzlib note at the top of this file
		if(!m_ResponseBuffer.isEmpty() && GetHeader("Accept-Encoding").contains("gzip"))
		{
			if(gzip_arr(m_ResponseBuffer))
				SetHeader("Content-Encoding", "gzip");
		}
#endif
		m_UploadSize = m_ResponseBuffer.size();
	}

	ASSERT(!HeaderSet("Content-Length"));
	if(m_UploadSize != -1)
		SetHeader("Content-Length", QString::number(m_UploadSize));
	else
		SetHeader("Transfer-Encoding", "chunked");

	if(!HeaderSet("Content-Type"))
		SetHeader("Content-Type", GetHttpContentType(m_RequestPath));

	if(m_KeepAlive)
	{
		SetHeader("Connection", "keep-alive");
		if(!GetHeader("Keep-Alive").isEmpty())
			SetHeader("Keep-Alive", QString("timeout=%1").arg(m_KeepAlive)); // "timeout=%1, max=%2"
	}
	else
		SetHeader("Connection", "close");
		
	SetHeader("Server", GetServer()->GetServerName());

	QStringList ResponseHeader;
	ResponseHeader.append(QString("HTTP/1.1 %1 ").arg(m_ResponseCode) + GetHttpErrorString(m_ResponseCode));
	foreach(const QString& Tag, m_ResponseHeader.keys())
	{
		// values carry file names and urls from elsewhere: a line break in one was a header of its own
		QString Line = Tag + ": " + m_ResponseHeader.value(Tag);
		ResponseHeader.append(Line.remove('\r').remove('\n'));
	}
	ResponseHeader.append("\r\n");

	m_ResponseBuffer.prepend(ResponseHeader.join("\r\n").toUtf8());

	ASSERT(m_TransactionState == CHttpSocket::eHandling || m_TransactionState == CHttpSocket::eFailing);
	m_TransactionState = eWriting;

	TrySendBuffer();
}

bool CHttpSocket::TrySendBuffer()
{
	const qint64 BufferBound = HttpTransport::GetReadBufferSize(m_pSocket);
	while(m_FileBuffer && m_ResponseBuffer.size() < BufferBound)
	{
		const qint64 Size = 16*1024;
		char Buffer[Size];
		qint64 uRead = m_FileBuffer->read(Buffer,Size);
		if(uRead > 0)
		{
			if(m_UploadSize == -1) m_ResponseBuffer.append(QByteArray::number(uRead, 16).append("\r\n"));
			m_ResponseBuffer.append(Buffer,uRead);
			if(m_UploadSize == -1) m_ResponseBuffer.append("\r\n");
		}
		else if(uRead == -1 || (!m_FileBuffer->isSequential() && m_FileBuffer->atEnd()))
		{
			if(m_UploadSize == -1) m_ResponseBuffer.append("0\r\n\r\n");
			m_FileBuffer = NULL;
		}
		else if(uRead == 0)
			break;
	}

	if(!m_ResponseBuffer.isEmpty() && m_pSocket->bytesToWrite() < BufferBound)
	{
		quint64 Writen = m_pSocket->write(m_ResponseBuffer);
		if(Writen > 0)
			m_ResponseBuffer.remove(0,Writen);
		else if(Writen == -1)
		{
			GetServer()->LogLine(CHttpServer::eLogError, tr("Error While Sending response"));
			m_pSocket->disconnect(this);
			return false;
		}
	}

	if(m_ResponseBuffer.isEmpty() && (!m_FileBuffer || (m_UploadSize != -1 && m_UploadSize <= m_Uploaded))) // is buffer empty and nothing more to be put into it?
	{
		//TRACE(L"Socket %d finished sending", (int)this);
		ASSERT(m_TransactionState == eWriting);

		if(CHttpHandler* pHandler = GetServer()->GetHandler(GetPath(), GetLocalPort()))
			pHandler->ReleaseRequest(this);
		Reset();
	}
	return true;
}

void CHttpSocket::RespondWithFile(QString FilePath)
{
	ASSERT(m_Uploaded == 0);

	QFile File(FilePath);
	if(!File.open(QIODevice::ReadOnly))
		RespondWithError(404);
	else
	{
		SetHeader("Content-Type", GetHttpContentType(FilePath));
		write(File.readAll());
		File.close();
	}
}

void CHttpSocket::RespondWithError(int Code, QString Response)
{
	ASSERT(m_Uploaded == 0);
	m_TransactionState = eFailing;
	m_ResponseCode = Code;

	THttpArgs Variables;
	Variables["Code"] = QString::number(Code);
	Variables["Error"] = GetHttpErrorString(Code);
	Variables["Message"] = Response;
	write(FillTemplate(GetTemplate(":/httpError"),Variables).toUtf8());
}

qint64 CHttpSocket::readData(char *data, qint64 maxlen)
{
	if(!m_RawData)
		return -1;	// you can't read unless its raw

	qint64 uToGo = m_RequestBuffer.size();
	if(uToGo > maxlen)
		uToGo = maxlen;
	memcpy(data, m_RequestBuffer.data(), uToGo);
	m_RequestBuffer.remove(0,uToGo);
	return uToGo;
}

qint64 CHttpSocket::writeData(const char *data, qint64 len)
{
	if(m_UploadSize != -1) // are we in long uplaod mode
	{
		if(m_ResponseBuffer.size() > (1024 * 1024))
			return 0;

		ASSERT(m_UploadSize >= m_Uploaded);
		ASSERT((quint64)len <= m_UploadSize - m_Uploaded);
	}
	m_ResponseBuffer.append(data,len);
	m_Uploaded += len;
	return len;
}

QString CHttpSocket::GetHeader(const QString &Name)
{
	for(THttpArgs::iterator I = m_RequestHeader.begin(); I != m_RequestHeader.end(); I++)
	{
		if(I.key().compare(Name, Qt::CaseInsensitive) == 0)
			return I.value();
	}
	return "";
}

QStringList CHttpSocket::GetPostKeys()
{
	QStringList Keys;
	foreach(SHttpPost* Post, m_PostedData)
		Keys.append(Post->Name);
	return Keys;
}

QString CHttpSocket::GetPostValue(const QString &Name)
{
	foreach(SHttpPost* Post, m_PostedData)
	{
		if(Post->Name == Name)
			return Post->Value;
	}
	return "";
}

QIODevice* CHttpSocket::GetPostFile(const QString &Name)
{
	foreach(SHttpPost* Post, m_PostedData)
	{
		if(Post->Name == Name)
		{
			if(!Post->Buffer) // a plain form field, not a file - keep looking,
				continue;     // a part of the same name may still carry one
			ASSERT(Post->Buffer->pos() == Post->Buffer->size());
			Post->Buffer->seek(0);
			return Post->Buffer;
		}
	}
	return NULL;
}

void CHttpSocket::SetCaching(time_t uMaxAge)
{
	if(uMaxAge == 0)
	{
		SetHeader("Pragma", "no-cache");
		SetHeader("Cache-Control", "no-cache, private, must-revalidate, max-stale=0, post-check=0, pre-check=0, no-store");
	}
	else
	{
		SetHeader("Pragma", "public");
		SetHeader("Cache-Control", "must-revalidate, max-age=" + QString::number(uMaxAge));
	}
	// addSecs returns the shifted time rather than shifting in place, so this
	// used to expire now; and the format has to be RFC 1123 in the C locale,
	// which is what GetHttpDate reads back.
	QDateTime Expires = QDateTime::currentDateTimeUtc().addSecs(uMaxAge);
	SetHeader("Expires", QLocale::c().toString(Expires, "ddd, dd MMM yyyy hh:mm:ss") + " GMT");
}