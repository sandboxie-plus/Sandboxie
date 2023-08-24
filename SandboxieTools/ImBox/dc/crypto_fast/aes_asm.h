#ifndef _AES_ASM_H_
#define _AES_ASM_H_

#include "aes_key.h"

#ifdef _M_IX86
 void _stdcall aes256_asm_set_key(const unsigned char *key, aes256_key *skey);
#else
 #define aes256_asm_set_key aes256_set_key
#endif
void _stdcall aes256_asm_encrypt(const unsigned char *in, unsigned char *out, aes256_key *key);
void _stdcall aes256_asm_decrypt(const unsigned char *in, unsigned char *out, aes256_key *key);

#endif