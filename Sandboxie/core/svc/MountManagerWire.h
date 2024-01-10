/*
 * Copyright 2023 David Xanatos, xanasoft.com
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
// Mount Manager Server -- using PipeServer
//---------------------------------------------------------------------------


#ifndef _MY_MOUNTMANAGERWIRE_H
#define _MY_MOUNTMANAGERWIRE_H


//---------------------------------------------------------------------------
// Mount Manager - Create encrypted box image
//---------------------------------------------------------------------------


struct tagIMBOX_CREATE_REQ
{
	MSG_HEADER h;
	ULONG64 image_size;
	WCHAR password[128 + 1];
	WCHAR file_root[1];
};

struct tagIMBOX_CREATE_RPL
{
	MSG_HEADER h;
};

typedef struct tagIMBOX_CREATE_REQ    IMBOX_CREATE_REQ;
typedef struct tagIMBOX_CREATE_RPL    IMBOX_CREATE_RPL;


//---------------------------------------------------------------------------
// Mount Manager - Mount box root
//---------------------------------------------------------------------------


struct tagIMBOX_MOUNT_REQ
{
	MSG_HEADER h;
	WCHAR password[128 + 1];
	BOOL protect_root;
	BOOL auto_unmount;
	WCHAR reg_root[MAX_REG_ROOT_LEN];
	WCHAR file_root[1];
};

struct tagIMBOX_MOUNT_RPL
{
	MSG_HEADER h;
};

typedef struct tagIMBOX_MOUNT_REQ    IMBOX_MOUNT_REQ;
typedef struct tagIMBOX_MOUNT_RPL    IMBOX_MOUNT_RPL;


//---------------------------------------------------------------------------
// Mount Manager - Unmount box root
//---------------------------------------------------------------------------


struct tagIMBOX_UNMOUNT_REQ
{
	MSG_HEADER h;
	WCHAR reg_root[MAX_REG_ROOT_LEN];
};

struct tagIMBOX_UNMOUNT_RPL
{
	MSG_HEADER h;
};

typedef struct tagIMBOX_UNMOUNT_REQ    IMBOX_UNMOUNT_REQ;
typedef struct tagIMBOX_UNMOUNT_RPL    IMBOX_UNMOUNT_RPL;


//---------------------------------------------------------------------------
// Mount Manager - Enum box roots
//---------------------------------------------------------------------------


struct tagIMBOX_ENUM_REQ
{
	MSG_HEADER h;
};

struct tagIMBOX_ENUM_RPL
{
	MSG_HEADER h;
	WCHAR reg_roots[1];
};

typedef struct tagIMBOX_ENUM_REQ    IMBOX_ENUM_REQ;
typedef struct tagIMBOX_ENUM_RPL    IMBOX_ENUM_RPL;


//---------------------------------------------------------------------------
// Mount Manager - Query box root
//---------------------------------------------------------------------------


struct tagIMBOX_QUERY_REQ
{
	MSG_HEADER h;
	WCHAR reg_root[MAX_REG_ROOT_LEN];
};

struct tagIMBOX_QUERY_RPL
{
	MSG_HEADER h;
	//WCHAR boxname[BOXNAME_COUNT];
	ULONG64 disk_size;
	ULONG64 used_size;
	WCHAR disk_root[1];
};

typedef struct tagIMBOX_QUERY_REQ    IMBOX_QUERY_REQ;
typedef struct tagIMBOX_QUERY_RPL    IMBOX_QUERY_RPL;


//---------------------------------------------------------------------------
// Mount Manager - Update box image properties
//---------------------------------------------------------------------------


struct tagIMBOX_UPDATE_REQ
{
	MSG_HEADER h;
	WCHAR password[128 + 1];
	WCHAR new_password[128 + 1];
	ULONG64 new_image_size;
	WCHAR file_root[1];
};

struct tagIMBOX_UPDATE_RPL
{
	MSG_HEADER h;
};

typedef struct tagIMBOX_UPDATE_REQ    IMBOX_UPDATE_REQ;
typedef struct tagIMBOX_UPDATE_RPL    IMBOX_UPDATE_RPL;


#endif  // _MY_MOUNTMANAGERWIRE_H
