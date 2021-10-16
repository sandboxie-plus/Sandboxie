#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CXml {
public:
	static QString Serialize(const QVariant& Variant, bool bLazy = false);
	static QVariant Parse(const QString& String, bool bLazy = false);

	static void Serialize(const QVariant& Variant, QFile* pFile);
	static QVariant Parse(QFile* pFile);

	static void Write(const QVariant& Variant, const QString& FileName);
	static QVariant Read(const QString& FileName);

protected:
	static void Serialize(const QVariant& Variant, QXmlStreamWriter &xml, bool bLazy = false);
	static QVariant Parse(QXmlStreamReader &xml, bool bLazy = false);

	static void Serialize(const QString& Name, const QVariant& Variant, QXmlStreamWriter &xml, bool bLazy = false);
	static bool Parse(QString &Name, QVariant &Variant, QXmlStreamReader &xml, bool bLazy = false);

	static QString GetTypeStr(int Type);
	static QVariant::Type GetType(QString Type);
};
