#include "stdafx.h"
#pragma hdrstop

#include "ESceneAIMapTools.h"
#include "ui_leveltools.h"
#include "ESceneAIMapControls.h"
#include "FrameAIMap.h"

void ESceneAIMapTool::CreateControls()
{
	inherited::CreateDefaultControls(estDefault);
	// node tools
//    AddControl(xr_new<TUI_ControlAIMapNodeSelect>	(estDefault,		etaSelect, 	this));
	AddControl(xr_new<TUI_ControlAIMapNodeAdd>		(estDefault,		etaAdd, 	this));
	AddControl(xr_new<TUI_ControlAIMapNodeMove>		(estDefault,		etaMove,	this));
    AddControl(xr_new<TUI_ControlAIMapNodeRotate>	(estDefault,		etaRotate,	this));
	// frame
    pFrame 			= xr_new<TfraAIMap>((TComponent*)0,this);
}
//----------------------------------------------------

void ESceneAIMapTool::RemoveControls()
{
	inherited::RemoveControls();
}
//----------------------------------------------------

