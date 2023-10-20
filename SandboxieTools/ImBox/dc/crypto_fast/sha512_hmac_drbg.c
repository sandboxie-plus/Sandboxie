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
#include "sha512_hmac_drbg.h"

/*
	Internal function, implements HMAC_DBRG_Update according NIST SP 800-90 specification.
*/
static void sha512_hmac_drbg_update( sha512_hmac_drbg_ctx *ctx, const void *provided_1, size_t provided_1_len,
	                                                            const void *provided_2, size_t provided_2_len )
{
	sha512_hmac_ctx hmac;
	unsigned char   nullbyte = 0x00, onebyte = 0x01;

	// Key = HMAC ( Key, Val || 0x00 || provided_data )
	sha512_hmac_init(&hmac, ctx->key, sizeof(ctx->key));
	sha512_hmac_hash(&hmac, ctx->val, sizeof(ctx->val));
	sha512_hmac_hash(&hmac, &nullbyte, sizeof(nullbyte));
	sha512_hmac_hash(&hmac, provided_1, provided_1_len);
	sha512_hmac_hash(&hmac, provided_2, provided_2_len);
	sha512_hmac_done(&hmac, ctx->key);

	// Val = HMAC ( Key, Val )
	sha512_hmac_init(&hmac, ctx->key, sizeof(ctx->key));
	sha512_hmac_hash(&hmac, ctx->val, sizeof(ctx->val));
	sha512_hmac_done(&hmac, ctx->val);

	if (provided_1_len != 0 || provided_2_len != 0)
	{
		// Key = HMAC ( Key, Val || 0x01 || provided_data )
		sha512_hmac_init(&hmac, ctx->key, sizeof(ctx->key));
		sha512_hmac_hash(&hmac, ctx->val, sizeof(ctx->val));
		sha512_hmac_hash(&hmac, &onebyte, sizeof(onebyte));
		sha512_hmac_hash(&hmac, provided_1, provided_1_len);
		sha512_hmac_hash(&hmac, provided_2, provided_2_len);
		sha512_hmac_done(&hmac, ctx->key);

		// Val = HMAC ( Key, Val )
		sha512_hmac_init(&hmac, ctx->key, sizeof(ctx->key));
		sha512_hmac_hash(&hmac, ctx->val, sizeof(ctx->val));
		sha512_hmac_done(&hmac, ctx->val);
	}

	// prevent leaks
	static_assert( !(sizeof(hmac) % sizeof(unsigned long)), "sizeof(hmac) must be 4 byte aligned");
	__stosd((unsigned long*)&hmac, 0, (sizeof(hmac) / sizeof(unsigned long)));
}

/*
	Instantiate SHA512_HMAC_DRBG:
		ctx          - SHA512_HMAC_DRBG internal state to be initialised
		entropy      - Entropy input (nonce must be included within the entropy input)
		entropy_len  - Length of entropy input
		personal     - Personalisation string
		personal_len - Length of personalisation string
*/
int _stdcall sha512_hmac_drbg_instantiate( sha512_hmac_drbg_ctx *ctx, const void *entropy,  size_t entropy_len,
	                                                                  const void *personal, size_t personal_len )
{
	// entropy input must contain at least 3/2 * security_strength bits of entropy
	if ( (entropy_len < SHA512_HMAC_DRBG_MIN_ENTROPY_BYTES * 3 / 2) ||
		 (entropy_len > SHA512_HMAC_DRBG_MAX_ENTROPY_BYTES) ||
		 (personal_len > SHA512_HMAC_DRBG_MAX_PERSONAL_BYTES) )
	{
		return -1;
	}

	// Key = 0x00, 00..00
	memset(ctx->key, 0x00, sizeof(ctx->key));

	// Val = 0x01, 01...01
	memset(ctx->val, 0x01, sizeof(ctx->val));

	// ( Key, Val ) = HMAC_DBRG_Update ( entropy || personal, Key, Val )
	sha512_hmac_drbg_update(ctx, entropy, entropy_len, personal, personal_len);

	// reseed_counter = 1
	ctx->reseed_counter = 1;

	// return SUCCESS
	return 0;
}

/*
	Reseed SHA512_HMAC_DRBG:
		ctx            - SHA512_HMAC_DRBG internal state
		entropy        - Entropy input
		entropy_len    - Length of entropy input
		additional     - Additional input
		additional_len - Length of additional input
*/
int _stdcall sha512_hmac_drbg_reseed( sha512_hmac_drbg_ctx *ctx, const void *entropy,    size_t entropy_len,
	                                                             const void *additional, size_t additional_len )
{
	if ( (entropy_len < SHA512_HMAC_DRBG_MIN_ENTROPY_BYTES) ||
		 (entropy_len > SHA512_HMAC_DRBG_MAX_ENTROPY_BYTES) ||
		 (additional_len > SHA512_HMAC_DRBG_MAX_ADDITIONAL_BYTES) )
	{
		return -1;
	}

	// ( Key, Val ) = HMAC_DBRG_Update ( entropy || additional, Key, Val )
	sha512_hmac_drbg_update(ctx, entropy, entropy_len, additional, additional_len);

	// reset reseed counter
	ctx->reseed_counter = 1;

	// return SUCCESS
	return 0;
}

/*
	Generate pseudorandom bits using SHA512_HMAC_DRBG:
		ctx            - SHA512_HMAC_DRBG internal state
		additional     - Additional input
		additional_len - Length of additional input
		output         - Output buffer
		output_len     - Length of output buffer
*/
int _stdcall sha512_hmac_drbg_generate( sha512_hmac_drbg_ctx *ctx, const void    *additional, size_t additional_len,
	                                                               unsigned char *output,     size_t output_len )
{
	if ( (ctx->reseed_counter > SHA512_HMAC_DRBG_RESEED_INTERVAL) || // If reseed_counter > reseed_interval, then return an indication that a reseed is required
		 (additional_len > SHA512_HMAC_DRBG_MAX_ADDITIONAL_BYTES) || // check max_additional_input_length    (NIST SP 800-90A specification)
		 (output_len > SHA512_HMAC_DRBG_MAX_GENERATED_BYTES) )       // check max_number_of_bits_per_request (NIST SP 800-90A specification)
	{
		return -1;
	}

	// If additional_input != Null,
	// then ( Key, Val ) = HMAC_DRBG_Update ( additional_input, Key, Val )
	if (additional_len != 0) {
		sha512_hmac_drbg_update(ctx, additional, additional_len, NULL, 0);
	}

	// While ( len ( temp ) < requested_number_of_bits ) do
	while ( output_len )
	{
		size_t part_len = output_len < SHA512_DIGEST_SIZE ? output_len : SHA512_DIGEST_SIZE;

		// Val = HMAC ( Key, Val )
		sha512_hmac(ctx->key, sizeof(ctx->key), ctx->val, sizeof(ctx->val), ctx->val);

		// temp = temp || V
		// returned_bits = Leftmost requested_number_of_bits of temp
		memcpy(output, ctx->val, part_len);
		output += part_len;
		output_len -= part_len;
	}

	// ( Key, Val ) = HMAC_DRBG_Update ( additional_input, Key, Val )
	sha512_hmac_drbg_update(ctx, additional, additional_len, NULL, 0);

	// reseed_counter = reseed_counter + 1
	ctx->reseed_counter++;

	// return SUCCESS
	return 0;
}
