#ifndef rc4_INCLUDED
#  define rc4_INCLUDED

typedef struct rc4_sbox_s
{
	unsigned char state[256]; 
	unsigned int x;
	unsigned int y;
} rc4_sbox_t;

#ifdef __cplusplus
extern "C" 
{
#endif

void rc4_init(rc4_sbox_t *rc4_sbox, const unsigned char *key_ptr, unsigned int key_len);

void rc4_transform(rc4_sbox_t *rc4_sbox, unsigned char *buffer_ptr, unsigned int buffer_len);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* rc4_INCLUDED */
