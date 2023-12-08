#pragma once

#define UPDATE_DOMAIN		"sandboxie-plus.com"

#define UPDATE_FILE			"update.json"
#define ADDONS_FILE			"addons.json"
#define ADDONS_PATH			"\\addons\\"


#define SCOPE_CORE_FILES	L"32\\SbieDll.dll\0"\
							L"32\\SbieSvc.exe\0"\
							L"64\\SbieDll.dll\0"\
							L"KmdUtil.exe\0"\
							L"Sandboxie*.exe\0"\
							L"SbieCtrl.exe\0"\
							L"SbieIni.exe\0"\
							L"SbieSvc.exe\0"\
							L"Start.exe\0"\
							L"SbieDll.dll\0"\
							L"SboxHostDll.dll\0"\
							L"SbieDrv.sys\0"\
							L"Manifest?.txt\0"
#define SCOPE_LANG_FILES	L"SbieMsg.dll\0"\
							L"translations.7z\0"
#define SCOPE_TMPL_FILES	L"Templates.ini\0"


#define ERROR_INVALID		(-1)	// invalid parameter
#define ERROR_GET			(-2)	// failed to download update information
#define ERROR_LOAD			(-3)	// failed to load update json file
#define ERROR_DOWNLOAD		(-4)	// failed to download a particular file
#define ERROR_SCAN			(-5)	// failed to scan existing installation
#define ERROR_SIGN			(-6)	// update signature is invalid !!!
#define ERROR_HASH			(-7)	// downloaded file is corrupted
#define ERROR_EXEC			(-8)
#define ERROR_CANCELED		(-9)
#define ERROR_INTERNAL		(-10)	// internal error, should not happen
#define ERROR_NO_ADDON		(-11)	// addon not found
#define ERROR_NO_ADDON2		(-12)	// no addon for this platform or framework
#define ERROR_BAD_ADDON		(-13)
#define ERROR_BAD_ADDON2	(-14)
#define ERROR_DELETE		(-15)
#define ERROR_GET_CERT		(-16)


