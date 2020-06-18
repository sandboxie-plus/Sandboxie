#include "rc4.h"

void 
rc4_swap(unsigned char &a, unsigned char &b) 
{
	unsigned char c = a; 
	a = b; 
	b = c; 
}

void 
rc4_init(rc4_sbox_t *rc4_sbox, const unsigned char *key_ptr, unsigned int key_len)
{
	rc4_sbox->x = 0; 
	rc4_sbox->y = 0;

	// Initialisation of the permutation
	unsigned int i; 
	for(i = 0; i < 256; i++)
		rc4_sbox->state[i] = (char)i;

	// Mixing permutation
	unsigned int j = 0;
	unsigned int k;
	for(i = 0; i < 256; i++)
	{ 
		k = i % key_len; 

		j = (key_ptr[k] + rc4_sbox->state[i] + j) & 0xff; 
		rc4_swap(rc4_sbox->state[i], rc4_sbox->state[j]);
	} 
}

void 
rc4_transform(rc4_sbox_t *rc4_sbox, unsigned char *buffer_ptr, unsigned int buffer_len)
{ 
	unsigned int i;
	for(i = 0; i < buffer_len; i ++)
	{ 
		// The pseudo-random generation algorithm
		rc4_sbox->x = (rc4_sbox->x + 1) & 0xff;
		rc4_sbox->y = (rc4_sbox->y + rc4_sbox->state[rc4_sbox->x]) & 0xff; 
		rc4_swap(rc4_sbox->state[rc4_sbox->x], rc4_sbox->state[rc4_sbox->y]);
		unsigned char keyChar = rc4_sbox->state[(rc4_sbox->state[rc4_sbox->x] + rc4_sbox->state[rc4_sbox->y]) & 0xff];

		if(buffer_ptr) // NULL when seeking
			buffer_ptr[i] ^= keyChar; 
	} 
}