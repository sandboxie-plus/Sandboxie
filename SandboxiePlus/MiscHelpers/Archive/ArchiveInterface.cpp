#include "stdafx.h"
#include "ArchiveInterface.h"
//#include "../Functions.h"
#ifdef __APPLE__
#include <QCoreApplication>
#endif

#ifdef USE_7Z

CArchiveInterface::CArchiveInterface()
: m_7z("7z")
{
	m_Operational = false;
}

bool CArchiveInterface::Init()
{
	QMutexLocker Locker(&m_Mutex);

	if(m_Operational)
		return true;

#ifdef __APPLE__
	m_7z.setFileName(QCoreApplication::applicationDirPath() + "/7z.so");
#endif

	//LogLine(LOG_INFO | LOG_DEBUG, QObject::tr("7z: Loading Library..."));
	if(!m_7z.load())
	{
		//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: failed to load!"));
		return false;
	}

	// Load Available Coders
	/*GetMethodPropertyFunc getMethodPropertyFunc = (GetMethodPropertyFunc)m_7z.resolve("GetMethodProperty");
	GetNumberOfMethodsFunc getNumberOfMethodsFunc = (GetNumberOfMethodsFunc)m_7z.resolve("GetNumberOfMethods");
	if(!getNumberOfMethodsFunc || !getMethodPropertyFunc)
		return false;

	UInt32 numMethods = 1;
	if(getNumberOfMethodsFunc(&numMethods) != S_OK)
		return false;

	for(UInt32 i = 0; i < numMethods; i++)
	{
		CCdrInfo info;
		info.Index = i;
		Q_ASSERT(GetCoderClass(getMethodPropertyFunc, i, NMethodPropID::kEncoder, info.Encoder, info.EncoderIsAssigned) == S_OK);
		Q_ASSERT(GetCoderClass(getMethodPropertyFunc, i, NMethodPropID::kDecoder, info.Decoder, info.DecoderIsAssigned) == S_OK);
		m_Coders.append(info);
	}*/

	// Load Supported Formats
	GetHandlerPropertyFunc getProp = NULL;
	GetHandlerPropertyFunc2 getProp2 = (GetHandlerPropertyFunc2)m_7z.resolve("GetHandlerProperty2");
	if (getProp2 == NULL)
	{
		getProp = (GetHandlerPropertyFunc)m_7z.resolve("GetHandlerProperty");
		if(!getProp)
		{
			//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to resolve GetHandlerProperty!"));
			return false;
		}
	}

	UInt32 numFormats = 1;
	GetNumberOfFormatsFunc getNumberOfFormats = (GetNumberOfFormatsFunc)m_7z.resolve("GetNumberOfFormats");
	if (getNumberOfFormats != NULL)
	{
		if(getNumberOfFormats(&numFormats) != S_OK)
		{
			//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to enumerate Formats!"));
			return false;
		}
	}
	if (getProp2 == NULL)
		numFormats = 1;

	for(UInt32 i = 0; i < numFormats; i++)
	{
		CArcInfoEx info;

		if(ReadStringProp(getProp, getProp2, i, NArchive::kName, info.Name) != S_OK)
		{
			//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to get Formats %1 Name!").arg(i));
			return false;
		}

		NWindows::NCOM::CPropVariant prop;
		if (ReadProp(getProp, getProp2, i, NArchive::kClassID, prop) != S_OK)
			continue;
		if (prop.vt != VT_BSTR)
			continue;
		info.ClassID = *(const GUID *)prop.bstrVal;
		prop.Clear();

		QString ext, addExt;
		if(ReadStringProp(getProp, getProp2, i, NArchive::kExtension, ext) != S_OK)
		{
			//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to get Formats %1 Property kExtension!").arg(i));
			return false;
		}
		if(ReadStringProp(getProp, getProp2, i, NArchive::kAddExtension, addExt) != S_OK)
		{
			//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to get Formats %1 Property kAddExtension!").arg(i));
			return false;
		}
		info.AddExts(ext, addExt);

		//TRACE(L"Archive Format %S supported %S, %S", QS2CS(info.Name), QS2CS(ext), QS2CS(addExt));

		ReadBoolProp(getProp, getProp2, i, NArchive::kUpdate, info.UpdateEnabled);
		if (info.UpdateEnabled)
			ReadBoolProp(getProp, getProp2, i, NArchive::kKeepName, info.KeepName);

#ifdef _SFX
		if (ReadProp(getProp, getProp2, i, NArchive::kStartSignature, prop) == S_OK)
		{
			if (prop.vt == VT_BSTR)
				info.StartSignature = QByteArray((char*)prop.bstrVal, ::SysStringByteLen(prop.bstrVal));
		}
#endif

		m_Formats.append(info);
	}

	createObjectFunc = (CreateObjectFunc)m_7z.resolve("CreateObject");
	if(createObjectFunc == NULL)
	{
		//LogLine(LOG_ERROR | LOG_DEBUG, QObject::tr("7z: Failed to resolve CreateObjectFunc!"));
		return false;
	}

	//LogLine(LOG_SUCCESS | LOG_DEBUG, QObject::tr("7z: Loaded Successfully"));
	m_Operational = true;
	return true;
}

int CArchiveInterface::FindByExt(const QString &Ext) const
{
	QMutexLocker Locker(&m_Mutex);
	for(int i=0; i<m_Formats.count(); i++)
	{
		if(Ext.compare(m_Formats[i].GetMainExt(), Qt::CaseInsensitive) == 0)
			return i+1;
	}
	return 0;
}

/*QList<int> CArchiveInterface::FindByExt(const QString &Ext)
{
	QMutexLocker Locker(&m_Mutex);
	QList<int> ArcIndex;
	for(int i=0; i<m_Formats.count(); i++)
	{
		if(m_Formats[i].FindExtension(Ext) != -1)
			ArcIndex.append(i+1);
	}
	return ArcIndex;
}*/

QString CArchiveInterface::GetArchiveName(int formatIndex)
{
	QMutexLocker Locker(&m_Mutex);
	if(formatIndex > 0 && formatIndex <= m_Formats.count())
		return m_Formats[formatIndex-1].Name;
	return "";
}

bool CArchiveInterface::CreateInArchive(int formatIndex, CMyComPtr<IInArchive> &archive) const
{
	QMutexLocker Locker(&m_Mutex);
	if(formatIndex > 0 && formatIndex <= m_Formats.count())
		return CreateArchiveHandler(m_Formats[formatIndex-1], (void **)&archive, false);
	return false;
}

bool CArchiveInterface::CreateOutArchive(int formatIndex, CMyComPtr<IOutArchive> &archive) const
{
	QMutexLocker Locker(&m_Mutex);
	if(formatIndex > 0 && formatIndex <= m_Formats.count())
		return CreateArchiveHandler(m_Formats[formatIndex-1], (void **)&archive, true);
	return false;
}

bool CArchiveInterface::CreateArchiveHandler(const CArcInfoEx &ai, void **archive, bool outHandler) const
{
	if(createObjectFunc)
		return createObjectFunc(&ai.ClassID, outHandler ? &IID_IOutArchive : &IID_IInArchive, (void **)archive) == S_OK;
	return false;
}

#endif
