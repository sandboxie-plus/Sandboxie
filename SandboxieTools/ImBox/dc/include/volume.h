#ifndef _VOLUME_H_
#define _VOLUME_H_

#include "defines.h"

#define DC_VOLUME_SIGN 0x50524344

// Header key derivation
#define PKCS5_SALT_SIZE			64

// Master key + secondary key (LRW mode)
#define DISKKEY_SIZE			256
#define MAX_KEY_SIZE            (32*3)
#define PKCS_DERIVE_MAX         (MAX_KEY_SIZE*2)

#define SECTOR_SIZE                 512
#define MAX_SECTOR_SIZE             2048
#define CD_SECTOR_SIZE              2048

#define MIN_PASSWORD			1		// Minimum password length
#define MAX_PASSWORD			128		// Maximum password length

#define DC_HDR_VERSION 1

#define VF_NONE           0x00
#define VF_TMP_MODE       0x01 /* temporary encryption mode */
#define VF_REENCRYPT      0x02 /* volume re-encryption in progress */
#define VF_STORAGE_FILE   0x04 /* redirected area are placed in file */
#define VF_NO_REDIR       0x08 /* redirection area is not present */
#define VF_EXTENDED       0x10 /* this volume placed on extended partition */

#define ENC_BLOCK_SIZE  (1280 * 1024)

#pragma pack (push, 1)

typedef struct _dc_pass {
	int     size; // in bytes
	wchar_t pass[MAX_PASSWORD];
} dc_pass;

typedef struct _dc_header {
	u8  salt[PKCS5_SALT_SIZE]; /* pkcs5.2 salt */
	u32 sign;                  /* signature 'DCRP' */
	u32 hdr_crc;               /* crc32 of decrypted volume header */
	u16 version;               /* volume format version */
	u32 flags;                 /* volume flags */
	u32 disk_id;               /* unigue volume identifier */
	int alg_1;                 /* crypt algo 1 */
	u8  key_1[DISKKEY_SIZE];   /* crypt key 1  */
	int alg_2;                 /* crypt algo 2 */
	u8  key_2[DISKKEY_SIZE];   /* crypt key 2  */

#pragma warning(disable:4201)
	union {
		u64 stor_off;    /* redirection area offset */
		u64 data_off;    /* volume data offset, if redirection area is not used */
	};
#pragma warning(default:4201)
	u64 use_size;    /* user available volume size */
	u64 tmp_size;    /* temporary part size      */
	u8  tmp_wp_mode; /* data wipe mode */

	u8  reserved[1422 - 1];

} dc_header;

#define IS_INVALID_VOL_FLAGS(_f) ( ((_f) & VF_NO_REDIR) && \
	((_f) & (VF_TMP_MODE | VF_REENCRYPT | VF_STORAGE_FILE)) )

#define IS_INVALID_SECTOR_SIZE(_s) ( ((_s) % SECTOR_SIZE) || \
	((_s) > MAX_SECTOR_SIZE) || (sizeof(dc_header) % (_s)) )


#define DC_AREA_SIZE         (2 * 1024)
#define DC_AREA_SECTORS      (DC_AREA_SIZE / SECTOR_SIZE)
#define DC_ENCRYPTEDDATASIZE (DC_AREA_SIZE - PKCS5_SALT_SIZE)
#define DC_CRC_AREA_SIZE     (DC_ENCRYPTEDDATASIZE - 8)


#pragma pack (pop)


#endif