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
// Pattern Matching
//---------------------------------------------------------------------------


#ifndef __MY_PATTERN_H
#define __MY_PATTERN_H


#include "common/pool.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _PATTERN PATTERN;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


//
// Pattern_Create:  creates a PATTERN object allocated from pool 'pool',
// based on the string 'string'.  If 'lower' is TRUE, the pattern is compiled
// based on a lowercase version of the source 'string'.
// If 'string' does not contain wildcard star characters (*) then a
// short-circuit pattern is created, where Pattern_Match() becomes wcscmp().
// The PATTERN object, although opaque, is guaranteed to begin with an unused
// LIST_ELEM member (see list.h), so it can be inserted into a list.
//

PATTERN *Pattern_Create(POOL *pool, const WCHAR *string, BOOLEAN lower, ULONG level);

//
// Pattern_Free:  free the PATTERN object 'pattern'
//

void Pattern_Free(PATTERN *pat);

//
// Pattern_Source:  returns the string used to create 'pattern'.
//

const WCHAR *Pattern_Source(PATTERN *pat);

//
// Pattern_Level:  returns the match level with the process.
//

ULONG Pattern_Level(PATTERN *pat);

//
// Pattern_Wildcards:  returns count of wildcards in the pattern, not counting the tailing * when rpresent
//

USHORT Pattern_Wildcards(PATTERN *pat);

//
// Pattern_Exact:  returns true if the pattern has no wildcard at the end
//

BOOLEAN Pattern_Exact(PATTERN *pat);

//
// Pattern_Match:  returns TRUE if 'string' matches PATTERN object 'pattern'.
// If 'lower' was TRUE at time of Pattern_Create(), the 'string' passed here
// should be all lowercase characters.
//

BOOLEAN Pattern_Match(PATTERN *pat, const WCHAR *string, int string_len);
int Pattern_MatchX(PATTERN *pat, const WCHAR *string, int string_len);

//
// Pattern_MatchPathList:  match a specific path
//

int Pattern_MatchPathList(
    WCHAR* path_lwr, ULONG path_len, LIST* list, ULONG* plevel, BOOLEAN* pexact, USHORT* pwildc, const WCHAR** patsrc);
BOOLEAN Pattern_MatchPathListEx(
    WCHAR* path_lwr, ULONG path_len, LIST* list, ULONG* plevel, int* pmatch_len, BOOLEAN* pexact, USHORT* pwildc, const WCHAR** patsrc);

//---------------------------------------------------------------------------


#endif // __MY_PATTERN_H
