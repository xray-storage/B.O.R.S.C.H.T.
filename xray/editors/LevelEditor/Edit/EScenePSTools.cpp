#include "stdafx.h"
#pragma hdrstop

#include "EScenePSTools.h"
#include "ui_leveltools.h"
#include "EScenePSControls.h"
#ifndef NO_VCL
#include "FramePS.h"
#endif
#include "EParticlesObject.h"

void EScenePSTool::CreateControls()
{
	inherited::CreateDefaultControls(estDefault);
    AddControl		(xr_new<TUI_ControlPSAdd>(estDefault,etaAdd,		this));
#ifndef NO_VCL
	// frame
    pFrame 			= xr_new<TfraPS>((TComponent*)0);
#endif
}
//----------------------------------------------------

void EScenePSTool::RemoveControls()
{
	inherited::RemoveControls();
}
//----------------------------------------------------

CCustomObject* EScenePSTool::CreateObject(LPVOID data, LPCSTR name)
{
	CCustomObject* O	= xr_new<EParticlesObject>(data,name);
    O->ParentTool		= this;
    return O;
}
//----------------------------------------------------
bool  EScenePSTool::ExportGame(SExportStreams* F)
{
	return inherited::ExportGame	(F);
}

