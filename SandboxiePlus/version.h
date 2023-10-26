#pragma once

#define VERSION_MJR		1
#define VERSION_MIN 	12
#define VERSION_REV 	0
#define VERSION_UPD 	2

#ifndef STR
#define STR2(X) #X
#define STR(X) STR2(X)
#endif

#if VERSION_UPD > 0
  #define VERSION_BIN VERSION_MJR,VERSION_MIN,VERSION_REV,VERSION_UPD
  #define VERSION_STR STR(VERSION_MJR.VERSION_MIN.VERSION_REV.VERSION_UPD)
#else
  #define VERSION_BIN VERSION_MJR,VERSION_MIN,VERSION_REV
  #define VERSION_STR STR(VERSION_MJR.VERSION_MIN.VERSION_REV)
#endif

#define MY_PRODUCT_NAME_STRING  "Sandboxie-Plus"
#define MY_COMPANY_NAME_STRING  "sandboxie-plus.com"
#define MY_COPYRIGHT_STRING     "Copyright (C) 2020-2023 by David Xanatos (xanasoft.com)"

