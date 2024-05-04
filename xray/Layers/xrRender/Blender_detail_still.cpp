// Blender_Vertex_aref.cpp: implementation of the CBlender_Detail_Still class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#include "Blender_Detail_still.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_Detail_Still::CBlender_Detail_Still()
{
	description.CLS		= B_DETAIL;
	description.version	= 0;
}

CBlender_Detail_Still::~CBlender_Detail_Still()
{

}

void	CBlender_Detail_Still::Save		(IWriter& fs )
{
	IBlender::Save		(fs);
	xrPWRITE_PROP		(fs,"Alpha-blend",	xrPID_BOOL,		oBlend);
}

void	CBlender_Detail_Still::Load		(IReader& fs, u16 version )
{
	IBlender::Load		(fs,version);
	xrPREAD_PROP		(fs,xrPID_BOOL,		oBlend);
}

#if RENDER==R_R1
void	CBlender_Detail_Still::Compile	(CBlender_Compile& C)
{
	IBlender::Compile	(C);
	
	BOOL bInstancing = !!strstr(Core.Params,"-details_instanced");
	LPCSTR vs_name;
	
	if (C.bEditor)
	{
		C.PassBegin		();
		{
			C.PassSET_ZB		(TRUE,TRUE);
			if (oBlend.value)	C.PassSET_Blend_BLEND	(TRUE, 200);
			else				C.PassSET_Blend_SET		(TRUE, 200);
			C.PassSET_LightFog	(TRUE,TRUE);
			
			// Stage1 - Base texture
			C.StageBegin		();
			C.StageSET_Color	(D3DTA_TEXTURE,	  D3DTOP_MODULATE,	D3DTA_DIFFUSE);
			C.StageSET_Alpha	(D3DTA_TEXTURE,	  D3DTOP_MODULATE,	D3DTA_DIFFUSE);
			C.StageSET_TMC		(oT_Name,"$null","$null",0);
			C.StageEnd			();
		}
		C.PassEnd			();
	} else {
		switch (C.iElement)
		{
		case SE_R1_NORMAL_HQ:
			vs_name = bInstancing ? "inst_detail_wave" : "detail_wave";
			C.r_Pass	(vs_name,	"detail",FALSE,TRUE,TRUE,FALSE, D3DBLEND_ONE,D3DBLEND_ZERO,oBlend.value?TRUE:FALSE,oBlend.value?200:0);
			C.r_Sampler	("s_base",	C.L_textures[0]);
			C.r_End		();
			break;
		case SE_R1_NORMAL_LQ:
			vs_name = bInstancing ? "inst_detail_still" : "detail_still";
			C.r_Pass	(vs_name,	"detail",FALSE,TRUE,TRUE,FALSE, D3DBLEND_ONE,D3DBLEND_ZERO,oBlend.value?TRUE:FALSE,oBlend.value?200:0);
			C.r_Sampler	("s_base",	C.L_textures[0]);
			C.r_End		();
			break;
		case SE_R1_LPOINT:
			vs_name = bInstancing ? "inst_detail_point" : "detail_point";
			C.r_Pass		(vs_name,"add_point",FALSE,TRUE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE,oBlend.value?TRUE:FALSE,oBlend.value?200:0);
			C.r_Sampler		("s_base",	C.L_textures[0]);
			C.r_Sampler_clf	("s_lmap",	TEX_POINT_ATT		);
			C.r_Sampler_clf	("s_att",	TEX_POINT_ATT		);
			C.r_End			();
			break;
		case SE_R1_LSPOT:
			vs_name = bInstancing ? "inst_detail_spot" : "detail_spot";
			C.r_Pass		(vs_name,"add_spot",FALSE,TRUE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE,oBlend.value?TRUE:FALSE,oBlend.value?200:0);
			C.r_Sampler		("s_base",	C.L_textures[0]);
			C.r_Sampler_clf	("s_lmap",	"internal\\internal_light_att",		true);
			C.r_Sampler_clf	("s_att",	TEX_SPOT_ATT		);
			C.r_End			();
			break;
		case SE_R1_LMODELS:
			break;
		}
	}
}
#elif RENDER==R_R2
//////////////////////////////////////////////////////////////////////////
// R2
//////////////////////////////////////////////////////////////////////////
#include "uber_deffer.h"
void	CBlender_Detail_Still::Compile	(CBlender_Compile& C)
{
	IBlender::Compile	(C);

	BOOL bInstancing = !!strstr(Core.Params,"-details_instanced");
	LPCSTR vs_name;

	switch(C.iElement) 
	{
	case SE_R2_NORMAL_HQ: 		// deffer wave
		vs_name = bInstancing ? "detail_w_inst" : "detail_w";
		uber_deffer				(C,false,vs_name,"base",true);
		break;
	case SE_R2_NORMAL_LQ: 		// deffer still
		vs_name = bInstancing ? "detail_s_inst" : "detail_s";
		uber_deffer				(C,false,vs_name,"base",true);
		break;
	}
}
#else
//////////////////////////////////////////////////////////////////////////
// R3
//////////////////////////////////////////////////////////////////////////
#include "uber_deffer.h"
void	CBlender_Detail_Still::Compile	(CBlender_Compile& C)
{
	IBlender::Compile	(C);

	bool bUseATOC = (RImplementation.o.dx10_msaa_alphatest==CRender::MSAA_ATEST_DX10_0_ATOC);

	switch(C.iElement) 
	{
	case SE_R2_NORMAL_HQ: 		// deffer wave
		if (bUseATOC)
		{
			uber_deffer		(C,false,"detail_w","base_atoc",true,0,true);
			C.r_Stencil		( TRUE,D3DCMP_ALWAYS,0xff,0x7f,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
			C.r_StencilRef	(0x01);
			C.r_ColorWriteEnable(false, false, false, false);
			C.r_CullMode	(D3DCULL_NONE);
			//	Alpha to coverage.
			C.RS.SetRS	(XRDX10RS_ALPHATOCOVERAGE,	TRUE);
			C.r_End			();
		}
		

		uber_deffer		(C,false,"detail_w","base",true, 0, true);
		C.r_Stencil		( TRUE,D3DCMP_ALWAYS,0xff,0x7f,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
		C.r_StencilRef	(0x01);
		C.r_CullMode	(D3DCULL_NONE);
		if (bUseATOC)
			C.RS.SetRS	( D3DRS_ZFUNC, D3DCMP_EQUAL);
		C.r_End			();
		break;
	case SE_R2_NORMAL_LQ: 		// deffer still
		if (bUseATOC)
		{
			uber_deffer		(C,false,"detail_s","base_atoc",true,0,true);
			C.r_Stencil		( TRUE,D3DCMP_ALWAYS,0xff,0x7f,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
			C.r_StencilRef	(0x01);
			C.r_CullMode	(D3DCULL_NONE);
			C.r_ColorWriteEnable(false, false, false, false);
			//	Alpha to coverage.
			C.RS.SetRS	(XRDX10RS_ALPHATOCOVERAGE,	TRUE);
			C.r_End			();
		}

		uber_deffer		(C,false,"detail_s","base",true, 0, true);
		C.r_Stencil		( TRUE,D3DCMP_ALWAYS,0xff,0x7f,D3DSTENCILOP_KEEP,D3DSTENCILOP_REPLACE,D3DSTENCILOP_KEEP);
		C.r_StencilRef	(0x01);
		C.r_CullMode	(D3DCULL_NONE);
		//	Need this for ATOC
		if (bUseATOC)
			C.RS.SetRS	( D3DRS_ZFUNC, D3DCMP_EQUAL);
		C.r_End			();
		break;
	}
}
#endif
