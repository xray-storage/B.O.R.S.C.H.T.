#pragma once

#include "../../version.h"

#define QRCDKEY_INTEGRATION
#define SB_ICMP_SUPPORT

#undef DEMO_BUILD

//#define DEMO_BUILD

#ifdef DEMO_BUILD
	#define	GAMESPY_GAMENAME				"stalkercsd"
	#define GAMESPY_GAMEID					2264
	#define GAMESPY_PRODUCTID				11528
	#define GAME_VERSION					"1.5.10"
	#define REGISTRY_PATH					"Software\\GSC Game World\\STALKER-STCS_DBG\\"
#else
	#define	GAMESPY_GAMENAME				"stalkercs"
	#define GAMESPY_GAMEID					2264
	#define GAMESPY_PRODUCTID				11528
	#define GAME_VERSION					BORSCHT_APP_VERSION
	#define REGISTRY_PATH					"Software\\GSC Game World\\STALKER-STCS\\"
#endif

#define	GAMESPY_QR2_BASEPORT				5447
#define GAMESPY_BROWSER_MAX_UPDATES			20

#define START_PORT							0
#define END_PORT							65535
#define START_PORT_LAN						5445
#define END_PORT_LAN						START_PORT_LAN + 250//GameSpy only process 500 ports

#define REGISTRY_BASE						HKEY_LOCAL_MACHINE
#define REGISTRY_VALUE_GSCDKEY				"InstallCDKEY"
#define REGISTRY_VALUE_VERSION				"InstallVers"
#define REGISTRY_VALUE_USERNAME				"InstallUserName"
#define REGISTRY_VALUE_SKU					"InstallSource"
#define REGISTRY_VALUE_INSTALL_PATCH_ID		"InstallPatchID"
#define REGISTRY_VALUE_LANGUAGE				"InstallLang"

#define GAMESPY_PATCHING_VERSIONUNIQUE_ID	"test_version_1"
#define GAMESPY_PATCHING_DISTRIBUTION_ID	0

 

#ifndef XRGAMESPY_API
	#ifdef XRGAMESPY_EXPORTS
		#define XRGAMESPY_API				__declspec(dllexport)
	#endif
#endif

#define EXPORT_FN_DECL(r, f, p)		XRGAMESPY_API r xrGS_##f p;

extern void FillSecretKey (char* SecretKey);
