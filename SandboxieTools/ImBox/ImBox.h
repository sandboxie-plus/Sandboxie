#pragma once

#include "resource.h"

#define ERR_OK				0
#define ERR_UNKNOWN_TYPE	1
#define ERR_FILE_NOT_OPENED	2
#define ERR_UNKNOWN_CIPHER	3
#define ERR_WRONG_PASSWORD	4
#define ERR_KEY_REQUIRED	5
#define ERR_PRIVILEGE		6
#define ERR_INTERNAL		7
#define ERR_FILE_MAPPING	8
#define ERR_CREATE_EVENT	9
#define ERR_IMDISK_FAILED	10
#define ERR_IMDISK_TIMEOUT	11
#define ERR_UNKNOWN_COMMAND	12
#define ERR_MALLOC_ERROR	13
#define ERR_INVALID_PARAM	14
#define ERR_DATA_TO_LONG	15
#define ERR_DATA_NOT_FOUND	16

#define DC_MAX_PASSWORD 128

struct SSection
{
	union {
		struct {
			WCHAR pass[DC_MAX_PASSWORD + 1]; // 0 terminated
		} in;
		struct {
			WCHAR mount[MAX_PATH + 1]; // 0 terminated
		} out;
		UCHAR buffer[0x400 - 8]; // 1KB
	};
	ULONG magic; // = 'dcsp'
	USHORT id;
	USHORT size;
	BYTE data[1024];
};

#define SECTION_MAGIC 'dcsp'
#define SECTION_PARAM_ID_KEY		0x0001
#define SECTION_PARAM_ID_DATA		0x0002

const int SSection_test_offset = FIELD_OFFSET(SSection, data);
const int SSection_test_size = sizeof(SSection);