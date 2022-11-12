// Common/MyInitGuid.h

#ifndef __COMMON_MY_INITGUID_H
#define __COMMON_MY_INITGUID_H

#ifdef _WIN32
#ifdef UNDER_CE
#include <basetyps.h>
#endif
#include <initguid.h>
#ifdef UNDER_CE
DEFINE_GUID(IID_IUnknown,
0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif
#else
#define INITGUID
#include "MyGuidDef.h"
DEFINE_GUID(IID_IUnknown,
0x00000000, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif

#endif
