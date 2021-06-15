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

void xrLightStubHemi()
{
    for (u32 _z = 0, zend = gl_data.slots_data.size_z(); _z < zend; _z++) {
        for (u32 _x = 0, xend = gl_data.slots_data.size_x(); _x < xend; _x++) {
            DetailSlot& DS = gl_data.slots_data.get_slot(_x, _z);
            DS.c_hemi = 15;
        }
    }
}

void xrCompiler()
{
	Phase		("Loading level...");
	gl_data.xrLoad	();

	bool noLighting = gl_data.b_norgb && gl_data.b_nosun && gl_data.b_nohemi;
    if (!noLighting) {
		Phase		("Lighting nodes...");
		xrLight		();
    } else {
        Phase("Stub hemi...");
        xrLightStubHemi();
    }

	gl_data.slots_data.Free();
	
}
