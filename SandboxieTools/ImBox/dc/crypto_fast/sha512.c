/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.com
 * modified by ntldr, http://diskcryptor.net/
 */
#include <memory.h>
#include <intrin.h>
#include "sha512.h"

// the K array
static const unsigned __int64 K[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

// Various logical functions
#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y)) 
#define S(x, n)         (_rotr64(x, n))
#define R(x, n)         ((unsigned __int64)(x) >> (unsigned __int64)(n))
#define Sigma0(x)       (S(x, 28) ^ S(x, 34) ^ S(x, 39))
#define Sigma1(x)       (S(x, 14) ^ S(x, 18) ^ S(x, 41))
#define Gamma0(x)       (S(x, 1) ^ S(x, 8) ^ R(x, 7))
#define Gamma1(x)       (S(x, 19) ^ S(x, 61) ^ R(x, 6))

// compress 1024-bits
static void sha512_compress(sha512_ctx *ctx, const unsigned char *buf)
{
	unsigned __int64 S[8], W[80], t0, t1;
	int              i;

	// copy state into S
	memcpy(S, ctx->hash, sizeof(S));

	// copy the state into 1024-bits into W[0..15]
	for (i = 0; i < 16; i++) {
		W[i] = _byteswap_uint64(((unsigned __int64*)buf)[i]);
	}

	// fill W[16..79]
	for (i = 16; i < 80; i++) {
		W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
	}

	// Compress
#define RND(a,b,c,d,e,f,g,h,i)                       \
	t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i]; \
	t1 = Sigma0(a) + Maj(a, b, c);                  \
	d += t0;                                        \
	h  = t0 + t1;

	for (i = 0; i < 80; i += 8) {
		RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i+0);
		RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i+1);
		RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i+2);
		RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i+3);
		RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i+4);
		RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i+5);
		RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i+6);
		RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i+7);
	}
	ctx->hash[0] += S[0]; ctx->hash[1] += S[1]; 
	ctx->hash[2] += S[2]; ctx->hash[3] += S[3]; 
	ctx->hash[4] += S[4]; ctx->hash[5] += S[5];
	ctx->hash[6] += S[6]; ctx->hash[7] += S[7];

	// test buffers size alignment at compile-time
	static_assert( !(sizeof(S) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");
	static_assert( !(sizeof(W) % sizeof(unsigned long)), "sizeof must be 4 byte aligned");

	// prevent leaks
	__stosd((unsigned long*)&S, 0, (sizeof(S) / sizeof(unsigned long)));
	__stosd((unsigned long*)&W, 0, (sizeof(W) / sizeof(unsigned long)));
}


// Initialize the hash state
void _stdcall sha512_init(sha512_ctx *ctx)
{
	ctx->curlen   = 0;
	ctx->length   = 0;
	ctx->hash[0] = 0x6a09e667f3bcc908;
	ctx->hash[1] = 0xbb67ae8584caa73b;
	ctx->hash[2] = 0x3c6ef372fe94f82b;
	ctx->hash[3] = 0xa54ff53a5f1d36f1;
	ctx->hash[4] = 0x510e527fade682d1;
	ctx->hash[5] = 0x9b05688c2b3e6c1f;
	ctx->hash[6] = 0x1f83d9abfb41bd6b;
	ctx->hash[7] = 0x5be0cd19137e2179;
}

// Process a block of memory though the hash
void _stdcall sha512_hash(sha512_ctx *ctx, const unsigned char *in, size_t inlen) 
{
	size_t n;

	while (inlen > 0)
	{
		if (ctx->curlen == 0 && inlen >= SHA512_BLOCK_SIZE)
		{
			sha512_compress(ctx, in);

			ctx->length += SHA512_BLOCK_SIZE * 8;
			in          += SHA512_BLOCK_SIZE;
			inlen       -= SHA512_BLOCK_SIZE;
		} else
		{
			if ( (n = (size_t)(SHA512_BLOCK_SIZE - ctx->curlen)) > inlen ) n = inlen;
			memcpy(ctx->buf + ctx->curlen, in, n);
			ctx->curlen += (unsigned long)n;
			in          += n;
			inlen       -= n;

			if (ctx->curlen == SHA512_BLOCK_SIZE)
			{
				sha512_compress(ctx, ctx->buf);
				ctx->length += 8 * SHA512_BLOCK_SIZE;
				ctx->curlen = 0;
			}
		}                                                                                
	}
} 

// Terminate the hash to get the digest
void _stdcall sha512_done(sha512_ctx *ctx, unsigned char *out)
{
	int i;

	// increase the length of the message
	ctx->length += ctx->curlen * 8;

	// append the '1' bit
	ctx->buf[ctx->curlen++] = 0x80;

	/* if the length is currently above 112 bytes we append zeros
	 * then compress.  Then we can fall back to padding zeros and length
	 * encoding like normal.
	 */
	if (ctx->curlen > 112)
	{
		while (ctx->curlen < SHA512_BLOCK_SIZE) {
			ctx->buf[ctx->curlen++] = 0;
		}
		sha512_compress(ctx, ctx->buf);
		ctx->curlen = 0;
	}

	/* pad up to 120 bytes of zeroes 
	 * note: that from 112 to 120 is the 64 MSB of the length.  We assume that you won't hash
	 * > 2^64 bits of data... :-)
	 */
	while (ctx->curlen < 120) {
		ctx->buf[ctx->curlen++] = 0;
	}

	// store length
	((unsigned __int64*)ctx->buf)[15] = _byteswap_uint64(ctx->length);
	sha512_compress(ctx, ctx->buf);

	// copy output
	for (i = 0; i < 8; i++) {
		((unsigned __int64*)out)[i] = _byteswap_uint64(ctx->hash[i]);
	}
}