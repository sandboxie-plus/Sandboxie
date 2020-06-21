#pragma once

#include "../mischelpers_global.h"

typedef struct MISCHELPERS_EXPORT rc4_sbox_s
{
	unsigned char state[256];
	unsigned int x;
	unsigned int y;
} rc4_sbox_t;

void MISCHELPERS_EXPORT rc4_init(rc4_sbox_t *rc4_sbox, const unsigned char *key_ptr, unsigned int key_len);

void MISCHELPERS_EXPORT rc4_transform(rc4_sbox_t *rc4_sbox, unsigned char *buffer_ptr, unsigned int buffer_len);

void MISCHELPERS_EXPORT rc4_init(rc4_sbox_t *rc4_sbox, const QByteArray& Key);

QByteArray MISCHELPERS_EXPORT rc4_transform(rc4_sbox_t *rc4_sbox, const QByteArray& Data);