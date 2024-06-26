#include "stdafx.h"
#include "build.h"

#include "xrPhase_MergeLM_Rect.h"
#include "../xrlc_light/xrdeflector.h"
#include "../xrlc_light/xrlc_globaldata.h"
#include "../xrlc_light/lightmap.h"

#include <tbb/tbb.h>

#include <optional>

// Surface access
extern void _InitSurface	();
extern bool _rect_place(L_rect& r, lm_layer* D, bool& rotated);
extern void _rect_register(L_rect& R, lm_layer* D, BOOL bRotate);
extern bool rectPlaceY(u32 y, L_rect& r, lm_layer* D, bool& rotated);
extern bool checkFreeSpace(u32 minCell);

IC int	compare_defl		(CDeflector* D1, CDeflector* D2)
{
	// First  - by material
	u16 M1		= D1->GetBaseMaterial();
	u16 M2		= D2->GetBaseMaterial();
	if (M1<M2)	return	1;  // less
	if (M1>M2)	return	0;	// more
	return				2;	// equal
}

// should define LESS(D1<D2) behaviour
// sorting - in increasing order
IC int	sort_defl_analyze	(CDeflector* D1, CDeflector* D2)
{
	// first  - get material index
	u16 M1		= D1->GetBaseMaterial();
	u16 M2		= D2->GetBaseMaterial();

	// 1. material area
	u32	 A1		= pBuild->materials()[M1].internal_max_area;
	u32	 A2		= pBuild->materials()[M2].internal_max_area;
	if (A1<A2)	return	2;	// A2 better
	if (A1>A2)	return	1;	// A1 better

	// 2. material sector (geom - locality)
	u32	 s1		= pBuild->materials()[M1].sector;
	u32	 s2		= pBuild->materials()[M2].sector;
	if (s1<s2)	return	2;	// s2 better
	if (s1>s2)	return	1;	// s1 better

	// 3. just material index
	if (M1<M2)	return	2;	// s2 better
	if (M1>M2)	return	1;	// s1 better

	// 4. deflector area
	u32 da1		= D1->layer.Area();
	u32 da2		= D2->layer.Area();
	if (da1<da2)return	2;	// s2 better
	if (da1>da2)return	1;	// s1 better

	// 5. they are EQUAL
	return				0;	// equal
}

// should define LESS(D1<D2) behaviour
// sorting - in increasing order
IC bool	sort_defl_complex	(CDeflector* D1, CDeflector* D2)
{
	switch (sort_defl_analyze(D1,D2))	
	{
	case 1:		return true;	// 1st is better 
	case 2:		return false;	// 2nd is better
	case 0:		return false;	// none is better
	default:	return false;
	}
}

class	pred_remove { public: IC bool	operator() (CDeflector* D) { { if (0==D) return TRUE;}; if (D->bMerged) {D->bMerged=FALSE; return TRUE; } else return FALSE;  }; };

bool rectPlaceTbb(L_rect& r, lm_layer* D, bool& rotated);

void CBuild::xrPhase_MergeLM()
{
	using TRectPlace = bool(*)(L_rect &, lm_layer*, bool&);
    TRectPlace rectPlace = lc_global_data()->useTbb() ? rectPlaceTbb : _rect_place;

	vecDefl			Layer;

	// **** Select all deflectors, which contain this light-layer
	Layer.clear	();
	for (u32 it=0; it<lc_global_data()->g_deflectors().size(); it++)
	{
		CDeflector*	D		= lc_global_data()->g_deflectors()[it];
		if (D->bMerged)		continue;
		Layer.push_back		(D);
	}

	// Merge this layer (which left unmerged)
	while (Layer.size()) 
	{
		VERIFY( lc_global_data() );
		string512	phase_name;
		sprintf		(phase_name,"Building lightmap %d...", lc_global_data()->lightmaps().size());
		Phase		(phase_name);

		// Sort layer by similarity (state changes)
		// + calc material area
		Status		("Selection...");
		for (u32 it=0; it<materials().size(); it++) materials()[it].internal_max_area	= 0;
		for (u32 it=0; it<Layer.size(); it++)	{
			CDeflector*	D		= Layer[it];
			materials()[D->GetBaseMaterial()].internal_max_area	= _max(D->layer.Area(),materials()[D->GetBaseMaterial()].internal_max_area);
		}
		std::stable_sort(Layer.begin(),Layer.end(),sort_defl_complex);

		// Select first deflectors which can fit
		u32 maxarea		= c_LMAP_size*c_LMAP_size*8;	// Max up to 8 lm selected
		u32 curarea		= 0;
		u32 merge_count	= 0;
		for (u32 it=0; it<(int)Layer.size(); it++)	{
			int		defl_area	= Layer[it]->layer.Area();
			if (curarea + defl_area > maxarea) break;
			curarea		+=	defl_area;
			merge_count ++;
		}

		// Startup
		Status		("Processing...");
		_InitSurface			();
		CLightmap*	lmap		= xr_new<CLightmap> ();
		VERIFY( lc_global_data() );
		lc_global_data()->lightmaps().push_back	(lmap);

		// Process 
		for (u32 it=0; it<merge_count; it++) 
		{
			lm_layer&	L		= Layer[it]->layer;
			L_rect		rT,rS; 
			rS.a.set	(0,0);
			rS.b.set	(L.width+2*BORDER-1, L.height+2*BORDER-1);
			rS.iArea	= L.Area();
			rT			= rS;
			bool rotated;
			if (_rect_place(rT, &L, rotated)) 
			{
				_rect_register		(rT, &L, rotated);
				lmap->Capture		(Layer[it],rT.a.x,rT.a.y,rT.SizeX(),rT.SizeY(),rotated);
				Layer[it]->bMerged	= TRUE;
			}
			else if (!checkFreeSpace(1 + 2 * BORDER))
				break;
			Progress(_sqrt(float(it)/float(merge_count)));
		}
		Progress	(1.f);

		// Remove merged lightmaps
		Status			("Cleanup...");
		vecDeflIt last	= std::remove_if	(Layer.begin(),Layer.end(),pred_remove());
		Layer.erase		(last,Layer.end());

		// Save
		Status			("Saving...");
		VERIFY			( pBuild );
		lmap->Save		(pBuild->path);
	}
	VERIFY( lc_global_data() );
	clMsg		( "%d lightmaps builded", lc_global_data()->lightmaps().size() );

	// Cleanup deflectors
	Progress	(1.f);
	Status		("Destroying deflectors...");
	for (u32 it=0; it<lc_global_data()->g_deflectors().size(); it++)
		xr_delete(lc_global_data()->g_deflectors()[it]);
	lc_global_data()->g_deflectors().clear_and_free	();
}

struct Place {
    L_rect rect;
    bool rotated;
};
using PlaceResult = std::optional<Place>;
void merge(PlaceResult& dest, const PlaceResult& x)
{
    if (!x)
        return;
    if (!dest || x.value().rect.a.y < dest.value().rect.a.y)
        dest = x;
}

bool rectPlaceTbb(L_rect& tr, lm_layer* D, bool& rotated)
{
    tbb::combinable<PlaceResult> temp;
    tbb::parallel_for(tbb::blocked_range<u32>(0, c_LMAP_size), [&temp, tr, D](const tbb::blocked_range<u32>& r) {
        L_rect rect = tr;
        for (u32 y = r.begin(); y != r.end(); ++y) {
            bool rotated;
            if (rectPlaceY(y, rect, D, rotated)) {
                merge(temp.local(), Place { rect, rotated });
                break;
            }
        }
    });
    PlaceResult result;
    temp.combine_each([&result](const PlaceResult& x) { merge(result, x); });
    if (result) {
        tr = result.value().rect;
        rotated = result.value().rotated;
    }
    return result.has_value();
}