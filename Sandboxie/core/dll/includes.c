/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Includes
//---------------------------------------------------------------------------

#include "dll.h"

#include <windows.h>
#include "common/win32_ntddk.h"

/* List */

#include "common/list.c"

/* Pool */

extern const ULONG tzuk;
#define POOL_TAG tzuk
#include "common/pool.c"

/* Map */

#include "common/map.c"

/* Stream */

#include "common/stream.c"

/* NetFw */

#include "common/netfw.c"

/* StrUtil */

#include "common/str_util.c"