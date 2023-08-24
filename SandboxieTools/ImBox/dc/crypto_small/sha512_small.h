#ifndef _SHA512_SMALL_H_
#define _SHA512_SMALL_H_

typedef struct _sha512_ctx {
	unsigned __int64 hash[8];
	unsigned __int64 length;
	unsigned long curlen;
	unsigned char buf[128];

} sha512_ctx;

#define SHA512_DIGEST_SIZE 64
#define SHA512_BLOCK_SIZE  128

void sha512_init(sha512_ctx *ctx);
void sha512_add(sha512_ctx *ctx, const unsigned char *in, unsigned long inlen);
void sha512_done(sha512_ctx *ctx, unsigned char *out);

#endif