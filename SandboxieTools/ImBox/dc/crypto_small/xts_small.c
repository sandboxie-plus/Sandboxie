/*
    *
    * Copyright (c) 2010-2013
    * ntldr <ntldr@diskcryptor.net> PGP key ID - 0xC48251EB4F8E4E6E
    * based on rijndael-alg-fst.c
    *  @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
    *  @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
    *  @author Paulo Barreto <paulo.barreto@terra.com.br>
	*  @author Serge Trusov <serge.trusov@gmail.com>
    *

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "xts_small.h"

typedef void (*set_key_p)(const unsigned char *key, void *skey);
typedef void (*encrypt_p)(const unsigned char *in, unsigned char *out, void *key);

typedef struct _cipher_desc {
	set_key_p set_key;
	encrypt_p encrypt;
	encrypt_p decrypt;
	int       ctxsz;
} cipher_desc;

typedef __declspec(align(1)) union _m128 {
    unsigned long    v32[4];    
    unsigned __int64 v64[2];    
} m128;

static cipher_desc aes256 = {
	(set_key_p)aes256_set_key, (encrypt_p)aes256_encrypt, (encrypt_p)aes256_decrypt, sizeof(aes256_key)
};

#ifndef AES_ONLY
static cipher_desc twofish256 = {
	(set_key_p)twofish256_set_key, (encrypt_p)twofish256_encrypt, (encrypt_p)twofish256_decrypt, sizeof(twofish256_key)
};
static cipher_desc serpent256 = {
	(set_key_p)serpent256_set_key, (encrypt_p)serpent256_encrypt, (encrypt_p)serpent256_decrypt, sizeof(serpent256_key)
};

static cipher_desc *algs[7][3] = {
	{ &aes256,                          },
	{ &twofish256,                      },
	{ &serpent256,                      },
	{ &twofish256, &aes256,             },
	{ &serpent256, &twofish256,         },
	{ &aes256,     &serpent256,         },
	{ &serpent256, &twofish256, &aes256 }
};
#endif

static void xts_process(
		const unsigned char *in, unsigned char *out, unsigned long len, 
		unsigned __int64 offset, encrypt_p crypt_p, encrypt_p tweak_p, void *crypt_k, void *tweak_k
		)
{
	__declspec(align(16)) unsigned char tmp[XTS_BLOCK_SIZE];
	__declspec(align(16)) m128 t, idx;
	unsigned long              i, cf;
	
	idx.v64[0] = offset / XTS_SECTOR_SIZE;
	idx.v64[1] = 0;

	for (; len; len -= XTS_SECTOR_SIZE)
	{
//#ifdef _M_IX86
//		if (tweak_p == (encrypt_p)aes256_padlock_encrypt) {
//			aes256_padlock_rekey();
//		}
//#endif
		/* update tweak unit index */
		idx.v64[0]++;
		/* derive first tweak value */
		tweak_p((unsigned char*)&idx, (unsigned char*)&t, tweak_k);
//#ifdef _M_IX86
//		if (tweak_p == (encrypt_p)aes256_padlock_encrypt) {
//			aes256_padlock_rekey();
//		}
//#endif
		for (i = 0; i < XTS_BLOCKS_IN_SECTOR; i++)
		{
			((unsigned __int64*)tmp)[0] = ((unsigned __int64*)in)[0] ^ t.v64[0];
			((unsigned __int64*)tmp)[1] = ((unsigned __int64*)in)[1] ^ t.v64[1];
			
			crypt_p(tmp, tmp, crypt_k);

			((unsigned __int64*)out)[0] = ((unsigned __int64*)tmp)[0] ^ t.v64[0];
			((unsigned __int64*)out)[1] = ((unsigned __int64*)tmp)[1] ^ t.v64[1];

			/* update pointers */
			in += XTS_BLOCK_SIZE; out += XTS_BLOCK_SIZE;
			/* derive next tweak value */
			cf = (t.v32[3] >> 31) * 135;
			t.v64[1] <<= 1;
			t.v32[2] |= t.v32[1] >> 31;
			t.v64[0] <<= 1;
			t.v32[0] ^= cf;
		}
	}
}

#ifndef AES_ONLY

void xts_set_key(const unsigned char *key, int alg, xts_key *skey)
{
	cipher_desc   *p_alg;
	unsigned char *p_ctx;
	int            i;
	
	/* set encryption key */
	for (i = 0, p_ctx = skey->crypt_k; (i < 3) && ((p_alg = algs[alg][i]) != 0); i++) {
		p_alg->set_key(key, p_ctx); key += XTS_KEY_SIZE; p_ctx += p_alg->ctxsz;
	}
	/* set tweak key */
	for (i = 0, p_ctx = skey->tweak_k; (i < 3) && ((p_alg = algs[alg][i]) != 0); i++) {
		p_alg->set_key(key, p_ctx); key += XTS_KEY_SIZE; p_ctx += p_alg->ctxsz;
	}
	skey->algs  = (void**)algs[alg];
	skey->max   = i-1;
	skey->ctxsz = (unsigned long)(p_ctx - skey->tweak_k);
}

void xts_encrypt(const unsigned char *in, unsigned char *out, unsigned long len, unsigned __int64 offset, xts_key *key)
{
	cipher_desc   *p_alg;
	unsigned char *p_crypt_k = key->crypt_k;
	unsigned char *p_tweak_k = key->tweak_k;
	int            i = 0;

	do
	{
		p_alg = (cipher_desc*)(key->algs[i]);
		xts_process(in, out, len, offset, p_alg->encrypt, p_alg->encrypt, p_crypt_k, p_tweak_k); 
		in = out; p_crypt_k += p_alg->ctxsz; p_tweak_k += p_alg->ctxsz;
	} while (++i <= key->max);
}

void xts_decrypt(const unsigned char *in, unsigned char *out, unsigned long len, unsigned __int64 offset, xts_key *key)
{
	cipher_desc   *p_alg;
	unsigned char *p_crypt_k = key->crypt_k + key->ctxsz;
	unsigned char *p_tweak_k = key->tweak_k + key->ctxsz;
	int            i = key->max;

	do
	{
		p_alg = (cipher_desc*)(key->algs[i]); p_crypt_k -= p_alg->ctxsz; p_tweak_k -= p_alg->ctxsz;
		xts_process(in, out, len, offset, p_alg->decrypt, p_alg->encrypt, p_crypt_k, p_tweak_k);
		in = out;
	} while (--i >= 0);
}
#else

void xts_set_key(const unsigned char *key, int alg, xts_key *skey)
{
	aes256_set_key(key, (aes256_key*)&skey->crypt_k);
	aes256_set_key(key + XTS_KEY_SIZE, (aes256_key*)&skey->tweak_k);
}

void xts_encrypt(const unsigned char *in, unsigned char *out, unsigned long len, unsigned __int64 offset, xts_key *key)
{
	xts_process(in, out, len, offset, aes256.encrypt, aes256.encrypt, &key->crypt_k, &key->tweak_k); 
}

void xts_decrypt(const unsigned char *in, unsigned char *out, unsigned long len, unsigned __int64 offset, xts_key *key)
{
	xts_process(in, out, len, offset, aes256.decrypt, aes256.encrypt, &key->crypt_k, &key->tweak_k);
}
#endif

int xts_init(int hw_crypt)
{
	aes256_gentab();
//#ifdef _M_IX86
//	if ( (hw_crypt != 0) && (aes256_padlock_available() != 0) ) {
//		aes256.encrypt = (encrypt_p) aes256_padlock_encrypt;
//		aes256.decrypt = (encrypt_p) aes256_padlock_decrypt;
//	}
//#endif
	return 0;
}