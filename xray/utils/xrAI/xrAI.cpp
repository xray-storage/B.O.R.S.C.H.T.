// xrAI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../../xrcore/xr_ini.h"
#include "process.h"
#include "xrAI.h"

#include "xr_graph_merge.h"
#include "game_spawn_constructor.h"
#include "xrCrossTable.h"
//#include "path_test.h"
#include "game_graph_builder.h"
#include <mmsystem.h>
#include "spawn_patcher.h"

extern LPCSTR LEVEL_GRAPH_NAME;

extern void	xrCompiler			(LPCSTR name, bool draft_mode, bool pure_covers, LPCSTR out_name, u32 numThread, bool useTbb);
extern void logThread			(void *dummy);
extern volatile BOOL bClose;
extern void test_smooth_path	(LPCSTR name);
extern void test_hierarchy		(LPCSTR name);
extern void	xrConvertMaps		();
extern void	test_goap			();
extern void	smart_cover			(LPCSTR name);
extern void	verify_level_graph	(LPCSTR name, bool verbose);
//extern void connectivity_test	(LPCSTR);
extern void compare_graphs		(LPCSTR level_name);
extern void test_levels			();

static const char* h_str = 
	"The following keys are supported / required:\n"
	"-? or -h   == this help\n"
	"-f<NAME>   == compile level in gamedata/levels/<NAME>/\n"
	"-o         == modify build options\n"
	"-s         == build game spawn data\n"
	"-silent    == suppress congratulation message\n"
	"-verify\n"
	"-out\n"
	"-draft\n"
	"-pure_covers\n"
	"-start\n"
	"-no_separator_check\n"
	"-thread <COUNT> == multi-threaded cover calculation\n"
	"-use_tbb   == cover calculation use Intel TBB";

void Help()
{	MessageBox(0,h_str,"Command line options",MB_OK|MB_ICONINFORMATION); }

string_path INI_FILE;

extern  HWND logWindow;

extern LPCSTR GAME_CONFIG;

extern void clear_temp_folder	();

void execute	(LPSTR cmd)
{
	// Load project
	string4096 name;
	name[0]=0; 
	if (strstr(cmd,"-f"))
		sscanf	(strstr(cmd,"-f")+2,"%s",name);
	else if (strstr(cmd,"-s"))
		sscanf	(strstr(cmd,"-s")+2,"%s",name);
	else if (strstr(cmd,"-t"))
		sscanf	(strstr(cmd,"-t")+2,"%s",name);
	else if (strstr(cmd,"-verify"))
		sscanf	(strstr(cmd,"-verify")+xr_strlen("-verify"),"%s",name);

	if (xr_strlen(name))
		strcat			(name,"\\");

	string_path			prjName;
	prjName				[0] = 0;
	bool				can_use_name = false;
	if (xr_strlen(name) < sizeof(string_path)) {
		can_use_name	= true;
		FS.update_path	(prjName,"$game_levels$",name);
	}

	FS.update_path		(INI_FILE,"$game_config$",GAME_CONFIG);
	
	if (strstr(cmd,"-f")) {
		R_ASSERT3		(can_use_name,"Too big level name",name);
		
		char			*output = strstr(cmd,"-out");
		string256		temp0;
		if (output) {
			output		+= xr_strlen("-out");
			sscanf		(output,"%s",temp0);
			_TrimLeft	(temp0);
			output		= temp0;
		}
		else
			output		= (pstr)LEVEL_GRAPH_NAME;

		const char* threadOption = strstr(cmd, "-thread");
		u32 numThread = 0;
		if (threadOption)
			sscanf(threadOption + strlen("-thread"), "%lu", &numThread);
		if (numThread == 0 || numThread > 256) {
			numThread = CPU::ID.threadCount;
		}

		xrCompiler(prjName, !!strstr(cmd,"-draft"), !!strstr(cmd,"-pure_covers"), output, numThread, !!strstr(cmd, "-use_tbb"));
	}
	else if (strstr(cmd,"-s")) {
		if (xr_strlen(name))
			name[xr_strlen(name) - 1] = 0;
		char				*output = strstr(cmd,"-out");
		string256			temp0, temp1;
		if (output) {
			output			+= xr_strlen("-out");
			sscanf			(output,"%s",temp0);
			_TrimLeft		(temp0);
			output			= temp0;
		}
		char				*start = strstr(cmd,"-start");
		if (start) {
			start			+= xr_strlen("-start");
			sscanf			(start,"%s",temp1);
			_TrimLeft		(temp1);
			start			= temp1;
		}
		char				*no_separator_check = strstr(cmd,"-no_separator_check");
		CGameSpawnConstructor(name,output,start,!!no_separator_check);
		clear_temp_folder	();
	}
	else if (strstr(cmd,"-verify")) {
		R_ASSERT3			(can_use_name,"Too big level name",name);
		verify_level_graph	(prjName,!strstr(cmd,"-noverbose"));
	}
}

void Startup(LPSTR     lpCmdLine)
{
	string4096 cmd;
	BOOL bModifyOptions		= FALSE;

	strcpy_s(cmd,lpCmdLine);
	strlwr(cmd);
	if (strstr(cmd,"-?") || strstr(cmd,"-h"))			{ Help(); return; }
	if ((strstr(cmd,"-f")==0) && (strstr(cmd,"-g")==0) && (strstr(cmd,"-m")==0) && (strstr(cmd,"-s")==0) && (strstr(cmd,"-t")==0) && (strstr(cmd,"-c")==0) && (strstr(cmd,"-verify")==0) && (strstr(cmd,"-patch")==0))	{ Help(); return; }
	if (strstr(cmd,"-o"))								bModifyOptions = TRUE;

	// Give a LOG-thread a chance to startup
	InitCommonControls	();
	Sleep				(150);
	thread_spawn		(logThread,	"log-update", 1024*1024,0);
	while				(!logWindow)	Sleep		(150);
	
	u32					dwStartupTime	= timeGetTime();
	execute				(cmd);
	// Show statistic
	char				stats[256];
	extern				std::string make_time(u32 sec);
	extern				HWND logWindow;
	u32					dwEndTime = timeGetTime();
	sprintf				(stats,"Time elapsed: %s",make_time((dwEndTime-dwStartupTime)/1000).c_str());
	clMsg("Build succesful!\n%s", stats);

	if (!strstr(cmd, "-silent"))
		MessageBox			(logWindow,stats,"Congratulation!",MB_OK|MB_ICONINFORMATION);

	bClose				= TRUE;
	FlushLog			();
	Sleep				(500);
}

#include "factory_api.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	Debug._initialize		(false);
	Core._initialize		("xrAI");
	factory_init();

	Msg("Command line: '%s'\n", lpCmdLine);
	Startup					(lpCmdLine);

	factory_done();
	Core._destroy			();

	return					(0);
}