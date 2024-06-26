#pragma once

#include "../shader_xrlc.h"
#include "../../xrcore/xrPool.h"
//#include "xrface.h"
#include "xrfacedefs.h"
#include "xrdeflectordefs.h"
#include "b_build_texture.h"
#include "base_lighting.h"
//#include "mu_model_face.h"
//#include "mu_model_face_defs.h"
//struct _face;
//struct _vertex;
namespace CDB{
	struct WorkPayload;
	template<class Payload> class MODEL_Generic;
	using MODEL_Work = MODEL_Generic<WorkPayload>;
	template<class Payload> class CollectorPacked;
	using CollectorPacked_Work = CollectorPacked<WorkPayload>;
};
class CLightmap;
class xrMU_Model;
class xrMU_Reference;
class base_Vertex;
class base_Face;
class	XRLC_LIGHT_API xrLC_GlobalData
{
	
		xr_vector<b_BuildTexture>		_textures;
		xr_vector<b_material>			_materials;
		Shader_xrLC_LIB					_shaders;				
		CMemoryWriter					_err_invalid;
		b_params						_g_params;
		xr_vector<CLightmap*>			_g_lightmaps;
		xr_vector<xrMU_Model*>			_mu_models;
		xr_vector<xrMU_Reference*>		_mu_refs;
		vecVertex						_g_vertices;
		vecFace							_g_faces;
		vecDefl							_g_deflectors;
		base_lighting					_L_static;
		CDB::MODEL_Work*				_RCAST_Model;
		bool							_b_nosun;
		bool							_gl_linear;
		bool							_b_norgb;
		bool							_b_no_lmaps;
		bool							_b_skip_invalid;
		bool							_b_lmap_rgba;
		u32								_numThread;
		bool							m_skipAhtTesselate;
		bool                            m_cformOnly = false;
		bool							m_useTbb = false;

public:

public:
									xrLC_GlobalData	():_RCAST_Model (0), _b_nosun(false),_gl_linear(false),
										_b_norgb(false), _b_no_lmaps(false), _b_skip_invalid(false),
										_b_lmap_rgba(false), _numThread(8) {}
									~xrLC_GlobalData();
		IC xr_vector<b_BuildTexture>& textures		()		{	return _textures; }
		IC xr_vector<CLightmap*>	& lightmaps		()		{	return _g_lightmaps; }
		IC xr_vector<b_material>	& materials		()		{	return _materials; }
		IC Shader_xrLC_LIB			& shaders		()		{	return _shaders; }
		IC CMemoryWriter			&err_invalid	()		{	return _err_invalid; }
		IC b_params					&g_params		()		{	return _g_params; }
			
		Face						*create_face	()		;
		void						destroy_face	(Face* &f );

		Vertex						*create_vertex	()		;
		void						destroy_vertex	(Vertex* &f );

		vecVertex					&g_vertices		()		{	return	_g_vertices; }
		vecFace						&g_faces		()		{	return	_g_faces; }
		vecDefl						&g_deflectors	()		{	return	_g_deflectors; }
		bool						b_r_vertices	()		;
		bool						b_r_faces		()		;
		base_lighting				&L_static		()		{	return _L_static; }
		CDB::MODEL_Work*			RCAST_Model		()		{	return _RCAST_Model; }
		xr_vector<xrMU_Model*>		&mu_models		()		{	return _mu_models; }
		xr_vector<xrMU_Reference*>	&mu_refs		()		{	return _mu_refs; }
		bool						b_nosun			()		{	return _b_nosun; }
		bool						gl_linear		()		{	return _gl_linear; }
IC		void						b_nosun_set		(bool v){	_b_nosun = v; }
		void						initialize		()		;
		void						destroy_rcmodel	()		;
		void						create_rcmodel	(CDB::CollectorPacked_Work& CL);
		void						create_write_faces()	;
		void						destroy_write_faces()	;
		void						create_read_faces()		;
		void						destroy_read_faces()	;
		void						gl_mesh_clear	()		;

		bool						b_norgb()				{ return _b_norgb; }
		void						b_norgb_set(bool v)		{ _b_norgb = v; }
		bool						b_no_lmaps()			{ return _b_no_lmaps; }
		void						b_no_lmaps_set(bool v)	{ _b_no_lmaps = v; }
		bool						b_skip_invalid()		{ return _b_skip_invalid; }
		void						b_skip_invalid_set(bool v){ _b_skip_invalid = v; }
		bool						b_lmap_rgba() { return _b_lmap_rgba; }
		void						b_lmap_rgba_set(bool v) { _b_lmap_rgba = v; }
		u32							numThread() { return _numThread; }
		void						setNumThread(u32 v) { _numThread = v; }
		bool						skipAhtTesselate() { return m_skipAhtTesselate; }
		void						setSkipAhtTesselate(bool v) { m_skipAhtTesselate = v; }
		bool						cformOnly() const{ return m_cformOnly; }
		void						setCformOnly(bool v) { m_cformOnly = v; }
		bool						useTbb() const { return m_useTbb; }
		void						setUseTbb(bool v) { m_useTbb = v; }

private:
//std::pair<u32,u32>					get_id		( const _face * v ) const;
//std::pair<u32,u32>					get_id		( const _vertex * v ) const;
public:
		void						read			( INetReader &r );
		void						write			( IWriter	&w ) const ;

		void						read			( INetReader &r, base_Face* &f);
		void						write			( IWriter &r, const base_Face *f ) const ;
		void						clear			();
	//	void						cdb_read_create	() ;
private:
		void						close_models_read		();
		void						close_models_write		()const ;

};													

extern "C" XRLC_LIGHT_API xrLC_GlobalData*	lc_global_data();
extern "C" XRLC_LIGHT_API void				create_global_data();
extern "C" XRLC_LIGHT_API void				destroy_global_data();
extern "C" XRLC_LIGHT_API u32				InvalideFaces();
		   XRLC_LIGHT_API void				ImplicitLighting();
extern xrLC_GlobalData* data;
IC xrLC_GlobalData* inlc_global_data() { return data; }
static LPCSTR gl_data_net_file_name = "tmp_global_data";


#ifdef _DEBUG
static LPCSTR libraries = "XRLC_LightStab.dll,XRLC_Light.dll,xrCore.dll,xrCDB.dll,DXT.dll,BugTrapD.dll,msvcr80.dll,Microsoft.VC80.CRT.manifest";
#else
static LPCSTR libraries = "XRLC_LightStab.dll,XRLC_Light.dll,xrCore.dll,xrCDB.dll,DXT.dll,BugTrap.dll,msvcr80.dll,Microsoft.VC80.CRT.manifest";
#endif
//#define NET_CMP
//#define LOAD_GL_DATA
