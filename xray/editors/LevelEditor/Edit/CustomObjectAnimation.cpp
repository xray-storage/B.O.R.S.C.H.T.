#include "stdafx.h"
#pragma hdrstop

#include "customobject.h"
#include <xrEngine/motion.h>
#include <xrEngine/envelope.h>
#include <Layers/xrRender/D3DUtils.h>
#include "../ECore/Editor/ui_main.h"

#include "../ECore/ImGui/IM_PropertyTree.h"

void  CCustomObject::OnMotionableChange(PropValue* sender)
{
	if (m_CO_Flags.is(flMotion)){
    	m_Motion		= xr_new<COMotion>();
        m_MotionParams	= xr_new<SAnimParams>();
    }else{
    	xr_delete		(m_Motion);
    	xr_delete		(m_MotionParams);
    }
    ExecCommand			(COMMAND_UPDATE_PROPERTIES);
}

void CCustomObject::AnimationCreateKey(float t)
{
    Fvector R;		R.set(-GetRotation().x,-GetRotation().y,-GetRotation().z);
    m_Motion->CreateKey(t,GetPosition(),R);
}

void CCustomObject::AnimationDeleteKey(float t)
{
    m_Motion->DeleteKey(t);
}

//float speed = 0.f;
void CCustomObject::AnimationUpdate(float t)
{
    Fvector R,P,r;   
    m_Motion->_Evaluate		(t,P,r);
    R.set					(-r.x,-r.y,-r.z);

//    speed					= speed*0.9f+(P.distance_to(PPosition)/Device.fTimeDelta)*0.1f;
//    Log("speed: ",speed);
    SetPosition 			(P);
    SetRotation				(R);
    BOOL bAK				= m_CO_Flags.is(flAutoKey);
    m_CO_Flags.set			(flAutoKey,FALSE);
    UpdateTransform			(true);
    m_CO_Flags.set			(flAutoKey,bAK);
    if (m_CO_Flags.is(flCameraView))
    	Device.m_Camera.Set	(-r.y,-r.x,-r.z,P.x,P.y,P.z);
}

void CCustomObject::AnimationOnFrame()
{
	VERIFY (m_Motion);

    if (Selected()&&m_MotionParams->bPlay)
    {
    	AnimationUpdate			(m_MotionParams->Frame());
        m_MotionParams->Update	(Device.fTimeDelta,1.f,true);
    }
}

static FvectorVec path_points;
void CCustomObject::AnimationDrawPath()
{
    // motion path
	VERIFY (m_Motion);
#ifdef _EDITOR
	if (EPrefs->object_flags.is(epoDrawAnimPath)){
        float fps 				= m_Motion->FPS();
        float min_t				= (float)m_Motion->FrameStart()/fps;
        float max_t				= (float)m_Motion->FrameEnd()/fps;

        Fvector 				T,r;
        u32 clr					= 0xffffffff;
        path_points.clear		();
        for (float t=min_t; (t<max_t)||fsimilar(t,max_t,EPS_L); t+=1/30.f){
            m_Motion->_Evaluate	(t,T,r);
            path_points.push_back(T);
        }

        Device.SetShader		(Device.m_WireShader);
        RCache.set_xform_world	(Fidentity);
        if (!path_points.empty())
        	DUImpl.DrawPrimitiveL		(D3DPT_LINESTRIP,path_points.size()-1,path_points.data(),path_points.size(),clr,true,false);
        CEnvelope* E 			= m_Motion->Envelope();
        for (KeyIt k_it=E->keys.begin(); k_it!=E->keys.end(); k_it++){
            m_Motion->_Evaluate	((*k_it)->time,T,r);
            if (Device.m_Camera.GetPosition().distance_to_sqr(T)<50.f*50.f){
                DUImpl.DrawCross	(T,0.1f,0.1f,0.1f, 0.1f,0.1f,0.1f, clr,false);
                string64 temp; sprintf(temp,"K: %3.3f",(*k_it)->time);
                DUImpl.OutText		(T,temp,0xffffffff,0x00000000);
            }
        }
    }
#endif    
}

void 	CCustomObject::OnMotionControlClick(ButtonValue* value, bool& bModif, bool& bSafe)
{
	ButtonValue* B = dynamic_cast<ButtonValue*>(value); R_ASSERT(B);
	switch(B->btn_num){
    case 0:{
		m_MotionParams->t_current 	= m_MotionParams->min_t;
        m_MotionParams->tmp = m_MotionParams->t_current;

		m_MotionParams->bPlay= FALSE;
    }break;
    case 1:{
    	float min_k;
    	float max_k;
		m_Motion->FindNearestKey(m_MotionParams->t_current, min_k, max_k);
        m_MotionParams->t_current	= min_k;
        m_MotionParams->tmp = m_MotionParams->t_current;
		m_MotionParams->bPlay= FALSE;
    }break;
    case 2:{
		m_MotionParams->t_current	-= 1.f/30.f;
        m_MotionParams->tmp = m_MotionParams->t_current;
		m_MotionParams->bPlay= FALSE;
    }break;
    case 3:{
		m_MotionParams->bPlay= TRUE;
    }break;
    case 4:{
		m_MotionParams->bPlay= FALSE;
    }break;
    case 5:{
		m_MotionParams->t_current	+= 1.f/30.f;
        m_MotionParams->tmp = m_MotionParams->t_current;
		m_MotionParams->bPlay= FALSE;
    }break;
    case 6:{
    	float min_k;
    	float max_k;
		m_Motion->FindNearestKey(m_MotionParams->t_current, min_k, max_k);
        m_MotionParams->t_current	= max_k;
        m_MotionParams->tmp = m_MotionParams->t_current;
		m_MotionParams->bPlay= FALSE;
    }break;
    case 7:{
		m_MotionParams->t_current 	= m_MotionParams->max_t;
        m_MotionParams->tmp 		= m_MotionParams->t_current;
		m_MotionParams->bPlay		= FALSE;
    }break;
    }
    AnimationUpdate			(m_MotionParams->Frame());
    ExecCommand 			(COMMAND_UPDATE_PROPERTIES);
    bModif = false;
}

void 	CCustomObject::OnMotionCommandsClick(ButtonValue* value, bool& bModif, bool& bSafe)
{
	ButtonValue* B = dynamic_cast<ButtonValue*>(value); R_ASSERT(B);
	switch(B->btn_num){
    case 0:
    	AnimationCreateKey	(m_MotionParams->t_current);
    break;
    case 1:
    	AnimationDeleteKey	(m_MotionParams->t_current);
    break;
    case 2:{
			class ScaleKeysDelegate
			{
				public:
				IM_PropertiesWnd *m_Wnd;
				
    		SAnimParams*	m_MotionParams;
    		COMotion*		m_Motion;
    		float from_time,to_time,scale_factor;
				
				ScaleKeysDelegate(COMotion *motion, SAnimParams *motion_params)
				{
					m_Wnd = xr_new<IM_PropertiesWnd>("Scale keys");
					m_Wnd->Modal = true;
					m_Wnd->OnOK.bind(this, &ScaleKeysDelegate::OnOK);
					m_Wnd->OnClose.bind(this, &ScaleKeysDelegate::OnClose);
					m_Wnd->Open();
					
					m_MotionParams = motion_params;
					m_Motion = motion;
					
					from_time=m_MotionParams->min_t;
					to_time=m_MotionParams->max_t;
					scale_factor=1.f;
					
					PropItemVec items;
					PHelper().CreateFloat	(items,"From Time", 	&from_time, 	from_time, to_time, 	1.f/30.f, 3);
					PHelper().CreateFloat	(items,"To Time",   	&to_time, 		from_time, to_time, 	1.f/30.f, 3);
					PHelper().CreateFloat	(items,"Scale",			&scale_factor, 	-1000.f, 1000.f);
					m_Wnd->AssignItems(items);
					
					UI->AddIMWindow(m_Wnd);
				}
				
				void OnOK()
				{
					m_Motion->ScaleKeys(from_time,to_time,scale_factor);
					float mx; m_Motion->GetLength(0,&mx);
					if (m_MotionParams->max_t<mx){ 
						m_MotionParams->max_t=mx;
						m_Motion->SetParam	(m_MotionParams->min_t*30.f,m_MotionParams->max_t*30.f,30.f);
        	}
				}
				
				void OnClose()
				{
					UI->RemoveIMWindow(m_Wnd);
					xr_delete(m_Wnd);
					
					ScaleKeysDelegate *_this = this;
					xr_delete(_this);
				}
			};
			
			xr_new<ScaleKeysDelegate>(m_Motion, m_MotionParams);
    }break;
    case 3:{
			class NormalizeKeysDelegate
			{
				public:
				IM_PropertiesWnd *m_Wnd;
				
    		SAnimParams*	m_MotionParams;
    		COMotion*		m_Motion;
    		float from_time,to_time,speed;
				
				NormalizeKeysDelegate(COMotion *motion, SAnimParams *motion_params)
				{
					m_Wnd = xr_new<IM_PropertiesWnd>("Normalize keys");
					m_Wnd->Modal = true;
					m_Wnd->OnOK.bind(this, &NormalizeKeysDelegate::OnOK);
					m_Wnd->OnClose.bind(this, &NormalizeKeysDelegate::OnClose);
					m_Wnd->Open();
					
					m_MotionParams = motion_params;
					m_Motion = motion;
					
					from_time=m_MotionParams->min_t;
					to_time=m_MotionParams->max_t;
					speed=5.f;
					
					PropItemVec items;
					PHelper().CreateFloat	(items,"From Time", 	&from_time, 	from_time, to_time, 	1.f/30.f, 3);
					PHelper().CreateFloat	(items,"To Time",   	&to_time, 		from_time, to_time, 	1.f/30.f, 3);
					PHelper().CreateFloat	(items,"Speed (m/sec)", &speed, 		0.f, 100.f);
					m_Wnd->AssignItems(items);
					
					UI->AddIMWindow(m_Wnd);
				}
				
				void OnOK()
				{
					m_Motion->NormalizeKeys(from_time,to_time,speed);
					float mx; m_Motion->GetLength(0,&mx);
					if (m_MotionParams->max_t<mx){ 
						m_MotionParams->max_t=mx;
						m_Motion->SetParam	(m_MotionParams->min_t*30.f,m_MotionParams->max_t*30.f,30.f);
        	}
				}
				
				void OnClose()
				{
					UI->RemoveIMWindow(m_Wnd);
					xr_delete(m_Wnd);
					
					NormalizeKeysDelegate *_this = this;
					xr_delete(_this);
				}
			};
			
			xr_new<NormalizeKeysDelegate>(m_Motion, m_MotionParams);
    }break;
    case 4:{
    	float mn,mx;
        m_Motion->GetLength		(&mn,&mx);
        m_MotionParams->min_t	= mn;
        m_MotionParams->max_t	= mx;
        m_Motion->SetParam		(mn*30.f,mx*30.f,30.f);
    }break;
	}
    AnimationUpdate			(m_MotionParams->Frame());
	bModif = true;
}

void 	CCustomObject::OnMotionFilesClick(ButtonValue* value, bool& bModif, bool& bSafe)
{
	ButtonValue* B = dynamic_cast<ButtonValue*>(value); R_ASSERT(B);
    bModif = false;
    xr_string fn;
	switch(B->btn_num){
    case 0:
        if(EFS.GetOpenName("$game_anims$", fn)){
            m_Motion->LoadMotion(fn.c_str());
            m_MotionParams->Set	(m_Motion);
            AnimationUpdate		(m_MotionParams->Frame());
			bModif 				= true;
		    ExecCommand			(COMMAND_UPDATE_PROPERTIES);
        }
    break;
    case 1:
        if(EFS.GetSaveName("$game_anims$", fn))
            m_Motion->SaveMotion(fn.c_str());
    break;
	}
}

void 	CCustomObject::OnMotionFrameChange(PropValue* value)
{
	m_Motion->SetParam	(m_MotionParams->min_t*30.f,m_MotionParams->max_t*30.f,30.f);
    ExecCommand			(COMMAND_UPDATE_PROPERTIES);
}

void 	CCustomObject::OnMotionKeyTimeChange(PropValue* value)
{
	float t = m_MotionParams->tmp;
    Fvector P,R;

    m_Motion->_Evaluate(m_MotionParams->t_current,P, R);

    m_Motion->DeleteKey(m_MotionParams->t_current);
    m_Motion->CreateKey(t,P,R);
}

void 	CCustomObject::OnMotionCurrentFrameChange(PropValue* value)
{
	if (m_MotionParams->t_current<m_MotionParams->min_t)
    	m_MotionParams->min_t = m_MotionParams->t_current;

    else if (m_MotionParams->t_current>m_MotionParams->max_t)
    	m_MotionParams->max_t = m_MotionParams->t_current;

	m_Motion->SetParam	(m_MotionParams->min_t*30.f,m_MotionParams->max_t*30.f,30.f);
    m_MotionParams->bPlay= FALSE;
    AnimationUpdate		(m_MotionParams->Frame());
    ExecCommand			(COMMAND_UPDATE_PROPERTIES);
}

void 	CCustomObject::OnMotionCameraViewChange(PropValue* value)
{
	if (m_CO_Flags.is(flCameraView))
	    AnimationUpdate	(m_MotionParams->Frame());
}

void 	CCustomObject::OnTransformChange(PropValue* value)
{
	UpdateTransform();
}

void CCustomObject::AnimationFillProp(LPCSTR pref, PropItemVec& items)
{
    PropValue* V		= PHelper().CreateFlag32(items,PrepareKey(pref,"Flags\\Motionable"),&m_CO_Flags, flMotion);
    V->OnChangeEvent.bind(this,&CCustomObject::OnMotionableChange);
	if (Motionable()){
       				      PHelper().CreateCaption		(items,PrepareKey(pref,"Motion\\Hint"),		"Make KEY only on Parent CS");
	    ButtonValue* B	= PHelper().CreateButton		(items,PrepareKey(pref,"Motion\\Files"),	"Import,Export", 0);
        B->OnBtnClickEvent.bind(this,&CCustomObject::OnMotionFilesClick);
	    B				= PHelper().CreateButton		(items,PrepareKey(pref,"Motion\\Commands"),	"+ K,- K,Scale,Norm,Clamp", 0);
        B->OnBtnClickEvent.bind(this,&CCustomObject::OnMotionCommandsClick);
	    B				= PHelper().CreateButton		(items,PrepareKey(pref,"Motion\\Controls"),	" |<<, +<<, <<, >, ||, >>, >>+, >>|", 0); 
        B->OnBtnClickEvent.bind(this,&CCustomObject::OnMotionControlClick);
        				  PHelper().CreateFlag32		(items,PrepareKey(pref,"Motion\\Flags\\Auto Key"), 		&m_CO_Flags, flAutoKey);
        V				= PHelper().CreateFlag32		(items,PrepareKey(pref,"Motion\\Flags\\Camera View"), 	&m_CO_Flags, flCameraView);
        V->OnChangeEvent.bind(this,&CCustomObject::OnMotionCameraViewChange);
	    V				= PHelper().CreateFloat		(items,PrepareKey(pref,"Motion\\Start Frame (sec)"),		&m_MotionParams->min_t, -10000.f, m_MotionParams->max_t, 	1.f/30.f, 3);
    	V->OnChangeEvent.bind(this,&CCustomObject::OnMotionFrameChange);
		V				= PHelper().CreateFloat		(items,PrepareKey(pref,"Motion\\End Frame (sec)"),		&m_MotionParams->max_t, m_MotionParams->min_t, 10000.f, 	1.f/30.f, 3);
    	V->OnChangeEvent.bind(this,&CCustomObject::OnMotionFrameChange);
		V				= PHelper().CreateFloat		(items,PrepareKey(pref,"Motion\\Current Frame (sec)"),	&m_MotionParams->t_current, -10000.f, 10000.f,	1.f/30.f, 3);
    	V->OnChangeEvent.bind(this,&CCustomObject::OnMotionCurrentFrameChange);

		V				= PHelper().CreateFloat		(items,PrepareKey(pref,"Motion\\ChangeKeyTime(sec)"),	&m_MotionParams->tmp, -10000.f, 10000.f,	1.f/30.f, 3);
    	V->OnChangeEvent.bind(this,&CCustomObject::OnMotionKeyTimeChange);

        				  PHelper().CreateCaption		(items,PrepareKey(pref,"Motion\\Key Count"),			shared_str().sprintf("%d",m_Motion->KeyCount()));
        				  PHelper().CreateCaption		(items,PrepareKey(pref,"Motion\\Length (sec)"),		shared_str().sprintf("%3.2f",m_Motion->GetLength()));
	}
}
 
