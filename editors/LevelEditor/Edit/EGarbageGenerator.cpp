//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/EditMesh.h"
#include "EGarbageGenerator.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ESceneObjectTools.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void EGarbageGenerator::Generate(CSceneObject *terrain)
{
	ESceneObjectTool* ot = dynamic_cast<ESceneObjectTool*>(Scene->GetTool(OBJCLASS_SCENEOBJECT));

	if (!ot->m_AppendRandomObjects.size()) {
		 ELog.DlgMsg(mtError, "Append random object list is empty!");
		 return;
	}

	// verify if terrain has any of allowed materials
	if(ot->m_AppendRandomMaterials.size()) {
		SurfaceVec& surfaces = terrain->GetReference()->Surfaces();

		for(SurfaceIt it = surfaces.begin(); it != surfaces.end(); it++) {
			for(size_t j = 0; j < ot->m_AppendRandomMaterials.size(); j++) {
				if(ot->m_AppendRandomMaterials[j] == (*it)->_GameMtlName()) {
					goto terrain_maybe_ok;
                }
            }
		}

		ELog.DlgMsg(mtError, "Terrain object doesn't contain surfaces with any of allowed materials!");
		return;

		terrain_maybe_ok:
    }

	Fbox bbox;
	terrain->GetBox(bbox);

	float objects_per_m2 = ot->m_AppendRandomObjectsPerM2;
	float area = (bbox.x2 - bbox.x1) * (bbox.z2 - bbox.z1);
	int nobjects = iFloor(objects_per_m2 * area + 1.f);

	if(ELog.DlgMsg(mtInformation, TMsgDlgButtons() << mbYes << mbNo, "Terrain area is %f quadratic meters, %d object will be generated. Do you want to continue?", area, nobjects) != mrYes)
		return;

	Scene->SelectObjects(false,OBJCLASS_SCENEOBJECT);

	for(int i = 0; i < nobjects;) {
		float rnd_x = ::Random.randF() * (bbox.x2 - bbox.x1) + bbox.x1;
		float rnd_z = ::Random.randF() * (bbox.z2 - bbox.z1) + bbox.z1;

		Fvector start; start.set(rnd_x, bbox.y2 + 1.f, rnd_z);
		Fvector dir; dir.set(0.f, -1.f, 0.f);

		SPickQuery PQ;
		PQ.prepare_rq(
			start,
			dir,
			bbox.y2 - bbox.y1 + 1.f,
			CDB::OPT_CULL|CDB::OPT_ONLYNEAREST
		);

		terrain->RayQuery(PQ);

		if(!PQ.r_count()) {
			if(ot->IsAppendRandomVaryingCountActive()) i++;
			continue;
		}

		SPickQuery::SResult *R = PQ.r_begin();

		if(ot->m_AppendRandomMaterials.size()) {
			shared_str material = R->e_mesh->GetSurfaceByFaceID(R->tag)->_GameMtlName();
			BOOL b_found = FALSE;
			for(size_t i = 0; i < ot->m_AppendRandomMaterials.size(); i++) {
				if(ot->m_AppendRandomMaterials[i] == material) {
					b_found = TRUE;
					break;
				}
			}

			if(!b_found) {
				if(ot->IsAppendRandomVaryingCountActive()) i++;
				continue;
			}
        }
		
		Fvector pos = start;
		pos.mad(dir, R->range);

		Fvector up;
		if(ot->IsAppendRandomNormalAlignmentActive())
			up.mknormal(R->verts[0], R->verts[1], R->verts[2]);
		else
			up.set(0.f,1.f,0.f);

		LPCSTR N = ot->m_AppendRandomObjects[::Random.randI(ot->m_AppendRandomObjects.size())].c_str();

		string256 namebuffer;
		Scene->GenObjectName(OBJCLASS_SCENEOBJECT, namebuffer, N);
		CSceneObject *obj = xr_new<CSceneObject>((LPVOID)0,namebuffer);

		CEditableObject* ref = obj->SetReference(N);
		if (!ref){
			ELog.DlgMsg(mtError,"Can't load reference object.");
			xr_delete(obj);
			return;
		}

		if (ot->IsAppendRandomRotationActive()){
			Fvector p;
			p.set(	::Random.randF(ot->m_AppendRandomMinRotation.x,ot->m_AppendRandomMaxRotation.x),
					::Random.randF(ot->m_AppendRandomMinRotation.y,ot->m_AppendRandomMaxRotation.y),
					::Random.randF(ot->m_AppendRandomMinRotation.z,ot->m_AppendRandomMaxRotation.z));
			obj->PRotation = p;
		}
		if (ot->IsAppendRandomScaleActive()){
			Fvector s;
			if (ot->IsAppendRandomScaleProportional()){
				s.x		= ::Random.randF(ot->m_AppendRandomMinScale.x,ot->m_AppendRandomMaxScale.x);
				s.set	(s.x,s.x,s.x);
			}else{
				s.set(	::Random.randF(ot->m_AppendRandomMinScale.x,ot->m_AppendRandomMaxScale.x),
						::Random.randF(ot->m_AppendRandomMinScale.y,ot->m_AppendRandomMaxScale.y),
						::Random.randF(ot->m_AppendRandomMinScale.z,ot->m_AppendRandomMaxScale.z));
			}
			obj->PScale = s;
		}

		obj->MoveTo(pos, up);
		obj->Select(TRUE);
		Scene->AppendObject(obj, false);

		i++;
	}
}
