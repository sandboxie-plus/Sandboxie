#ifndef _AES_PADLOCK_H_
#define _AES_PADLOCK_H_

#include "aes_key.h"

#ifdef _M_IX86
static void __forceinline aes256_padlock_rekey() {
	__asm {
		 pushfd
		 popfd
	 }
}
#else
#define aes256_padlock_rekey() __writeeflags(__readeflags())
#endif

int  _stdcall aes256_padlock_available();
void _stdcall aes256_padlock_encrypt(const unsigned char *in, unsigned char *out, int n_blocks, aes256_key *key);
void _stdcall aes256_padlock_decrypt(const unsigned char *in, unsigned char *out, int n_blocks, aes256_key *key);

#endif