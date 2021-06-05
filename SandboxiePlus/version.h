#pragma once

#define VERSION_MJR		0
#define VERSION_MIN 	7
#define VERSION_REV 	5
#define VERSION_UPD 	0

#ifndef STR
#define STR2(X) #X
#define STR(X) STR2(X)
#endif

#define VERSION_BIN VERSION_MJR,VERSION_MIN,VERSION_REV,VERSION_UPD
#define VERSION_STR STR(VERSION_MJR.VERSION_MIN.VERSION_REV.VERSION_UPD)

#define MY_PRODUCT_NAME_STRING  "Sandboxie-Plus"
#define MY_COMPANY_NAME_STRING  "sandboxie-plus.com"
#define MY_COPYRIGHT_STRING     "Copyright (C) 2020-2021 by David Xanatos (xanasoft.com)"

