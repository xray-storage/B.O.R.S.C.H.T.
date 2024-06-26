// Build.cpp: implementation of the CBuild class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "build.h"

#include "../xrLC_Light/xrMU_Model.h"
#include "../xrLC_Light/xrMU_Model_Reference.h"

#include "../xrlc_light/xrThread.h"
#include "../../xrcore/xrSyncronize.h"


#include "../xrLC_Light/xrLC_GlobalData.h"
#include "../xrLC_Light/xrface.h"

void	calc_ogf		( xrMU_Model &	mu_model );
void	export_geometry	( xrMU_Model &	mu_model );

void	export_ogf		( xrMU_Reference& mu_reference );

using namespace			std;
struct OGF_Base;
xr_vector<OGF_Base *>	g_tree;
BOOL					b_R2		= FALSE;
BOOL					b_noise		= FALSE;
BOOL					b_radiosity	= FALSE;
BOOL					b_net_light	= FALSE;
vec2Face				g_XSplit;

CThreadManager			mu_base;
CThreadManager			mu_secondary;

//////////////////////////////////////////////////////////////////////

CBuild::CBuild()
{
	

}

CBuild::~CBuild()
{
	destroy_global_data();
}
 
CMemoryWriter&	CBuild::err_invalid()
{
	VERIFY(lc_global_data()); 
	return lc_global_data()->err_invalid(); 
}

xr_vector<b_material>&	CBuild::materials()	
{
	VERIFY(lc_global_data()); 
	return lc_global_data()->materials(); 
}
xr_vector<b_BuildTexture>&	CBuild::textures()		
{
	VERIFY(lc_global_data());
	return lc_global_data()->textures(); 
}

base_lighting&	CBuild::L_static()
{
	VERIFY(lc_global_data()); return lc_global_data()->L_static(); 
}

Shader_xrLC_LIB&	CBuild::shaders()		
{
	VERIFY(lc_global_data()); 
	return lc_global_data()->shaders(); 
}

extern u16		RegisterShader		(LPCSTR T);

// mu-light
class CMULight	: public CThread
{
	u32			low;
	u32			high;
public:
	CMULight	(u32 ID, u32 _low, u32 _high) : CThread(ID)	{	thMessages	= FALSE; low=_low; high=_high;	}

	virtual void	Execute	()
	{
		// Priority
		SetThreadPriority	(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		Sleep				(0);

		// Light references
		for (u32 m=low; m<high; m++)
		{
			pBuild->mu_refs()[m]->calc_lighting	();
			thProgress							= (float(m-low)/float(high-low));
		}
	}
};

class CMUThread : public CThread
{
public:
	CMUThread	(u32 ID) : CThread(ID)
	{
		thMessages	= FALSE;
	}
	virtual void	Execute()
	{
		u32					m;

		// Priority
		SetThreadPriority	(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		Sleep				(0);

		// Light models
		for (m=0; m<pBuild->mu_models().size(); m++)
		{
			pBuild->mu_models()[m]->calc_materials();
			pBuild->mu_models()[m]->calc_lighting	();
		}

		// Light references
        auto numThreads = lc_global_data()->numThread();
		u32	stride			= pBuild->mu_refs().size()/numThreads;
		u32	last			= pBuild->mu_refs().size()-stride*(numThreads-1);
		for (u32 thID=0; thID<numThreads; thID++)
			mu_secondary.start	(xr_new<CMULight> (thID,thID*stride,thID*stride+((thID==(numThreads-1))?last:stride)));
	}
};

void CBuild::Light_prepare()
{
	for (vecFaceIt I=lc_global_data()->g_faces().begin();	I!=lc_global_data()->g_faces().end(); I++) (*I)->CacheOpacity();
	for (u32 m=0; m<mu_models().size(); m++)	mu_models()[m]->calc_faceopacity();
}

#ifdef LOAD_GL_DATA
void net_light ();
#endif
void CBuild::Run	(LPCSTR P)
{
	lc_global_data()->initialize();
#ifdef LOAD_GL_DATA
	net_light ();
	return;
#endif
	//****************************************** Open Level
	strconcat					(sizeof(path),path,P,"\\")	;
	string_path					lfn				;
	IWriter* fs					= FS.w_open		(strconcat(sizeof(lfn),lfn,path,"level."));
	fs->open_chunk				(fsL_HEADER)	;
	hdrLEVEL H;	
	H.XRLC_version				= XRCL_PRODUCTION_VERSION;
	H.XRLC_quality				= g_params().m_quality;
	fs->w						(&H,sizeof(H));
	fs->close_chunk				();

	//****************************************** Dumb entry in shader-registration
	RegisterShader				("");

	//****************************************** Saving lights
	{
		string256			fn;
		IWriter*		fs	= FS.w_open	(strconcat(sizeof(fn),fn,pBuild->path,"build.lights"));
		fs->w_chunk			(0,&*L_static().rgb.begin(),L_static().rgb.size()*sizeof(R_Light));
		fs->w_chunk			(1,&*L_static().hemi.begin(),L_static().hemi.size()*sizeof(R_Light));
		fs->w_chunk			(2,&*L_static().sun.begin(),L_static().sun.size()*sizeof(R_Light));
		FS.w_close			(fs);
	}

	//****************************************** Optimizing + checking for T-junctions
	FPU::m64r					();
	Phase						("Optimizing...");
	mem_Compact					();
	PreOptimize					();
	CorrectTJunctions			();

	//****************************************** HEMI-Tesselate
	FPU::m64r					();
	Phase						("Adaptive HT...");
	mem_Compact					();
	xrPhase_AdaptiveHT			();

	//****************************************** Building normals
	FPU::m64r					();
	Phase						("Building normals...");
	mem_Compact					();
	CalcNormals					();
	//SmoothVertColors			(5);

	//****************************************** Collision DB
	//should be after normals, so that double-sided faces gets separated
	FPU::m64r					();
	Phase						("Building collision database...");
	mem_Compact					();
	BuildCForm					();

	BuildPortals				(*fs);

	//****************************************** T-Basis
	{
		FPU::m64r					();
		Phase						("Building tangent-basis...");
		xrPhase_TangentBasis		();
		mem_Compact					();
	}

	//****************************************** GLOBAL-RayCast model
	FPU::m64r					();
	Phase						("Building rcast-CFORM model...");
	mem_Compact					();
	Light_prepare				();
	BuildRapid					(TRUE);

	if (lc_global_data()->cformOnly())
		return;

	//****************************************** GLOBAL-ILLUMINATION
	if (b_radiosity)			
	{
		FPU::m64r					();
		Phase						("Radiosity-Solver...");
		mem_Compact					();
		Light_prepare				();
		xrPhase_Radiosity			();
	}

	//****************************************** Starting MU
	FPU::m64r					();
	Phase						("LIGHT: Starting MU...");
	mem_Compact					();
	Light_prepare				();
	StartMu						();
	//****************************************** Resolve materials
	FPU::m64r					();
	Phase						("Resolving materials...");
	mem_Compact					();
	xrPhase_ResolveMaterials	();
	IsolateVertices				(TRUE);

	//****************************************** UV mapping
	{
		FPU::m64r					();
		Phase						("Build UV mapping...");
		mem_Compact					();
		xrPhase_UVmap				();
		IsolateVertices				(TRUE);
	}

	//****************************************** Subdivide geometry
	FPU::m64r					();
	Phase						("Subdividing geometry...");
	mem_Compact					();
	xrPhase_Subdivide			();
	IsolateVertices				(TRUE);

	//****************************************** All lighting + lmaps building and saving
#ifdef NET_CMP
	mu_base.wait				(500);
	mu_secondary.wait			(500);
#endif
	if (!lc_global_data()->b_no_lmaps())
		Light					();
	RunAfterLight				( fs );

}
void	CBuild::StartMu	()
{
  mu_base.start				(xr_new<CMUThread> (0));
}
void CBuild::	RunAfterLight			( IWriter* fs	)
{
 	//****************************************** Merge geometry
	FPU::m64r					();
	Phase						("Merging geometry...");
	mem_Compact					();
	xrPhase_MergeGeometry		();

	//****************************************** Convert to OGF
	FPU::m64r					();
	Phase						("Converting to OGFs...");
	mem_Compact					();
	Flex2OGF					();

	//****************************************** Wait for MU
	FPU::m64r					();
	Phase						("LIGHT: Waiting for MU-thread...");
	mem_Compact					();
	mu_base.wait				(500);
	mu_secondary.wait			(500);

	//****************************************** Export MU-models
	FPU::m64r					();
	Phase						("Converting MU-models to OGFs...");
	mem_Compact					();
	{
		u32 m;
		Status			("MU : Models...");
		for (m=0; m<mu_models().size(); m++)	{
			calc_ogf			(*mu_models()[m]);
			export_geometry		(*mu_models()[m]);
		}

		Status			("MU : References...");
		for (m=0; m<mu_refs().size(); m++)
			export_ogf(*mu_refs()[m]);
	}

	//****************************************** Destroy RCast-model
	FPU::m64r		();
	Phase			("Destroying ray-trace model...");
	mem_Compact		();
	lc_global_data()->destroy_rcmodel();

	//****************************************** Build sectors
	FPU::m64r		();
	Phase			("Building sectors...");
	mem_Compact		();
	BuildSectors	();

	//****************************************** Saving MISC stuff
	FPU::m64r		();
	Phase			("Saving...");
	mem_Compact		();
	SaveLights		(*fs);

	fs->open_chunk	(fsL_GLOWS);
	
	for (u32 i=0; i<glows.size(); i++)
	{
		b_glow&	G	= glows[i];
		fs->w		(&G,4*sizeof(float));
		string1024	sid;
		strconcat	(sizeof(sid),sid,
			shader_render[materials()[G.dwMaterial].shader].name,
			"/",
			textures()		[materials()[G.dwMaterial].surfidx].name
			);
		fs->w_u16	(RegisterShader(sid));
	}
	fs->close_chunk	();

	SaveTREE		(*fs);
	SaveSectors		(*fs);

	err_save		();
}

void CBuild::err_save	()
{
	string_path		log_name;
	strconcat		(sizeof(log_name),log_name,"build_",Core.UserName,".err");
	FS.update_path	(log_name,"$logs$",log_name);

	IWriter*		fs	= FS.w_open(log_name);
	IWriter&		err = *fs;

	// t-junction
	err.open_chunk	(0);
	err.w_u32		(err_tjunction.size()/(1*sizeof(Fvector)));
	err.w			(err_tjunction.pointer(), err_tjunction.size());
	err.close_chunk	();

	// m-edje
	err.open_chunk	(1);
	err.w_u32		(err_multiedge.size()/(2*sizeof(Fvector)));
	err.w			(err_multiedge.pointer(), err_multiedge.size());
	err.close_chunk	();

	// invalid
	err.open_chunk	(2);
	err.w_u32		(err_invalid().size()/(3*sizeof(Fvector)));
	err.w			(err_invalid().pointer(), err_invalid().size());
	err.close_chunk	();
	
	FS.w_close(fs);
}

void CBuild::MU_ModelsCalculateNormals()
{
		for		(u32 m=0; m<mu_models().size(); m++)
			calc_normals( *mu_models()[m] );
}

xr_vector<xrMU_Model*>&CBuild::mu_models()
{
	VERIFY(lc_global_data()); 
	return lc_global_data()->mu_models(); 
}

xr_vector<xrMU_Reference*>&CBuild::mu_refs()
{
	VERIFY(lc_global_data()); 
	return lc_global_data()->mu_refs(); 
}

void CBuild::ImplicitLighting()
{
	::ImplicitLighting();
}