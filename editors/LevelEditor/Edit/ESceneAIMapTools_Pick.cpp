#include "stdafx.h"
#pragma hdrstop

#include "ESceneAIMapTools.h"
#include "scene.h"
#include "../ECore/Editor/ui_main.h"
#include "ui_leveltools.h"
#include "ESceneAIMapControls.h"
#include "ui_levelmain.h"

SAINode* ESceneAIMapTool::PickNode(const Fvector& start, const Fvector& dir, float& dist)
{
	SAINode* R = 0;
	CFrustum frustum;
	LUI->m_CurrentCp.add(1); 			// fake add min vaalue for calculate frustum
	float psz	= (m_Params.fPatchSize/2)*(m_Params.fPatchSize/2);

	float cell_x = (m_AIBBox.max.x - m_AIBBox.min.x) / HDIM_X;
	float cell_z = (m_AIBBox.max.z - m_AIBBox.min.z) / HDIM_Z;

	if (LUI->SelectionFrustum(frustum)){
		for(int x = 0; x < HDIM_X+1; x++) {
			for(int z = 0; z < HDIM_Z+1; z++) {
				Fbox cell_bb;
				u32 mask = 0xffff;

				cell_bb.min.set(x * cell_x + m_AIBBox.min.x, m_AIBBox.min.y, z * cell_z + m_AIBBox.min.z);
				cell_bb.max.set((x+1) * cell_x + m_AIBBox.min.x, m_AIBBox.max.y, (z+1) * cell_z + m_AIBBox.min.z);

				if(frustum.testAABB(cell_bb.data(),mask)) {
					AINodeIt it = m_HASH[x][z].begin();
					AINodeIt end = m_HASH[x][z].end();
					for( ; it != end; it++) {
						SAINode& N 	= **it;
						u32 mask 	= 0xffff;
						Fbox bb; bb.set(N.Pos,N.Pos); bb.min.sub(m_Params.fPatchSize*0.35f); bb.max.add(m_Params.fPatchSize*0.35f);
						if (frustum.testSAABB(N.Pos,m_Params.fPatchSize,bb.data(),mask)){
							Fvector dest;
							if (N.Plane.intersectRayPoint(start,dir,dest)){
								if (N.Pos.distance_to_sqr(dest)<psz){
									float d = start.distance_to(dest);
									if (d<dist){
										R 	= &N;
										dist= d;
									}
								}
							}
						}
                    }
				}
			}
		}
	}

    return R;
}
bool ESceneAIMapTool::PickObjects(Fvector& dest, const Fvector& start, const Fvector& dir, float dist)
{         	         
	SPickQuery	PQ;
	if (!GetSnapList()->empty()){
        if (Scene->RayQuery(PQ,start,dir,dist,CDB::OPT_ONLYNEAREST|CDB::OPT_CULL,GetSnapList())){
            dest.mad(start,dir,PQ.r_begin()->range);
            return true;
        }
    }else{
    	ELog.DlgMsg(mtInformation,"Fill object list and try again.");
    }
    return false;
}
                             
int ESceneAIMapTool::RaySelect(int flag, float& distance, const Fvector& start, const Fvector& direction, BOOL bDistanceOnly)
{
	int count=0;
	if (!m_Flags.is(flHideNodes)){
		SAINode * N = PickNode(start, direction, distance);
		if (N&&!bDistanceOnly){
			if (flag==-1) 	SelectNode(N, !N->flags.is(SAINode::flSelected));
			else 			SelectNode(N, flag);
			count++;
		}

        UI->RedrawScene		();
	}

    return count;
}

int ESceneAIMapTool::FrustumSelect(int flag, const CFrustum& frustum)
{
    int count = 0;
	if (!m_Flags.is(flHideNodes)){
		float cell_x = (m_AIBBox.max.x - m_AIBBox.min.x) / HDIM_X;
		float cell_z = (m_AIBBox.max.z - m_AIBBox.min.z) / HDIM_Z;

		for(int x = 0; x < HDIM_X+1; x++) {
			for(int z = 0; z < HDIM_Z+1; z++) {
				Fbox cell_bb;
				u32 mask = 0xffff;

				cell_bb.min.set(x * cell_x + m_AIBBox.min.x, m_AIBBox.min.y, z * cell_z + m_AIBBox.min.z);
				cell_bb.max.set((x+1) * cell_x + m_AIBBox.min.x, m_AIBBox.max.y, (z+1) * cell_z + m_AIBBox.min.z);

				if(frustum.testAABB(cell_bb.data(),mask)) {
					AINodeIt it = m_HASH[x][z].begin();
					AINodeIt end = m_HASH[x][z].end();
					for( ; it != end; it++) {
						SAINode& N 	= **it;
						u32 mask 	= 0xffff;
						Fbox bb; bb.set(N.Pos,N.Pos); bb.min.sub(m_Params.fPatchSize*0.35f); bb.max.add(m_Params.fPatchSize*0.35f);
						if (frustum.testSAABB(N.Pos,m_Params.fPatchSize,bb.data(),mask)){
							if (flag==-1) 	SelectNode(*it, !(*it)->flags.is(SAINode::flSelected));
							else 			SelectNode(*it, flag);
							count++;
						}
                    }
                }
			}
		}
		
        UI->RedrawScene		();
 	}
    return count;
}

