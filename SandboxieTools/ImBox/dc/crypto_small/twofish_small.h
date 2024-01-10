#ifndef _TWOFISH_SMALL_H_
#define _TWOFISH_SMALL_H_

typedef struct _twofish256_key {
	unsigned long S[4][256], K[40];
} twofish256_key;

#define TWOFISH_KEY_SIZE   32
#define TWOFISH_BLOCK_SIZE 16

void twofish256_set_key(const unsigned char *key, twofish256_key *skey);
void twofish256_encrypt(const unsigned char *in, unsigned char *out, twofish256_key *key);
void twofish256_decrypt(const unsigned char *in, unsigned char *out, twofish256_key *key);

#endif
