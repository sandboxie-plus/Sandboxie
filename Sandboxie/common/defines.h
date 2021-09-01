/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// Common Defines
//---------------------------------------------------------------------------


#ifndef _COMMON_DEFINES_H
#define _COMMON_DEFINES_H

#include <wchar.h>

//---------------------------------------------------------------------------

#define CDS_UNKNOWNFLAG     0x00000040      // unknown ChangeDisplaySetting flag used by some apps.


#ifndef ALIGNED
#define ALIGNED   // __declspec(align(16))
#endif

#ifndef NOINLINE
#define NOINLINE  __declspec(noinline)
#endif

#ifndef _FX
#define _FX       // ALIGNED NOINLINE
#endif


//---------------------------------------------------------------------------


#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define DYNAMIC_PORT_NAME_CHARS     96  // number of wchars in an Epmapper dynamic endpoint port name
#define DYNAMIC_PORT_ID_CHARS       81


//---------------------------------------------------------------------------


#ifndef SECONDS
#define SECONDS(n64)            (((LONGLONG)n64) * 10000000L)
#endif

#ifndef MINUTES
#define MINUTES(n64)            (SECONDS(n64) * 60)
#endif

#ifndef HOURS
#define HOURS(n64)              (MINUTES(n64) * 60)
#endif

#ifndef DAYS
#define DAYS(n64)               (HOURS(n64) * 24)
#endif


//---------------------------------------------------------------------------
// Inline Wide String Operations
//---------------------------------------------------------------------------


//#define wmemcmp(dst,src,len)    memcmp((dst),(src),(len)*sizeof(WCHAR))
//#define wmemcpy(dst,src,len)    memcpy((dst),(src),(len)*sizeof(WCHAR))
//#define wmemmove(dst,src,len)   memmove((dst),(src),(len)*sizeof(WCHAR))
#define memzero(mem,len)        memset((mem),0,(len))
#define wmemzero(mem,len)       memzero((mem),(len)*sizeof(WCHAR))
//#define wmemchr(mem,c, len)   memchr((mem), (c), (len)*sizeof(WCHAR))


#define SPOOLER_PORT_ID				L"PrintSpooler"
#define WPAD_PORT_ID				L"WPAD"
#define GAME_CONFIG_STORE_PORT_ID	L"GamePort"
#define SMART_CARD_PORT_ID			L"SmartCard"
#define BT_PORT_ID					L"bthserv"
#define SSDP_PORT_ID				L"ssdpsrv"

//---------------------------------------------------------------------------


#endif /* _COMMON_DEFINES_H */
