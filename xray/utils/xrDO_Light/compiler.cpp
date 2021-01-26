#include "stdafx.h"
#include "../../xrEngine/xrlevel.h"

#include "xrThread.h"

#include "global_calculation_data.h"
#include "lightthread.h"

void	xrLight			()
{
	u32	range				= gl_data.slots_data.size_z();

	// Start threads, wait, continue --- perform all the work
	CThreadManager		Threads;
	CTimer				start_time;
	start_time.Start();
	u32	stride			= range/gl_data.numThread;
	u32	last			= range-stride*	(gl_data.numThread-1);
	for (u32 thID=0; thID<gl_data.numThread; thID++)	{
		CThread*	T		= xr_new<LightThread> (thID,thID*stride,thID*stride+((thID==(gl_data.numThread-1))?last:stride));
		T->thMessages		= FALSE;
		T->thMonitor		= FALSE;
		Threads.start		(T);
	}
	Threads.wait			();
	Msg						("%d seconds elapsed.",(start_time.GetElapsed_ms())/1000);
}

void xrCompiler()
{
	Phase		("Loading level...");
	gl_data.xrLoad	();

	Phase		("Lighting nodes...");
	xrLight		();

	gl_data.slots_data.Free();
	
}
