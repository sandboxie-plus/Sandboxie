/*
 * Copyright 2024 David Xanatos, xanasoft.com
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
// Simple INSECURE Encryption Functions
//---------------------------------------------------------------------------


#ifdef RC4_HEADER_ONLY


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

void rc4_crypt(const unsigned char *key_ptr, unsigned int key_len, unsigned int stream_pos, unsigned char *buffer_ptr, unsigned int buffer_len);

#ifdef __cplusplus
} // extern "C"
#endif


//---------------------------------------------------------------------------
// Body
//---------------------------------------------------------------------------


#else RC4_HEADER_ONLY

typedef struct rc4_sbox_s
{
	unsigned char state[256];
	unsigned int x;
	unsigned int y;
} rc4_sbox_t;

void rc4_swap(unsigned char &a, unsigned char &b)
{
	unsigned char c = a;
	a = b;
	b = c;
}

void rc4_init(rc4_sbox_t *rc4_sbox, const unsigned char *key_ptr, unsigned int key_len)
{
	rc4_sbox->x = 0;
	rc4_sbox->y = 0;

	// Initialisation of the permutation
	unsigned int i;
	for (i = 0; i < 256; i++)
		rc4_sbox->state[i] = (char)i;

	// Mixing permutation
	unsigned int j = 0;
	unsigned int k;
	for (i = 0; i < 256; i++)
	{
		k = i % key_len;

		j = (key_ptr[k] + rc4_sbox->state[i] + j) & 0xff;
		rc4_swap(rc4_sbox->state[i], rc4_sbox->state[j]);
	}
}

void rc4_transform(rc4_sbox_t *rc4_sbox, unsigned char *buffer_ptr, unsigned int buffer_len)
{
	unsigned int i;
	for (i = 0; i < buffer_len; i++)
	{
		// The pseudo-random generation algorithm
		rc4_sbox->x = (rc4_sbox->x + 1) & 0xff;
		rc4_sbox->y = (rc4_sbox->y + rc4_sbox->state[rc4_sbox->x]) & 0xff;
		rc4_swap(rc4_sbox->state[rc4_sbox->x], rc4_sbox->state[rc4_sbox->y]);
		unsigned char keyChar = rc4_sbox->state[(rc4_sbox->state[rc4_sbox->x] + rc4_sbox->state[rc4_sbox->y]) & 0xff];

		if (buffer_ptr) // NULL when seeking
			buffer_ptr[i] ^= keyChar;
	}
}

void rc4_crypt(const unsigned char* key_ptr, unsigned int key_len, unsigned int stream_pos, unsigned char* buffer_ptr, unsigned int buffer_len)
{
	rc4_sbox_s sbox;
	rc4_init(&sbox, key_ptr, key_len);
	if(stream_pos) // RC4 is very insecure but the first few kb are espetially insecure
		rc4_transform(&sbox, NULL, stream_pos);
	rc4_transform(&sbox, buffer_ptr, buffer_len);
}

#endif RC4_HEADER_ONLY