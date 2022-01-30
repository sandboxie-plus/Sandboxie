/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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
 // Queue Server -- using PipeServer
 //---------------------------------------------------------------------------


#ifndef _MY_INTERACTIVEWIRE_H
#define _MY_INTERACTIVEWIRE_H


#include "../../common/defines.h"
#include "msgids.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define INTERACTIVE_QUEUE_NAME L"MANPROXY"

#define MAN_FILE_MIGRATION 1
#define MAN_INET_BLOCKADE 2


//---------------------------------------------------------------------------
// FILE_MIGRATION
//---------------------------------------------------------------------------


struct tagMAN_FILE_MIGRATION_REQ
{
	ULONG msgid;
	ULONGLONG file_size;
	WCHAR file_path[256];
};

struct tagMAN_FILE_MIGRATION_RPL
{
	ULONG status;
	ULONG retval;
};

typedef struct tagMAN_FILE_MIGRATION_REQ MAN_FILE_MIGRATION_REQ;
typedef struct tagMAN_FILE_MIGRATION_RPL MAN_FILE_MIGRATION_RPL;

//---------------------------------------------------------------------------
// MAN_INET_BLOCKADE
//---------------------------------------------------------------------------


struct tagMAN_INET_BLOCKADE_REQ
{
	ULONG msgid;
};

struct tagMAN_INET_BLOCKADE_RPL
{
	ULONG status;
	ULONG retval;
};

typedef struct tagMAN_INET_BLOCKADE_REQ MAN_INET_BLOCKADE_REQ;
typedef struct tagMAN_INET_BLOCKADE_RPL MAN_INET_BLOCKADE_RPL;

//---------------------------------------------------------------------------


#endif /* _MY_INTERACTIVEWIRE_H */
