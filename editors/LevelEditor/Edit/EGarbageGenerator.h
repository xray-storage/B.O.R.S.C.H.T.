//---------------------------------------------------------------------------

#ifndef EGarbageGeneratorH
#define EGarbageGeneratorH
//---------------------------------------------------------------------------

class CSceneObject;
class ESceneShapeTool;

struct EGarbageGeneratorRestrictor
{
	Fvector pos;
	float radius;
};

class EGarbageGenerator
{
	public:
	bool m_shape_emitter_mode;
	bool m_shape_restrictor_mode;
	xr_vector<EGarbageGeneratorRestrictor> m_restrictors;

	void Generate(CSceneObject *terrain);
	void GenerateEmitter(CSceneObject *terrain);
	void InitRestrictors(ESceneShapeTool *st);
};

#endif
