/*
    *
    * Copyright (c) 2010-2012
    * ntldr <ntldr@diskcryptor.net> PGP key ID - 0x1B6A24550F33E44A
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
#include <intrin.h>
#include <emmintrin.h>
//#include <excpt.h>
#include "xts_fast.h"
#include "aes_asm.h"
#include "aes_padlock.h"
#include "xts_aes_ni.h"
#include "xts_serpent_sse2.h"
#include "xts_serpent_avx.h"

typedef __declspec(align(1)) union _m128 {
    unsigned long    v32[4];    
    unsigned __int64 v64[2];    
} m128;

static xts_proc aes_selected_encrypt;
static xts_proc aes_selected_decrypt;
static xts_proc serpent_selected_encrypt;
static xts_proc serpent_selected_decrypt;

#ifdef _M_X64
#define def_tweak \
	unsigned __int64 t0, t1; m128

#define load_tweak() do {         \
	t0 = t.v64[0]; t1 = t.v64[1]; \
} while (0)

#define tweak_xor(_in, _out) do {                                        \
	((unsigned __int64*)(_out))[0] = ((unsigned __int64*)(_in))[0] ^ t0; \
	((unsigned __int64*)(_out))[1] = ((unsigned __int64*)(_in))[1] ^ t1; \
} while (0)

#define next_tweak() do {        \
	cf = (t1 >> 63) * 135;       \
	t1 = (t1 << 1) | (t0 >> 63); \
	t0 = (t0 << 1) ^ cf;         \
} while (0)

#define copy_tweak(_buf) do {            \
	((unsigned __int64*)(_buf))[0] = t0; \
	((unsigned __int64*)(_buf))[1] = t1; \
} while (0)
#else
#define def_tweak    m128
#define load_tweak()

#define tweak_xor(_in, _out) do {                                              \
	((unsigned __int64*)(_out))[0] = ((unsigned __int64*)(_in))[0] ^ t.v64[0]; \
	((unsigned __int64*)(_out))[1] = ((unsigned __int64*)(_in))[1] ^ t.v64[1]; \
} while (0)

#define next_tweak() do {        \
	cf = (t.v32[3] >> 31) * 135; \
	t.v64[1] <<= 1;              \
	t.v32[2] |= t.v32[1] >> 31;  \
	t.v64[0] <<= 1;              \
	t.v32[0] ^= cf;              \
} while (0)

#define copy_tweak(_buf) do {    \
	memcpy(_buf, &t, sizeof(t)); \
} while (0)
#endif

#define DEF_XTS_PROC(func_name, tweak_name, crypt_name, key_field)                                                             \
                                                                                                                               \
static void _stdcall func_name(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key) \
{                                                                                      \
	def_tweak     t;                                                                   \
	m128          idx;                                                                 \
	size_t        cf;                                                                  \
	unsigned long i;                                                                   \
	                                                                                   \
	idx.v64[0] = offset / XTS_SECTOR_SIZE;                                             \
	idx.v64[1] = 0;                                                                    \
	do                                                                                 \
	{                                                                                  \
		/* update tweak unit index */                                                  \
		idx.v64[0]++;                                                                  \
		/* derive first tweak value */                                                 \
		tweak_name((unsigned char*)&idx, (unsigned char*)&t, &key->tweak_k.key_field); \
		load_tweak();                                                                  \
                                                                                       \
		for (i = 0; i < XTS_BLOCKS_IN_SECTOR; i++)                                     \
		{                                                                              \
			tweak_xor(in, out);                                                        \
			crypt_name(out, out, &key->crypt_k.key_field);                             \
			tweak_xor(out, out);                                                       \
                                                                                       \
			/* update pointers */                                                      \
			in += XTS_BLOCK_SIZE; out += XTS_BLOCK_SIZE;                               \
                                                                                       \
			/* derive next tweak value */                                              \
			next_tweak();                                                              \
		}                                                                              \
	} while (len -= XTS_SECTOR_SIZE);                                                  \
}

#define DEF_XTS_AES_PADLOCK(func_name, crypt_name)                                                                             \
	                                                                                                                           \
static void _stdcall func_name(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key) \
{                                                                                                       \
	def_tweak     __declspec(align(16)) t;                                                              \
	m128          __declspec(align(16)) idx;                                                            \
	unsigned char __declspec(align(16)) buff[XTS_SECTOR_SIZE], tweak[XTS_SECTOR_SIZE];                  \
	size_t                              cf, i;                                                          \
	                                                                                                    \
	idx.v64[0] = offset / XTS_SECTOR_SIZE;                                                              \
	idx.v64[1] = 0;                                                                                     \
	do                                                                                                  \
	{                                                                                                   \
		/* update tweak unit index */                                                                   \
		idx.v64[0]++;                                                                                   \
		/* derive first tweak value */                                                                  \
		aes256_padlock_rekey();                                                                         \
		aes256_padlock_encrypt((unsigned char*)&idx, (unsigned char*)&t, 1, &key->tweak_k.aes);         \
		load_tweak();                                                                                   \
		                                                                                                \
	   /* derive all tweak values for sector */                                                         \
		for (i = 0; i < XTS_BLOCKS_IN_SECTOR; i++) {                                                    \
			copy_tweak(tweak + i*XTS_BLOCK_SIZE);                                                       \
			next_tweak();                                                                               \
		}                                                                                               \
		for (i = 0; i < XTS_SECTOR_SIZE / sizeof(unsigned __int64); i++) {                              \
			((unsigned __int64*)buff)[i] = ((unsigned __int64*)in)[i] ^ ((unsigned __int64*)tweak)[i];  \
		}                                                                                               \
		aes256_padlock_rekey();                                                                         \
		crypt_name(buff, buff, XTS_BLOCKS_IN_SECTOR, &key->crypt_k.aes);                                \
		                                                                                                \
		for (i = 0; i < XTS_SECTOR_SIZE / sizeof(unsigned __int64); i++) {                              \
			((unsigned __int64*)out)[i] = ((unsigned __int64*)buff)[i] ^ ((unsigned __int64*)tweak)[i]; \
		}                                                                                               \
		/* update pointers */                                                                           \
		in += XTS_SECTOR_SIZE; out += XTS_SECTOR_SIZE;                                                  \
	} while (len -= XTS_SECTOR_SIZE);                                                                   \
}

DEF_XTS_PROC(xts_aes_basic_encrypt, aes256_asm_encrypt, aes256_asm_encrypt, aes);
DEF_XTS_PROC(xts_aes_basic_decrypt, aes256_asm_encrypt, aes256_asm_decrypt, aes);

DEF_XTS_PROC(xts_twofish_encrypt, twofish256_encrypt, twofish256_encrypt, twofish);
DEF_XTS_PROC(xts_twofish_decrypt, twofish256_encrypt, twofish256_decrypt, twofish);

#ifdef _M_IX86
 DEF_XTS_PROC(xts_serpent_basic_encrypt, serpent256_encrypt, serpent256_encrypt, serpent);
 DEF_XTS_PROC(xts_serpent_basic_decrypt, serpent256_encrypt, serpent256_decrypt, serpent);
#endif

DEF_XTS_AES_PADLOCK(xts_aes_padlock_encrypt, aes256_padlock_encrypt);
DEF_XTS_AES_PADLOCK(xts_aes_padlock_decrypt, aes256_padlock_decrypt);

#ifdef _M_IX86

static void _stdcall xts_aes_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	unsigned char fpustate[32];
	xts_proc      selected;

	if ( (selected = aes_selected_encrypt) == xts_aes_ni_encrypt )
	{
		if (save_fpu_state(fpustate) >= 0) {
			xts_aes_ni_encrypt(in, out, len, offset, key);
			load_fpu_state(fpustate);
		} else {
			xts_aes_basic_encrypt(in, out, len, offset, key);
		}
	} else {
		selected(in, out, len, offset, key);
	}
}

static void _stdcall xts_aes_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	unsigned char fpustate[32];
	xts_proc      selected;

	if ( (selected = aes_selected_decrypt) == xts_aes_ni_decrypt )
	{
		if (save_fpu_state(fpustate) >= 0) {
			xts_aes_ni_decrypt(in, out, len, offset, key);
			load_fpu_state(fpustate);
		} else {
			xts_aes_basic_decrypt(in, out, len, offset, key);
		}
	} else {
		selected(in, out, len, offset, key);
	}
}

static void _stdcall xts_serpent_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	unsigned char fpustate[32];
	xts_proc      selected = serpent_selected_encrypt;

	if (selected != xts_serpent_basic_encrypt && save_fpu_state(fpustate) >= 0) {
		selected(in, out, len, offset, key);
		load_fpu_state(fpustate);
	} else {
		xts_serpent_basic_encrypt(in, out, len, offset, key);
	}
}

static void _stdcall xts_serpent_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	unsigned char fpustate[32];
	xts_proc      selected = serpent_selected_decrypt;

	if (selected != xts_serpent_basic_decrypt && save_fpu_state(fpustate) >= 0) {
		selected(in, out, len, offset, key);
		load_fpu_state(fpustate);
	} else {
		xts_serpent_basic_decrypt(in, out, len, offset, key);
	}
}

#else
	#define xts_aes_encrypt     aes_selected_encrypt
	#define xts_aes_decrypt     aes_selected_decrypt
	#define xts_serpent_encrypt serpent_selected_encrypt
	#define xts_serpent_decrypt serpent_selected_decrypt
#endif

static void _stdcall xts_aes_twofish_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_twofish_encrypt(in, out, len, offset, key);
	xts_aes_encrypt(out, out, len, offset, key);
}

static void _stdcall xts_aes_twofish_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_aes_decrypt(in, out, len, offset, key);
	xts_twofish_decrypt(out, out, len, offset, key);
}

static void _stdcall xts_twofish_serpent_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_serpent_encrypt(in, out, len, offset, key);
	xts_twofish_encrypt(out, out, len, offset, key);
}

static void _stdcall xts_twofish_serpent_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_twofish_decrypt(in, out, len, offset, key);
	xts_serpent_decrypt(out, out, len, offset, key);
}

static void _stdcall xts_serpent_aes_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_aes_encrypt(in, out, len, offset, key);
	xts_serpent_encrypt(out, out, len, offset, key);
}

static void _stdcall xts_serpent_aes_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_serpent_decrypt(in, out, len, offset, key);
	xts_aes_decrypt(out, out, len, offset, key);
}

static void _stdcall xts_aes_twofish_serpent_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_serpent_encrypt(in, out, len, offset, key);
	xts_twofish_encrypt(out, out, len, offset, key);
	xts_aes_encrypt(out, out, len, offset, key);
}

static void _stdcall xts_aes_twofish_serpent_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
{
	xts_aes_decrypt(in, out, len, offset, key);
	xts_twofish_decrypt(out, out, len, offset, key);
	xts_serpent_decrypt(out, out, len, offset, key);
}

void _stdcall xts_set_key(const unsigned char *key, int alg, xts_key *skey)
{
	switch (alg) 
	{
		case CF_AES:
			aes256_asm_set_key(key, &skey->crypt_k.aes);
			aes256_asm_set_key(key + XTS_KEY_SIZE, &skey->tweak_k.aes);

			skey->encrypt = xts_aes_encrypt;
			skey->decrypt = xts_aes_decrypt;
		break;
		case CF_TWOFISH:
			twofish256_set_key(key, &skey->crypt_k.twofish);
			twofish256_set_key(key + XTS_KEY_SIZE, &skey->tweak_k.twofish);

			skey->encrypt = xts_twofish_encrypt;
			skey->decrypt = xts_twofish_decrypt;
		break;
		case CF_SERPENT:
			serpent256_set_key(key, &skey->crypt_k.serpent);
			serpent256_set_key(key + XTS_KEY_SIZE, &skey->tweak_k.serpent);

			skey->encrypt = xts_serpent_encrypt;
			skey->decrypt = xts_serpent_decrypt;
		break;
		case CF_AES_TWOFISH:
			twofish256_set_key(key, &skey->crypt_k.twofish);
			aes256_asm_set_key(key + XTS_KEY_SIZE, &skey->crypt_k.aes);			
			twofish256_set_key(key + XTS_KEY_SIZE*2, &skey->tweak_k.twofish);
			aes256_asm_set_key(key + XTS_KEY_SIZE*3, &skey->tweak_k.aes);

			skey->encrypt = xts_aes_twofish_encrypt;
			skey->decrypt = xts_aes_twofish_decrypt;
		break;
		case CF_TWOFISH_SERPENT:
			serpent256_set_key(key, &skey->crypt_k.serpent);
			twofish256_set_key(key + XTS_KEY_SIZE, &skey->crypt_k.twofish);			
			serpent256_set_key(key + XTS_KEY_SIZE*2, &skey->tweak_k.serpent);
			twofish256_set_key(key + XTS_KEY_SIZE*3, &skey->tweak_k.twofish);

			skey->encrypt = xts_twofish_serpent_encrypt;
			skey->decrypt = xts_twofish_serpent_decrypt;
		break;
		case CF_SERPENT_AES:
			aes256_asm_set_key(key, &skey->crypt_k.aes);
			serpent256_set_key(key + XTS_KEY_SIZE, &skey->crypt_k.serpent);			
			aes256_asm_set_key(key + XTS_KEY_SIZE*2, &skey->tweak_k.aes);
			serpent256_set_key(key + XTS_KEY_SIZE*3, &skey->tweak_k.serpent);

			skey->encrypt = xts_serpent_aes_encrypt;
			skey->decrypt = xts_serpent_aes_decrypt;
		break;
		case CF_AES_TWOFISH_SERPENT:
			serpent256_set_key(key, &skey->crypt_k.serpent);
			twofish256_set_key(key + XTS_KEY_SIZE, &skey->crypt_k.twofish);
			aes256_asm_set_key(key + XTS_KEY_SIZE*2, &skey->crypt_k.aes);
			serpent256_set_key(key + XTS_KEY_SIZE*3, &skey->tweak_k.serpent);
			twofish256_set_key(key + XTS_KEY_SIZE*4, &skey->tweak_k.twofish);
			aes256_asm_set_key(key + XTS_KEY_SIZE*5, &skey->tweak_k.aes);

			skey->encrypt = xts_aes_twofish_serpent_encrypt;
			skey->decrypt = xts_aes_twofish_serpent_decrypt;
		break;
	}
}

#ifdef _M_IX86
long save_fpu_state(unsigned char state[32]) {
	//if (KeGetCurrentIrql() > DISPATCH_LEVEL) return STATUS_UNSUCCESSFUL;
	//return KeSaveFloatingPointState((PKFLOATING_SAVE)state);
	return 1;
}
void load_fpu_state(unsigned char state[32]) {
	//KeRestoreFloatingPointState((PKFLOATING_SAVE)state);
}
#endif

int _declspec(noinline) _stdcall xts_aes_ni_available()
{
	int           CPUInfo[4], res = 0;
	__m128i       enc;
#ifdef _M_IX86
	unsigned char fpustate[32];
#endif

	// check for AES-NI support via CPUID.01H:ECX.AES[bit 25]
	__cpuid(CPUInfo, 1);
	if ( CPUInfo[2] & 0x02000000 ) return 1;

	// Special workaround for AES-NI on Hyper-V server and virtual machines
	if ( (CPUInfo[2] & 0x80000000) == 0 ) return 0;
	__cpuid(CPUInfo, 0x40000000);
	if ( CPUInfo[1] != 'rciM' || CPUInfo[2] != 'foso' || CPUInfo[3] != 'vH t' ) return 0;

#ifdef _M_IX86
	if (save_fpu_state(fpustate) >= 0)
	{
#endif
		//__try {
			enc = _mm_aesenc_si128(_mm_set_epi32(0,1,2,3), _mm_set_epi32(4,5,6,7));
			res = enc.m128i_u64[0] == 0x5f77774d4b7b7b54 && enc.m128i_u64[1] == 0x63636367427c7c58;
		//}
		//__except(/*EXCEPTION_EXECUTE_HANDLER*/ 1) {
		//	res = 0;
		//}
#ifdef _M_IX86
		load_fpu_state(fpustate);
	}
#endif
	return res;
}

int _stdcall xts_init(int hw_crypt)
{
#ifdef _M_IX86
	if (xts_serpent_sse2_available() != 0) {
		serpent_selected_encrypt = xts_serpent_sse2_encrypt;
		serpent_selected_decrypt = xts_serpent_sse2_decrypt;		
	} else {
		serpent_selected_encrypt = xts_serpent_basic_encrypt;
		serpent_selected_decrypt = xts_serpent_basic_decrypt;
	}
#else
	serpent_selected_encrypt = xts_serpent_sse2_encrypt;
	serpent_selected_decrypt = xts_serpent_sse2_decrypt;
#endif
	if (xts_serpent_avx_available() != 0) {
		serpent_selected_encrypt = xts_serpent_avx_encrypt;
		serpent_selected_decrypt = xts_serpent_avx_decrypt;
	}
	if ( hw_crypt != 0 && xts_aes_ni_available() != 0 ) {
		aes_selected_encrypt = xts_aes_ni_encrypt;
		aes_selected_decrypt = xts_aes_ni_decrypt;
		return 1;
	}
	if ( hw_crypt != 0 && aes256_padlock_available() != 0 )
	{
		aes_selected_encrypt = xts_aes_padlock_encrypt;
		aes_selected_decrypt = xts_aes_padlock_decrypt;
		return 2;
	}
	aes_selected_encrypt = xts_aes_basic_encrypt;
	aes_selected_decrypt = xts_aes_basic_decrypt;
	return 0;
}