#ifndef _TWOFISH_H_
#define _TWOFISH_H_

/* Structure for an expanded Twofish key.  s contains the key-dependent
 * S-boxes composed with the MDS matrix; w contains the eight "whitening"
 * subkeys, K[0] through K[7].	k holds the remaining, "round" subkeys.  Note
 * that k[i] corresponds to what the Twofish paper calls K[i+8]. */

typedef struct _twofish256_key {
	unsigned long s[4][256], w[8], k[32];
} twofish256_key;

#define TWOFISH_KEY_SIZE   32
#define TWOFISH_BLOCK_SIZE 16

void _stdcall twofish256_set_key(const unsigned char *key, twofish256_key *skey);
void _stdcall twofish256_encrypt(const unsigned char *in, unsigned char *out, twofish256_key *key);
void _stdcall twofish256_decrypt(const unsigned char *in, unsigned char *out, twofish256_key *key);

#endif
