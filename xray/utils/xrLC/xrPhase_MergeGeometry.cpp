#include "stdafx.h"
#include "build.h"
#include "../xrlc_light/xrface.h"
#include "../xrLC_Light/xrLC_GlobalData.h"

IC BOOL	FaceEqual		(Face* F1, Face* F2)
{
	if (F1->dwMaterial  != F2->dwMaterial)		return FALSE;
	if (F1->tc.size()	!= F2->tc.size())		return FALSE;
	if (F1->lmap_layer  != F2->lmap_layer)		return FALSE;
	return TRUE;
}

BOOL	NeedMerge		(vecFace& subdiv, Fbox& bb_base)
{
	// 1. Amount of polygons
	if (subdiv.size()>=u32(3*c_SS_HighVertLimit/4))	return FALSE;
	
	// 2. Bounding box
	bb_base.invalidate	();
	for (u32 it=0; it<subdiv.size(); it++)
	{
		Face* F = subdiv[it];
		bb_base.modify(F->v[0]->P);
		bb_base.modify(F->v[1]->P);
		bb_base.modify(F->v[2]->P);
	}
	bb_base.grow		(EPS_S);	// Enshure non-zero volume

	Fvector sz_base;	bb_base.getsize(sz_base);
	if (sz_base.x<c_SS_maxsize)		return TRUE;
	if (sz_base.y<c_SS_maxsize)		return TRUE;
	if (sz_base.z<c_SS_maxsize)		return TRUE;
	return FALSE;
}

float	Cuboid			(Fbox& BB)
{
	Fvector sz;			BB.getsize(sz);
	float	min			= sz.x;
	if (sz.y<min)	min = sz.y;
	if (sz.z<min)	min = sz.z;
	
	float	volume_cube	= min*min*min;
	float	volume		= sz.x*sz.y*sz.z;
	return  powf(volume_cube / volume, 1.f/7.f);
}

IC void	MakeCube		(Fbox& BB_dest, Fbox& BB_src)
{
	Fvector C,D;
	BB_src.get_CD		(C,D);
	float	max			= D.x;
	if (D.y>max)	max = D.y;
	if (D.z>max)	max = D.z;

	BB_dest.set			(C,C);
	BB_dest.grow		(max);
}

IC BOOL ValidateMergeLinearSize( const Fvector & merged, const Fvector & orig1, const Fvector & orig2, int iAxis)
{
	if ( ( merged[iAxis] > (4*c_SS_maxsize/3) ) &&
		( merged[iAxis] > (orig1[iAxis]+1) ) && 
		( merged[iAxis] > (orig2[iAxis]+1) ) )
		return FALSE;
	else
		return TRUE;
}

IC BOOL	ValidateMerge	(u32 f1, Fbox& bb_base, const Fbox& bb_base_orig, u32 f2, Fbox& bb, float& volume)
{
	// Polygons
	if ((f1+f2) > u32(4*c_SS_HighVertLimit/3))		return FALSE;	// Don't exceed limits (4/3 max POLY)	

	// Size
	Fbox	merge;	merge.merge		(bb_base,bb);
	Fvector sz;		merge.getsize	(sz);
	Fvector orig1;	bb_base_orig.getsize(orig1);
	Fvector orig2;	bb.getsize		(orig2);
//	if (sz.x>(4*c_SS_maxsize/3))			return FALSE;	// Don't exceed limits (4/3 GEOM)
//	if (sz.y>(4*c_SS_maxsize/3))			return FALSE;
//	if (sz.z>(4*c_SS_maxsize/3))			return FALSE;

	if (!ValidateMergeLinearSize(sz, orig1, orig2, 0))	return FALSE;	// Don't exceed limits (4/3 GEOM)
	if (!ValidateMergeLinearSize(sz, orig1, orig2, 1))	return FALSE;
	if (!ValidateMergeLinearSize(sz, orig1, orig2, 2))	return FALSE;

	// Volume
	Fbox		bb0,bb1;
	MakeCube	(bb0,bb_base);	float	v1	= bb0.getvolume	();
	MakeCube	(bb1,bb);		float	v2	= bb1.getvolume	();
	volume		= merge.getvolume	(); // / Cuboid(merge);
	if (volume > 2*2*2*(v1+v2))						return FALSE;	// Don't merge too distant groups (8 vol)

	// OK
	return TRUE;
}

void xrPhase_MergeGeometry_Legacy()
{
    for (u32 split = 0; split < g_XSplit.size(); split++) {
        vecFace& subdiv = *(g_XSplit[split]);
        bool bb_base_orig_inited = false;
        Fbox bb_base_orig;
        Fbox bb_base;
        while (NeedMerge(subdiv, bb_base)) {
            //	Save original AABB for later tests
            if (!bb_base_orig_inited) {
                bb_base_orig_inited = true;
                bb_base_orig = bb_base;
            }

            // **OK**. Let's find the best candidate for merge
            u32 selected = split;
            float selected_volume = flt_max;
            for (u32 test = split + 1; test < g_XSplit.size(); test++) {
                Fbox bb;
                float volume;
                vecFace& TEST = *(g_XSplit[test]);

                if (!FaceEqual(subdiv.front(), TEST.front()))
                    continue;
                if (!NeedMerge(TEST, bb))
                    continue;
                if (!ValidateMerge(subdiv.size(), bb_base, bb_base_orig, TEST.size(), bb, volume))
                    continue;

                if (volume < selected_volume) {
                    selected = test;
                    selected_volume = volume;
                }
            }
            if (selected == split)
                break; // No candidates for merge

            // **OK**. Perform merge
            subdiv.insert(subdiv.begin(), g_XSplit[selected]->begin(), g_XSplit[selected]->end());
            xr_delete(g_XSplit[selected]);
            g_XSplit.erase(g_XSplit.begin() + selected);
        }
        Progress(_sqrt(_sqrt(float(split) / float(g_XSplit.size()))));
    }
}

void xrPhase_MergeGeometry_Tbb();

void CBuild::xrPhase_MergeGeometry()
{
    Status("Processing...");
    validate_splits();
    if (lc_global_data()->useTbb())
        xrPhase_MergeGeometry_Tbb();
    else
        xrPhase_MergeGeometry_Legacy();
    clMsg("%d subdivisions.", g_XSplit.size());
    validate_splits();
}
