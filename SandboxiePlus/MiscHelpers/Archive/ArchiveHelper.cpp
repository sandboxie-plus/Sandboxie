#include "stdafx.h"

#ifndef WIN32
#define INITGUID
#endif

#include "ArchiveHelper.h"

#ifdef USE_7Z

HRESULT GetCoderClass(GetMethodPropertyFunc getMethodProperty, UInt32 index, PROPID propId, CLSID &clsId, bool &isAssigned)
{
	NWindows::NCOM::CPropVariant prop;
	isAssigned = false;
	RINOK(getMethodProperty(index, propId, &prop));
	if (prop.vt == VT_BSTR)
	{
		isAssigned = true;
		clsId = *(const GUID *)prop.bstrVal;
	}
	else if (prop.vt != VT_EMPTY)
		return E_FAIL;
	return S_OK;
}

HRESULT ReadProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, NWindows::NCOM::CPropVariant &prop)
{
	if (getProp2)
		return getProp2(index, propID, &prop);;
	return getProp(propID, &prop);
}

HRESULT ReadStringProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, QString &res)
{
	NWindows::NCOM::CPropVariant prop;
	RINOK(ReadProp(getProp, getProp2, index, propID, prop));
	if (prop.vt == VT_BSTR)
		res = QString::fromStdWString(prop.bstrVal ? prop.bstrVal : L"");
	else if (prop.vt != VT_EMPTY)
		return E_FAIL;
	return S_OK;
}

HRESULT ReadBoolProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, bool &res)
{
	NWindows::NCOM::CPropVariant prop;
	RINOK(ReadProp(getProp, getProp2, index, propID, prop));
	if (prop.vt == VT_BOOL)
		res = VARIANT_BOOLToBool(prop.boolVal);
	else if (prop.vt != VT_EMPTY)
		return E_FAIL;
	return S_OK;
}

HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
	NWindows::NCOM::CPropVariant prop;
	RINOK(archive->GetProperty(index, propID, &prop));
	if (prop.vt == VT_BOOL)
		result = VARIANT_BOOLToBool(prop.boolVal);
	else if (prop.vt == VT_EMPTY)
		result = false;
	else
		return E_FAIL;
	return S_OK;
}

struct SPropertyNames{
	SPropertyNames();
	QMap<PROPID,QString> Map;
} SPropertyNames;


SPropertyNames::SPropertyNames()
{
	Map.insert(kpidPath, "Path");
	Map.insert(kpidName, "Name");
	Map.insert(kpidExtension, "Extension");
	Map.insert(kpidIsDir, "IsDir");
	Map.insert(kpidSize, "Size");
	Map.insert(kpidPackSize, "PackSize");
	Map.insert(kpidAttrib, "Attrib");
	Map.insert(kpidCTime, "CTime");
	Map.insert(kpidATime, "ATime");
	Map.insert(kpidMTime, "MTime");
	Map.insert(kpidSolid, "Solid");
	Map.insert(kpidCommented, "Commented");
	Map.insert(kpidEncrypted, "Encrypted");
	Map.insert(kpidSplitBefore, "SplitBefore");
	Map.insert(kpidSplitAfter, "SplitAfter");
	Map.insert(kpidDictionarySize, "DictionarySize");
	Map.insert(kpidCRC, "CRC");
	Map.insert(kpidType, "Type");
	Map.insert(kpidIsAnti, "IsAnti");
	Map.insert(kpidMethod, "Method");
	Map.insert(kpidHostOS, "HostOS");
	Map.insert(kpidFileSystem, "FileSystem");
	Map.insert(kpidUser, "User");
	Map.insert(kpidGroup, "Group");
	Map.insert(kpidBlock, "Block");
	Map.insert(kpidComment, "Comment");
	Map.insert(kpidPosition, "Position");
	Map.insert(kpidPrefix, "Prefix");
	Map.insert(kpidNumSubDirs, "NumSubDirs");
	Map.insert(kpidNumSubFiles, "NumSubFiles");
	Map.insert(kpidUnpackVer, "UnpackVer");
	Map.insert(kpidVolume, "Volume");
	Map.insert(kpidIsVolume, "IsVolume");
	Map.insert(kpidOffset, "Offset");
	Map.insert(kpidLinks, "Links");
	Map.insert(kpidNumBlocks, "NumBlocks");
	Map.insert(kpidNumVolumes, "NumVolumes");

	Map.insert(kpidBit64, "Bit64");
	Map.insert(kpidBigEndian, "BigEndian");
	Map.insert(kpidCpu, "Cpu");
	Map.insert(kpidPhySize, "PhySize");
	Map.insert(kpidHeadersSize, "HeadersSize");
	Map.insert(kpidChecksum, "Checksum");
	Map.insert(kpidCharacts, "Characters");
	Map.insert(kpidVa, "Va");
	Map.insert(kpidId, "Id");
	Map.insert(kpidShortName, "ShortName");
	Map.insert(kpidCreatorApp, "CreatorApp");
	Map.insert(kpidSectorSize, "SectorSize");
	Map.insert(kpidPosixAttrib, "PosixAttrib");
	Map.insert(kpidLink, "Link");
	Map.insert(kpidError, "Error");
	 
	Map.insert(kpidTotalSize, "TotalSize");
	Map.insert(kpidFreeSpace, "FreeSpace");
	Map.insert(kpidClusterSize, "ClusterSize");
	Map.insert(kpidVolumeName, "VolumeName");

	Map.insert(kpidLocalName, "LocalName");
	Map.insert(kpidProvider, "Provider");
};

QString GetPropertyName(PROPID PropID)
{
	return SPropertyNames.Map.value(PropID, "unknown");
}


#if 1
#include "./7z/CPP/Windows/PropVariant.cpp"
#else
#ifndef WIN32
#include "../../7-Zip/p7zip_9.20.1/CPP/Windows/PropVariant.cpp"
#include "../../7-Zip/p7zip_9.20.1/CPP/Common/MyWindows.cpp"
#else
#include "../../7-Zip/7z/CPP/Windows/PropVariant.cpp"
#endif
#endif

#endif
