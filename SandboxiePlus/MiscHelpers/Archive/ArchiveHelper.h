#pragma once

#include "Archive.h"

#ifdef USE_7Z

#if 1
#include "./7z/CPP/Common/MyInitGuid.h"
#include "./7z/CPP/Common/MyCom.h"
#include "./7z/CPP/7zip/Archive/IArchive.h"
#include "./7z/CPP/7zip/IPassword.h"
#include "./7z/CPP/7zip/IProgress.h"
#include "./7z/CPP/Windows/PropVariant.h"
#include "./7z/CPP/Common/Defs.h"
#include "./7z/CPP/Windows/Defs.h"
#include "./7z/CPP/7zip/ICoder.h"
#else
#ifndef WIN32
#include "../../7-Zip/p7zip_9.20.1/CPP/myWindows/StdAfx.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/Common/MyCom.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/Common/MyGuidDef.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/7zip/Archive/IArchive.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/7zip/IPassword.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/7zip/IProgress.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/Windows/PropVariant.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/Common/Defs.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/Windows/Defs.h"
#include "../../7-Zip/p7zip_9.20.1/CPP/7zip/ICoder.h"
#else
#include "../../7-Zip/7z920/CPP/Common/MyInitGuid.h"
#include "../../7-Zip/7z920/CPP/Common/MyCom.h"
#include "../../7-Zip/7z920/CPP/7zip/Archive/IArchive.h"
#include "../../7-Zip/7z920/CPP/7zip/IPassword.h"
#include "../../7-Zip/7z920/CPP/7zip/IProgress.h"
#include "../../7-Zip/7z920/CPP/Windows/PropVariant.h"
#include "../../7-Zip/7z920/CPP/Common/Defs.h"
#include "../../7-Zip/7z920/CPP/Windows/Defs.h"
#include "../../7-Zip/7z920/CPP/7zip/ICoder.h"
#endif
#endif

#include <QLibrary>

#include "ArchiveIO.h"

typedef UInt32 (WINAPI *GetMethodPropertyFunc)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetNumberOfMethodsFunc)(UInt32 *numMethods);

typedef UInt32 (WINAPI *GetHandlerPropertyFunc)(PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetHandlerPropertyFunc2)(UInt32 index, PROPID propID, PROPVARIANT *value);
typedef UInt32 (WINAPI *GetNumberOfFormatsFunc)(UInt32 *numFormats);

typedef UInt32 (WINAPI *CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);


HRESULT GetCoderClass(GetMethodPropertyFunc getMethodProperty, UInt32 index, PROPID propId, CLSID &clsId, bool &isAssigned);
HRESULT ReadProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, NWindows::NCOM::CPropVariant &prop);
HRESULT ReadStringProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, QString &res);
HRESULT ReadBoolProp(GetHandlerPropertyFunc getProp, GetHandlerPropertyFunc2 getProp2, UInt32 index, PROPID propID, bool &res);
HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result);

QString GetPropertyName(PROPID PropID);

/*
DEFINE_GUID(CLSID_7z,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);	// 7z - 7-Zip Archive
DEFINE_GUID(CLSID_rar,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);	// rar r00 - WinRar Archive
DEFINE_GUID(CLSID_cab,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);	// cab - Cabinet File
DEFINE_GUID(CLSID_tar,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00);	// tar - Tar Archive
DEFINE_GUID(CLSID_zip,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);	// zip jar xpi odt ods docx xlsx - Zip Archive
DEFINE_GUID(CLSID_hfs,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE3, 0x00, 0x00);	// hfs - HFS Image
DEFINE_GUID(CLSID_iso,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE7, 0x00, 0x00);	// iso img - ISO Image
DEFINE_GUID(CLSID_udf,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00);	// iso img - UDF Image
DEFINE_GUID(CLSID_wim,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE6, 0x00, 0x00);	// wim swm - Windows Image
DEFINE_GUID(CLSID_chm,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE9, 0x00, 0x00);	// chm chi chq chw hxs hxi hxr hxq hxw lit - Compressed HTML Help
DEFINE_GUID(CLSID_msi,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE5, 0x00, 0x00);	// msi msp doc xls ppt - Microsoft Installer
DEFINE_GUID(CLSID_nsis,	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x09, 0x00, 0x00);	// - Nullsoft Scriptable Install System
*/

#ifndef TRACE
#define TRACE
#endif

#define QS2CS(x) (x.toLatin1().data())

#endif
