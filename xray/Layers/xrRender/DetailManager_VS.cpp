#include "stdafx.h"


#include "detailmanager.h"

#ifdef _EDITOR
#	include <xrEngine/igame_persistent.h>
#	include <xrEngine/environment.h>
#else
#	include "../../xrEngine/igame_persistent.h"
#	include "../../xrEngine/environment.h"
#endif

#include "../xrRenderDX10/dx10BufferUtils.h"

const int			quant	= 16384;
const int			c_hdr	= 15;
const int			c_size	= 4;

static D3DVERTEXELEMENT9 dwDecl[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos
	{ 0, 12, D3DDECLTYPE_SHORT4,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },	// uv
	D3DDECL_END()
};

struct HWInstanceData
{
	Fvector4 M1;
	Fvector4 M2;
	Fvector4 M3;
	Fvector4 color;
};
const u32 dwInstanceSize = 64;

static D3DVERTEXELEMENT9 dwDecl_instanced[] =
{
	{ 0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,	0 },	// pos
	{ 0, 12, D3DDECLTYPE_SHORT4,	D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,	0 },	// uv
	{ 1, 0,  D3DDECLTYPE_FLOAT4,    D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD,  1 },    // M1
	{ 1, 16, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD,  2 },    // M2
	{ 1, 32, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD,  3 },    // M3
	{ 1, 48, D3DDECLTYPE_FLOAT4,	D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD,  4 },    // color
	D3DDECL_END()
};

#pragma pack(push,1)
struct	vertHW
{
	float		x,y,z;
	short		u,v,t,mid;
};
#pragma pack(pop)

short QC (float v)
{
	int t=iFloor(v*float(quant)); clamp(t,-32768,32767);
	return short(t&0xffff);
}

void CDetailManager::hw_Load	()
{
#ifndef USE_DX10
	bInstancing = !!strstr(Core.Params,"-details_instanced");
	dwMaxInstances = 2048;

	if (bInstancing)
		hw_Load_Geom_instanced();
	else
#endif
		hw_Load_Geom();
	hw_Load_Shaders();
}

void CDetailManager::hw_Load_Geom()
{
	// Analyze batch-size
	hw_BatchSize	= (u32(HW.Caps.geometry.dwRegisters)-c_hdr)/c_size;
	clamp			(hw_BatchSize,(u32)0,(u32)64);
	Msg				("* [DETAILS] VertexConsts(%d), Batch(%d)",u32(HW.Caps.geometry.dwRegisters),hw_BatchSize);

	// Pre-process objects
	u32			dwVerts		= 0;
	u32			dwIndices	= 0;
	for (u32 o=0; o<objects.size(); o++)
	{
		const CDetail& D	=	*objects[o];
		dwVerts		+=	D.number_vertices*hw_BatchSize;
		dwIndices	+=	D.number_indices*hw_BatchSize;
	}
	u32			vSize		= sizeof(vertHW);
	Msg("* [DETAILS] %d v(%d), %d p",dwVerts,vSize,dwIndices/3);

#ifndef	USE_DX10
	// Determine POOL & USAGE
	u32 dwUsage		=	D3DUSAGE_WRITEONLY;

	// Create VB/IB
	R_CHK			(HW.pDevice->CreateVertexBuffer(dwVerts*vSize,dwUsage,0,D3DPOOL_MANAGED,&hw_VB,0));
	R_CHK			(HW.pDevice->CreateIndexBuffer(dwIndices*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&hw_IB,0));
#endif	//	USE_DX10
	Msg("* [DETAILS] Batch(%d), VB(%dK), IB(%dK)",hw_BatchSize,(dwVerts*vSize)/1024, (dwIndices*2)/1024);

	// Fill VB
	{
		vertHW*			pV;
#ifdef	USE_DX10
		vertHW*			pVOriginal;
		pVOriginal	=	xr_alloc<vertHW>(dwVerts);
		pV = pVOriginal;		
#else	//	USE_DX10
		R_CHK			(hw_VB->Lock(0,0,(void**)&pV,0));
#endif	//	USE_DX10
		for (u32 o=0; o<objects.size(); o++)
		{
			const CDetail& D		=	*objects[o];
			for (u32 batch=0; batch<hw_BatchSize; batch++)
			{
				u32 mid	=	batch*c_size;
				for (u32 v=0; v<D.number_vertices; v++)
				{
					const Fvector&	vP = D.vertices[v].P;
					pV->x	=	vP.x;
					pV->y	=	vP.y;
					pV->z	=	vP.z;
					pV->u	=	QC(D.vertices[v].u);
					pV->v	=	QC(D.vertices[v].v);
					pV->t	=	QC(vP.y/(D.bv_bb.max.y-D.bv_bb.min.y));
					pV->mid	=	short(mid);
					pV++;
				}
			}
		}
#ifdef	USE_DX10
		R_CHK(dx10BufferUtils::CreateVertexBuffer(&hw_VB, pVOriginal, dwVerts*vSize));
		xr_free(pVOriginal);
#else	//	USE_DX10
		R_CHK			(hw_VB->Unlock());
#endif	//	USE_DX10
	}

	// Fill IB
	{
		u16*			pI;
#ifdef	USE_DX10
		u16*			pIOriginal;
		pIOriginal = xr_alloc<u16>(dwIndices);
		pI	= pIOriginal;
#else	//	USE_DX10
		R_CHK			(hw_IB->Lock(0,0,(void**)(&pI),0));
#endif	//	USE_DX10
		for (u32 o=0; o<objects.size(); o++)
		{
			const CDetail& D		=	*objects[o];
			u16		offset	=	0;
			for (u32 batch=0; batch<hw_BatchSize; batch++)
			{
				for (u32 i=0; i<u32(D.number_indices); i++)
					*pI++	=	u16(u16(D.indices[i]) + u16(offset));
				offset		=	u16(offset+u16(D.number_vertices));
			}
		}
#ifdef	USE_DX10
		R_CHK(dx10BufferUtils::CreateIndexBuffer(&hw_IB, pIOriginal, dwIndices*2));
		xr_free(pIOriginal);
#else	//	USE_DX10
		R_CHK			(hw_IB->Unlock());
#endif	//	USE_DX10
	}

	// Declare geometry
	hw_Geom.create		(dwDecl, hw_VB, hw_IB);
}

#ifndef USE_DX10
void CDetailManager::hw_Load_Geom_instanced()
{
	Msg("* [DETAILS] Instanced(%d)", dwMaxInstances);

	// Pre-process objects
	u32			dwVerts = 0;
	u32			dwIndices = 0;
	for (u32 o = 0; o < objects.size(); o++)
	{
		const CDetail& D = *objects[o];
		dwVerts += D.number_vertices;
		dwIndices += D.number_indices;
	}
	u32			vSize = sizeof(vertHW);
	Msg("* [DETAILS] %d v(%d), %d p", dwVerts, vSize, dwIndices / 3);

	// Determine POOL & USAGE
	u32 dwUsage = D3DUSAGE_WRITEONLY;

	// Create VB/IB
	R_CHK(HW.pDevice->CreateVertexBuffer(dwInstanceSize * dwMaxInstances, dwUsage, 0, D3DPOOL_MANAGED, &hw_VBInstances, 0));
	R_CHK(HW.pDevice->CreateVertexBuffer(dwVerts * vSize, dwUsage, 0, D3DPOOL_MANAGED, &hw_VB, 0));
	R_CHK(HW.pDevice->CreateIndexBuffer(dwIndices * 2, dwUsage, D3DFMT_INDEX16, D3DPOOL_MANAGED, &hw_IB, 0));

	Msg("* [DETAILS] Instanced(%d), VB(%dK), IB(%dK)", dwMaxInstances, (dwVerts * vSize) / 1024, (dwIndices * 2) / 1024);

	// Fill VB
	{
		vertHW* pV;
		R_CHK(hw_VB->Lock(0, 0, (void**)&pV, 0));
		for (u32 o = 0; o < objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			for (u32 v = 0; v < D.number_vertices; v++)
			{
				const Fvector& vP = D.vertices[v].P;
				pV->x = vP.x;
				pV->y = vP.y;
				pV->z = vP.z;
				pV->u = QC(D.vertices[v].u);
				pV->v = QC(D.vertices[v].v);
				pV->t = QC(vP.y / (D.bv_bb.max.y - D.bv_bb.min.y));
				pV->mid = 0;
				pV++;
			}
		}
		R_CHK(hw_VB->Unlock());
	}

	// Fill IB
	{
		u16* pI;
		R_CHK(hw_IB->Lock(0, 0, (void**)(&pI), 0));
		for (u32 o = 0; o < objects.size(); o++)
		{
			const CDetail& D = *objects[o];
			for (u32 i = 0; i < u32(D.number_indices); i++)
				*pI++ = u16(D.indices[i]);
		}
		R_CHK(hw_IB->Unlock());
	}

	// Declare geometry
	hw_Geom.create(dwDecl_instanced, hw_VB, hw_IB);
}
#endif

void CDetailManager::hw_Unload()
{
	// Destroy VS/VB/IB
	hw_Geom.destroy				();
	_RELEASE					(hw_IB);
	_RELEASE					(hw_VB);

#ifndef USE_DX10
	if (bInstancing)
		_RELEASE(hw_VBInstances);
#endif
}

#ifndef	USE_DX10
void CDetailManager::hw_Load_Shaders()
{
	// Create shader to access constant storage
	ref_shader		S;	S.create("details\\set");
	R_constant_table&	T0	= *(S->E[0]->passes[0]->constants);
	R_constant_table&	T1	= *(S->E[1]->passes[0]->constants);
	hwc_consts			= T0.get("consts");
	hwc_wave			= T0.get("wave");
	hwc_wind			= T0.get("dir2D");
	hwc_array			= T0.get("array");
	hwc_s_consts		= T1.get("consts");
	hwc_s_xform			= T1.get("xform");
	hwc_s_array			= T1.get("array");

#if RENDER==R_R1
	R_constant_table&	T2	= *(S->E[SE_R1_LPOINT]->passes[0]->constants);
	hwc_lp_consts		= T2.get("consts");
	hwc_lp_wave			= T2.get("wave");
	hwc_lp_wind			= T2.get("dir2D");
	hwc_lp_array		= T2.get("array");

	R_constant_table&	T3	= *(S->E[SE_R1_LSPOT]->passes[0]->constants);
	hwc_ls_consts		= T3.get("consts");
	hwc_ls_wave			= T3.get("wave");
	hwc_ls_wind			= T3.get("dir2D");
	hwc_ls_array		= T3.get("array");
#endif
}

void CDetailManager::hw_Render(u16 dlight_flag)
{
	// Render-prepare
	//	Update timer
	//	Can't use Device.fTimeDelta since it is smoothed! Don't know why, but smoothed value looks more choppy!
	float fDelta = Device.fTimeGlobal-m_global_time_old;
	if ( (fDelta<0) || (fDelta>1))	fDelta = 0.03;
	m_global_time_old = Device.fTimeGlobal;

	m_time_rot_1	+= (PI_MUL_2*fDelta/swing_current.rot1);
	m_time_rot_2	+= (PI_MUL_2*fDelta/swing_current.rot2);
	m_time_pos		+= fDelta*swing_current.speed;

	//float		tm_rot1		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot1);
	//float		tm_rot2		= (PI_MUL_2*Device.fTimeGlobal/swing_current.rot2);
	float		tm_rot1		= m_time_rot_1;
	float		tm_rot2		= m_time_rot_2;

	Fvector4	dir1,dir2;
	dir1.set				(_sin(tm_rot1),0,_cos(tm_rot1),0).normalize().mul(swing_current.amp1);
	dir2.set				(_sin(tm_rot2),0,_cos(tm_rot2),0).normalize().mul(swing_current.amp2);

	// Setup geometry and DMA
	RCache.set_Geometry		(hw_Geom);
	if (bInstancing)
	{
		R_CHK(HW.pDevice->SetStreamSource(1, hw_VBInstances, 0, 64));
		R_CHK(HW.pDevice->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1));
	}

	// Wave0
	float		scale			=	1.f/float(quant);
	Fvector4	wave;

	if (dlight_flag == 1) 
	{ 
		//wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	m_time_pos);
		RCache.set_c			(&*hwc_lp_consts,	scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);	// consts
		RCache.set_c			(&*hwc_lp_wave,	wave.div(PI_MUL_2));															// wave
		RCache.set_c			(&*hwc_lp_wind,	dir1);																			// wind-dir
		if (bInstancing)
			hw_Render_instanced	(1, SE_R1_LPOINT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_lp_array,	1, SE_R1_LPOINT, dlight_flag );

		// Wave1
		//wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	m_time_pos);
		RCache.set_c			(&*hwc_lp_wave,	wave.div(PI_MUL_2));															// wave
		RCache.set_c			(&*hwc_lp_wind,	dir2);																			// wind-dir
		if (bInstancing)
			hw_Render_instanced	(2, SE_R1_LPOINT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_lp_array,	2, SE_R1_LPOINT, dlight_flag );

		// Still
		RCache.set_c			(&*hwc_lp_wind,		0.f, 0.f, 0.f, 0.f);
		if (bInstancing)
			hw_Render_instanced	(0, SE_R1_LPOINT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_lp_array,	0, SE_R1_LPOINT, dlight_flag );
	} 
	else if (dlight_flag == 2) 
	{ 
		//wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	m_time_pos);
		RCache.set_c			(&*hwc_ls_consts,	scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);	// consts
		RCache.set_c			(&*hwc_ls_wave,	wave.div(PI_MUL_2));															// wave
		RCache.set_c			(&*hwc_ls_wind,	dir1);																			// wind-dir
		if (bInstancing)
			hw_Render_instanced	(1, SE_R1_LSPOT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_ls_array,	1, SE_R1_LSPOT, dlight_flag );

		// Wave1
		//wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	m_time_pos);
		RCache.set_c			(&*hwc_ls_wave,	wave.div(PI_MUL_2));															// wave
		RCache.set_c			(&*hwc_ls_wind,	dir2);																			// wind-dir
		if (bInstancing)
			hw_Render_instanced	(2, SE_R1_LSPOT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_ls_array,	2, SE_R1_LSPOT, dlight_flag );

		// Still
		RCache.set_c			(&*hwc_ls_wind,		0.f, 0.f, 0.f, 0.f);
		if (bInstancing)
			hw_Render_instanced	(0, SE_R1_LSPOT, dlight_flag);
		else
			hw_Render_dump		(&*hwc_ls_array,	0, SE_R1_LSPOT, dlight_flag );
	} 
	else 
	{
		//wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/5.f,		1.f/7.f,	1.f/3.f,	m_time_pos);
		RCache.set_c			(&*hwc_consts,	scale,		scale,		ps_r__Detail_l_aniso,	ps_r__Detail_l_ambient);				// consts
		RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
		RCache.set_c			(&*hwc_wind,	dir1);																					// wind-dir
		if (bInstancing)
			hw_Render_instanced	(1, 0, dlight_flag);
		else
			hw_Render_dump		(&*hwc_array,	1, 0, dlight_flag );

		// Wave1
		//wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	Device.fTimeGlobal*swing_current.speed);
		wave.set				(1.f/3.f,		1.f/7.f,	1.f/5.f,	m_time_pos);
		RCache.set_c			(&*hwc_wave,	wave.div(PI_MUL_2));	// wave
		RCache.set_c			(&*hwc_wind,	dir2);																					// wind-dir
		if (bInstancing)
			hw_Render_instanced	(2, 0, dlight_flag);
		else
			hw_Render_dump		(&*hwc_array,	2, 0, dlight_flag );

		// Still
		RCache.set_c			(&*hwc_s_consts,scale,		scale,		scale,				1.f);
		RCache.set_c			(&*hwc_s_xform,	Device.mFullTransform);
		if (bInstancing)
			hw_Render_instanced	(0, 1, dlight_flag);
		else
			hw_Render_dump		(&*hwc_array,	0, 1, dlight_flag );
	}


	if (bInstancing)
	{
		R_CHK(HW.pDevice->SetStreamSource(1, 0, 0, 0));
		R_CHK(HW.pDevice->SetStreamSourceFreq(0, 1));
		R_CHK(HW.pDevice->SetStreamSourceFreq(1, 1));
	}
}

void	CDetailManager::hw_Render_dump		(ref_constant x_array, u32 var_id, u32 lod_id, u16 dlight_flag)
{
#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_Count	= 0;
#else
	Statistic.RenderDUMP_DT_Count	= 0;
#endif

	// Matrices and offsets
	u32		vOffset	=	0;
	u32		iOffset	=	0;

	vis_list& list	=	m_visibles	[var_id];

	Fvector					c_sun,c_ambient,c_hemi;
#ifndef _EDITOR
	CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
	c_sun.set				(desc.sun_color.x,	desc.sun_color.y,	desc.sun_color.z);	c_sun.mul(.5f);
	c_ambient.set			(desc.ambient.x,	desc.ambient.y,		desc.ambient.z);
	c_hemi.set				(desc.hemi_color.x, desc.hemi_color.y,	desc.hemi_color.z);
#else
	c_sun.set				(1,1,1);	c_sun.mul(.5f);
	c_ambient.set			(1,1,1);
	c_hemi.set				(1,1,1);
#endif    

	VERIFY(objects.size()<=list.size());

	// Iterate
	for (u32 O=0; O<objects.size(); O++){
		CDetail&	Object				= *objects	[O];
		xr_vector <SlotItemVec* >& vis	= list		[O];
		if (!vis.empty()){
			// Setup matrices + colors (and flush it as nesessary)
			RCache.set_Element				(Object.shader->E[lod_id]);
			RImplementation.apply_lmaterial	();
			u32			c_base				= x_array->vs.index;
			Fvector4*	c_storage			= RCache.get_ConstantCache_Vertex().get_array_f().access(c_base);

			u32 dwBatch	= 0;

			xr_vector <SlotItemVec* >::iterator _vI = vis.begin();
			xr_vector <SlotItemVec* >::iterator _vE = vis.end();
			for (; _vI!=_vE; _vI++){
				SlotItemVec*	items		= *_vI;
				SlotItemVecIt _iI			= items->begin();
				SlotItemVecIt _iE			= items->end();
				for (; _iI!=_iE; _iI++){
					SlotItem&	Instance	= **_iI;
					u32			base		= dwBatch*4;

					if (dlight_flag != 1234)
					if (Instance.dlight_flag!=dlight_flag) continue;

					// Build matrix ( 3x4 matrix, last row - color )
					float		scale		= Instance.scale_calculated;
					Fmatrix&	M			= Instance.mRotY;
					c_storage[base+0].set	(M._11*scale,	M._21*scale,	M._31*scale,	M._41	);
					c_storage[base+1].set	(M._12*scale,	M._22*scale,	M._32*scale,	M._42	);
					c_storage[base+2].set	(M._13*scale,	M._23*scale,	M._33*scale,	M._43	);

					// Build color
#if RENDER==R_R1
					Fvector C;
					C.set					(c_ambient);
//					C.mad					(c_lmap,Instance.c_rgb);
					C.mad					(c_hemi,Instance.c_hemi);
					C.mad					(c_sun,	Instance.c_sun);
					c_storage[base+3].set	(C.x,			C.y,			C.z,			1.f		);
#else
					// R2 only needs hemisphere
					float		h			= Instance.c_hemi;
					float		s			= Instance.c_sun;
					c_storage[base+3].set	(s,				s,				s,				h		);
#endif
					dwBatch	++;
					if (dwBatch == hw_BatchSize)	{
						// flush
#ifdef _EDITOR
						Device.Statistic->RenderDUMP_DT_Count			+=	dwBatch;
#else
						Statistic.RenderDUMP_DT_Count					+=	dwBatch;
#endif
						u32 dwCNT_verts			= dwBatch * Object.number_vertices;
						u32 dwCNT_prims			= (dwBatch * Object.number_indices)/3;
						RCache.get_ConstantCache_Vertex().b_dirty				=	TRUE;
						RCache.get_ConstantCache_Vertex().get_array_f().dirty	(c_base,c_base+dwBatch*4);
						RCache.Render			(D3DPT_TRIANGLELIST,vOffset, 0, dwCNT_verts,iOffset,dwCNT_prims);
						RCache.stat.r.s_details.add	(dwCNT_verts);

						// restart
						dwBatch					= 0;
					}
				}
			}
			// flush if nessecary
			if (dwBatch)
			{
#ifdef _EDITOR
				Device.Statistic->RenderDUMP_DT_Count	+= dwBatch;
#else
				Statistic.RenderDUMP_DT_Count	+= dwBatch;
#endif
				u32 dwCNT_verts			= dwBatch * Object.number_vertices;
				u32 dwCNT_prims			= (dwBatch * Object.number_indices)/3;
				RCache.get_ConstantCache_Vertex().b_dirty				=	TRUE;
				RCache.get_ConstantCache_Vertex().get_array_f().dirty	(c_base,c_base+dwBatch*4);
				RCache.Render				(D3DPT_TRIANGLELIST,vOffset,0,dwCNT_verts,iOffset,dwCNT_prims);
				RCache.stat.r.s_details.add	(dwCNT_verts);
			}
		}
		vOffset		+=	hw_BatchSize * Object.number_vertices;
		iOffset		+=	hw_BatchSize * Object.number_indices;
	}
}

void	CDetailManager::hw_Render_instanced	(u32 var_id, u32 lod_id, u16 dlight_flag)
{
#ifdef _EDITOR
	Device.Statistic->RenderDUMP_DT_Count	= 0;
#else
	Statistic.RenderDUMP_DT_Count	= 0;
#endif

	// Matrices and offsets
	u32		vOffset	=	0;
	u32		iOffset	=	0;

	vis_list& list	=	m_visibles	[var_id];

	Fvector					c_sun,c_ambient,c_hemi;
#ifndef _EDITOR
	CEnvDescriptor&	desc	= *g_pGamePersistent->Environment().CurrentEnv;
	c_sun.set				(desc.sun_color.x,	desc.sun_color.y,	desc.sun_color.z);	c_sun.mul(.5f);
	c_ambient.set			(desc.ambient.x,	desc.ambient.y,		desc.ambient.z);
	c_hemi.set				(desc.hemi_color.x, desc.hemi_color.y,	desc.hemi_color.z);
#else
	c_sun.set				(1,1,1);	c_sun.mul(.5f);
	c_ambient.set			(1,1,1);
	c_hemi.set				(1,1,1);
#endif    

	VERIFY(objects.size()<=list.size());

	// Iterate
	for (u32 O=0; O<objects.size(); O++){
		CDetail&	Object				= *objects	[O];
		xr_vector <SlotItemVec* >& vis	= list		[O];
		if (!vis.empty()){
			// Setup matrices + colors (and flush it as nesessary)
			RCache.set_Element				(Object.shader->E[lod_id]);
			RImplementation.apply_lmaterial	();

			HWInstanceData* pInstData;
			R_CHK(hw_VBInstances->Lock(0, dwMaxInstances*dwInstanceSize, (void**)&pInstData, D3DLOCK_DISCARD));
			u32 dwInstance = 0;

			xr_vector <SlotItemVec* >::iterator _vI = vis.begin();
			xr_vector <SlotItemVec* >::iterator _vE = vis.end();
			for (; _vI!=_vE; _vI++){
				SlotItemVec*	items		= *_vI;
				SlotItemVecIt _iI			= items->begin();
				SlotItemVecIt _iE			= items->end();
				for (; _iI!=_iE; _iI++){
					SlotItem&	Instance	= **_iI;
					HWInstanceData& InstData = pInstData[dwInstance];

					if (dlight_flag != 1234)
					if (Instance.dlight_flag!=dlight_flag) continue;

					// Build matrix ( 3x4 matrix, last row - color )
					float		scale		= Instance.scale_calculated;
					Fmatrix&	M			= Instance.mRotY;
					InstData.M1.set			(M._11*scale,	M._21*scale,	M._31*scale,	M._41	);
					InstData.M2.set			(M._12*scale,	M._22*scale,	M._32*scale,	M._42	);
					InstData.M3.set			(M._13*scale,	M._23*scale,	M._33*scale,	M._43	);

					// Build color
#if RENDER==R_R1
					Fvector C;
					C.set					(c_ambient);
//					C.mad					(c_lmap,Instance.c_rgb);
					C.mad					(c_hemi,Instance.c_hemi);
					C.mad					(c_sun,	Instance.c_sun);
					InstData.color.set		(C.x,			C.y,			C.z,			1.f		);
#else
					// R2 only needs hemisphere
					float		h			= Instance.c_hemi;
					float		s			= Instance.c_sun;
					InstData.color.set		(s,				s,				s,				h		);
#endif
					dwInstance++;
					if (dwInstance == dwMaxInstances)	{
						// flush
#ifdef _EDITOR
						Device.Statistic->RenderDUMP_DT_Count			+=	dwInstance;
#else
						Statistic.RenderDUMP_DT_Count					+=	dwInstance;
#endif
						u32 dwCNT_verts			= Object.number_vertices;
						u32 dwCNT_prims			= Object.number_indices/3;

						R_CHK(hw_VBInstances->Unlock());
						R_CHK(HW.pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | dwInstance));

						RCache.Render			(D3DPT_TRIANGLELIST,vOffset,0,dwCNT_verts,iOffset,dwCNT_prims);
						RCache.stat.r.s_details.add	(dwCNT_verts);

						// restart
						dwInstance					= 0;
						R_CHK(hw_VBInstances->Lock(0, dwMaxInstances*dwInstanceSize, (void**)&pInstData, D3DLOCK_DISCARD));
					}
				}
			}

			R_CHK(hw_VBInstances->Unlock());

			// flush if nessecary
			if (dwInstance)
			{
				// flush
#ifdef _EDITOR
				Device.Statistic->RenderDUMP_DT_Count			+=	dwInstance;
#else
				Statistic.RenderDUMP_DT_Count					+=	dwInstance;
#endif
				u32 dwCNT_verts			= Object.number_vertices;
				u32 dwCNT_prims			= Object.number_indices/3;

				R_CHK(HW.pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | dwInstance));
				RCache.Render			(D3DPT_TRIANGLELIST,vOffset,0,dwCNT_verts,iOffset,dwCNT_prims);
				RCache.stat.r.s_details.add	(dwCNT_verts);
			}
		}
		vOffset		+=	Object.number_vertices;
		iOffset		+=	Object.number_indices;
	}
}

#endif	//	USE_DX10