// xrLC.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "math.h"
#include "build.h"
#include "../xrLC_Light/xrLC_GlobalData.h"
#include "cl_log.h"
#include "../xrLC_Light/xrDeflector.h"

#define PROTECTED_BUILD

#ifdef PROTECTED_BUILD
#	define TRIVIAL_ENCRYPTOR_ENCODER
#	define TRIVIAL_ENCRYPTOR_DECODER
#	include "../../xrEngine/trivial_encryptor.h"
#	undef TRIVIAL_ENCRYPTOR_ENCODER
#	undef TRIVIAL_ENCRYPTOR_DECODER
#endif // PROTECTED_BUILD

CBuild*	pBuild		= NULL;
u32		version		= 0;

extern void logThread(void *dummy);
extern volatile BOOL bClose;

static const char* h_str = 
	"The following keys are supported / required:\n"
	"-? or -h      == this help\n"
	"-o            == modify build options\n"
	"-nosun        == disable sun-lighting\n"
	"-norgb        == disable common lightmap calculating\n"
	"-nolmaps      == disable lightmaps calculating\n"
	"-skipinvalid  == skip crash if invalid faces exists\n"
	"-lmap_quality == lightmap quality\n"
	"-lmap_rgba    == save lightmaps with lossless format\n"
	"-thread <COUNT> == multi-threaded light implicit\n"
	"-skip_aht_tess == skip Adaptive HT Tesselating stage\n"
	"-cform_only   == make *.cform only\n"
	"-use_tbb      == cover calculation use Intel TBB\n"
	"-f <NAME>     == compile level in GameData\\Levels\\<NAME>\\\n"
	"\n"
	"NOTE: The last key is required for any functionality\n";

void Help()
{
	MessageBox(0,h_str,"Command line options",MB_OK|MB_ICONINFORMATION);
}

// computing build id
XRCORE_API	LPCSTR	build_date;
XRCORE_API	u32		build_id;
static LPSTR month_id[12] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
};

static int days_in_month[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int start_day = 31;	// 31
static int start_month = 1;	// January
static int start_year = 1999;	// 1999

void compute_build_id()
{
	build_date = __DATE__;

	int					days;
	int					months = 0;
	int					years;
	string16			month;
	string256			buffer;
	strcpy_s(buffer, __DATE__);
	sscanf(buffer, "%s %d %d", month, &days, &years);

	for (int i = 0; i<12; i++) {
		if (_stricmp(month_id[i], month))
			continue;

		months = i;
		break;
	}

	build_id = (years - start_year) * 365 + days - start_day;

	for (int i = 0; i<months; ++i)
		build_id += days_in_month[i];

	for (int i = 0; i<start_month - 1; ++i)
		build_id -= days_in_month[i];
}

typedef int __cdecl xrOptions(b_params* params, u32 version, bool bRunBuild);

void logParams(const b_params& param)
{
    Msg("light_pixel_per_meter = %.1f", param.m_lm_pixels_per_meter);
    Msg("light_jitter_samples  = %lu", param.m_lm_jitter_samples);
    Msg("light_quality         = %u", param.m_quality);
}

void Startup(LPSTR     lpCmdLine)
{
	
	create_global_data();
	char cmd[512],name[256];
	BOOL bModifyOptions		= FALSE;

	strcpy_s(cmd,lpCmdLine);
	strlwr(cmd);
	if (strstr(cmd,"-?") || strstr(cmd,"-h"))			{ Help(); return; }
	if (strstr(cmd,"-f")==0)							{ Help(); return; }
	if (strstr(cmd,"-o"))								bModifyOptions	= TRUE;
	if (strstr(cmd,"-gi"))								b_radiosity		= TRUE;
	if (strstr(cmd,"-noise"))							b_noise			= TRUE;
	if (strstr(cmd,"-net"))								b_net_light		= TRUE;
	VERIFY( lc_global_data() );
	lc_global_data()->b_nosun_set						( !!strstr(cmd,"-nosun") );
	//if (strstr(cmd,"-nosun"))							b_nosun			= TRUE;
	lc_global_data()->b_norgb_set(strstr(cmd, "-norgb") != nullptr);
	lc_global_data()->b_no_lmaps_set(strstr(cmd, "-nolmaps") != nullptr);
	lc_global_data()->b_skip_invalid_set(strstr(cmd, "-skipinvalid") != nullptr);
	lc_global_data()->b_lmap_rgba_set(strstr(cmd, "-lmap_rgba") != nullptr);
	lc_global_data()->setSkipAhtTesselate(strstr(cmd, "-skip_aht_tess") != nullptr);
	lc_global_data()->setCformOnly(strstr(cmd, "-cform_only"));
	lc_global_data()->setUseTbb(strstr(cmd, "-use_tbb"));
	const char* threadOption = strstr(cmd, "-thread");
	u32 numThread = 0;
	if (threadOption)
		sscanf(threadOption + strlen("-thread"), "%lu", &numThread);
	if (numThread == 0 || numThread > 256) {
		numThread = CPU::ID.threadCount;
	}
	lc_global_data()->setNumThread(numThread);
	
	// Give a LOG-thread a chance to startup
	//_set_sbh_threshold(1920);
	InitCommonControls		();
	thread_spawn			(logThread, "log-update",	1024*1024,0);
	Sleep					(150);
	
	// Faster FPU 
	SetPriorityClass		(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);

	/*
	u32	dwMin			= 1800*(1024*1024);
	u32	dwMax			= 1900*(1024*1024);
	if (0==SetProcessWorkingSetSize(GetCurrentProcess(),dwMin,dwMax))
	{
		clMsg("*** Failed to expand working set");
	};
	*/
	
	// Load project
	name[0]=0;				sscanf(strstr(cmd,"-f")+2,"%s",name);

	extern  HWND logWindow;
	string256				temp;
	sprintf_s				(temp, "%s - Levels Compiler", name);
	SetWindowText			(logWindow, temp);

	string_path				prjName;
	FS.update_path			(prjName,"$game_levels$",strconcat(sizeof(prjName),prjName,name,"\\build.prj"));
	string256				phaseName;
	Phase					(strconcat(sizeof(phaseName),phaseName,"Reading project [",name,"]..."));

	string256 inf;
	IReader*	F			= FS.r_open(prjName);
	if (NULL==F){
		sprintf				(inf,"Build failed!\nCan't find level: '%s'",name);
		clMsg				(inf);
		MessageBox			(logWindow,inf,"Error!",MB_OK|MB_ICONERROR);
		return;
	}

	// Version
	F->r_chunk			(EB_Version,&version);
	clMsg				("version: %d",version);
	R_ASSERT(XRCL_CURRENT_VERSION==version);

	// Header
	b_params				Params;
	F->r_chunk			(EB_Parameters,&Params);

	const char* quality = strstr(cmd, "-lmap_quality ");
	if (quality) {
		int sz = xr_strlen("-lmap_quality ");
		sscanf(quality + sz, "%f", &Params.m_lm_pixels_per_meter);
	}

	logParams(Params);

	// Show options if needed
	if (bModifyOptions)		
	{
		Phase		("Project options...");
		HMODULE		L = LoadLibrary		("xrLC_Options.dll");
		void*		P = GetProcAddress	(L,"_frmScenePropertiesRun");
		R_ASSERT	(P);
		xrOptions*	O = (xrOptions*)P;
		int			R = O(&Params,version,false);
		FreeLibrary	(L);
		if (R==2)	{
			ExitProcess(0);
		}
	}
	
	// Conversion
	Phase					("Converting data structures...");
	pBuild					= xr_new<CBuild>();
	pBuild->Load			(Params,*F);
	FS.r_close				(F);
	
	// Call for builder
	string_path				lfn;
	CTimer	dwStartupTime;	dwStartupTime.Start();
	FS.update_path			(lfn,_game_levels_,name);
	pBuild->Run				(lfn);
	xr_delete				(pBuild);

	// Show statistic
	Msg						("Phase times:");
	LogPhaseTimes			();
    LogCounters				();
	Msg						("");

	u32	dwEndTime			= dwStartupTime.GetElapsed_ms();
	sprintf					(inf,"Time elapsed: %s",make_time(dwEndTime/1000).c_str());
	Msg						("------------------------------------------\n%s",inf);
	Msg						("Build succesful!");

	if (!strstr(cmd,"-silent"))
		MessageBox			(logWindow,inf,"Congratulation!",MB_OK|MB_ICONINFORMATION);

	// Close log
	bClose					= TRUE;
	Sleep					(500);
}

//typedef void DUMMY_STUFF (const void*,const u32&,void*);
//XRCORE_API DUMMY_STUFF	*g_temporary_stuff;
//XRCORE_API DUMMY_STUFF	*g_dummy_stuff;



int APIENTRY WinMain(HINSTANCE hInst,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    Log_WriteTimestamp();
//	g_temporary_stuff	= &trivial_encryptor::decode;
//	g_dummy_stuff		= &trivial_encryptor::encode;

	// Initialize debugging
	Debug._initialize	(false);
	compute_build_id();
	Core._initialize	("xrLC");

	if(strstr(Core.Params,"-nosmg"))
		g_using_smooth_groups = false;

	Msg("Command line: '%s'\n", lpCmdLine);
	Startup				(lpCmdLine);
	Core._destroy		();
	
	return 0;
}
