//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "Custom2DProjector.h"
#include "../ECore/Editor/ImageManager.h"
#include "SceneObject.h"
#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/EditMesh.h"

//------------------------------------------------------------------------------
// SBase
//------------------------------------------------------------------------------
#define MAX_BUF_SIZE 0xFFFF

CCustom2DProjector::CCustom2DProjector()
{
	name			= "";
	shader_overlap	= 0;
    shader_blended	= 0;
}

bool CCustom2DProjector::LoadImage(LPCSTR nm)
{
	name			= nm;
    ImageLib.LoadTextureData(*name,data,w,h);
    return Valid();
}

void CCustom2DProjector::CreateRMFromObjects(const Fbox& box, ObjectList& lst)
{
	geom.destroy();

    size_t faceCount = 0;
	for (ObjectIt it=lst.begin(); it!=lst.end(); it++){
    	CSceneObject*	 S = (CSceneObject*)(*it);
    	CEditableObject* O = S->GetReference(); VERIFY(O);
        for (EditMeshIt m_it = O->FirstMesh(); m_it != O->LastMesh(); m_it++)
            faceCount   += (*m_it)->GetFCount();
    }
    mesh.resize	(faceCount * 3);
    size_t vertIdx = 0;
	for (ObjectIt it=lst.begin(); it!=lst.end(); it++){
    	CSceneObject*	 S = (CSceneObject*)(*it);
    	CEditableObject* O = S->GetReference(); VERIFY(O);

        Fmatrix T; S->GetFullTransformToWorld(T);
        for (EditMeshIt m_it=O->FirstMesh(); m_it!=O->LastMesh(); m_it++){
	        for (u32 f_id=0; f_id!=(*m_it)->GetFCount(); f_id++){
                for (int k=0; k<3; k++){
                    FVF::V& v = mesh[vertIdx];
                	T.transform_tiny(v.p,(*m_it)->GetVertices()[(*m_it)->GetFaces()[f_id].pv[k].pindex]);
					v.t.x = GetUFromX(v.p.x,box);
					v.t.y = GetVFromZ(v.p.z,box);
                    vertIdx++;
                }
            }
        }
    }
	geom.create(FVF::F_V,RCache.Vertex.Buffer(),0);
}

void CCustom2DProjector::Render(bool blended)
{
	if (!Valid()) return;
    Device.RenderNearer(0.001f);
	RCache.set_xform_world(Fidentity);
    Device.SetShader	(blended?shader_blended:shader_overlap);
    div_t cnt 			= div(mesh.size(),MAX_BUF_SIZE);
    u32 vBase;
    _VertexStream* Stream = &RCache.Vertex;
    for (int k=0; k<cnt.quot; k++){
		FVF::V*	pv	 	= (FVF::V*)Stream->Lock(MAX_BUF_SIZE,geom->vb_stride,vBase);
		CopyMemory		(pv,mesh.data()+k*MAX_BUF_SIZE,sizeof(FVF::V)*MAX_BUF_SIZE);
		Stream->Unlock	(MAX_BUF_SIZE,geom->vb_stride);
		Device.DP		(D3DPT_TRIANGLELIST,geom,vBase,MAX_BUF_SIZE/3);
    }
    if (cnt.rem){
		FVF::V*	pv	 	= (FVF::V*)Stream->Lock(cnt.rem,geom->vb_stride,vBase);
		CopyMemory		(pv,mesh.data()+cnt.quot*MAX_BUF_SIZE,sizeof(FVF::V)*cnt.rem);
		Stream->Unlock	(cnt.rem,geom->vb_stride);
		Device.DP		(D3DPT_TRIANGLELIST,geom,vBase,cnt.rem/3);
    }
    Device.ResetNearer	();
}

void CCustom2DProjector::CreateShader()
{
	DestroyShader		();
	if (Valid()){
		shader_blended.create	("editor\\do_base",*name);
		shader_overlap.create	("default",*name);
		geom.create				(FVF::F_V,RCache.Vertex.Buffer(),0);
	}
}

void CCustom2DProjector::DestroyShader()
{
	geom.destroy();
	shader_blended.destroy();
	shader_overlap.destroy();
}

void CCustom2DProjector::OnImageChange	(PropValue* prop)
{
	LoadImage				(*name);
	DestroyShader			();
    CreateShader			();
}

void CCustom2DProjector::ReloadImage()
{
	LoadImage				(*name);
}

