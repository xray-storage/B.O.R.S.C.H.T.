#include "stdafx.h"
#pragma hdrstop

#include "ESceneObjectTools.h"
#include "ui_leveltools.h"
#include "ESceneObjectControls.h"
#include "FrameObject.h"
#include "SceneObject.h"
#include "../ECore/Editor/library.h"
#include "Scene.h"
#include "../ECore/Editor/ui_main.h"
#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/EditMesh.h"

ESceneObjectTool::ESceneObjectTool():ESceneCustomOTool(OBJCLASS_SCENEOBJECT)
{
    m_AppendRandomMinScale.set		(1.f,1.f,1.f);
    m_AppendRandomMaxScale.set      (1.f,1.f,1.f);
    m_AppendRandomMinRotation.set   (0.f,0.f,0.f);
    m_AppendRandomMaxRotation.set   (0.f,0.f,0.f);
    m_AppendRandomObjectsPerM2      = 0.02f;
	m_Flags.zero    ();
    m_Props         = 0;
}

void ESceneObjectTool::CreateControls()
{
	inherited::CreateDefaultControls(estDefault);
    AddControl		(xr_new<TUI_ControlObjectAdd >(estDefault,etaAdd,		this));
	// frame
    pFrame 			= xr_new<TfraObject>((TComponent*)0,this);
}
//----------------------------------------------------
 
void ESceneObjectTool::RemoveControls()
{
	inherited::RemoveControls();
}
//----------------------------------------------------

bool ESceneObjectTool::Validate(bool full_test)
{
    bool bRes = inherited::Validate(full_test);
    // verify position & refs duplicate
    CSceneObject *A, *B;
    for (ObjectIt a_it=m_Objects.begin(); a_it!=m_Objects.end(); a_it++){
        A = (CSceneObject*)(*a_it);
	    for (ObjectIt b_it=m_Objects.begin(); b_it!=m_Objects.end(); b_it++){
            B = (CSceneObject*)(*b_it);
        	if (A==B) continue;
            if (A->RefCompare(B->GetReference())){
            	if (A->PPosition.similar(B->PPosition,EPS_L)){
                	bRes = false;
                    ELog.Msg(mtError,"Duplicate object position '%s'-'%s' with reference '%s'.",A->Name,B->Name,A->RefName());
                }
            }
        }
	    // validate lods
        if (full_test&&A->IsMUStatic()){
			CEditableObject* E	= A->GetReference(); VERIFY(E);
            xr_string lod_name 	= E->GetLODTextureName();
            xr_string l_name	= lod_name.c_str();
            string_path fn;
            int age,age_nm;
//.          FS.update_path		(fn,_textures_,EFS.ChangeFileExt(l_name,".tga").c_str());
            FS.update_path		(fn,_game_textures_,EFS.ChangeFileExt(l_name,".dds").c_str());
            age					= FS.get_file_age(fn);
            if(age==-1)         Msg("!There is no texture '%s'", fn);
            l_name 				+= "_nm";
//.         FS.update_path		(fn,_textures_,EFS.ChangeFileExt(l_name,".tga").c_str());
            FS.update_path		(fn,_game_textures_,EFS.ChangeFileExt(l_name,".dds").c_str());
            age_nm				= FS.get_file_age(fn);
            if(age_nm==-1)      Msg("!There is no texture '%s'", fn);

            if(age_nm==-1 || age==-1)
               bRes 			= false;
/*
            if ((age!=E->Version()) || (age_nm!=E->Version()) )
            {
                Msg				("!Invalid LOD texture version: '%s'",E->GetName());
                Msg             ("tex=%d obj=%d", age, E->Version());
                Msg             ("tex=%d obj=%d", age_nm, E->Version());
                bRes 			= false;
            }
*/            
        }
    }
    
    return bRes;
}
//----------------------------------------------------

void ESceneObjectTool::OnChangeAppendRandomFlags(PropValue* prop)
{
    m_Flags.set					(flAppendRandomUpdateProps,TRUE);
	_SequenceToList				(m_AppendRandomObjects,*m_AppendRandomObjectsStr);
	_SequenceToList				(m_AppendRandomMaterials,*m_AppendRandomMaterialsStr);
}
//----------------------------------------------------

void ESceneObjectTool::OnAppendRandomFileBtnClick(ButtonValue* B, bool& bModif, bool& bSafe)
{
    bModif = false;
    xr_string fn;
	switch(B->btn_num){
    case 0:
        if(EFS.GetOpenName("$import$", fn, false, NULL, 6)){
            CInifile* I = xr_new<CInifile>(fn.c_str(), TRUE, TRUE, TRUE);
            
            m_Flags.set(flAppendRandomScale, I->r_bool("append_random", "scale"));
            m_Flags.set(flAppendRandomScaleProportional, I->r_bool("append_random", "scale_proportional"));
            m_AppendRandomMinScale = I->r_fvector3("append_random", "scale_min");
            m_AppendRandomMaxScale = I->r_fvector3("append_random", "scale_max");
            m_Flags.set(flAppendRandomRotation, I->r_bool("append_random", "rotation"));
            m_AppendRandomMinRotation = I->r_fvector3("append_random", "rotation_min");
            m_AppendRandomMaxRotation = I->r_fvector3("append_random", "rotation_max");
            
            m_AppendRandomObjects.resize(I->r_u32("append_random", "objects_count"));
            for(size_t i = 0; i < m_AppendRandomObjects.size(); i++)
            	m_AppendRandomObjects[i] = I->r_string("append_random", shared_str().sprintf("object_%u", i).c_str());

			m_AppendRandomObjects.resize(I->r_u32("append_random", "materials_count"));
			for(size_t i = 0; i < m_AppendRandomMaterials.size(); i++)
				m_AppendRandomMaterials[i] = I->r_string("append_random", shared_str().sprintf("material_%u", i).c_str());

			m_Flags.set(flAppendRandomNormalAlignment, I->r_bool("append_random", "normal_alignment"));
			m_AppendRandomObjectsPerM2 = I->r_float("append_random", "objects_per_m2");
            
            xr_delete(I);
            
			m_Flags.set(flAppendRandomUpdateProps,TRUE);
        }
    break;
    case 1:
        if(EFS.GetSaveName("$import$", fn, NULL, 6)){
		    CInifile* I = xr_new<CInifile>(fn.c_str(), FALSE, FALSE, TRUE);
            
            I->w_bool("append_random", "scale", m_Flags.is(flAppendRandomScale));
            I->w_bool("append_random", "scale_proportional", m_Flags.is(flAppendRandomScaleProportional));
            I->w_fvector3("append_random", "scale_min", m_AppendRandomMinScale);
            I->w_fvector3("append_random", "scale_max", m_AppendRandomMaxScale);
			I->w_bool("append_random", "rotation", m_Flags.is(flAppendRandomRotation));
            I->w_fvector3("append_random", "rotation_min", m_AppendRandomMinRotation);
            I->w_fvector3("append_random", "rotation_max", m_AppendRandomMaxRotation);

            I->w_u32("append_random", "objects_count", m_AppendRandomObjects.size());
            for(size_t i = 0; i < m_AppendRandomObjects.size(); i++)
            	I->w_string("append_random", shared_str().sprintf("object_%u", i).c_str(), *m_AppendRandomObjects[i]); 

			I->w_u32("append_random", "materials_count", m_AppendRandomMaterials.size());
			for(size_t i = 0; i < m_AppendRandomMaterials.size(); i++)
				I->w_string("append_random", shared_str().sprintf("material_%u", i).c_str(), *m_AppendRandomMaterials[i]);

			I->w_bool("append_random", "normal_alignment", m_Flags.is(flAppendRandomNormalAlignment));
			I->w_float("append_random", "objects_per_m2", m_AppendRandomObjectsPerM2);
            
            xr_delete(I);
        }
    break;
	}
}
//----------------------------------------------------

void ESceneObjectTool::FillAppendRandomProperties(bool bUpdateOnly)
{
	if (!bUpdateOnly) m_Props       = TProperties::CreateModalForm("Random Append Properties",false);

	m_AppendRandomObjectsStr        = _ListToSequence(m_AppendRandomObjects).c_str();
    m_AppendRandomMaterialsStr      = _ListToSequence(m_AppendRandomMaterials).c_str();

	PropValue* V;
	PropItemVec                     items;
	V=PHelper().CreateFlag32        (items,"Scale",              &m_Flags, flAppendRandomScale);
	V->OnChangeEvent.bind           (this,&ESceneObjectTool::OnChangeAppendRandomFlags);
	if(m_Flags.is(flAppendRandomScale)) {
		V=PHelper().CreateFlag32    (items,"Scale\\Proportional",&m_Flags, flAppendRandomScaleProportional);
		V->OnChangeEvent.bind       (this,&ESceneObjectTool::OnChangeAppendRandomFlags);
		if(m_Flags.is(flAppendRandomScaleProportional)) {
			PHelper().CreateFloat   (items,"Scale\\Minimum",    &m_AppendRandomMinScale.x,0.001f,1000.f,0.001f,3);
			PHelper().CreateFloat   (items,"Scale\\Maximum",    &m_AppendRandomMaxScale.x,0.001f,1000.f,0.001f,3);
		} else {
			PHelper().CreateVector  (items,"Scale\\Minimum",    &m_AppendRandomMinScale,0.001f,1000.f,0.001f,3);
			PHelper().CreateVector  (items,"Scale\\Maximum",    &m_AppendRandomMaxScale,0.001f,1000.f,0.001f,3);
        }
    }

	V=PHelper().CreateFlag32        (items,"Rotate",            &m_Flags, flAppendRandomRotation);
	V->OnChangeEvent.bind           (this,&ESceneObjectTool::OnChangeAppendRandomFlags);
    if(m_Flags.is(flAppendRandomRotation)) {
		PHelper().CreateAngle3      (items,"Rotate\\Minimum",   &m_AppendRandomMinRotation);
		PHelper().CreateAngle3      (items,"Rotate\\Maximum",   &m_AppendRandomMaxRotation);
    }
	V=PHelper().CreateChoose        (items,"Objects",&m_AppendRandomObjectsStr,smObject,0,0,128);
	V->OnChangeEvent.bind           (this,&ESceneObjectTool::OnChangeAppendRandomFlags);
    
	V=PHelper().CreateFlag32        (items,"Scatter Random Objects\\Normal Alignment", &m_Flags,                    flAppendRandomNormalAlignment);
	V=PHelper().CreateFlag32        (items,"Scatter Random Objects\\Shape Restrict",   &m_Flags,                    flAppendRandomShapeRestrict);
	V=PHelper().CreateFlag32        (items,"Scatter Random Objects\\Shape Emitter",    &m_Flags,                    flAppendRandomShapeEmitter);
	V=PHelper().CreateFloat         (items,"Scatter Random Objects\\Objects per m^2",  &m_AppendRandomObjectsPerM2, 0.f, 10.f, 0.001f, 3);

	V=PHelper().CreateChoose        (items,"Scatter Random Objects\\Allowed Materials",&m_AppendRandomMaterialsStr, smGameMaterial, 0, 0, 32, cfAllowNone|cfMultiSelect);
	V->OnChangeEvent.bind           (this,&ESceneObjectTool::OnChangeAppendRandomFlags);

    ButtonValue* B = PHelper().CreateButton(items,"File","Load,Save", 0);
	B->OnBtnClickEvent.bind(this,&ESceneObjectTool::OnAppendRandomFileBtnClick);

	m_Props->AssignItems            (items);

	if(!bUpdateOnly) {
        if (mrOk==m_Props->ShowPropertiesModal())
			Scene->UndoSave         ();
		TProperties::DestroyForm    (m_Props);
    }
}
//----------------------------------------------------

void ESceneObjectTool::Clear		(bool bSpecific)
{
	inherited::Clear				(bSpecific);
    m_AppendRandomMinScale.set		(1.f,1.f,1.f);
    m_AppendRandomMaxScale.set		(1.f,1.f,1.f);
    m_AppendRandomMinRotation.set	(0.f,0.f,0.f);
    m_AppendRandomMaxRotation.set	(0.f,0.f,0.f);
    m_AppendRandomObjects.clear		();
    m_Flags.zero					();
}

bool ESceneObjectTool::GetBox		(Fbox& bb)
{
	bb.invalidate					();
    Fbox bbo;
    for (ObjectIt a_it=m_Objects.begin(); a_it!=m_Objects.end(); a_it++){
    	(*a_it)->GetBox				(bbo);
        bb.merge					(bbo);
    }
    return bb.is_valid();
}

void ESceneObjectTool::OnFrame		()
{
	inherited::OnFrame				();
	if (m_Flags.is(flAppendRandomUpdateProps)){
    	m_Flags.set					(flAppendRandomUpdateProps,FALSE);
        FillAppendRandomProperties	(true);
    }
}

CCustomObject* ESceneObjectTool::CreateObject(LPVOID data, LPCSTR name)
{
	CCustomObject* O	= xr_new<CSceneObject>(data,name);                     
    O->ParentTool		= this;
    return O;
}
//----------------------------------------------------

void ESceneObjectTool::HighlightTexture(LPCSTR tex_name, bool allow_ratio, u32 t_width, u32 t_height, BOOL mark)
{
	if (tex_name&&tex_name[0]){
        for (ObjectIt a_it=m_Objects.begin(); a_it!=m_Objects.end(); a_it++){
            CSceneObject* s_obj		= dynamic_cast<CSceneObject*>(*a_it);
            if (s_obj->Visible()){
                CEditableObject* e_obj	= s_obj->GetReference(); VERIFY(e_obj);
                SurfaceVec& s_vec		= e_obj->Surfaces();
                for (SurfaceIt it=s_vec.begin(); it!=s_vec.end(); it++){
                    if (0==stricmp((*it)->_Texture(),tex_name)){
                        Fvector 		verts[3];
                        for (EditMeshIt mesh_it=e_obj->FirstMesh(); mesh_it!=e_obj->LastMesh(); mesh_it++){
                            const SurfFaces& surf_faces			= (*mesh_it)->GetSurfFaces();
                            SurfFaces::const_iterator  sf_it 	= surf_faces.find(*it);
                            if (sf_it!=surf_faces.end()){
                                const IntVec& lst				= sf_it->second;
                                for (IntVec::const_iterator i_it=lst.begin(); i_it!=lst.end(); i_it++){
                                    e_obj->GetFaceWorld	(s_obj->_Transform(),*mesh_it,*i_it,verts);
                                    u32 clr	= 0x80FFFFFF;
                                    if (allow_ratio){
                                        // select color
                                        const Fvector2* tc[3];
                                        Fvector 		c,e01,e02;
                                        e01.sub			(verts[1],verts[0]);
                                        e02.sub			(verts[2],verts[0]);
                                        float area		= c.crossproduct(e01,e02).magnitude()/2.f;
                                        (*mesh_it)->GetFaceTC(*i_it,tc);
                                        e01.sub			(Fvector().set(tc[1]->x,tc[1]->y,0),Fvector().set(tc[0]->x,tc[0]->y,0));
                                        e02.sub			(Fvector().set(tc[2]->x,tc[2]->y,0),Fvector().set(tc[0]->x,tc[0]->y,0));
                                        float p_area	= c.crossproduct(e01,e02).magnitude()/2.f;
                                        float pm		= _sqrt((p_area*t_width*t_height)/area);
                                        clr 			= SSceneSummary::SelectPMColor(pm);
                                    }
                                    Tools->m_DebugDraw.AppendSolidFace(verts[0],verts[1],verts[2],clr,false);
                                    if (mark)	Tools->m_DebugDraw.AppendWireFace(verts[0],verts[1],verts[2],clr,false);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
//----------------------------------------------------

