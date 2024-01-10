#ifndef _AES_KEY_H_
#define _AES_KEY_H_

#define ROUNDS 14
#define AES_KEY_SIZE   32
#define AES_BLOCK_SIZE 16

typedef __declspec(align(16)) struct _aes256_key {
	__declspec(align(16)) unsigned long enc_key[4 *(ROUNDS + 1)];
	__declspec(align(16)) unsigned long dec_key[4 *(ROUNDS + 1)];
#ifdef _M_IX86
	__declspec(align(16)) unsigned char ek_code[3072];
	__declspec(align(16)) unsigned char dk_code[3072];
#endif
} aes256_key;

void _stdcall aes256_set_key(const unsigned char *key, aes256_key *skey);

#endif