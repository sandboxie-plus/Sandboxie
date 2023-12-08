#ifndef _DC_HEADER_H_
#define _DC_HEADER_H_

#include "..\volume.h"

//#ifdef _M_IX86
//#define SMALL
//#endif

#ifdef SMALL
#include "..\..\crypto_small\xts_small.h"
#else
#include "..\..\crypto_fast\xts_fast.h"
#endif

int dc_decrypt_header(dc_header *header, dc_pass *password);

#endif