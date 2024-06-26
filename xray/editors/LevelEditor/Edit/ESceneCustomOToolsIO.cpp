#include "stdafx.h"
#pragma hdrstop

#include "ESceneCustomOTools.h"
#include "scene.h"
#include "../ECore/Editor/ui_main.h"
#include "builder.h"
#include "CustomObject.h"

using namespace std::placeholders;

// chunks
//----------------------------------------------------
static const u32 CHUNK_VERSION			= 0x0001;
static const u32 CHUNK_OBJECT_COUNT		= 0x0002;
static const u32 CHUNK_OBJECTS			= 0x0003;
static const u32 CHUNK_FLAGS			= 0x0004;
//----------------------------------------------------

bool ESceneCustomOTool::OnLoadSelectionAppendObject(CCustomObject* obj)
{
    string256 				buf;
    Scene->GenObjectName	(obj->ClassID,buf,obj->GetName());
    obj->SetName			(buf);
    Scene->AppendObject		(obj, false);
    obj->Select				(1);
    return					true;
}
//----------------------------------------------------

bool ESceneCustomOTool::OnLoadAppendObject(CCustomObject* O)
{
	Scene->AppendObject	(O,false);
    return true;
}
//----------------------------------------------------

bool ESceneCustomOTool::LoadSelection(IReader& F)
{
    int count					= 0;
	F.r_chunk					(CHUNK_OBJECT_COUNT,&count);

    SPBItem* pb 				= UI->ProgressStart(count,(AnsiString("Loading ") + ClassDesc()  + "'s...").c_str());
    Scene->ReadObjectsStream	(F,CHUNK_OBJECTS,std::bind(&ESceneCustomOTool::OnLoadSelectionAppendObject,this,_1),pb);
    UI->ProgressEnd				(pb);

    return true;
}
//----------------------------------------------------

void ESceneCustomOTool::SaveSelection(IWriter& F)
{
	F.open_chunk	(CHUNK_OBJECTS);
    int count		= 0;
    for(ObjectIt it = m_Objects.begin();it!=m_Objects.end();++it)
    {
    	if ((*it)->Selected() && !(*it)->IsDeleted())
        {
	        F.open_chunk(count++);
    	    Scene->SaveObjectStream(*it,F);
        	F.close_chunk();
        }
    }
	F.close_chunk	();

	F.w_chunk		(CHUNK_OBJECT_COUNT,&count,sizeof(count));
}
//----------------------------------------------------
bool ESceneCustomOTool::LoadLTX(CInifile& ini)
{
	inherited::LoadLTX	(ini);

    u32 count			= ini.r_u32("main", "objects_count");

	SPBItem* pb 		= UI->ProgressStart(count,(AnsiString("Loading ")+ ClassDesc() + "...").c_str());

    u32 i				= 0;
    string128			buff;

	for(i=0; i<count; ++i)
    {
        CCustomObject* obj	= NULL;
        sprintf				(buff, "object_%d", i);

		if( Scene->ReadObjectLTX(ini, buff, obj) )
        {
        	if (!OnLoadAppendObject(obj))
            	xr_delete(obj);
		}
        pb->Inc();
    }

	UI->ProgressEnd		(pb);

    return true;
}

bool ESceneCustomOTool::LoadStream(IReader& F)
{
	inherited::LoadStream		(F);

    int count					= 0;
	F.r_chunk					(CHUNK_OBJECT_COUNT,&count);

    SPBItem* pb 				= UI->ProgressStart(count,(AnsiString("Loading ") + ClassDesc() + "...").c_str());
    Scene->ReadObjectsStream	(F,CHUNK_OBJECTS,std::bind(&ESceneCustomOTool::OnLoadAppendObject,this,_1),pb);
    UI->ProgressEnd				(pb);

    return true;
}
//----------------------------------------------------

void ESceneCustomOTool::SaveLTX(CInifile& ini)
{
	inherited::SaveLTX	(ini);

	u32 count			= 0;
    for(ObjectIt it=m_Objects.begin(); it!=m_Objects.end(); ++it)
	{
    	CCustomObject* O = (*it);
    	if (O->IsDeleted() || O->m_CO_Flags.test(CCustomObject::flObjectInGroup) )
        	continue;
            
        string128				buff;
        sprintf					(buff,"object_%d",count);
        Scene->SaveObjectLTX	(*it,  buff, ini);
        count++;
	}

	ini.w_u32			("main", "objects_count", count);
}

void ESceneCustomOTool::SaveStream(IWriter& F)
{
	inherited::SaveStream	(F);

    int Objcount		= 0;

	F.open_chunk		(CHUNK_OBJECTS);
    int count			= 0;
    for(ObjectIt it = m_Objects.begin();it!=m_Objects.end();++it)
	{
    	if ( (*it)->IsDeleted() || (*it)->m_CO_Flags.test(CCustomObject::flObjectInGroup) )
        continue;

        F.open_chunk			(count++);
        Scene->SaveObjectStream	(*it,F);
        F.close_chunk			();
    }
	F.close_chunk	();

	F.w_chunk		(CHUNK_OBJECT_COUNT,&Objcount,sizeof(Objcount));
}
//----------------------------------------------------

bool ESceneCustomOTool::Export(LPCSTR path)
{
	return true;
}
//----------------------------------------------------
 
bool ESceneCustomOTool::ExportGame(SExportStreams* F)
{
	bool bres=true;
    for(ObjectIt it = m_Objects.begin();it!=m_Objects.end();it++)
        if (!(*it)->ExportGame(F)) bres=false;
	return bres;
}
//----------------------------------------------------

bool ESceneCustomOTool::ExportStatic(SceneBuilder* B, bool b_selected_only)
{
	return B->ParseStaticObjects(m_Objects, NULL, b_selected_only);
}
//----------------------------------------------------
 