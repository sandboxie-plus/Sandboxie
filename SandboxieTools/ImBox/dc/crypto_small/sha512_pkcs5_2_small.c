/*
    *
    * DiskCryptor - open source partition encryption tool
    * Copyright (c) 2010-2013
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
#include "sha512_small.h"
#include "sha512_pkcs5_2_small.h"

void sha512_hmac(const void *k, unsigned long k_len, const void *d, unsigned long d_len, unsigned char *out)
{
	sha512_ctx    ctx;
	unsigned char buf[SHA512_BLOCK_SIZE];
	unsigned char hval[SHA512_DIGEST_SIZE];
	unsigned long i;

	// zero key buffer
	__stosb(buf, 0, sizeof(buf));

	// compress hmac key
	if (k_len > SHA512_BLOCK_SIZE) {
		sha512_init(&ctx);
		sha512_add(&ctx, (const unsigned char*)k, k_len);
		sha512_done(&ctx, buf);
	} else {
		__movsb(buf, (const unsigned char*)k, k_len);
	}

	// create the hash initial vector
	for (i = 0; i < SHA512_BLOCK_SIZE; i++) {
		buf[i] ^= 0x36;
	}

	// hash key and data
	sha512_init(&ctx);
	sha512_add(&ctx, buf, SHA512_BLOCK_SIZE);
	sha512_add(&ctx, (const unsigned char*)d, d_len);
	sha512_done(&ctx, hval);

	// create the second HMAC vector
	for (i = 0; i < SHA512_BLOCK_SIZE; i++) {
		buf[i] ^= 0x6A;
	} 

	// calculate "outer" hash
	sha512_init(&ctx);
	sha512_add(&ctx, buf, SHA512_BLOCK_SIZE);
	sha512_add(&ctx, hval, SHA512_DIGEST_SIZE);
	sha512_done(&ctx, out);

	// prevent leaks
	__stosb(buf, 0, sizeof(buf));
	__stosb(hval, 0, sizeof(hval));
	__stosb((unsigned char*)&ctx, 0, sizeof(ctx));
}

void sha512_pkcs5_2(int i_count, const void *pwd, unsigned long pwd_len, const void *salt, unsigned long salt_len, unsigned char *dk, unsigned long dklen)
{
	unsigned char buff[128 + sizeof(unsigned long)];
	unsigned char blk[SHA512_DIGEST_SIZE];
	unsigned char hmac[SHA512_DIGEST_SIZE];
	unsigned long block = 1;
	unsigned long c_len;
	int           j, i;

	while (dklen != 0)
	{
		// first iteration
		__movsb(buff, (const unsigned char*)salt, salt_len);
		((unsigned long*)(buff + salt_len))[0] = _byteswap_ulong(block);
		sha512_hmac(pwd, pwd_len, buff, salt_len + sizeof(unsigned long), hmac);
		__movsb(blk, hmac, SHA512_DIGEST_SIZE);

		// next iterations
		for (i = 1; i < i_count; i++) 
		{
			sha512_hmac(pwd, pwd_len, hmac, SHA512_DIGEST_SIZE, hmac);

			for (j = 0; j < SHA512_DIGEST_SIZE; j++) {
				blk[j] ^= hmac[j];
			}
		}
		__movsb((unsigned char*)dk, blk, (c_len = dklen < SHA512_DIGEST_SIZE ? dklen : SHA512_DIGEST_SIZE));
		dk += c_len; dklen -= c_len; block++;
	}
	
	// prevent leaks
	__stosb(buff, 0, sizeof(buff));
	__stosb(blk, 0, sizeof(blk));
	__stosb(hmac, 0, sizeof(hmac));
}
