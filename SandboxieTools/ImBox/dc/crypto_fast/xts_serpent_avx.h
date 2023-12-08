#ifndef _XTS_SERPENT_AVX_H_
#define _XTS_SERPENT_AVX_H_

int  _stdcall xts_serpent_avx_available();
void _stdcall xts_serpent_avx_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);
void _stdcall xts_serpent_avx_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);

#endif