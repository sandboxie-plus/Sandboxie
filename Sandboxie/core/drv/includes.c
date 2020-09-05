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

#include "my_winnt.h"

#define KERNEL_MODE
#undef  DEBUG_MEMORY

/* List */

#include "common/list.c"

/* Pool */

extern const ULONG tzuk;
#define POOL_TAG tzuk
#ifdef DEBUG_MEMORY
#define Pool_Alloc Original_Pool_Alloc
#define Pool_Free  Original_Pool_Free
#endif
#include "common/pool.c"
#ifdef DEBUG_MEMORY
#undef  Pool_Alloc
#undef  Pool_Free
#include "dbgmem.c"
#endif

/* Stream */

#include "common/stream.c"

/* Pattern */

#include "mem.h"
#define Pool_Alloc(pool,size) Mem_Alloc((pool),(size))
#include "common/pattern.c"
#undef  Pool_Alloc

/* CRC */

//#define CRC_WITH_ADLERTZUK64
//#include "common/crc.c"

/* BigNum */

#define BIGNUM_WITH_FROM_STRING
#include "common/bignum.c"
