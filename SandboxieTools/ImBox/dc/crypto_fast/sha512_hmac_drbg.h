#ifndef _SHA512_HMAC_DRBG_H_
#define _SHA512_HMAC_DRBG_H_

#include "sha512.h"
#include "sha512_hmac.h"

/* The minimum required entropy interval for instantiate and reseed is security_strength bits.
   according NIST SP 800-90 Section 10.1 Table 2
*/
#define SHA512_HMAC_DRBG_MIN_ENTROPY_BYTES ( 256 / 8 )

/* The maximum entropy input length for HMAC_DRBG is 2^35 bits
   according NIST SP 800-90 Section 10.1 Table 2
*/
#define SHA512_HMAC_DRBG_MAX_ENTROPY_BYTES  ( (1ull << 35) / 8 )

/* The maximum permitted personalisation string length for HMAC_DRBG is 2^35 bits
   according NIST SP 800-90 Section 10.1 Table 2
*/
#define SHA512_HMAC_DRBG_MAX_PERSONAL_BYTES ( (1ull << 35) / 8 )

/* The maximum permitted additional input length for HMAC_DRBG is 2^35 bits
   according NIST SP 800-90 Section 10.1 Table 2
*/
#define SHA512_HMAC_DRBG_MAX_ADDITIONAL_BYTES ( (1ull << 35) / 8 )

/* The maximum number of bits per request for HMAC_DRBG is 2^19 bits
   according NIST SP 800-90 Section 10.1 Table 2
*/
#define SHA512_HMAC_DRBG_MAX_GENERATED_BYTES ( (1 << 19) / 8 )

/* The maximum permitted reseed interval for HMAC_DRBG is 2^48, 
   We choose a very conservative reseed interval.
*/
#define SHA512_HMAC_DRBG_RESEED_INTERVAL 128

typedef struct _sha512_hmac_drbg_ctx {
	// The outlen-bit Key, which is updated at least once each time that the DRBG mechanism generates pseudorandom bits.
	unsigned char key[SHA512_DIGEST_SIZE];

	// The value V of outlen bits, which is updated each time another outlen bits of output are produced.
	unsigned char val[SHA512_DIGEST_SIZE];

	// A counter that indicates the number of generation requests since instantiation or reseeding.
	unsigned long reseed_counter;

} sha512_hmac_drbg_ctx;


int _stdcall sha512_hmac_drbg_instantiate( sha512_hmac_drbg_ctx *ctx, const void *entropy,  size_t entropy_len,
	                                                                  const void *personal, size_t personal_len );

int _stdcall sha512_hmac_drbg_reseed( sha512_hmac_drbg_ctx *ctx, const void *entropy,    size_t entropy_len,
	                                                             const void *additional, size_t additional_len );

int _stdcall sha512_hmac_drbg_generate( sha512_hmac_drbg_ctx *ctx, const void    *additional, size_t additional_len,
	                                                               unsigned char *output,     size_t output_len );

#endif