//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "../ECore/Editor/EditObject.h"
#include "../ECore/Editor/EditMesh.h"
#include "EGarbageGenerator.h"
#include "Scene.h"
#include "SceneObject.h"
#include "ESceneObjectTools.h"
#include "ESceneShapeTools.h"
#include "EShape.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

void EGarbageGenerator::Generate(CSceneObject *terrain)
{
	ESceneObjectTool* ot = dynamic_cast<ESceneObjectTool*>(Scene->GetTool(OBJCLASS_SCENEOBJECT));

	if (!ot->m_AppendRandomObjects.size()) {
		 ELog.DlgMsg(mtError, "Append random object list is empty!");
		 return;
	}

	if (m_shape_restrictor_mode && m_restrictors.empty()) {
		 ELog.DlgMsg(mtError, "Shape restriction mode is enabled but no selected sphere shapes found, cannot continue");
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

		// ray missed terrain, don't create object
		if(!PQ.r_count()) {
			i++;
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

			// ray hit surface with not allowed game-material, don't create object
			if(!b_found) {
				i++;
				continue;
			}
		}
		
		Fvector pos = start;
		pos.mad(dir, R->range);

		if(m_shape_restrictor_mode) {
			for(size_t j = 0; j < m_restrictors.size(); j++) {
				Fvector &P = m_restrictors[j].pos;
				float R = m_restrictors[j].radius;
				if(pos.distance_to_sqr(P) < R*R) {
					goto restrictor_ok;
				}
			}

			// restrictor-test failed
			i++;
			continue;
		}

        // restrictor test passed
		restrictor_ok:

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

void EGarbageGenerator::GenerateEmitter(CSceneObject *terrain)
{
	ESceneObjectTool* ot = dynamic_cast<ESceneObjectTool*>(Scene->GetTool(OBJCLASS_SCENEOBJECT));

	if (!ot->m_AppendRandomObjects.size()) {
		 ELog.DlgMsg(mtError, "Append random object list is empty!");
		 return;
	}

	if (m_restrictors.empty()) {
		 ELog.DlgMsg(mtError, "Shape emitter mode is enabled but no selected sphere shapes found, cannot continue");
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

	Fbox terrain_bbox;
	terrain->GetBox(terrain_bbox);
	xr_vector<EGarbageGeneratorRestrictor> &emitters = m_restrictors;

	float objects_per_m2 = ot->m_AppendRandomObjectsPerM2;
	float area_total = 0.f;

	for(size_t i = 0; i < emitters.size(); i++) {
		area_total += M_PI*2.f*emitters[i].radius;
	}

	// not sure if it's correct lol
	int nobjects_total = iFloor(objects_per_m2 * area_total + 1.f);

	if(ELog.DlgMsg(mtInformation, TMsgDlgButtons() << mbYes << mbNo, "Emitters area is %f quadratic meters, up to %d objects may be generated. Do you want to continue?", area_total, nobjects_total) != mrYes)
		return;

	Scene->SelectObjects(false,OBJCLASS_SCENEOBJECT);

	for(size_t i = 0; i < m_restrictors.size(); i++) {

	float emm_area = M_PI*2.f*emitters[i].radius;
	int nobjects = iFloor(objects_per_m2 * emm_area + 1.f);

	for(int j = 0; j < nobjects;) {
		float rnd_angle = ::Random.randF() * M_PI * 2.f;
		float rnd_dist = ::Random.randF() * emitters[i].radius;

		Fvector start;
		start.set(emitters[i].pos.x + sin(rnd_angle) * rnd_dist,
				  emitters[i].pos.y + emitters[i].radius,
				  emitters[i].pos.z + cos(rnd_angle) * rnd_dist);

		Fvector dir;
		dir.set(0.f, -1.f, 0.f);

		SPickQuery PQ;
		PQ.prepare_rq(
			start,
			dir,
			terrain_bbox.y2 - terrain_bbox.y1 + 1.f,
			CDB::OPT_CULL|CDB::OPT_ONLYNEAREST
		);

		terrain->RayQuery(PQ);

		if(!PQ.r_count()) {
			j++;
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
				j++;
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

		j++;
	}

	}
}

void EGarbageGenerator::InitRestrictors(ESceneShapeTool *st)
{
	ObjectList ol;
	st->GetQueryObjects(ol,1,1,-1);

	for(ObjectIt it = ol.begin(); it != ol.end(); it++) {
		CEditShape *shape = (CEditShape*)*it;
		CShapeData::ShapeVec &shapes = shape->GetShapes();

		for(size_t i = 0; i < shapes.size(); i++) {
			if(shapes[i].type == CShapeData::cfSphere) {
				EGarbageGeneratorRestrictor r;
				shape->_Transform().transform_tiny(r.pos, shapes[i].data.sphere.P);
				r.radius = shapes[i].data.sphere.R*shape->_Scale().x; // apply scale
				m_restrictors.push_back(r);
            }
		}
	}
}
