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


#include "common/pattern.h"
#include "common/defines.h"
#include "common/list.h"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct _PATTERN {

    // unused list_elem can be used by caller
    LIST_ELEM list_elem;

    // length of the entire PATTERN object
    ULONG length;

    // pattern info
    union {
        ULONG v;
        USHORT num_cons;        // number of constant parts
        struct {
            int unused : 16;
            int star_missing : 1;
            int star_at_head : 1;
            int star_at_tail : 1;
            int have_a_qmark : 1;
        } f;
    } info;

    // pointer to source pattern string, allocated as part of
    // this PATTERN object
    WCHAR *source;

    // a value denoting the match level for the process
    ULONG level;

    // array of pointers to constant parts.  the actual number of
    // elements is indicate by info.num_cons, and the strings are
    // allocated as part of this PATTERN object
    struct {
        BOOLEAN hex;
        USHORT len;
        WCHAR *ptr;
    } cons[0];

};


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static int Pattern_Match2(
    PATTERN *pat,
    const WCHAR *string, int string_len,
    int str_index, int con_index);

static int Pattern_Match3(
    PATTERN *pat,
    const WCHAR *string, int string_len,
    int str_index, int con_index);

#ifdef KERNEL_MODE

static int Pattern_wcstol(const WCHAR *text, WCHAR **endptr);

#else

//#include <wchar.h>
_Check_return_ _CRTIMP long   __cdecl wcstol(_In_z_ const wchar_t *_Str, _Out_opt_ _Deref_post_z_ wchar_t ** _EndPtr, int _Radix);

#define Pattern_wcstol(text,endptr) wcstol(text,endptr,10)

#endif KERNEL_MODE

static const WCHAR *Pattern_wcsnstr(
    const WCHAR *hstr, const WCHAR *nstr, int nlen);


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR *Pattern_Hex = L"__hex";


//---------------------------------------------------------------------------
// Pattern_Create
//---------------------------------------------------------------------------


_FX PATTERN *Pattern_Create(
    POOL *pool, const WCHAR *string, BOOLEAN lower, ULONG level)
{
    ULONG num_cons;
    const WCHAR *iptr;
    const WCHAR *iptr2;
    ULONG len_ptr;
    ULONG len_pat;
    PATTERN *pat;
    WCHAR *optr;
    BOOLEAN any_hex_cons;

    //
    // count number of constant parts in the input string, and
    // also number of bytes used by these parts
    //

    num_cons = 0;
    len_pat = sizeof(PATTERN);
    if (string)
        len_pat += wcslen(string) * sizeof(WCHAR);
    len_pat += sizeof(WCHAR);

    iptr = string;
    while (iptr) {

        while (*iptr == L'*')
            ++iptr;
        iptr2 = wcschr(iptr, L'*');

        if (iptr2) {
            len_ptr = (ULONG)(iptr2 - iptr);
            ++iptr2;
        } else
            len_ptr = wcslen(iptr);

        if (len_ptr) {
            len_pat += (len_ptr + 1) * sizeof(WCHAR)
                        // plus one entry in cons array:
                    +  sizeof(((PATTERN *)NULL)->cons[0]);
            ++num_cons;
        }

        iptr = iptr2;
    }

    //
    // allocate the PATTERN object with the following length:
    // - the size of the PATTERN structure
    // - length of source string, in bytes, including NULL character
    // - length of constant parts, in bytes, each including a NULL char
    // - number of constant parts * sizeof(WCHAR *), for pointer array
    // the length was already computed above.
    //

    pat = (PATTERN*)Pool_Alloc(pool, len_pat);
    if (! pat)
        return NULL;

    memzero(&pat->list_elem, sizeof(LIST_ELEM));
    pat->length = len_pat;

    //
    // copy constant parts into pattern.  we copy the partial strings
    // beginning just after the cons array in PATTERN, and point the
    // elements of that array to each copied string
    //

    any_hex_cons = FALSE;

    optr = (WCHAR *)&pat->cons[num_cons];
    num_cons = 0;

    iptr = string;
    while (iptr) {

        while (*iptr == L'*')
            ++iptr;
        iptr2 = wcschr(iptr, L'*');

        if (iptr2) {
            len_ptr = (ULONG)(iptr2 - iptr);
            ++iptr2;
        } else
            len_ptr = wcslen(iptr);

        if (len_ptr) {

            // put the char count of the constant part before the data

            pat->cons[num_cons].len = (USHORT)len_ptr;
            pat->cons[num_cons].ptr = optr;

            wmemcpy(optr, iptr, len_ptr);
            optr[len_ptr] = L'\0';
            if (lower)
                _wcslwr(optr);

            if (Pattern_wcsnstr(optr, Pattern_Hex, 5)) {
                any_hex_cons = TRUE;
                pat->cons[num_cons].hex = TRUE;
            } else
                pat->cons[num_cons].hex = FALSE;

            ++num_cons;
            optr += len_ptr + 1;
        }

        iptr = iptr2;
    }

    //
    // place source string in the pattern, past all the constant parts,
    // and initialize info.
    //

    if (string)
        wcscpy(optr, string);
    else
        *optr = L'\0';
    pat->source = optr;

    pat->level = level;

    pat->info.v = 0;
    pat->info.num_cons = (USHORT)num_cons;

    if (string && string[0] == L'*')
        pat->info.f.star_at_head = TRUE;
    if (string && string[wcslen(string) - 1] == L'*')
        pat->info.f.star_at_tail = TRUE;

    if (num_cons <= 1 &&
        (! pat->info.f.star_at_head) &&
        (! pat->info.f.star_at_tail) &&
        (! any_hex_cons))
    {
        pat->info.f.star_missing = TRUE;

        if (wcschr(string, L'?'))
            pat->info.f.have_a_qmark = TRUE;
    }

    //
    // we're done
    //

    return pat;
}


//---------------------------------------------------------------------------
// Pattern_Free
//---------------------------------------------------------------------------


_FX void Pattern_Free(PATTERN *pat)
{
    Pool_Free(pat, pat->length);
}


//---------------------------------------------------------------------------
// Pattern_Source
//---------------------------------------------------------------------------


_FX const WCHAR *Pattern_Source(PATTERN *pat)
{
    return pat->source;
}


//---------------------------------------------------------------------------
// Pattern_Level
//---------------------------------------------------------------------------


_FX ULONG Pattern_Level(PATTERN *pat)
{
    return pat->level;
}


//---------------------------------------------------------------------------
// Pattern_Match
//---------------------------------------------------------------------------


_FX BOOLEAN Pattern_Match(
    PATTERN *pat, const WCHAR *string, int string_len)
{
    if (Pattern_MatchX(pat, string, string_len) != 0)
        return TRUE;
    return FALSE;
}


//---------------------------------------------------------------------------
// Pattern_MatchX
//---------------------------------------------------------------------------


_FX int Pattern_MatchX(
    PATTERN *pat, const WCHAR *string, int string_len)
{
    //
    // short-circuits:  if string is NULL, or if the pattern is NULL,
    // return FALSE.  if the pattern has no wildcard stars, use simple
    // string comparison
    //

    if (! string)
        return 0;

    if (pat->info.f.star_missing) {

        if (pat->info.num_cons == 0)
            return 0;
        if (string_len != pat->cons[0].len)
            return 0;

        if (pat->info.f.have_a_qmark) {

            const WCHAR *x = Pattern_wcsnstr(
                            string, pat->cons[0].ptr, pat->cons[0].len);
            if (x != string)
                return 0;

        } else {

            ULONG x = wmemcmp(string, pat->cons[0].ptr, pat->cons[0].len);
            if (x != 0)
                return 0;
        }

        return string_len;
    }

    //
    // otherwise stars were included and the string is valid
    //

    return Pattern_Match2(pat, string, string_len, 0, 0);
}


//---------------------------------------------------------------------------
// Pattern_Match2
//---------------------------------------------------------------------------


_FX int Pattern_Match2(
    PATTERN *pat,
    const WCHAR *string, int string_len,
    int str_index, int con_index)
{
    int match;

    if (con_index < pat->info.num_cons) {

        //
        // recursively try to find a match for the constant parts
        //

        while (1) {

            const WCHAR *ptr = Pattern_wcsnstr(
                string + str_index,
                pat->cons[con_index].ptr, pat->cons[con_index].len);

            if (! ptr) {

                if (pat->cons[con_index].hex) {
                    match = Pattern_Match3(
                        pat, string, string_len, str_index, con_index);
                } else
                    match = 0;
                break;
            }

            if (str_index == 0 && ptr > string &&
                    (! pat->info.f.star_at_head)) {
                match = 0;
                break;
            }

            str_index = (ULONG)(ptr - string) + pat->cons[con_index].len;
            match = Pattern_Match2(
                    pat, string, string_len, str_index, con_index + 1);
            if (match)
                break;
        }

    } else {

        //
        // if we think we have a match, just make sure there aren't
        // any trailing characters that break the pattern
        //

        if (str_index != string_len && (! pat->info.f.star_at_tail))
            match = 0;
        else
            match = str_index + 1;
    }

    return match;
}


//---------------------------------------------------------------------------
// Pattern_Match3
//---------------------------------------------------------------------------


_FX int Pattern_Match3(
    PATTERN *pat,
    const WCHAR *string, int string_len,
    int str_index, int con_index)
{
    const WCHAR *srcptr, *conptr, *seqptr, *tmpptr;
    WCHAR *endptr;
    int con_len, min_len, max_len, count;

    //
    // find the __hex__ part within the constant part,
    // skip any leading characters while making sure they match
    //

    srcptr = string + str_index;

    conptr = pat->cons[con_index].ptr;
    seqptr = Pattern_wcsnstr(conptr, Pattern_Hex, 5);
    if (! seqptr)
        return 0;

restart1:

    con_len = (int)(ULONG_PTR)(seqptr - conptr);

    if (con_len) {
        if (string_len - str_index < con_len)
            return 0;
        if (Pattern_wcsnstr(srcptr, conptr, con_len) != srcptr)
            return 0;
        srcptr += con_len;
    }

    //
    // get minimum and maximum lengths of hex sequence, if specified
    //

    seqptr += 5;        // length of Pattern_Hex

    min_len = Pattern_wcstol(seqptr, &endptr);
    if (*endptr && min_len >= 1 && min_len <= 255) {
        seqptr = endptr;
        max_len = Pattern_wcstol(seqptr + 1, &endptr);
        if (*endptr && max_len >= min_len && max_len <= 255)
            seqptr = endptr;
        else
            max_len = min_len;
    } else {
        min_len = 1;
        max_len = 255;
    }

    if (*seqptr != L'_')
        return 0;
    ++seqptr;
    if (*seqptr != L'_')
        return 0;
    ++seqptr;

    //
    // expect between min_len and max_len hex digits
    //

restart2:

    count = 0;
    while (1) {
        if ((*srcptr >= L'a' && *srcptr <= L'f') ||
            (*srcptr >= L'0' && *srcptr <= L'9')) {
            ++count;
            ++srcptr;
            if (count == max_len)
                break;
        } else
            break;
    }

    if (count < min_len || count > max_len) {

        //
        // we did not match the hex string at this string index,
        // but the constant part follows an asterisk, so check if
        // we have a match further in the string
        //

        if (con_index != 0 || pat->info.f.star_at_head) {
            srcptr = Pattern_wcsnstr(srcptr, conptr, con_len);
            if (srcptr) {
                srcptr += con_len;
                goto restart2;
            }
        }

        return 0;
    }

    //
    // if the constant part has another __hex__ part then restart the
    // process, which will also match all the characters between the
    // end of the last __hex__ and the beginning of the next __hex__
    //

    tmpptr = Pattern_wcsnstr(seqptr, Pattern_Hex, 5);
    if (tmpptr) {
        conptr = seqptr;
        seqptr = tmpptr;
        goto restart1;
    }

    //
    // otherwise match the rest of this constant part
    // and then resume normal processing
    //

    con_len = wcslen(seqptr);

    if (con_len) {
        if (Pattern_wcsnstr(srcptr, seqptr, con_len) != srcptr)
            return 0;
    }

    str_index = (int)(ULONG_PTR)(srcptr + con_len - string);
    return Pattern_Match2(pat, string, string_len, str_index, con_index + 1);
}


//---------------------------------------------------------------------------
// Pattern_wcstol
//---------------------------------------------------------------------------


#ifdef KERNEL_MODE

_FX int Pattern_wcstol(const WCHAR *text, WCHAR **endptr)
{
    int result = 0;
    while (*text >= L'0' && *text <= L'9') {
        result = (result * 10) + (*text - L'0');
        ++text;
    }
    *endptr = (WCHAR *)text;
    return result;
}

#endif KERNEL_MODE


//---------------------------------------------------------------------------
// Pattern_wcsnstr
//---------------------------------------------------------------------------


_FX const WCHAR *Pattern_wcsnstr(
    const WCHAR *hstr, const WCHAR *nstr, int nlen)
{
    int i;
    while (*hstr) {
        if (*hstr == *nstr || *nstr == L'?') {
            for (i = 0; i < nlen; ++i) {
                if ((hstr[i] != nstr[i]) &&
                        (hstr[i] == L'\0' || nstr[i] != L'?'))
                    break;
            }
            if (i == nlen)
                return hstr;
        }
        ++hstr;
    }
    return NULL;
}
