/*
    *
    * Copyright (c) 2007-2013
    * ntldr <ntldr@diskcryptor.net> PGP key ID - 0x1B6A24550F33E44A
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
#include <memory.h>
#include <intrin.h>
#include "sha512_hmac.h"
#include "sha512_pkcs5_2.h"

void _stdcall sha512_pkcs5_2(int i_count, const void *pwd, size_t pwd_len, const void *salt, size_t salt_len, unsigned char *dk, size_t dklen)
{
	sha512_hmac_ctx ctx;
	unsigned char blk[SHA512_DIGEST_SIZE], hmac[SHA512_DIGEST_SIZE];
	unsigned long block = 0x01000000; // _byteswap_ulong(1);
	size_t c_len;
	int    j, i;

	while (dklen != 0)
	{
		// first iteration
		sha512_hmac_init(&ctx, pwd, pwd_len);
		sha512_hmac_hash(&ctx, salt, salt_len);
		sha512_hmac_hash(&ctx, &block, sizeof(unsigned long));
		sha512_hmac_done(&ctx, hmac);
		memcpy(blk, hmac, SHA512_DIGEST_SIZE);

		// next iterations
		for (i = 1; i < i_count; i++) 
		{
			sha512_hmac_init(&ctx, pwd, pwd_len);
			sha512_hmac_hash(&ctx, hmac, SHA512_DIGEST_SIZE);
			sha512_hmac_done(&ctx, hmac);

			for (j = 0; j < (SHA512_DIGEST_SIZE / 4); j++) {
				((unsigned long*)blk)[j] ^= ((unsigned long*)hmac)[j];
			}
		}

		block = _byteswap_ulong(_byteswap_ulong(block) + 1);
		memcpy(dk, blk, (c_len = dklen < SHA512_DIGEST_SIZE ? dklen : SHA512_DIGEST_SIZE));
		dk += c_len; dklen -= c_len;
	}

	// test buffers size alignment at compile-time
	static_assert( !(sizeof(ctx) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");
	static_assert( !(sizeof(blk) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");
	static_assert( !(sizeof(hmac) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");

	// prevent leaks
	__stosd((unsigned long*)&ctx, 0, (sizeof(ctx) / sizeof(unsigned long)));
	__stosd((unsigned long*)&blk, 0, (sizeof(blk) / sizeof(unsigned long)));
	__stosd((unsigned long*)&hmac, 0, (sizeof(hmac) / sizeof(unsigned long)));
}
