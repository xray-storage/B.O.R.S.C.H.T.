// DetailManager.cpp: implementation of the CDetailManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include "DetailManager.h"
#include "../../Include/cl_intersect.h"

#ifdef _EDITOR
#	include <editors/LevelEditor/Edit/ESceneClassList.h>
#	include <editors/LevelEditor/Edit/Scene.h>
#	include <editors/LevelEditor/Edit/SceneObject.h>
#	include <xrEngine/igame_persistent.h>
#	include <xrEngine/environment.h>
#else
#	include "../../xrEngine/igame_persistent.h"
#	include "../../xrEngine/environment.h"
#endif

const float dbgOffset			= 0.f;
const int	dbgItems			= 128;

//--------------------------------------------------- Decompression
static int magic4x4[4][4] =
{
 	{ 0, 14,  3, 13},
	{11,  5,  8,  6},
	{12,  2, 15,  1},
	{ 7,  9,  4, 10}
};

void bwdithermap	(int levels, int magic[16][16])
{
	/* Get size of each step */
    float N = 255.0f / (levels - 1);

	/*
	* Expand 4x4 dither pattern to 16x16.  4x4 leaves obvious patterning,
	* and doesn't give us full intensity range (only 17 sublevels).
	*
	* magicfact is (N - 1)/16 so that we get numbers in the matrix from 0 to
	* N - 1: mod N gives numbers in 0 to N - 1, don't ever want all
	* pixels incremented to the next level (this is reserved for the
	* pixel value with mod N == 0 at the next level).
	*/

    float	magicfact = (N - 1) / 16;
    for ( int i = 0; i < 4; i++ )
		for ( int j = 0; j < 4; j++ )
			for ( int k = 0; k < 4; k++ )
				for ( int l = 0; l < 4; l++ )
					magic[4*k+i][4*l+j] =
					(int)(0.5 + magic4x4[i][j] * magicfact +
					(magic4x4[k][l] / 16.) * magicfact);
}
//--------------------------------------------------- Decompression

void CDetailManager::SSwingValue::lerp(const SSwingValue& A, const SSwingValue& B, float f)
{
	float fi	= 1.f-f;
	amp1		= fi*A.amp1  + f*B.amp1;
	amp2		= fi*A.amp2  + f*B.amp2;
	rot1		= fi*A.rot1  + f*B.rot1;
	rot2		= fi*A.rot2  + f*B.rot2;
	speed		= fi*A.speed + f*B.speed;
}
//---------------------------------------------------

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDetailManager::CDetailManager	()
{
	dtFS 		= 0;
	dtSlots		= 0;
	soft_Geom	= 0;
	hw_Geom		= 0;
	hw_BatchSize= 0;
	hw_VB		= 0;
	hw_IB		= 0;
	m_time_rot_1 = 0;
	m_time_rot_2 = 0;
	m_time_pos	= 0;
	m_global_time_old = 0;

	// KD: variable detail radius
	dm_size = dm_current_size;
	dm_cache_line = dm_current_cache_line;
	dm_cache1_line = dm_current_cache1_line;
	dm_cache_size = dm_current_cache_size;
	dm_fade = dm_current_fade;
	ps_r__Detail_density = ps_current_detail_density;
	cache_level1 = (CacheSlot1**)Memory.mem_alloc(dm_cache1_line * sizeof(CacheSlot1*)
#ifdef USE_MEMORY_MONITOR
		, "CDetailManager::cache_level1"
#endif
	);
	for (u32 i = 0; i < dm_cache1_line; ++i)
	{
		cache_level1[i] = (CacheSlot1*)Memory.mem_alloc(dm_cache1_line * sizeof(CacheSlot1)
#ifdef USE_MEMORY_MONITOR
			, "CDetailManager::cache_level1 " + i
#endif
		);
		for (u32 j = 0; j < dm_cache1_line; ++j)
			new (&(cache_level1[i][j])) CacheSlot1();
	}

	cache = (Slot***)Memory.mem_alloc(dm_cache_line * sizeof(Slot**)
#ifdef USE_MEMORY_MONITOR
		, "CDetailManager::cache"
#endif
	);
	for (u32 i = 0; i < dm_cache_line; ++i)
		cache[i] = (Slot**)Memory.mem_alloc(dm_cache_line * sizeof(Slot*)
#ifdef USE_MEMORY_MONITOR
			, "CDetailManager::cache " + i
#endif		
		);

	cache_pool = (Slot *)Memory.mem_alloc(dm_cache_size * sizeof(Slot)
#ifdef USE_MEMORY_MONITOR
		, "CDetailManager::cache_pool"
#endif
	);
	for (u32 i = 0; i < dm_cache_size; ++i)
		new (&(cache_pool[i])) Slot();
}

CDetailManager::~CDetailManager	()
{
	for (u32 i = 0; i < dm_cache_size; ++i)
		cache_pool[i].~Slot();
	Memory.mem_free(cache_pool);

	for (u32 i = 0; i < dm_cache_line; ++i)
		Memory.mem_free(cache[i]);
	Memory.mem_free(cache);

	for (u32 i = 0; i < dm_cache1_line; ++i)
	{
		for (u32 j = 0; j < dm_cache1_line; ++j)
			cache_level1[i][j].~CacheSlot1();
		Memory.mem_free(cache_level1[i]);
	}
	Memory.mem_free(cache_level1);
}
/*
*/
#ifndef _EDITOR

/*
void dump	(CDetailManager::vis_list& lst)
{
	for (int i=0; i<lst.size(); i++)
	{
		Msg("%8x / %8x / %8x",	lst[i]._M_start, lst[i]._M_finish, lst[i]._M_end_of_storage._M_data);
	}
}
*/
void CDetailManager::Load		()
{
	// Open file stream
	if (!FS.exist("$level$","level.details"))
	{
		dtFS	= NULL;
		return;
	}

	string_path			fn;
	FS.update_path		(fn,"$level$","level.details");
	dtFS				= FS.r_open(fn);

	// Header
	dtFS->r_chunk_safe	(0,&dtH,sizeof(dtH));
	R_ASSERT			(dtH.version == DETAIL_VERSION);
	u32 m_count			= dtH.object_count;

	// Models
	IReader* m_fs		= dtFS->open_chunk(1);
	for (u32 m_id = 0; m_id < m_count; m_id++)
	{
		CDetail*		dt	= xr_new<CDetail> ();
		IReader* S			= m_fs->open_chunk(m_id);
		dt->Load			(S);
		objects.push_back	(dt);
		S->close			();
	}
	m_fs->close		();

	// Get pointer to database (slots)
	IReader* m_slots	= dtFS->open_chunk(2);
	dtSlots				= (DetailSlot*)m_slots->pointer();
	m_slots->close		();

	// Initialize 'vis' and 'cache'
	for (u32 i=0; i<3; ++i)	m_visibles[i].resize(objects.size());
	cache_Initialize	();

	// Make dither matrix
	bwdithermap		(2,dither);

	// Hardware specific optimizations
	if (UseVS())	hw_Load		();
	else			soft_Load	();

	// swing desc
	// normal
	swing_desc[0].amp1	= pSettings->r_float("details","swing_normal_amp1");
	swing_desc[0].amp2	= pSettings->r_float("details","swing_normal_amp2");
	swing_desc[0].rot1	= pSettings->r_float("details","swing_normal_rot1");
	swing_desc[0].rot2	= pSettings->r_float("details","swing_normal_rot2");
	swing_desc[0].speed	= pSettings->r_float("details","swing_normal_speed");
	// fast
	swing_desc[1].amp1	= pSettings->r_float("details","swing_fast_amp1");
	swing_desc[1].amp2	= pSettings->r_float("details","swing_fast_amp2");
	swing_desc[1].rot1	= pSettings->r_float("details","swing_fast_rot1");
	swing_desc[1].rot2	= pSettings->r_float("details","swing_fast_rot2");
	swing_desc[1].speed	= pSettings->r_float("details","swing_fast_speed");

	m_mask.create("level_detail_mask");
	m_mask._get()->Load();
	m_mask._get()->convert(D3DFMT_A8R8G8B8);
	m_maskData = (u32*)m_mask._get()->getData();
}
#endif
void CDetailManager::Unload		()
{
#ifndef _EDITOR
    if (m_mask) {
        m_mask._get()->releaseData();
        m_mask.destroy();
    }
#endif

	if (UseVS())	hw_Unload	();
	else			soft_Unload	();

	for (DetailIt it=objects.begin(); it!=objects.end(); it++){
		(*it)->Unload();
		xr_delete		(*it);
    }
	objects.clear		();
	m_visibles[0].clear	();
	m_visibles[1].clear	();
	m_visibles[2].clear	();
	FS.r_close			(dtFS);
}

extern ECORE_API float r_ssaDISCARD;

void CDetailManager::UpdateVisibleM()
{
	// Clean up
	for (int i = 0; i != 3; i++)
		for (auto& vis: m_visibles[i])
			vis.clear_not_free();

	Fvector		EYE				= Device.vCameraPosition;
	
	CFrustum	View;
	View.CreateFromMatrix		(Device.mFullTransform, FRUSTUM_P_LRTB + FRUSTUM_P_FAR);

	float fade_limit			= dm_fade;	fade_limit=fade_limit*fade_limit;
	float fade_start			= 1.f;		fade_start=fade_start*fade_start;
	float fade_range			= fade_limit-fade_start;
	float		r_ssaCHEAP		= 16*r_ssaDISCARD;

	// Initialize 'vis' and 'cache'
	// Collect objects for rendering
#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_VIS.Begin	();
#else
	Statistic.RenderDUMP_DT_VIS.Begin	();
#endif
	for (int _mz=0; _mz<dm_cache1_line; _mz++){
		for (int _mx=0; _mx<dm_cache1_line; _mx++){
			CacheSlot1& MS		= cache_level1[_mz][_mx];
			if (MS.empty)		continue;
			u32 mask			= 0xff;
			u32 res				= View.testSAABB		(MS.vis.sphere.P,MS.vis.sphere.R,MS.vis.box.data(),mask);
			if (fcvNone==res)						 	continue;	// invisible-view frustum
			// test slots
			for (int _i=0; _i<dm_cache1_count*dm_cache1_count; _i++){
				Slot*	PS		= *MS.slots[_i];
				Slot& 	S 		= *PS;

				// if slot empty - continue
				if (S.empty)	continue;

				// if upper test = fcvPartial - test inner slots
				if (fcvPartial==res){
					u32 _mask	= mask;
					u32 _res	= View.testSAABB			(S.vis.sphere.P,S.vis.sphere.R,S.vis.box.data(),_mask);
					if (fcvNone==_res)						continue;	// invisible-view frustum
				}
#ifndef _EDITOR
				if (!RImplementation.HOM.visible(S.vis))	continue;	// invisible-occlusion
#endif
				// Add to visibility structures
				if (Device.dwFrame>S.frame){
					// Calc fade factor	(per slot)
					float	dist_sq		= EYE.distance_to_sqr	(S.vis.sphere.P);
					if		(dist_sq>fade_limit)				continue;
					float	alpha		= (dist_sq<fade_start)?0.f:(dist_sq-fade_start)/fade_range;
					float	alpha_i		= 1.f - alpha;
					float	dist_sq_rcp	= 1.f / dist_sq;

					S.frame			= Device.dwFrame+Random.randI(15,30);
					for (int sp_id=0; sp_id<dm_obj_in_slot; sp_id++){
						SlotPart&			sp	= S.G		[sp_id];
						if (sp.id==DetailSlot::ID_Empty)	continue;

						sp.r_items[0].clear_not_free();
						sp.r_items[1].clear_not_free();
						sp.r_items[2].clear_not_free();

						float				R		= objects	[sp.id]->bv_sphere.R;
						float				Rq_drcp	= R*R*dist_sq_rcp;	// reordered expression for 'ssa' calc

						SlotItem			**siIT=&(*sp.items.begin()), **siEND=&(*sp.items.end());
						for (; siIT!=siEND; siIT++){
							SlotItem& Item			= *(*siIT);
							float   scale			= Item.scale_calculated	= Item.scale*alpha_i;
							float	ssa				= scale*scale*Rq_drcp;
							if (ssa < r_ssaDISCARD) continue;
							u32		vis_id			= 0;
							if (ssa > r_ssaCHEAP)	vis_id = Item.vis_ID;
							
							sp.r_items[vis_id].push_back	(*siIT);

//2							visible[vis_id][sp.id].push_back(&Item);
						}
					}
				}
				for (int sp_id=0; sp_id<dm_obj_in_slot; sp_id++){
					SlotPart&			sp	= S.G		[sp_id];
					if (sp.id==DetailSlot::ID_Empty)	continue;
					if (!sp.r_items[0].empty()) m_visibles[0][sp.id].push_back(&sp.r_items[0]);
					if (!sp.r_items[1].empty()) m_visibles[1][sp.id].push_back(&sp.r_items[1]);
					if (!sp.r_items[2].empty()) m_visibles[2][sp.id].push_back(&sp.r_items[2]);
				}
			}
		}
	}
#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_VIS.End	();
#else
	Statistic.RenderDUMP_DT_VIS.End	();
#endif
}

u32 CDetailManager::UpdateVisibleM_LPoint(const Fvector& P, float R) 
{ 
	u32 count = 0;
	float R2 = R*R;
	int i, j, k, m; 

	for (i = 0; i < 3; i++) {
		for (j = 0; j < m_visibles[i].size(); j++) {
			for (k = 0; k < m_visibles[i][j].size(); k++) {
				SlotItemVec& vec = *m_visibles[i][j][k];
				for (m = 0; m < vec.size(); m++) {
					vec[m]->dlight_flag = 1234;
					if (P.distance_to_sqr(vec[m]->mRotY.c) < R2)
					{
						vec[m]->dlight_flag = 1;
						count++;
					}
				}
			}
		}
	}

	return count;
}

u32 CDetailManager::UpdateVisibleM_LSpot(const Fvector& P, const Fvector& Ldir, float angle, float R)
{
	u32 count = 0;
	float R2 = R*R;
	float cos_angle = cos(angle);
	int i, j, k, m;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < m_visibles[i].size(); j++) {
			for (k = 0; k < m_visibles[i][j].size(); k++) {
				SlotItemVec& vec = *m_visibles[i][j][k];
				for (m = 0; m < vec.size(); m++) {
					vec[m]->dlight_flag = 1234;
					if (P.distance_to_sqr(vec[m]->mRotY.c) < R2) {
						Fvector D; D.sub(vec[m]->mRotY.c, P);
						D.normalize();
						if (D.dotproduct(Ldir)>=cos_angle) {
							vec[m]->dlight_flag = 2;
							count++;
						}
					}
				}
			}
		}
	}

	return count;
}

void CDetailManager::Render	(u16 dlight_flag)
{
#ifndef _EDITOR
	if (0==dtFS)						return;
	if (!psDeviceFlags.is(rsDetails))	return;
#endif

	// MT
	MT_SYNC					();

#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_Render.Begin	();
#else
	Statistic.RenderDUMP_DT_Render.Begin	();
#endif

#ifndef _EDITOR
	float factor			= g_pGamePersistent->Environment().wind_strength_factor;
#else
	float factor			= 0.3f;
#endif
	swing_current.lerp		(swing_desc[0],swing_desc[1],factor);

	RCache.set_CullMode		(CULL_NONE);
	RCache.set_xform_world	(Fidentity);
	if (UseVS())			hw_Render	(dlight_flag);
	else					soft_Render	();
	RCache.set_CullMode		(CULL_CCW);
#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_Render.End	();
#else
	Statistic.RenderDUMP_DT_Render.End	();
#endif
	m_frame_rendered		= Device.dwFrame;
}

void CDetailManager::MT_CALC		()
{
#ifndef _EDITOR
	if (0==RImplementation.Details)		return;	// possibly deleted
	if (0==dtFS)						return;
	if (!psDeviceFlags.is(rsDetails))	return;
#endif    

	MT.Enter					();
	if (m_frame_calc!=Device.dwFrame)	
		if ((m_frame_rendered+1)==Device.dwFrame) //already rendered
		{
			Fvector		EYE				= Device.vCameraPosition;
			int s_x	= iFloor			(EYE.x/dm_slot_size+.5f);
			int s_z	= iFloor			(EYE.z/dm_slot_size+.5f);

#ifndef _EDITOR
			editMask(EYE);
#endif

#ifdef _EDITOR
			Device.Statistic->RenderDUMP_DT_Cache.Begin	();
#else
			Statistic.RenderDUMP_DT_Cache.Begin	();
#endif
			cache_Update				(s_x,s_z,EYE,dm_max_decompress);
#ifdef _EDITOR
			Device.Statistic->RenderDUMP_DT_Cache.End	();
#else
			Statistic.RenderDUMP_DT_Cache.End	();
#endif

			UpdateVisibleM				();
			m_frame_calc				= Device.dwFrame;
		}
	MT.Leave					        ();
}
#ifndef _EDITOR
void CDetailManager::setUseMask(bool useMask)
{
    m_useMask = useMask;
    invalidateCache();
}

void CDetailManager::setMaskEditMode(MaskEditModes mode) { m_maskEditMode = mode; }

bool CDetailManager::maskChanged() const { return m_maskChanged; }

void CDetailManager::saveMask()
{
    if (!m_maskChanged)
        return;

    m_mask._get()->releaseData();
    string_path fileName;
    FS.update_path(fileName, "$level$", "level_detail_mask.png");
    m_mask._get()->save(fileName);
    m_maskData = (u32*)m_mask._get()->getData();
    m_maskChanged = false;
}

void CDetailManager::editMask(const Fvector& p)
{
    if (m_maskEditMode == MaskEditModes::None)
        return;

    u32* mask = sampleMask(p);
    bool changed = false;
    if (m_maskEditMode == MaskEditModes::FillByCamera && *mask != 0xFFFFFFFF) {
        *mask = 0xFFFFFFFF;
        changed = true;
    }
    if (m_maskEditMode == MaskEditModes::ClearByCamera && *mask != 0xFF000000) {
        *mask = 0xFF000000;
        changed = true;
    }

    if (changed) {
        int s_x = iFloor(p.x / dm_slot_size + .5f);
        int s_z = iFloor(p.z / dm_slot_size + .5f);
        int gx = w2cg_X(s_x);
        int gz = w2cg_Z(s_z);
        // ����������� ������� 3�3 ������ �����
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                auto s = cache[gz + dz][gx + dx];
                s->frame = 0;
                for (int sp_id = 0; sp_id < dm_obj_in_slot; sp_id++) {
                    SlotPart& sp = s->G[sp_id];
                    sp.r_items[0].clear_not_free();
                    sp.r_items[1].clear_not_free();
                    sp.r_items[2].clear_not_free();
                }
                cache_Task(gx + dx, gz + dz, s);
            }
        }
        m_maskChanged = true;
    }
}

void CDetailManager::invalidateCache()
{
    for (u32 i = 0; i < dm_cache_line; i++)
        for (u32 j = 0; j < dm_cache_line; j++) {
            auto s = cache[i][j];
            s->frame = 0;
            for (int sp_id = 0; sp_id < dm_obj_in_slot; sp_id++) {
                SlotPart& sp = s->G[sp_id];
                sp.r_items[0].clear_not_free();
                sp.r_items[1].clear_not_free();
                sp.r_items[2].clear_not_free();
            }
            cache_Task(j, i, s);
        }
}

bool CDetailManager::isMaskCleared(const Fvector& p) const
{
    u32 mask = *sampleMask(p);
    return (mask & 0x00FFFFFF) == 0x00000000; // ������ � ����� ������������� => ����� �� ������
}

bool CDetailManager::useMask() const { return m_useMask; }

MaskEditModes CDetailManager::maskEditMode() const { return m_maskEditMode; }

u32* CDetailManager::sampleMask(const Fvector& p) const
{
    const Fbox& bb = g_pGameLevel->ObjectSpace.GetBoundingVolume();
    const u32 width = m_mask._get()->get_Width(), height = m_mask._get()->get_Height();
    u32 u = (p.x - bb.x1) / (bb.x2 - bb.x1) * width;
    u32 v = height - (p.z - bb.z1) / (bb.z2 - bb.z1) * height;
	if (u >= width || v >= height)
        return m_maskData;
    u32* mask = m_maskData + u + v * width;
    return mask;
}
#endif