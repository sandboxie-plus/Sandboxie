#ifndef _XTS_SERPENT_SSE2_H_
#define _XTS_SERPENT_SSE2_H_

int  _stdcall xts_serpent_sse2_available();
void _stdcall xts_serpent_sse2_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);
void _stdcall xts_serpent_sse2_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);

#endif