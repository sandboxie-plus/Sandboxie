/*#include <windows.h>
#include <stdlib.h>
#include <stdio.h>*/

size_t b64_encoded_size(size_t inlen)
{
	size_t ret;

	ret = inlen;
	if (inlen % 3 != 0)
		ret += 3 - (inlen % 3);
	ret /= 3;
	ret *= 4;

	return ret;
}

const wchar_t b64_chars[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int b64_encode(const unsigned char *in, size_t inlen, wchar_t *out, size_t outlen)
{
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || inlen == 0)
		return 0;

	for (i=0, j=0; i<inlen; i+=3, j+=4) {
		v = in[i];
		v = i+1 < inlen ? v << 8 | in[i+1] : v << 8;
		v = i+2 < inlen ? v << 8 | in[i+2] : v << 8;

		out[j]   = b64_chars[(v >> 18) & 0x3F];
		out[j+1] = b64_chars[(v >> 12) & 0x3F];
		if (i+1 < inlen) {
			out[j+2] = b64_chars[(v >> 6) & 0x3F];
		} else {
			out[j+2] = L'=';
		}
		if (i+2 < inlen) {
			out[j+3] = b64_chars[v & 0x3F];
		} else {
			out[j+3] = L'=';
		}
	}

	return 1;
}

size_t b64_decoded_size(const wchar_t *in)
{
	size_t len;
	size_t ret;
	size_t i;

	if (in == NULL)
		return 0;

	len = wcslen(in);
	ret = len / 4 * 3;

	for (i=len; i-->0; ) {
		if (in[i] == L'=') {
			ret--;
		} else {
			break;
		}
	}

	return ret;
}

//static void b64_generate_decode_table()
//{
//	int    inv[80];
//	size_t i;
//
//	memset(inv, -1, sizeof(inv));
//	for (i=0; i<sizeof(b64_chars)-1; i++) {
//		inv[b64_chars[i]-43] = i;
//	}
//}

int b64_invs[] = { 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
	21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51 };

static int b64_isvalidchar(wchar_t c)
{
	if (c >= L'0' && c <= L'9')
		return 1;
	if (c >= L'A' && c <= L'Z')
		return 1;
	if (c >= L'a' && c <= L'z')
		return 1;
	if (c == L'+' || c == L'/' || c == L'=')
		return 1;
	return 0;
}

int b64_decode(const wchar_t *in, unsigned char *out, size_t outlen)
{
	size_t len;
	size_t i;
	size_t j;
	int    v;

	if (in == NULL || out == NULL)
		return 0;

	len = wcslen(in);
	if (outlen < b64_decoded_size(in) || len % 4 != 0)
		return 0;

	for (i=0; i<len; i++) {
		if (!b64_isvalidchar(in[i])) {
			return 0;
		}
	}

	for (i=0, j=0; i<len; i+=4, j+=3) {
		v = b64_invs[in[i]-43];
		v = (v << 6) | b64_invs[in[i+1]-43];
		v = in[i+2]=='=' ? v << 6 : (v << 6) | b64_invs[in[i+2]-43];
		v = in[i+3]=='=' ? v << 6 : (v << 6) | b64_invs[in[i+3]-43];

		out[j] = (v >> 16) & 0xFF;
		if (in[i+2] != '=')
			out[j+1] = (v >> 8) & 0xFF;
		if (in[i+3] != '=')
			out[j+2] = v & 0xFF;
	}

	return 1;
}

