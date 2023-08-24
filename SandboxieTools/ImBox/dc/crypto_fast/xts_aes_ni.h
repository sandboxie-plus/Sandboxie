#ifndef _XTS_AES_NI_H_
#define _XTS_AES_NI_H_

void _stdcall xts_aes_ni_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);
void _stdcall xts_aes_ni_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key);

#endif