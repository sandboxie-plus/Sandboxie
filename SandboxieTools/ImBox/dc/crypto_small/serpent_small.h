#ifndef _SERPENT_SMALL_H_
#define _SERPENT_SMALL_H_

#define SERPENT_KEY_SIZE	 32
#define SERPENT_EXPKEY_WORDS 132
#define SERPENT_BLOCK_SIZE	 16

typedef struct _serpent256_key {
	unsigned long expkey[SERPENT_EXPKEY_WORDS];
} serpent256_key;

void serpent256_set_key(const unsigned char *key, serpent256_key *skey);
void serpent256_encrypt(const unsigned char *in, unsigned char *out, serpent256_key *key);
void serpent256_decrypt(const unsigned char *in, unsigned char *out, serpent256_key *key);

#endif