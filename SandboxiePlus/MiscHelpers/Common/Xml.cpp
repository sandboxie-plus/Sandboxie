#include "stdafx.h"
#include "Xml.h"

QString CXml::Serialize(const QVariant &Variant, bool bLazy)
{
	QString String;
	QXmlStreamWriter xml(&String);
	Serialize(Variant, xml, bLazy);
	return String;
}

QVariant CXml::Parse(const QString& String, bool bLazy)
{
	QXmlStreamReader xml(String);
	return Parse(xml, bLazy);
}

void CXml::Serialize(const QVariant& Variant, QFile* pFile)
{
	QXmlStreamWriter xml(pFile);
	Serialize(Variant, xml);
}

QVariant CXml::Parse(QFile* pFile)
{
	QXmlStreamReader xml(pFile);
	return Parse(xml);
}

void CXml::Write(const QVariant& Variant, const QString& FileName)
{
	QFile File(FileName + ".tmp");
	File.open(QFile::WriteOnly);
	Serialize(Variant, &File);

	QFile::rename(FileName, FileName + ".bak");
	File.rename(FileName);
	File.remove(FileName + ".bak");
}

QVariant CXml::Read(const QString& FileName)
{
	QFile File(FileName);
	File.open(QFile::ReadOnly);
	return Parse(&File);
}

void CXml::Serialize(const QVariant& Variant, QXmlStreamWriter &xml, bool bLazy)
{
#ifdef _DEBUG
	xml.setAutoFormatting(true);
#endif
	xml.writeStartDocument();
	Serialize("Variant", Variant, xml, bLazy);
	xml.writeEndDocument();
}

QVariant CXml::Parse(QXmlStreamReader &xml, bool bLazy)
{
	QVariant Variant;
	QString Temp;
	Parse(Temp, Variant, xml, bLazy);
	return Variant;
}

void CXml::Serialize(const QString& Name, const QVariant& Variant, QXmlStreamWriter &xml, bool bLazy)
{
	xml.writeStartElement(Name);
	xml.writeAttribute("Type", GetTypeStr(Variant.type()));
	switch(Variant.type())
	{
		case QVariant::Map:
		{
			QVariantMap Map = Variant.toMap();
			for(QVariantMap::iterator I = Map.begin(); I != Map.end(); ++I)
				Serialize(I.key(), I.value(), xml, bLazy);
			break;
		}
		case QVariant::Hash:
		{
			QVariantHash Hash = Variant.toHash();
			for(QVariantHash::iterator I = Hash.begin(); I != Hash.end(); ++I)
				Serialize(I.key(), I.value(), xml, bLazy);
			break;
		}
		case QVariant::List:
		{
			QVariantList List = Variant.toList();
			for(QVariantList::iterator I = List.begin(); I != List.end(); ++I)
				Serialize("Variant", *I, xml, bLazy);
			break;
		}
		case QVariant::StringList:
		{
			QStringList List = Variant.toStringList();
			for(QStringList::iterator I = List.begin(); I != List.end(); ++I)
				Serialize("Variant", *I, xml, bLazy);
			break;
		}
		case QVariant::ByteArray:
			if(!bLazy)
			{
				xml.writeCharacters (Variant.toByteArray().toBase64());
				break;
			}
		default:
			ASSERT(Variant.canConvert(QVariant::String));
			xml.writeCharacters(Variant.toString().toUtf8().toPercentEncoding(" :;/|,'+()"));
			//xml.writeCharacters (Variant.toString().replace("\\","\\\\").replace("\r","\\r").replace("\n","\\n"));
			break;
		case QVariant::Invalid:
			break;
	}
	xml.writeEndElement();
}

bool CXml::Parse(QString &Name, QVariant &Variant, QXmlStreamReader &xml, bool bLazy)
{
	bool bOpen = false;
	QVariant::Type eType = QVariant::Invalid;
	QString Text;
	while (!xml.atEnd())
	{
		xml.readNext();
		if (xml.error()) 
			break;
		if (xml.isEndDocument())
			continue;

		if (xml.isStartElement())
		{
			bOpen = true;
			Name = xml.name().toString();
			eType = GetType(xml.attributes().value("Type").toString());
			QString Temp;
			QVariant Item;
			switch(eType)
			{
				case QVariant::Map:
				{
					QVariantMap Map;
					while(Parse(Temp, Item, xml, bLazy))
						Map.insert(Temp, Item);
					Variant = Map;
					return true;
				}
				case QVariant::Hash:
				{
					QVariantHash Hash;
					while(Parse(Temp, Item, xml, bLazy))
						Hash.insert(Temp, Item);
					Variant = Hash;
					return true;
				}
				case QVariant::List:
				{
					QVariantList List;
					while(Parse(Temp, Item, xml, bLazy))
						List.append(Item);
					Variant = List;
					return true;
				}
				case QVariant::StringList:
				{
					QStringList List;
					while(Parse(Temp, Item, xml, bLazy))
						List.append(Item.toString());
					Variant = List;
					return true;
				}
			}
		}
		else if (xml.isCharacters())
		{
			if(bOpen)
				Text.append(xml.text().toString());
		}
		else if (xml.isEndElement())
		{
			if(bOpen)
			{
				if(eType == QVariant::ByteArray && !bLazy)
					Variant.setValue(QByteArray::fromBase64(Text.toLatin1()));
				else
				{
					/*bool bEsc = false;
					for(int i = 0; i < Text.size(); i++)
					{
						if(bEsc) // ESC sequence handling
						{
							switch(Text.at(i).unicode())
							{
								case L'\\':	Text.replace(--i,2,"\\");	break;
								case L'r':	Text.replace(--i,2,"\r");	break;
								case L'n':	Text.replace(--i,2,"\n");	break;
								default:	Text.replace(--i,2,"?");	break;
							}
							bEsc = false;
						}
						else if(Text.at(i) == L'\\')
							bEsc = true;
					}*/

					Variant = QString::fromUtf8(QByteArray::fromPercentEncoding(Text.toLatin1()));
					if(eType) // type is optional
					{
						ASSERT(Variant.canConvert(eType));
						Variant.convert(eType);
					}
				}
				return true;
			}
			return false;
		}
	}

	//ASSERT(0); // incomplete XML
	return false;
}


/* SQVariants provides a lookup list of all known QVariant types for to/from string conversion
*
*	Note: All commented out types don't have native to string conversion 
*			If there is a need to use them a manual conversion must be implemented in CXml
*/
struct SQVariants{
	SQVariants()
	{
		Map.insert("Invalid"	, QVariant::Invalid);

		Map.insert("Bool"		, QVariant::Bool);
		Map.insert("Int"		, QVariant::Int);
		Map.insert("UInt"		, QVariant::UInt);
		Map.insert("LongLong"	, QVariant::LongLong);
		Map.insert("ULongLong"	, QVariant::ULongLong);
		Map.insert("Double"		, QVariant::Double);
		Map.insert("Char"		, QVariant::Char);
		Map.insert("Map"		, QVariant::Map);	// container type
		Map.insert("List"		, QVariant::List);	// container type
		Map.insert("String"		, QVariant::String);
		Map.insert("StringList"	, QVariant::StringList);	// container type
		Map.insert("ByteArray"	, QVariant::ByteArray);
		//Map.insert("BitArray"	, QVariant::BitArray);
		Map.insert("Date"		, QVariant::Date);
		Map.insert("Time"		, QVariant::Time);
		Map.insert("DateTime"	, QVariant::DateTime);
		Map.insert("Url"		, QVariant::Url);
		/*Map.insert("Locale"		, 18);
		Map.insert("Rect"		, 19);
		Map.insert("RectF"		, 20);
		Map.insert("Size"		, 21);
		Map.insert("SizeF"		, 22);
		Map.insert("Line"		, 23);
		Map.insert("LineF"		, 24);
		Map.insert("Point"		, 25);
		Map.insert("PointF"		, 26);
		Map.insert("RegExp"		, 27);*/
		Map.insert("Hash"		, QVariant::Hash);	// container type

		/*Map.insert("Font"		, 64);
		Map.insert("Pixmap"		, 65);
		Map.insert("Brush"		, 66);
		Map.insert("Color"		, 67);
		Map.insert("Palette"	, 68);
		Map.insert("Icon"		, 69);
		Map.insert("Image"		, 70);
		Map.insert("Polygon"	, 71);
		Map.insert("Region"		, 72);
		Map.insert("Bitmap"		, 73);
		Map.insert("Cursor"		, 74);
		Map.insert("SizePolicy"	, 75);
		Map.insert("KeySequence", 76);
		Map.insert("Pen"		, 77);
		Map.insert("TextLength"	, 78);
		Map.insert("TextFormat"	, 79);
		Map.insert("Matrix"		, 80);
		Map.insert("Transform"	, 81);
		Map.insert("Matrix4x4"	, 82);
		Map.insert("Vector2D"	, 83);
		Map.insert("Vector3D"	, 84);
		Map.insert("Vector4D"	, 85);
		Map.insert("Quaternion"	, 86);*/
	}
	QMap<QString,int> Map;
} SQVariants;

QString CXml::GetTypeStr(int Type)			{return SQVariants.Map.key(Type, "Invalid");}
QVariant::Type CXml::GetType(QString Type)	{return (QVariant::Type)SQVariants.Map.value(Type, 0);}
