#include "stdafx.h"
#include "HttpHelper.h"
#include "../OtherFunctions.h"
#include <QRegularExpression>
#include <QTimeZone>

THttpArgs GetHttpArguments(const QString& Arguments, QChar Separator, QChar Assigner, QString* First)
{
	// timeout=5, max=100
	// attachment; filename="bla.blup"	
	// multipart/form-data; boundary=---------------------------16586216044489
	// form-data; name="pict"; filename="a.txt"
	// de-de,de; q=0.8,en-us; q=0.5,en; q=0.3
	// cookie1=value+test; expires=Sun, 08-May-2011 11:23:04 GMT; path=/

	THttpArgs ArgumentList;

	enum EStage
	{
		eOutside	= 0,
		eReadName	= 1,
		eReadValue	= 2,
		eReadString	= 3,
		eReadEsc	= 4,
	}		Stage = eOutside;
	QString Name;
	QString Value;
	//QChar Prime = L'\0';
	for(int i = 0; i < Arguments.size(); i++)
	{
		QChar Char = Arguments.at(i);

		if(Stage == eOutside) // outside
		{
			if(Char == u' ' || Char == u'\t')
				continue;
			Stage = eReadName;
		}
		
		if(Stage == eReadName) // reading argument name, or value for default argument
		{
			if(Char == Separator)
			{
				ArgumentList.insert("",Name);
				Name.clear();
				Stage = eOutside;
			}
			else if(Char == Assigner)
				Stage = eReadValue;
			else
				Name += Char;
		}
		else if(Stage == eReadString || Stage == eReadEsc) // inside a argument value string
		{
			if(Stage == eReadEsc) // ESC sequence handling
			{
				switch(Char.unicode())
				{
					case u'\\':	Value += u'\\';	break;
					case u'\'':	Value += u'\'';	break;
					case u'\"':	Value += u'\"';	break;
					case u'a':	Value += u'\a';	break;
					case u'b':	Value += u'\b';	break;
					case u'f':	Value += u'\f';	break;
					case u'n':	Value += u'\n';	break;
					case u'r':	Value += u'\r';	break;
					case u't':	Value += u'\t';	break;
					case u'v':	Value += u'\v';	break;
					default:	Value += u'?';	break;
				}
				Stage = eReadString;
			}
			else if(Char == u'\\')
				Stage = eReadEsc;
			//else if(Char == Prime) // end of the argument value
			else if(Char == u'\"')
				Stage = eReadValue;
			else
				Value += Char;
		}
		else if(Stage == eReadValue) // reading argument value
		{
			//if(Char == L'"' || Char == L'\'') // begin of a string
			if(Char == u'"')
			{
				//Prime = Char;
				Stage = eReadString;
			}
			else if(Char == Separator)
			{
				ArgumentList.insert(Name,Value);
				if(First) {*First = Name; First = NULL;}
				Name.clear();
				Value.clear();
				Stage = eOutside;
			}
			else
				Value += Char;
		}
	}

	if(!Name.isEmpty())
	{
		if(Stage == eReadValue)
		{
			ArgumentList.insert(Name,Value);
			if(First) {*First = Name;}
		}
		else
			ArgumentList.insert("",Name);
	}

	return ArgumentList;
}

QString	GetArgument(const THttpArgs& Arguments, const QString& Name)
{
	for(THttpArgs::const_iterator I = Arguments.begin(); I != Arguments.end(); ++I)
	{
		if(I.key().compare(Name, Qt::CaseInsensitive) == 0)
			return I.value();
	}
	return "";
}

QString Url2FileName(QString Url, bool bDecode)
{
	int QueryPos = Url.indexOf("?");
	if(QueryPos != -1)
		Url.truncate(QueryPos);

	if(bDecode)
		Url = QUrl::fromPercentEncoding(Url.toLatin1());

	while(!Url.isEmpty())
	{
		int Pos = Url.lastIndexOf("/");
		if(Pos == -1)
			return Url;
		if(Pos == Url.size()-1)
		{
			Url.truncate(Pos);
			continue;
		}
		return Url.mid(Pos+1);
	}
	return "";
}

QString GetFileExt(const QString& FileName)
{
	int Pos = FileName.lastIndexOf(".");
	if(Pos != -1)
		return FileName.mid(Pos+1).toLower();
	return "";
}

QDateTime GetHttpDate(const QString &value)
{
    // HTTP dates have three possible formats:
    //  RFC 1123/822      -   ddd, dd MMM yyyy hh:mm:ss "GMT"
    //  RFC 850           -   dddd, dd-MMM-yy hh:mm:ss "GMT"
    //  ANSI C's asctime  -   ddd MMM d hh:mm:ss yyyy
    // We only handle them exactly. If they deviate, we bail out.

    int pos = value.indexOf(',');
    QDateTime dt;
    if (pos == -1) {
        // no comma -> asctime(3) format
        dt = QDateTime::fromString(value, Qt::TextDate);
    } else {
        // eat the weekday, the comma and the space following it
		if (pos + 2 > value.size())
			return QDateTime(); // nothing after the comma: the QString below would
		                    // be built from a pointer past this one's terminator
		QString sansWeekday = QString(value.data() + pos + 2);

        QLocale c = QLocale::c();
        if (pos == 3)
            // must be RFC 1123 date
            dt = c.toDateTime(sansWeekday, QLatin1String("dd MMM yyyy hh:mm:ss 'GMT'"));
        else
            // must be RFC 850 date
            dt = c.toDateTime(sansWeekday, QLatin1String("dd-MMM-yy hh:mm:ss 'GMT'"));
    }

    if (dt.isValid())
        dt.setTimeZone(QTimeZone::UTC);
    return dt;
}

bool EscalatePath(QString& Path)
{
	QStringList Dirs = Path.split("/",Qt::KeepEmptyParts);
	for(int i=0; i < Dirs.size(); i++)
	{
		if(Dirs.at(i) == "<")
		{
			Dirs.removeAt(i--);
			if(i < 0)
				return false;
			Dirs.removeAt(i--);
		}
		else if(Dirs.at(i) == "<<")
		{
			if(i == 0) // nothing above the first component, same as a single '<'
				return false;
			while(i)
				Dirs.removeAt(i--);
		}
	}
	Path = Dirs.join("/");
	return true;
}

QString GetTemplate(const QString& File, const QString& Section)
{
	QString Template = ReadFileAsString(File);
	if(Section.isEmpty())
		return Template;

	int Start = Template.indexOf("<!-- " + Section + " -->");
	if (Start == -1) 
		return "<H2>Invalid Template Section</H2>";
	Start += Section.size() + 9;

	int End = Template.indexOf("<!-- /" + Section + " -->", Start);
	if (End == -1)
		return "<H2>Invalid Template Section</H2>";

	return Template.mid(Start, End-Start).trimmed();
}

QString FillTemplate(QString Template, const THttpArgs& Variables)
{
	int Start = 0;
	int End = 0;
	// QRegExp is gone in Qt6; anchoredPattern reproduces exactMatch's semantics.
	QRegularExpression RegExp(QRegularExpression::anchoredPattern("[A-Za-z0-9_]+"));
	while((Start = Template.indexOf('{', Start)) != -1)
	{
		End = Template.indexOf('}', Start);
		if (End == -1) 
			Template.insert(Start, "<H2>Invalid Template Variable: </H2>");

		QString Variable = Template.mid(Start+1, End-Start-1);
		if (Template.mid(Start+1,3) == "tr:")
			Variable = Variable.mid(3).toUtf8();
		else if(Variables.contains(Variable.trimmed()))
			Variable = Variables.value(Variable.trimmed());
		else if(!RegExp.match(Variable).hasMatch()) // its java script or a style sheet
		{
			Start = End;
			continue;
		}
		else
			Variable.prepend("<H2>Missing Template Variable: </H2>");
			
		Template.replace(Start, End-Start+1,Variable);
		Start += Variable.size();
	}
	return Template;
}



struct SHttpTypes{
	SHttpTypes()
	{
		Map.insert("html"	, "text/html; charset=\"utf-8\"");
		Map.insert("htm"	, "text/html; charset=\"utf-8\"");
		Map.insert("shtml"	, "text/html; charset=\"utf-8\"");
		Map.insert("ehtml"	, "text/html; charset=\"utf-8\"");
		Map.insert("css"	, "text/css");
		Map.insert("json"	, "application/json");
		Map.insert("xml"	, "text/xml");
		Map.insert("xsl"	, "text/xml");
		Map.insert("xbl"	, "text/xml");
		Map.insert("txt"	, "text/plain");
		Map.insert("text"	, "text/plain");
		Map.insert("js"		, "application/javascript"); //"application/x-javascript"

		Map.insert("gif"	, "image/gif");
		Map.insert("jpg"	, "image/jpeg");
		Map.insert("jpeg"	, "image/jpeg");
		Map.insert("png"	, "image/png");
		Map.insert("bmp"	, "image/bmp");
		Map.insert("ico"	, "image/x-icon");
		Map.insert("jfif"	, "image/jpeg");
		Map.insert("pjpeg"	, "image/jpeg");
		Map.insert("pjp"	, "image/jpeg");
		Map.insert("tiff"	, "image/tiff");
		Map.insert("tif"	, "image/tiff");
		Map.insert("xbm"	, "image/x-xbitmap");
		Map.insert("svg"	, "image/svg+xml");
		Map.insert("svgz"	, "image/svg+xml");

		Map.insert("mp4"	, "video/mp4");
		Map.insert("m4v"	, "video/mp4");
		Map.insert("ogv"	, "video/ogg");
		Map.insert("ogm"	, "video/ogg");
		Map.insert("m4a"	, "audio/x-m4a");
		Map.insert("mp3"	, "audio/mp3");
		Map.insert("ogg"	, "audio/ogg");
		Map.insert("oga"	, "audio/ogg");
		Map.insert("wav"	, "audio/wav");

		Map.insert("xhtml"	, "application/xhtml+xml");
		Map.insert("xht"	, "application/xhtml+xml");
		Map.insert("exe"	, "application/octet-stream");
		Map.insert("com"	, "application/octet-stream");
		Map.insert("bin"	, "application/octet-stream");
		Map.insert("gz"		, "application/gzip");
		Map.insert("pdf"	, "application/pdf");
		Map.insert("ps"		, "application/postscript");
		Map.insert("eps"	, "application/postscript");
		Map.insert("ai"		, "application/postscript");
		Map.insert("rss"	, "application/rss+xml");
		Map.insert("rdf"	, "application/rdf+xml");
		Map.insert("swf"	, "application/x-shockwave-flash");
		Map.insert("swl"	, "application/x-shockwave-flash");
	}

	QMap<QString,QString> Map;
} SHttpTypes;

QString GetHttpContentType(QString FileName)
{
	QString Ext = GetFileExt(FileName);
	if(Ext.isEmpty())
		return "text/html; charset=utf-8";
	return SHttpTypes.Map.value(Ext, "application/octet-stream");
}


struct SHttpCodes{
	SHttpCodes()
	{
		//1xx Informational
		Map.insert(100 , "Continue");
		Map.insert(101 , "Switching Protocols");
		Map.insert(102 , "Processing");
		Map.insert(122 , "Request-URI too long");

		//2xx Success
		Map.insert(200 , "OK");
		Map.insert(201 , "Created");
		Map.insert(202 , "Accepted");
		Map.insert(203 , "Non-Authoritative Information");
		Map.insert(204 , "No Content");
		Map.insert(205 , "Reset Content");
		Map.insert(206 , "Partial Content");
		Map.insert(207 , "Multi-Status");
		Map.insert(226 , "IM Used");

		//3xx Redirection
		Map.insert(300 , "Multiple Choices");
		Map.insert(301 , "Moved Permanently");
		Map.insert(302 , "Found");
		Map.insert(303 , "See Other");
		Map.insert(304 , "Not Modified");
		Map.insert(305 , "Use Proxy");
		Map.insert(306 , "Switch Proxy");
		Map.insert(307 , "Temporary Redirect");

		//4xx Client Error
		Map.insert(400 , "Bad Request");
		Map.insert(401 , "Unauthorized");
		Map.insert(402 , "Payment Required"); // wtf
		Map.insert(403 , "Forbidden");
		Map.insert(404 , "Not Found");
		Map.insert(405 , "Method Not Allowed");
		Map.insert(406 , "Not Acceptable");
		Map.insert(407 , "Proxy Authentication Required");
		Map.insert(408 , "Request Timeout");
		Map.insert(409 , "Conflict");
		Map.insert(410 , "Gone");
		Map.insert(411 , "Length Required");
		Map.insert(412 , "Precondition Failed");
		Map.insert(413 , "Request Entity Too Large");
		Map.insert(414 , "Request-URI Too Long");
		Map.insert(415 , "Unsupported Media Type");
		Map.insert(416 , "Requested Range Not Satisfiable");
		Map.insert(417 , "Expectation Failed");
		Map.insert(422 , "Unprocessable Entity");
		Map.insert(423 , "Locked");
		Map.insert(424 , "Failed Dependency");
		Map.insert(425 , "Unordered Collection");
		Map.insert(426 , "Upgrade Required");
		Map.insert(444 , "No Response");
		Map.insert(449 , "Retry With");

		//5xx Server Error
		Map.insert(500 , "Internal Server Error");
		Map.insert(501 , "Not Implemented");
		Map.insert(502 , "Bad Gateway");
		Map.insert(503 , "Service Unavailable");
		Map.insert(504 , "Gateway Timeout");
		Map.insert(505 , "HTTP Version Not Supported");
		Map.insert(506 , "Variant Also Negotiates");
		Map.insert(507 , "Insufficient Storage");
		Map.insert(509 , "Bandwidth Limit Exceeded");
		Map.insert(510 , "Not Extended");
	}

	QMap<int,QString> Map;
} SHttpCodes;

QString GetHttpErrorString(int Code)
{
	return SHttpCodes.Map.value(Code);
}

QString GetContentDisposition(const QString& FileName)
{
	// a file name is whatever a peer or a hoster called the file: quoted (RFC 6266), with the
	// full name once more percent encoded for anything outside ASCII (RFC 5987)
	QString Ascii;
	foreach(const QChar& Char, FileName)
	{
		if(Char.unicode() < 0x20 || Char.unicode() > 0x7E)
			Ascii += '_';
		else if(Char == '"' || Char == '\\')
			Ascii += QString('\\') + Char;
		else
			Ascii += Char;
	}
	return "attachment; filename=\"" + Ascii + "\"; filename*=UTF-8''" + QString::fromLatin1(QUrl::toPercentEncoding(FileName));
}
