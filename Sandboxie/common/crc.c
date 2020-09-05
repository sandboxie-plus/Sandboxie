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
// Simple Hash Functions
//---------------------------------------------------------------------------


#ifdef CRC_HEADER_ONLY


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif


ULONG CRC_Adler32(const UCHAR *data, int len);
ULONG CRC_Tzuk32(const UCHAR *data, int len);
ULONG64 CRC_AdlerTzuk64(const UCHAR *data, int len);


#ifdef __cplusplus
} // extern "C"
#endif


//---------------------------------------------------------------------------
// Body
//---------------------------------------------------------------------------


#else CRC_HEADER_ONLY


#ifdef CRC_WITH_ADLERTZUK64
#define CRC_WITH_ADLER32
#define CRC_WITH_TZUK32
#endif CRC_WITH_ADLERTZUK64


//---------------------------------------------------------------------------
// CRC_Adler32
//---------------------------------------------------------------------------


#ifdef CRC_WITH_ADLER32


_FX ULONG CRC_Adler32(const UCHAR *data, int len)
{
    const ULONG MOD_ADLER = 65521;
    ULONG a = 1, b = 0;

    while (len) {
        size_t tlen = len > 5550 ? 5550 : len;
        len -= tlen;
        do {
            a += *data++;
            b += a;
        } while (--tlen);
        a = (a & 0xffff) + (a >> 16) * (65536-MOD_ADLER);
        b = (b & 0xffff) + (b >> 16) * (65536-MOD_ADLER);
    }

    if (a >= MOD_ADLER)
        a -= MOD_ADLER;
    b = (b & 0xffff) + (b >> 16) * (65536-MOD_ADLER);
    if (b >= MOD_ADLER)
        b -= MOD_ADLER;

    b = (b << 16) | a;
    return b;
}


#endif CRC_WITH_ADLER32


//---------------------------------------------------------------------------
// CRC_Tzuk32
//---------------------------------------------------------------------------


#ifdef CRC_WITH_TZUK32


#define ROTATE_LEFT(x, n)       (((x) << (n)) | ((x) >> (32-(n))))


_FX ULONG CRC_Tzuk32(const UCHAR *data, int len)
{
    ULONG hash_val = 0x0BADF00D;
    UCHAR *hash_ptr = (UCHAR *)&hash_val;
    UCHAR b, c;
    int j;
    UCHAR sum = 0;

    while (len) {
        c = *data++;
        --len;
        for (j = 0; j < 8; ++j) {
            b = 1 << j;
            if (c & b)
                hash_ptr[j & 3] += ROTATE_LEFT(c, (j & 0x1F));
            else
                hash_ptr[j & 3] += b;
        }
        hash_val = ROTATE_LEFT(hash_val, 1);
        sum += c;
    }

    hash_val = ROTATE_LEFT(hash_val, (sum & 0x1F));
    return hash_val;
}


#undef ROTATE_LEFT


#endif CRC_WITH_TZUK32


//---------------------------------------------------------------------------
// CRC_AdlerTzuk64
//---------------------------------------------------------------------------


#ifdef CRC_WITH_ADLERTZUK64


_FX ULONG64 CRC_AdlerTzuk64(const UCHAR *data, int len)
{
    ULONG a = CRC_Adler32(data, len);
    ULONG b = CRC_Tzuk32(data, len);
    ULONG64 ab = (ULONG64)a;
    ab = (ab << 32) | b;
    return ab;
}


#endif CRC_WITH_ADLERTZUK64



ULONG CRC32(const char *buf, size_t len)
{
	ULONG crc = 0;
	static ULONG table[256];
	static int have_table = 0;
	ULONG rem;
	ULONG octet;
	int i, j;
	const char *p, *q;

	/* This check is not thread safe; there is no mutex. */
	if (have_table == 0) {
		/* Calculate CRC table. */
		for (i = 0; i < 256; i++) {
			rem = i;  /* remainder from polynomial division */
			for (j = 0; j < 8; j++) {
				if (rem & 1) {
					rem >>= 1;
					rem ^= 0xedb88320;
				}
				else
					rem >>= 1;
			}
			table[i] = rem;
		}
		have_table = 1;
	}

	crc = ~crc;
	q = buf + len;
	for (p = buf; p < q; p++) {
		octet = *p;  /* Cast to unsigned octet. */
		crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
	}
	return ~crc;
}

//---------------------------------------------------------------------------
// End
//---------------------------------------------------------------------------


#endif CRC_HEADER_ONLY
