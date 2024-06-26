#include "stdafx.h"
#pragma hdrstop

#include "ESceneSpawnTools.h"
#include "ui_leveltools.h"
#include "ESceneSpawnControls.h"
#ifndef NO_VCL
#include "FrameSpawn.h"
#endif
#include "Scene.h"
#include "SceneObject.h"
#include "spawnpoint.h"
#include "builder.h"
#ifndef NO_VCL
#include "EditLibrary.h"
#endif

static HMODULE hXRSE_FACTORY = 0;

#ifdef __MINGW32__
static LPCSTR xrse_factory_library	= "libxrSE_Factory.dll";

#ifdef _WIN64
static LPCSTR create_entity_func 	= "create_entity";
static LPCSTR destroy_entity_func 	= "destroy_entity";
#else
static LPCSTR create_entity_func 	= "create_entity@4";
static LPCSTR destroy_entity_func 	= "destroy_entity@4";
#endif

#else
static LPCSTR xrse_factory_library	= "xrSE_Factory.dll";

#ifdef _WIN64
static LPCSTR create_entity_func 	= "create_entity";
static LPCSTR destroy_entity_func 	= "destroy_entity";
#else
static LPCSTR create_entity_func 	= "_create_entity@4";
static LPCSTR destroy_entity_func 	= "_destroy_entity@4";
#endif

#endif

Tcreate_entity 	create_entity;
Tdestroy_entity destroy_entity;

static LPCSTR factory_init_func		= "factory_init";
static LPCSTR factory_done_func		= "factory_done";
typedef void (__cdecl *Tfactory_init)(void);
typedef void (__cdecl *Tfactory_done)(void);

CEditableObject* ESceneSpawnTool::get_draw_visual(u8 _RP_TeamID, u8 _RP_Type, const GameTypeChooser& _GameType)
{
	CEditableObject* ret = NULL;
	if(m_draw_RP_visuals.empty())
    {
#ifndef NO_VCL
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\artefakt_ah"));     		//0
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\artefakt_cta_blue"));     //1
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\artefakt_cta_green"));    //2
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\telo_ah_cta_blue"));      //3
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\telo_ah_cta_green"));     //4
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\telo_dm"));               //5
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\telo_tdm_blue"));         //6
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\telo_tdm_green"));        //7
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\spectator"));        		//8
        m_draw_RP_visuals.push_back(Lib.CreateEditObject("editor\\item_spawn"));       		//9
#endif
    }
    switch (_RP_Type)
    {
    	case rptActorSpawn: //actor spawn
        {
			if(_GameType.MatchType(eGameIDDeathmatch))
            {
            	if(_RP_TeamID==0)
            		ret = m_draw_RP_visuals[5];
            };
			if(_GameType.MatchType(eGameIDTeamDeathmatch))
            {
            	if(_RP_TeamID==2)
            		ret = m_draw_RP_visuals[7];
                else
            	if(_RP_TeamID==1)
            		ret = m_draw_RP_visuals[6];
            };
            
			if(_GameType.MatchType(eGameIDCaptureTheArtefact))
            {
            	if(_RP_TeamID==0)
                	Msg("! incorrect ActorRP teamID [%d] for CTA",_RP_TeamID);
                else
            	if(_RP_TeamID==1)
            		ret = m_draw_RP_visuals[4];
                else
            	if(_RP_TeamID==2)
            		ret = m_draw_RP_visuals[3];
            };
			if(_GameType.MatchType(eGameIDArtefactHunt))
            {
            	if(_RP_TeamID==0)
            		ret = m_draw_RP_visuals[8]; //spactator
                else
            	if(_RP_TeamID==1)
            		ret = m_draw_RP_visuals[4];
                else
            	if(_RP_TeamID==2)
            		ret = m_draw_RP_visuals[3];
            };
        
        }break;
    	case rptArtefactSpawn: //AF spawn
        {
			if(_GameType.MatchType(eGameIDCaptureTheArtefact))
            {
            	if(_RP_TeamID==1)
            		ret = m_draw_RP_visuals[2];
                else
            	if(_RP_TeamID==2)
            		ret = m_draw_RP_visuals[1];
                else
                	Msg("! incorrect AF teamID [%d] for CTA",_RP_TeamID);
            }else
			if(_GameType.MatchType(eGameIDArtefactHunt))
            {
          		ret = m_draw_RP_visuals[0];
            }
        }break;
    	case rptItemSpawn:
        {
        	ret = m_draw_RP_visuals[9];
        }break;
    }
    return ret;
}

void __stdcall  FillSpawnItems	(ChooseItemVec& lst, void* param)
{
	LPCSTR gcs					= (LPCSTR)param;
    ObjectList objects;
    Scene->GetQueryObjects		(objects,OBJCLASS_SPAWNPOINT,-1,-1,-1);
    
    xr_string itm;
    int cnt 					= _GetItemCount(gcs);
    for (int k=0; k<cnt; k++){
        _GetItem				(gcs,k,itm);
        for (ObjectIt it=objects.begin(); it!=objects.end(); it++)
            if ((*it)->OnChooseQuery(itm.c_str()))	lst.push_back(SChooseItem((*it)->GetName(),""));
    }
}

ESceneSpawnTool::ESceneSpawnTool():ESceneCustomOTool(OBJCLASS_SPAWNPOINT)
{
	m_Flags.zero();
#ifndef NO_VCL
    TfrmChoseItem::AppendEvents	(smSpawnItem,		"Select Spawn Item",		FillSpawnItems,		0,0,0,0);
#endif

    hXRSE_FACTORY	= LoadLibrary(xrse_factory_library);									R_ASSERT3(hXRSE_FACTORY,"Can't load library:",xrse_factory_library);

    Tfactory_init factory_init = (Tfactory_init)GetProcAddress(hXRSE_FACTORY, factory_init_func);
    if(factory_init != NULL)	factory_init();

    create_entity 	= (Tcreate_entity)	GetProcAddress(hXRSE_FACTORY,create_entity_func);  	R_ASSERT3(create_entity,"Can't find func:",create_entity_func);
    destroy_entity 	= (Tdestroy_entity)	GetProcAddress(hXRSE_FACTORY,destroy_entity_func);	R_ASSERT3(destroy_entity,"Can't find func:",destroy_entity_func);

    m_Classes.clear			();
    CInifile::Root& data 	= pSettings->sections();
    for (CInifile::RootIt it=data.begin(); it!=data.end(); it++){
    	LPCSTR val;
    	if ((*it)->line_exist	("$spawn",&val)){
        	CLASS_ID cls_id	= pSettings->r_clsid((*it)->Name,"class");
        	shared_str v	= pSettings->r_string_wb((*it)->Name,"$spawn");
        	m_Classes[cls_id].push_back(SChooseItem(*v,*(*it)->Name));
        }
    }
    
}

ESceneSpawnTool::~ESceneSpawnTool()
{
	Tfactory_done factory_done = (Tfactory_done)GetProcAddress(hXRSE_FACTORY, factory_done_func);
    if(factory_done != NULL)	factory_done();

	FreeLibrary		(hXRSE_FACTORY);
    m_Icons.clear	();

	xr_vector<CEditableObject*>::iterator it 	= m_draw_RP_visuals.begin();
	xr_vector<CEditableObject*>::iterator it_e 	= m_draw_RP_visuals.end();
    for(;it!=it_e;++it)
    {
#ifndef NO_VCL
		Lib.RemoveEditObject(*it);
#endif
    }
    m_draw_RP_visuals.clear();
}

void ESceneSpawnTool::CreateControls()
{
	inherited::CreateDefaultControls(estDefault);
    AddControl		(xr_new<TUI_ControlSpawnAdd>(estDefault,etaAdd,		this));
#ifndef NO_VCL
	// frame
    pFrame 			= xr_new<TfraSpawn>((TComponent*)0);
#endif
}
//----------------------------------------------------
 
void ESceneSpawnTool::RemoveControls()
{
	inherited::RemoveControls();
}
//----------------------------------------------------

void ESceneSpawnTool::FillProp(LPCSTR pref, PropItemVec& items)
{            
//.	PHelper().CreateFlag32(items, PrepareKey(pref,"Common\\Show Spawn Type"),	&m_Flags,		flShowSpawnType);
//.	PHelper().CreateFlag32(items, PrepareKey(pref,"Common\\Trace Visibility"),	&m_Flags,		flPickSpawnType);
	inherited::FillProp	(pref, items);
}
//------------------------------------------------------------------------------

ref_shader ESceneSpawnTool::CreateIcon(shared_str name)
{
    ref_shader S;
    if (pSettings->line_exist(name,"$ed_icon")){
	    LPCSTR tex_name = pSettings->r_string(name,"$ed_icon");
    	S.create("editor\\spawn_icon",tex_name);
        m_Icons[name] = S;
    }else{
        S = 0;
    }
    return S;
}

ref_shader ESceneSpawnTool::GetIcon(shared_str name)
{
	ShaderPairIt it = m_Icons.find(name);
	if (it==m_Icons.end())	return CreateIcon(name);
	else					return it->second;
}
//----------------------------------------------------
#include "EShape.h"

CCustomObject* ESceneSpawnTool::CreateObject(LPVOID data, LPCSTR name)
{
	CSpawnPoint* O	= xr_new<CSpawnPoint>(data,name);
    O->ParentTool		= this;

	if(data && name)
    {
        if(pSettings->line_exist( (LPCSTR)data, "$def_sphere") )
        {
        	float size 			= pSettings->r_float( (LPCSTR)data, "$def_sphere");

            CCustomObject* S	= Scene->GetOTool(OBJCLASS_SHAPE)->CreateObject(0,0);
            CEditShape* shape 	= dynamic_cast<CEditShape*>(S);
            R_ASSERT			(shape);

            Fsphere 			Sph;
            Sph.identity		();
            Sph.R				= size;
            shape->add_sphere	(Sph);
            O->AttachObject		(S);
        }
    }
    return O;
}
//----------------------------------------------------

int ESceneSpawnTool::MultiRenameObjects()
{
	int cnt			= 0;
    for (ObjectIt o_it=m_Objects.begin(); o_it!=m_Objects.end(); o_it++){
    	CCustomObject* obj	= *o_it;
    	if (obj->Selected()){
        	string256			pref;
            strconcat			(sizeof(pref),pref,Scene->LevelPrefix().c_str(),"_",obj->RefName());
            string256 			buf;
        	Scene->GenObjectName(obj->ClassID,buf,pref);
            if (obj->GetName()!=buf){//TODO Do we need compare pointer?
	            obj->SetName    (buf);
                cnt++; 
            }
        }
    }
    return cnt;
}
/*
void ESceneSpawnTool::GetStaticDesc(int& v_cnt, int& f_cnt, bool b_selected_only)
{
	for (ObjectIt it=m_Objects.begin(); it!=m_Objects.end(); it++){
    	CSceneObject* obj = (CSceneObject*)(*it);

        if(b_selected_only && !obj->Selected())
        	continue;

        CSpawnPoint* sp = dynamic_cast<CSpawnPoint*>(obj);
        if(!sp){
        	Msg("! ghm/ not a spawn object");
            continue;
        }
		if(sp->m_SpawnData.Valid())
        {
			f_cnt	+= obj->GetFaceCount();
    	    v_cnt	+= obj->GetVertexCount();
        }
    }
}

bool ESceneSpawnTool::ExportStatic(SceneBuilder* B, bool b_selected_only)
{
	return B->ParseStaticObjects(m_Objects, NULL, b_selected_only);
}
*/
