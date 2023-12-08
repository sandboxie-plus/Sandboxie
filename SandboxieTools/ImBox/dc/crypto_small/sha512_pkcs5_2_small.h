#ifndef _SHA512_PKCS5_2_SMALL_H_
#define _SHA512_PKCS5_2_SMALL_H_

void sha512_hmac(const void *k, unsigned long k_len, const void *d, unsigned long d_len, unsigned char *out);
void sha512_pkcs5_2(int i_count, const void *pwd, unsigned long pwd_len, const void *salt, unsigned long salt_len, unsigned char *dk, unsigned long dklen);

#endif