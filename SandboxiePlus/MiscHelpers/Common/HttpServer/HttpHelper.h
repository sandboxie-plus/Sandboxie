#pragma once

#include "../../mischelpers_global.h"
// ASSERT comes from DebugHelpers.h via stdafx.h, as elsewhere in MiscHelpers.
#include <QMap>
#include <QString>
#include <QDateTime>
#include <QBuffer>
#include <QIODevice>

// Deliberately NOT Common.h's TArguments. That one is a QMultiMap, where
// insert() appends instead of replacing and there is no operator[] - using it
// here would silently emit a duplicate header every time SetHeader was called
// twice for the same name. HTTP headers, post fields and template variables are
// all single-valued, so they get a plain QMap and their own parser.
typedef QMap<QString, QString> THttpArgs;
THttpArgs MISCHELPERS_EXPORT GetHttpArguments(const QString& Arguments, QChar Separator = u';', QChar Assigner = u'=', QString* First = NULL);

struct MISCHELPERS_EXPORT SHttpPost
{
	SHttpPost(QString sName = "", QString sValue = "")
	{
		Name = sName;
		Value = sValue;
		Buffer = NULL;
		Buff = eBuffNone;
	}
	SHttpPost(const SHttpPost* Entry)
	{
		Name = Entry->Name;
		Value = Entry->Value;
		Type = Entry->Type;
		Buffer = NULL;
		ASSERT(Entry->IsBufferClosed()); // we cant copy SHttpPost's that are in a intermediate state
		Buff = eBuffNone;
		// Entry->Buff, not Buff: both tests read the field that was just
		// cleared, so neither branch ever ran and every copy came back empty
		if(Entry->Buff == eBuffExtern || Entry->Buff == eBuffExternClosed)
			Buff = eBuffWaiting; // a buffer somebody else owns cannot be shared
		else if(Entry->Buff == eBuffIntern || Entry->Buff == eBuffInternClosed)
		{
			Buff = eBuffIntern;
			Buffer = new QBuffer();
			Buffer->open(QBuffer::ReadWrite);
			Entry->Buffer->seek(0);
			Buffer->write(Entry->Buffer->readAll());
		}
	}
	~SHttpPost()
	{
		if((Buff == eBuffIntern || Buff == eBuffInternClosed) && Buffer)
			delete Buffer;
	}

	QString				Name;
	QString				Value;
	// Multipart Form Submission
	QString				Type;
	QIODevice*			Buffer;
	enum				eState
	{
		eBuffNone = 0,
		eBuffWaiting,
		eBuffExtern,
		eBuffExternClosed,
		eBuffIntern,
		eBuffInternClosed,
	}					Buff;

	void	BufferFile(QByteArray& File)
	{
		ASSERT(Buff == eBuffNone);
		Buff = eBuffIntern;
		Buffer = new QBuffer();
		Buffer->open(QBuffer::ReadWrite);
		Buffer->write(File);
		Buffer->seek(0);
	}
	void	OpenBuffer()			{if(Buff == eBuffExternClosed) Buff = Buffer ? eBuffExtern : eBuffWaiting; else if(Buff == eBuffInternClosed) Buff = eBuffIntern; }
	void	CloseBuffer()			{if(Buff == eBuffExtern) Buff = eBuffExternClosed; else if(Buff == eBuffIntern) Buff = eBuffInternClosed; }
	bool	IsBufferClosed() const	{return Buff == eBuffNone || Buff == eBuffExternClosed || Buff == eBuffInternClosed;}
};

typedef MISCHELPERS_EXPORT QList<SHttpPost*> TPostList;

QString	MISCHELPERS_EXPORT GetArgument(const THttpArgs& Arguments, const QString& Name);

QString MISCHELPERS_EXPORT Url2FileName(QString Url, bool bDecode = true);
QString MISCHELPERS_EXPORT GetFileExt(const QString& FileName);

QDateTime MISCHELPERS_EXPORT GetHttpDate(const QString &value);

bool MISCHELPERS_EXPORT EscalatePath(QString& Path);

QString MISCHELPERS_EXPORT GetTemplate(const QString& File, const QString& Section = "");
QString MISCHELPERS_EXPORT FillTemplate(QString Template, const THttpArgs& Variables);

QString	MISCHELPERS_EXPORT GetHttpContentType(QString FileName);
QString MISCHELPERS_EXPORT GetContentDisposition(const QString& FileName);
QString MISCHELPERS_EXPORT GetHttpErrorString(int Code);
