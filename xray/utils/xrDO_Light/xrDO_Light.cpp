// xrAI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "process.h"
#include "global_calculation_data.h"
#include "cl_log.h"

extern void	xrCompiler			();
extern void logThread			(void *dummy);
extern volatile BOOL bClose;

static const char* h_str = 
	"The following keys are supported / required:\n"
	"-? or -h   == this help\n"
	"-o         == modify build options\n"
	"-f<NAME>   == compile level in gamedata\\levels\\<NAME>\\\n"
	"-norgb     == disable common lightmap calculating. Default: false\n"
	"-nosun     == disable sun-lighting. Default: false\n"
	"-nohemi    == disable hemi-lighting. Default: false\n"
	"-thread <COUNT> == number of threads for lighting calculation. Default: number of processor cores\n"
	"-samples <COUNT> == number of samples for detail slot. Calculated as (2*<COUNT> + 1)^2. Default: 7\n"
	"-silent    == supress congratulation message\n"
	"\n"
	"NOTE: The last key is required for any functionality\n";

void Help()
{	MessageBox(0,h_str,"Command line options",MB_OK|MB_ICONINFORMATION); }

void Startup(LPSTR     lpCmdLine)
{
	char cmd[512],name[256];
	BOOL bModifyOptions		= FALSE;

	strcpy_s(cmd,lpCmdLine);
	strlwr(cmd);
	if (strstr(cmd,"-?") || strstr(cmd,"-h"))			{ Help(); return; }
	if (strstr(cmd,"-f")==0)							{ Help(); return; }
	if (strstr(cmd,"-o"))								bModifyOptions = TRUE;
	if (strstr(cmd, "-norgb"))							gl_data.b_norgb = true;
	if (strstr(cmd, "-nosun"))							gl_data.b_nosun = true;
	if (strstr(cmd, "-nohemi"))							gl_data.b_nohemi = true;
	const char* threadOption = strstr(cmd, "-thread");
	u32 numThread = 0;
	if (threadOption)
		sscanf(threadOption + strlen("-thread"), "%lu", &numThread);
	if (numThread == 0 || numThread > 256) {
		numThread = CPU::ID.threadCount;
	}
	gl_data.numThread = numThread;
	const char* sampleOption = strstr(cmd, "-samples");
	gl_data.sampleCount = 0;
	if (sampleOption)
		sscanf(sampleOption + strlen("-samples"), "%d", &gl_data.sampleCount);
	if (gl_data.sampleCount < 0 || gl_data.sampleCount > 7) {
		gl_data.sampleCount = 7;
	}

	// Give a LOG-thread a chance to startup
	InitCommonControls	();
	thread_spawn		(logThread,	"log-update", 1024*1024,0);
	Sleep				(150);
	
	// Load project
	name[0]=0; sscanf	(strstr(cmd,"-f")+2,"%s",name);

	extern  HWND logWindow;
	string256			temp;
	sprintf_s			(temp, "%s - Detail Compiler", name);
	SetWindowText		(logWindow, temp);

	//FS.update_path	(name,"$game_levels$",name);
	FS.get_path			("$level$")->_set	(name);

	CTimer				dwStartupTime; dwStartupTime.Start();
	xrCompiler			();

	// Show statistic
	u32 dwEndTime = dwStartupTime.GetElapsed_ms();
	sprintf(temp, "Time elapsed: %s", make_time(dwEndTime / 1000).c_str());
	Msg("------------------------------------------\n%s", temp);
	Msg("Build succesful!");

	if (!strstr(cmd,"-silent"))
		MessageBox		(logWindow,temp,"Congratulation!",MB_OK|MB_ICONINFORMATION);

	bClose				= TRUE;
	Sleep				(500);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Initialize debugging
	Debug._initialize	(false);
	Core._initialize	("xrDO");
	Msg("Command line: '%s'\n", lpCmdLine);
	Startup				(lpCmdLine);
	
	return 0;
}
