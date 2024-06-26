#include "stdafx.h"
#pragma hdrstop

#include "ESceneSpawnControls.h"
#include "ui_leveltools.h"
#include "../ECore/Editor/ui_main.h"
#ifndef NO_VCL
#include "FrameSpawn.h"
#endif
#include "Scene.h"
#include "SpawnPoint.h"

#include "ImGui\IM_LeftBar.h"

using namespace std::placeholders;

//---------------------------------------------------------------------------
TUI_ControlSpawnAdd::TUI_ControlSpawnAdd(int st, int act, ESceneToolBase* parent):TUI_CustomControl(st,act,parent){
}

bool TUI_ControlSpawnAdd::AppendCallback(SBeforeAppendCallbackParams* p)
{
#ifndef NO_VCL
	LPCSTR ref_name = ((TfraSpawn*)parent_tool->pFrame)->Current();
#else
	LPCSTR ref_name = imLeftBar.fraSpawn.Current();
#endif
    if (!ref_name){
    	ELog.DlgMsg(mtInformation,"Nothing selected.");
    	return false;
    }
    if(Scene->LevelPrefix().c_str())
    {
		p->name_prefix 	= 	Scene->LevelPrefix().c_str();
    	p->name_prefix 	+= 	"_";
    }
	p->name_prefix 	+= 	ref_name;
	p->data 		= (void*)ref_name;
    return (0!=p->name_prefix.length());
}

bool TUI_ControlSpawnAdd::Start(TShiftState Shift)
{
#ifndef NO_VCL
    TfraSpawn* F = (TfraSpawn*)parent_tool->pFrame;
	if (F->ebAttachObject->Down){
		CCustomObject* from = Scene->RayPickObject(UI->ZFar(), UI->m_CurrentRStart, UI->m_CurrentRNorm, OBJCLASS_DUMMY, 0, 0);
        if (from->ClassID!=OBJCLASS_SPAWNPOINT){
            ObjectList 	lst;
            int cnt 	= Scene->GetQueryObjects(lst,OBJCLASS_SPAWNPOINT,1,1,0);
            if (1!=cnt)	ELog.DlgMsg(mtError,"Select one shape.");
            else{
                CSpawnPoint* base = dynamic_cast<CSpawnPoint*>(lst.back()); R_ASSERT(base);
                if (base->AttachObject(from)){
                    if (!Shift.Contains(ssAlt)){
                        F->ebAttachObject->Down	= false;
                        ResetActionToSelect		();
                    }
                }else{
		        	ELog.DlgMsg(mtError,"Attach impossible.");
                }
            }
        }else{
        	ELog.DlgMsg(mtError,"Attach impossible.");
        }
    }else{
	    DefaultAddObject(Shift,std::bind(&TUI_ControlSpawnAdd::AppendCallback,this,_1));             
    }
#else
    IM_FrameSpawn &F = imLeftBar.fraSpawn;
	if (F.m_attach_object){
		CCustomObject* from = Scene->RayPickObject(UI->ZFar(), UI->m_CurrentRStart, UI->m_CurrentRNorm, OBJCLASS_DUMMY, 0, 0);
        if (from->ClassID!=OBJCLASS_SPAWNPOINT){
            ObjectList 	lst;
            int cnt 	= Scene->GetQueryObjects(lst,OBJCLASS_SPAWNPOINT,1,1,0);
            if (1!=cnt)	ELog.DlgMsg(mtError,"Select one shape.");
            else{
                CSpawnPoint* base = dynamic_cast<CSpawnPoint*>(lst.back()); R_ASSERT(base);
                if (base->AttachObject(from)){
                    if (!Shift.Contains(ssAlt)){
                        F.m_attach_object	= false;
                        ResetActionToSelect		();
                    }
                }else{
		        	ELog.DlgMsg(mtError,"Attach impossible.");
                }
            }
        }else{
        	ELog.DlgMsg(mtError,"Attach impossible.");
        }
    }else{
	    DefaultAddObject(Shift,std::bind(&TUI_ControlSpawnAdd::AppendCallback,this,_1));             
    }
#endif
    return false;
}

