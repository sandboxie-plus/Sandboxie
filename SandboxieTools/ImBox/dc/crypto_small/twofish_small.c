/*
    *
    * Copyright (c) 2010 
	* Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.com
	* ntldr <ntldr@diskcryptor.net> PGP key ID - 0xC48251EB4F8E4E6E
    *

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef _M_ARM64
#include <stdlib.h>
#include <string.h>
#define __stosb memset
#define __movsb memmove
#else
#include <intrin.h>
#endif
#include "twofish_small.h"

/* the two polynomials */
#define MDS_POLY          0x169
#define RS_POLY           0x14D

/* The 4x4 MDS Linear Transform */
static const unsigned char MDS[4][4] = {
    { 0x01, 0xEF, 0x5B, 0x5B },
    { 0x5B, 0xEF, 0xEF, 0x01 },
    { 0xEF, 0x5B, 0x01, 0xEF },
    { 0xEF, 0x01, 0xEF, 0x5B }
};

/* The 4x8 RS Linear Transform */
static const unsigned char RS[4][8] = {
    { 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E },
    { 0xA4, 0x56, 0x82, 0xF3, 0X1E, 0XC6, 0X68, 0XE5 },
    { 0X02, 0XA1, 0XFC, 0XC1, 0X47, 0XAE, 0X3D, 0X19 },
    { 0XA4, 0X55, 0X87, 0X5A, 0X58, 0XDB, 0X9E, 0X03 }
};

/* sbox usage orderings */
static const unsigned char qord[4][5] = {
   { 1, 1, 0, 0, 1 },
   { 0, 1, 1, 0, 0 },
   { 0, 0, 0, 1, 1 },
   { 1, 0, 1, 1, 0 }
};

/* The Q-box tables */
static const unsigned char qbox[2][4][16] = { 
{
   { 0x8, 0x1, 0x7, 0xD, 0x6, 0xF, 0x3, 0x2, 0x0, 0xB, 0x5, 0x9, 0xE, 0xC, 0xA, 0x4 },
   { 0xE, 0XC, 0XB, 0X8, 0X1, 0X2, 0X3, 0X5, 0XF, 0X4, 0XA, 0X6, 0X7, 0X0, 0X9, 0XD },
   { 0XB, 0XA, 0X5, 0XE, 0X6, 0XD, 0X9, 0X0, 0XC, 0X8, 0XF, 0X3, 0X2, 0X4, 0X7, 0X1 },
   { 0XD, 0X7, 0XF, 0X4, 0X1, 0X2, 0X6, 0XE, 0X9, 0XB, 0X3, 0X0, 0X8, 0X5, 0XC, 0XA }
}, 
{
   { 0X2, 0X8, 0XB, 0XD, 0XF, 0X7, 0X6, 0XE, 0X3, 0X1, 0X9, 0X4, 0X0, 0XA, 0XC, 0X5 },
   { 0X1, 0XE, 0X2, 0XB, 0X4, 0XC, 0X3, 0X7, 0X6, 0XD, 0XA, 0X5, 0XF, 0X9, 0X0, 0X8 },
   { 0X4, 0XC, 0X7, 0X5, 0X1, 0X6, 0X9, 0XA, 0X0, 0XE, 0XD, 0X8, 0X2, 0XB, 0X3, 0XF },
   { 0xB, 0X9, 0X5, 0X1, 0XC, 0X3, 0XD, 0XE, 0X6, 0X4, 0X7, 0XF, 0X2, 0X0, 0X8, 0XA }
}
};

/* computes S_i[x] */
static unsigned long sbox(int i, unsigned long x)
{
	unsigned char a0,b0,a1,b1,a2,b2,a3,b3,a4,b4,y;

	/* a0,b0 = [x/16], x mod 16 */
	a0 = (x>>4)&15;
	b0 = (x)&15;
	/* a1 = a0 ^ b0 */
	a1 = a0 ^ b0;
	/* b1 = a0 ^ ROR(b0, 1) ^ 8a0 */
	b1 = (a0 ^ ((b0<<3)|(b0>>1)) ^ (a0<<3)) & 15;
	/* a2,b2 = t0[a1], t1[b1] */
	a2 = qbox[i][0][a1];
	b2 = qbox[i][1][b1];
	/* a3 = a2 ^ b2 */
	a3 = a2 ^ b2;
	/* b3 = a2 ^ ROR(b2, 1) ^ 8a2 */
	b3 = (a2 ^ ((b2<<3)|(b2>>1)) ^ (a2<<3)) & 15;
	/* a4,b4 = t2[a3], t3[b3] */
	a4 = qbox[i][2][a3];
	b4 = qbox[i][3][b3];
	/* y = 16b4 + a4 */
	y = (b4 << 4) + a4;
	/* return result */
	return y;
}

/* computes ab mod p */
static unsigned long gf_mult(unsigned long a, unsigned long b, unsigned long p)
{
	unsigned long result = 0;

	while (a) {
		if (a & 1) result ^= b;
		a >>= 1, b <<= 1;
		if (b & 0x100) b ^= p;
	}
	return (unsigned char)result;
}

/* computes [y0 y1 y2 y3] = MDS . [x0] */
static unsigned long mds_column_mult(unsigned char in, int col)
{
	return (gf_mult(in, MDS[0][col], MDS_POLY) << 0 ) | (gf_mult(in, MDS[1][col], MDS_POLY) << 8) |
		   (gf_mult(in, MDS[2][col], MDS_POLY) << 16) | (gf_mult(in, MDS[3][col], MDS_POLY) << 24);
}

/* computes h(x) */
static void h_func(const unsigned char *in, unsigned char *out, unsigned char *M, int offset)
{
	int           x, i;
	unsigned char y[4];

	y[0] = (unsigned char)(sbox(1, in[0]) ^ M[4 * (6 + offset) + 0]);
	y[1] = (unsigned char)(sbox(0, in[1]) ^ M[4 * (6 + offset) + 1]);
	y[2] = (unsigned char)(sbox(0, in[2]) ^ M[4 * (6 + offset) + 2]);
	y[3] = (unsigned char)(sbox(1, in[3]) ^ M[4 * (6 + offset) + 3]);
	y[0] = (unsigned char)(sbox(1, y[0]) ^ M[4 * (4 + offset) + 0]);
	y[1] = (unsigned char)(sbox(1, y[1]) ^ M[4 * (4 + offset) + 1]);
	y[2] = (unsigned char)(sbox(0, y[2]) ^ M[4 * (4 + offset) + 2]);
	y[3] = (unsigned char)(sbox(0, y[3]) ^ M[4 * (4 + offset) + 3]); 
	y[0] = (unsigned char)(sbox(1, sbox(0, sbox(0, y[0]) ^ M[4 * (2 + offset) + 0]) ^ M[4 * (0 + offset) + 0]));
	y[1] = (unsigned char)(sbox(0, sbox(0, sbox(1, y[1]) ^ M[4 * (2 + offset) + 1]) ^ M[4 * (0 + offset) + 1]));
	y[2] = (unsigned char)(sbox(1, sbox(1, sbox(0, y[2]) ^ M[4 * (2 + offset) + 2]) ^ M[4 * (0 + offset) + 2]));
	y[3] = (unsigned char)(sbox(0, sbox(1, sbox(1, y[3]) ^ M[4 * (2 + offset) + 3]) ^ M[4 * (0 + offset) + 3]));

	/* Computes [y0 y1 y2 y3] = MDS . [x0 x1 x2 x3] */
	for (x = 0; x < 4; x++) 
	{
		for (out[x] = 0, i = 0; i < 4; i++) {
			out[x] ^= gf_mult(y[i], MDS[x][i], MDS_POLY);
		}
	}
}

static unsigned long g_func(unsigned long x, twofish256_key *key)
{
	return key->S[0][(unsigned char)(x >> 0 )] ^ key->S[1][(unsigned char)(x >> 8 )] ^
		   key->S[2][(unsigned char)(x >> 16)] ^ key->S[3][(unsigned char)(x >> 24)];
}

void twofish256_set_key(const unsigned char *key, twofish256_key *skey)
{
	int g, z, i;
	unsigned char S[4*4];
	int x, y;
	unsigned char tmp[4], tmp2[4], M[8*4];
	unsigned long A, B;

	/* copy the key into M */
	__movsb(M, key, TWOFISH_KEY_SIZE);
	
	/* create the S[..] words */
	for (x = 0; x < 4; x++)
	{
		/* computes [y0 y1 y2 y3] = RS . [x0 x1 x2 x3 x4 x5 x6 x7] */
		for (i = 0; i < 4; i++) 
		{
			for ((S+(x*4))[i] = 0, y = 0; y < 8; y++) {
				(S+(x*4))[i] ^= gf_mult((M+(x*8))[y], RS[i][y], RS_POLY);
			}
		}
	}

	/* make subkeys */
	for (x = 0; x < 20; x++) 
	{
		/* A = h(p * 2x, Me) */
		__stosb(tmp, x+x, 4);
		h_func(tmp, tmp2, M, 0);
		A = ((unsigned long*)tmp2)[0];

		/* B = ROL(h(p * (2x + 1), Mo), 8) */
		memset(tmp, x+x+1, 4);
		h_func(tmp, tmp2, M, 1);
		B = _rotl(((unsigned long*)tmp2)[0], 8);
		
		/* K[2i] = A + B */
		skey->K[x+x] = A + B;

		/* K[2i+1] = (A + 2B) <<< 9 */
		skey->K[x+x+1] = _rotl(B + B + A, 9);
	}

	/* make the sboxes (large ram variant) */
	for (y = 0; y < 4; y++) 
	{
		for (x = 0; x < 256; x++) 
		{
			z = 0;
			/* do unkeyed substitution */
			g = sbox(qord[y][z++], x);
			/* first subkey */
			i = 0;

			/* do key mixing+sbox until z==5 */
			while (z != 5) {
				g = g ^ S[4*i++ + y];
				g = sbox(qord[y][z++], g);
			}
			/* multiply g by a column of the MDS */
			skey->S[y][x] = mds_column_mult((unsigned char)g, y);
		}
	}
}

void twofish256_encrypt(const unsigned char *in, unsigned char *out, twofish256_key *key)
{
	unsigned long t,t1,t2;
	unsigned long v[4];
    int           i;

	for (i = 0; i < 4; i++) {
		v[i] = ((unsigned long*)in)[i] ^ key->K[i];
	}
	for (i = 0; i < 16; i++) 
	{
		t1 = g_func(v[0], key);
        t2 = g_func(_rotl(v[1], 8), key);
        t2 += (t1 += t2);
        t1 += key->K[i+i+8];
        t2 += key->K[i+i+9];
        v[2]  ^= t1; v[2] = _rotr(v[2], 1);
        v[3]  = _rotl(v[3], 1) ^ t2;

		t = v[0]; v[0] = v[2]; v[2] = t;
		t = v[1]; v[1] = v[3]; v[3] = t;
	}
	/* output with "undo last swap" */
	((unsigned long*)out)[0] = v[2] ^ key->K[4];
	((unsigned long*)out)[1] = v[3] ^ key->K[5];
	((unsigned long*)out)[2] = v[0] ^ key->K[6];
	((unsigned long*)out)[3] = v[1] ^ key->K[7];
}

void twofish256_decrypt(const unsigned char *in, unsigned char *out, twofish256_key *key)
{
	unsigned long t,t1,t2;
	unsigned long v[4];
	int           i;

    /* load input and undo undo final swap */
	v[0] = ((unsigned long*)in)[0] ^ key->K[4];
	v[1] = ((unsigned long*)in)[1] ^ key->K[5];
	v[2] = ((unsigned long*)in)[2] ^ key->K[6];
    v[3] = ((unsigned long*)in)[3] ^ key->K[7];

    for (i = 15; i >= 0; i--) 
	{
		t1 = g_func(v[0], key);
        t2 = g_func(_rotl(v[1], 8), key);
        t2 += (t1 += t2);        
		t1 += key->K[i+i+8];        
		t2 += key->K[i+i+9];
        v[2]  = _rotl(v[2], 1) ^ t1;
        v[3]  = v[3] ^ t2; v[3] = _rotr(v[3], 1);
	
		t = v[0]; v[0] = v[2]; v[2] = t;
		t = v[1]; v[1] = v[3]; v[3] = t;
	}
	((unsigned long*)out)[0] = v[2] ^ key->K[0];
	((unsigned long*)out)[1] = v[3] ^ key->K[1];
	((unsigned long*)out)[2] = v[0] ^ key->K[2];
	((unsigned long*)out)[3] = v[1] ^ key->K[3];
}

