/*
 * Copyright 2021 David Xanatos, xanasoft.com
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

#ifndef STR_UTIL_H
#define STR_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

const WCHAR* SbieDll_GetTagValue(const WCHAR* str, const WCHAR* end, const WCHAR** value, ULONG* len, WCHAR sep);
typedef BOOLEAN (*SbieDll_TagEnumProc)(const WCHAR* name, ULONG name_len, const WCHAR* value, ULONG value_len, void* param);
VOID SbieDll_EnumTagValues(const WCHAR* string, SbieDll_TagEnumProc enumProc, void* param, WCHAR eq, WCHAR sep);
BOOLEAN SbieDll_FindTagValue(const WCHAR* string, const WCHAR* name, WCHAR* value, ULONG value_size, WCHAR eq, WCHAR sep);
BOOLEAN SbieDll_FindTagValuePtr(const WCHAR* string, const WCHAR* name, const WCHAR** value, ULONG* value_len, WCHAR eq, WCHAR sep);

#ifdef __cplusplus
}
#endif

#endif
