/*
 * Copyright 2026 David Xanatos, xanasoft.com
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

/*
 * Shared wildcard core for file recovery matching.
 * Keeps token semantics (*, ?, **) identical across DLL and UI layers.
 */

#ifndef __SBIE_RECOVERY_WILDCARD_H
#define __SBIE_RECOVERY_WILDCARD_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*SBIE_WILDCARD_CHAR_EQUAL_FN)(wchar_t pattern_char, wchar_t text_char);

static __inline int Sbie_IsPathSeparatorW(wchar_t ch)
{
    return ch == L'\\' || ch == L'/';
}

static __inline int Sbie_WildcardMatchWholeCoreW(
    const wchar_t *pattern, const wchar_t *text,
    size_t text_len, size_t match_start,
    unsigned char *rows, size_t rows_size,
    SBIE_WILDCARD_CHAR_EQUAL_FN char_equal)
{
    size_t index;
    size_t row_size;
    size_t required_size;
    unsigned char *previous;
    unsigned char *current;
    unsigned char *swap;
    wchar_t token;

    if (! pattern || ! text || ! rows || ! char_equal)
        return 0;

    if (text_len > ((((size_t)-1) / 2) - 1))
        return 0;

    row_size = text_len + 1;
    required_size = row_size * 2;
    if (rows_size < required_size)
        return 0;

    if (match_start > text_len)
        return 0;

    previous = rows;
    current = rows + row_size;

    memset(rows, 0, required_size);
    previous[match_start] = 1;
    for (index = match_start + 1; index <= text_len; ++index) {
        if (Sbie_IsPathSeparatorW(text[index - 1]))
            previous[index] = 1;
    }

    while (*pattern) {
        memset(current, 0, row_size);
        token = *pattern++;

        if (token == L'*' && *pattern == L'*') {
            ++pattern;
            for (index = 1; index <= text_len; ++index) {
                if (! Sbie_IsPathSeparatorW(text[index - 1])) {
                    if (previous[index - 1])
                        current[index] = 1;
                    else
                        current[index] = current[index - 1];
                }
            }
            for (index = 1; index < text_len; ++index) {
                if (! Sbie_IsPathSeparatorW(text[index]))
                    current[index] = 0;
            }

        } else if (token == L'*') {
            current[0] = previous[0];
            for (index = 1; index <= text_len; ++index)
                current[index] = previous[index] || current[index - 1];

        } else {
            for (index = 1; index <= text_len; ++index) {
                if ((token == L'?' &&
                        ! Sbie_IsPathSeparatorW(text[index - 1])) ||
                        char_equal(token, text[index - 1]))
                    current[index] = previous[index - 1];
            }
        }

        swap = previous;
        previous = current;
        current = swap;
    }

    return previous[text_len] != 0;
}

#ifdef __cplusplus
}
#endif

#endif // __SBIE_RECOVERY_WILDCARD_H
