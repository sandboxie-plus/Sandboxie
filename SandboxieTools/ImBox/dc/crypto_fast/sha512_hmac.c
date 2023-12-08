/*
    *
    * Copyright (c) 2013
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
#include <intrin.h>
#include "sha512_hmac.h"

void _stdcall sha512_hmac_init(sha512_hmac_ctx* ctx, const void* key, size_t keylen)
{
	unsigned long i;

	// zero-fill initial key state
	memset(ctx->padded_key, 0, sizeof(ctx->padded_key));

	// compress HMAC key if needed, or copy source key
	if (keylen > SHA512_BLOCK_SIZE) {
		sha512_init(&ctx->hash);
		sha512_hash(&ctx->hash, (const unsigned char*)key, keylen);
		sha512_done(&ctx->hash, ctx->padded_key);
	} else {
		memcpy(ctx->padded_key, key, keylen);
	}

	// start "inner" hash
	for (i = 0; i < (SHA512_BLOCK_SIZE / 4); i++) {
		((unsigned long*)ctx->padded_key)[i] ^= 0x36363636;
	}
	sha512_init(&ctx->hash);
	sha512_hash(&ctx->hash, ctx->padded_key, SHA512_BLOCK_SIZE);
}

void _stdcall sha512_hmac_hash(sha512_hmac_ctx* ctx, const void* ptr, size_t length)
{
	sha512_hash(&ctx->hash, (const unsigned char*)ptr, length);
}

void _stdcall sha512_hmac_done(sha512_hmac_ctx* ctx, unsigned char* out)
{
	unsigned char inner_hash[SHA512_DIGEST_SIZE];
	unsigned long i;

	// finalize "inner" hash
	sha512_done(&ctx->hash, inner_hash);

	// calculate "outer" hash
	for (i = 0; i < (SHA512_BLOCK_SIZE / 4); i++) {
		((unsigned long*)ctx->padded_key)[i] ^= 0x6A6A6A6A; // 0x36 ^ 0x64 = 0x5С
	}
	sha512_init(&ctx->hash);
	sha512_hash(&ctx->hash, ctx->padded_key, SHA512_BLOCK_SIZE);
	sha512_hash(&ctx->hash, inner_hash, SHA512_DIGEST_SIZE);
	sha512_done(&ctx->hash, out);

	// prevent leaks
	static_assert( !(sizeof(inner_hash) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");
	__stosd((unsigned long*)&inner_hash, 0, (sizeof(inner_hash) / sizeof(unsigned long)));
}

void _stdcall sha512_hmac(const void *k, size_t k_len, const void *d, size_t d_len, unsigned char *out)
{
	sha512_hmac_ctx ctx;

	sha512_hmac_init(&ctx, k, k_len);
	sha512_hmac_hash(&ctx, d, d_len);
	sha512_hmac_done(&ctx, out);

	// prevent leaks
	static_assert( !(sizeof(ctx) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");
	__stosd((unsigned long*)&ctx, 0, (sizeof(ctx) / sizeof(unsigned long)));
}
