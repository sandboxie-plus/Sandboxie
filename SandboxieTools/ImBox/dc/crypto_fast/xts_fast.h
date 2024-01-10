#ifndef _XTS_FAST_H_
#define _XTS_FAST_H_

#include <memory.h>
#include "aes_key.h"
#include "twofish.h"
#include "serpent.h"

#define CF_AES                 0
#define CF_TWOFISH             1
#define CF_SERPENT             2
#define CF_AES_TWOFISH         3
#define CF_TWOFISH_SERPENT     4
#define CF_SERPENT_AES         5
#define CF_AES_TWOFISH_SERPENT 6
#define CF_CIPHERS_NUM         7

#define XTS_SECTOR_SIZE      512
#define XTS_BLOCK_SIZE       16
#define XTS_BLOCKS_IN_SECTOR (XTS_SECTOR_SIZE / XTS_BLOCK_SIZE)

#define XTS_KEY_SIZE   32
#define XTS_FULL_KEY   (XTS_KEY_SIZE*3*2)

typedef void (_stdcall *xts_proc)(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, void *key);

typedef __declspec(align(16)) struct _xts_key {
	struct {
		aes256_key     aes;
		twofish256_key twofish;
		serpent256_key serpent;
	} crypt_k;
	struct {
		aes256_key     aes;
		twofish256_key twofish;
		serpent256_key serpent;
	} tweak_k;
	xts_proc encrypt;
	xts_proc decrypt;
	
} xts_key;

int _stdcall xts_init(int hw_crypt);
void _stdcall xts_set_key(const unsigned char *key, int alg, xts_key *skey);
int  _stdcall xts_aes_ni_available();

#define xts_encrypt(_in, _out, _len, _offset, _key) ( (_key)->encrypt(_in, _out, _len, _offset, _key) )
#define xts_decrypt(_in, _out, _len, _offset, _key) ( (_key)->decrypt(_in, _out, _len, _offset, _key) )

#ifdef _M_IX86
	extern long save_fpu_state(unsigned char state[32]);
	extern void load_fpu_state(unsigned char state[32]);
#endif

#endif