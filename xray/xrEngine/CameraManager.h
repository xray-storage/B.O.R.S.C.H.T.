#pragma once

#include "CameraDefs.h"

struct ENGINE_API SPPInfo {
	struct SColor{
		float r, g, b;
		SColor					(){}
		SColor					(float _r, float _g, float _b):r(_r),g(_g),b(_b){}
		IC operator u32()										{
			int		_r	= clampr	(iFloor(r*255.f+.5f),0,255);
			int		_g	= clampr	(iFloor(g*255.f+.5f),0,255);
			int		_b	= clampr	(iFloor(b*255.f+.5f),0,255);
			return color_rgba		(_r,_g,_b,0);
		}

		IC operator const Fvector&()										{
			return *((Fvector*)this);
		}

		IC SColor& operator +=	(const SColor &ppi)				{
			r += ppi.r; g += ppi.g; b += ppi.b; 
			return *this;
		}
		IC SColor& operator -=	(const SColor &ppi)				{
			r -= ppi.r; g -= ppi.g; b -= ppi.b; 
			return *this;
		}
		IC SColor& set			(float _r, float _g, float _b)	{
			r=_r;g=_g;b=_b;
			return *this;
		}
	};
	float		blur, gray;
	struct SDuality { 
		float h, v; 
		SDuality				(){}
		SDuality				(float _h, float _v):h(_h),v(_v){}
		IC SDuality& set		(float _h, float _v)			{
			h=_h;v=_v;
			return *this;
		}
	} duality;
	struct SNoise	{
		float		intensity, grain;
		float		fps;
		SNoise					(){}
		SNoise					(float _i, float _g, float _f):intensity(_i),grain(_g),fps(_f){}
		IC SNoise& set			(float _i, float _g, float _f){
			intensity=_i;grain=_g;fps=_f;
			return *this;
		}
	} noise;

	SColor		color_base;
	SColor		color_gray;
	SColor		color_add;
	float		fish_eye;
	Fvector2	vignette;

	IC SPPInfo& add (const SPPInfo &ppi) 
	{
		blur		+= ppi.blur;
		gray		+= ppi.gray;
		duality.h	+= ppi.duality.h; 
		duality.v += ppi.duality.v;
		
		noise.intensity	= _max(noise.intensity, ppi.noise.intensity);
		noise.grain		= _max(noise.grain, ppi.noise.grain);
		noise.fps		= _max(noise.fps, ppi.noise.fps);

//		noise.intensity += ppi.noise.intensity; 
//		noise.grain += ppi.noise.grain;
//		noise.fps	+= ppi.noise.fps;
		color_base	+= ppi.color_base;
		color_gray	+= ppi.color_gray;
		color_add	+= ppi.color_add;

		fish_eye = _max(fish_eye, ppi.fish_eye);
		vignette.x = _min(vignette.x, ppi.vignette.x);
		vignette.y = _max(vignette.y, ppi.vignette.y);

		return *this;
	}
	IC SPPInfo& sub (const SPPInfo &ppi) 
	{
		blur		-= ppi.blur;
		gray		-= ppi.gray;
		duality.h	-= ppi.duality.h; 
		duality.v	-= ppi.duality.v;
//		noise.intensity -= ppi.noise.intensity; 
//		noise.grain -= ppi.noise.grain;
//		noise.fps	-= ppi.noise.fps;
		color_base	-= ppi.color_base;
		color_gray	-= ppi.color_gray;
		color_add	-= ppi.color_add;
		return *this;
	}
	void normalize		();
	SPPInfo				()
	{
		blur = gray = duality.h = duality.v = 0;
		noise.intensity=0; noise.grain = 1; noise.fps = 10;
		color_base.set	(.5f,	.5f,	.5f);
		color_gray.set	(.333f, .333f,	.333f);
		color_add.set	(0.f,	0.f,	0.f);
		fish_eye = 0.0f;
		vignette.set(1.0f, 0.0f);
	}
	void zero()
	{
		blur = gray = duality.h = duality.v = fish_eye = vignette.x = vignette.y = 0.0f;
		noise.intensity = 0;
		noise.grain = 0.0f;
		noise.fps = 0.0f;
		color_base.set(0, 0, 0);
		color_gray.set(0, 0, 0);
		color_add.set(0, 0, 0);
	}
	SPPInfo&	lerp(const SPPInfo& def, const SPPInfo& to, float factor);
	void		validate(LPCSTR str);
};

DEFINE_VECTOR				(CEffectorCam*,EffectorCamVec,EffectorCamIt);
DEFINE_VECTOR				(CEffectorPP*,EffectorPPVec,EffectorPPIt);

struct SCamEffectorInfo;
class ENGINE_API CCameraManager
{
protected:
	SCamEffectorInfo		m_cam_info;

	EffectorCamVec			m_EffectorsCam;
	EffectorCamVec			m_EffectorsCam_added_deffered;
	EffectorPPVec			m_EffectorsPP;

	bool					m_bAutoApply;
	SPPInfo					pp_affected;
	void					UpdateDeffered();

	virtual void			UpdateCamEffectors		();
	virtual void			UpdatePPEffectors		();
	virtual bool			ProcessCameraEffector	(CEffectorCam* eff);
			void			OnEffectorReleased		(SBaseEffector* e);
public:
#ifdef DEBUG	
	u32						dbg_upd_frame;
#endif

	void					Dump					();
	CEffectorCam*			AddCamEffector			(CEffectorCam*		ef);
	CEffectorCam*			GetCamEffector			(ECamEffectorType	type);
	void					RemoveCamEffector		(ECamEffectorType	type);

	CEffectorPP*			GetPPEffector			(EEffectorPPType	type);
	CEffectorPP*			GetPPEffector			(const CEffectorPP*	ef);
	CEffectorPP*			AddPPEffector			(CEffectorPP*		ef);
	void					RemovePPEffector		(EEffectorPPType	type);
	void					RemovePPEffector		(const CEffectorPP*	ef);

	IC Fvector				Position				()	const { return m_cam_info.p;	}
	IC Fvector				Direction				()	const { return m_cam_info.d;}
	IC Fvector				Up						()	const { return m_cam_info.n;	}
	IC Fvector				Right					()	const { return m_cam_info.r;	}
	IC float				Fov						()	const { return m_cam_info.fFov; }
	IC float				Aspect					()	const { return m_cam_info.fAspect; }

	IC void					camera_Matrix			(Fmatrix& M){M.set(m_cam_info.r,m_cam_info.n,m_cam_info.d,m_cam_info.p);}
	void					Update					(const Fvector& P, const Fvector& D, const Fvector& N, float fFOV_Dest, float fASPECT_Dest, float fFAR_Dest, u32 flags);
	void					UpdateFromCamera		(const CCameraBase* C);
	
	void					ApplyDevice				(float _viewport_near);
	static void				ResetPP					();

							CCameraManager			(bool bApplyOnUpdate);
	virtual					~CCameraManager			();
};
ENGINE_API extern SPPInfo					pp_identity;
ENGINE_API extern SPPInfo					pp_zero;

ENGINE_API extern float						psCamInert;
ENGINE_API extern float						psCamSlideInert;
