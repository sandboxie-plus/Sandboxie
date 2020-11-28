/*
 * Copyright 2020 DavidXanatos, xanasoft.com
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

#ifndef rc4_INCLUDED
#  define rc4_INCLUDED

typedef struct rc4_sbox_s
{
	unsigned char state[256]; 
	unsigned int x;
	unsigned int y;
} rc4_sbox_t;

#ifdef __cplusplus
extern "C" 
{
#endif

void rc4_init(rc4_sbox_t *rc4_sbox, const unsigned char *key_ptr, unsigned int key_len);

void rc4_transform(rc4_sbox_t *rc4_sbox, unsigned char *buffer_ptr, unsigned int buffer_len);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* rc4_INCLUDED */
