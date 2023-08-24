#ifndef _SHA512_PKCS5_2_H_
#define _SHA512_PKCS5_2_H_

void _stdcall sha512_pkcs5_2(int i_count, const void *pwd, size_t pwd_len, const void *salt, size_t salt_len, unsigned char *dk, size_t dklen);

#endif