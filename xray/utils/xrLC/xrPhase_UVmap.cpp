#include "stdafx.h"
#include "build.h"
#include "utils.h"

#include "../xrLC_Light/xrDeflector.h"
#include "../xrLC_Light/xrLC_GlobalData.h"
#include "../xrLC_Light/xrface.h"

#include <tbb/tbb.h>

void CBuild::xrPhase_UVmap_Legacy()
{
    lc_global_data()->g_deflectors().reserve(64 * 1024);
    float p_cost = 1.f / float(g_XSplit.size());
    float p_total = 0.f;
    vecFace faces_affected;
    for (int SP = 0; SP < int(g_XSplit.size()); SP++) {
        Progress(1.f * SP / g_XSplit.size());
        IsolateVertices(FALSE);

        // Detect vertex-lighting and avoid this subdivision
        R_ASSERT(!g_XSplit[SP]->empty());
        Face* Fvl = g_XSplit[SP]->front();
        if (Fvl->Shader().flags.bLIGHT_Vertex)
            continue; // do-not touch (skip)
        if (!Fvl->Shader().flags.bRendering)
            continue; // do-not touch (skip)
        if (Fvl->hasImplicitLighting())
            continue; // do-not touch (skip)

        //   find first poly that doesn't has mapping and start recursion
        while (TRUE) {
            // Select maximal sized poly
            Face* msF = NULL;
            float msA = 0;
            for (vecFaceIt it = g_XSplit[SP]->begin(); it != g_XSplit[SP]->end(); it++) {
                if ((*it)->pDeflector == NULL) {
                    float a = (*it)->CalcArea();
                    if (a > msA) {
                        msF = (*it);
                        msA = a;
                    }
                }
            }
            if (msF) {
                CDeflector* deflector = xr_new<CDeflector>();
                lc_global_data()->g_deflectors().push_back(deflector);

                deflector->OA_SetNormal(msF->N);
                msF->OA_Unwarp(deflector);

                // break the cycle to startup again
                deflector->OA_Export();

                // Detach affected faces
                faces_affected.clear();
                for (int i = 0; i < int(g_XSplit[SP]->size()); i++) {
                    Face* F = (*g_XSplit[SP])[i];
                    if (F->pDeflector == deflector) {
                        faces_affected.push_back(F);
                        g_XSplit[SP]->erase(g_XSplit[SP]->begin() + i);
                        i--;
                    }
                }

                // detaching itself
                Detach(&faces_affected, lc_global_data()->g_vertices());
                g_XSplit.push_back(xr_new<vecFace>(faces_affected));
            } else {
                if (g_XSplit[SP]->empty()) {
                    xr_delete(g_XSplit[SP]);
                    g_XSplit.erase(g_XSplit.begin() + SP);
                    SP--;
                }
                // Cancel infine loop (while)
                break;
            }
        }
    }
}

struct UVmapResult {
    vec2Face newSplits;
    vecDefl deflectors;
};

void UVmapSplit(vecFace& split, UVmapResult& result)
{
    if (split.empty())
        return;
    Face* Fvl = split.front();
    if (Fvl->Shader().flags.bLIGHT_Vertex)
        return;
    if (!Fvl->Shader().flags.bRendering)
        return;
    if (Fvl->hasImplicitLighting())
        return;

    size_t start = result.deflectors.size();

    struct FaceWithArea {
        Face* f;
        float area;
    };
    xr_vector<FaceWithArea> sortedByAreaDesc(split.size());
    std::transform(split.begin(), split.end(), sortedByAreaDesc.begin(), [](Face* f) { return FaceWithArea {f, f->CalcArea()}; });
    std::stable_sort(sortedByAreaDesc.begin(), sortedByAreaDesc.end(), [](const auto& x, const auto& y) { return x.area > y.area; });

    for (auto [msF, _] : sortedByAreaDesc) {
        if (msF->pDeflector)
            continue;
        CDeflector* deflector = xr_new<CDeflector>();
        result.deflectors.push_back(deflector);
        deflector->OA_SetNormal(msF->N);
        msF->OA_Unwarp(deflector);
        deflector->OA_Export();
    }

    for (size_t i = start; i != result.deflectors.size(); ++i) {
        const auto& polys = result.deflectors[i]->UVpolys;
        vecFace* subSplit = xr_new<vecFace>(polys.size());
        std::transform(polys.begin(), polys.end(), subSplit->begin(), [](const auto& p) { return p.owner; });
        result.newSplits.push_back(subSplit);
    }
    split.clear();
}

template <typename Cont> void append(Cont& cont, const Cont& add) { cont.insert(cont.end(), add.begin(), add.end()); }

void RemoveEmptySplits()
{
    auto it = std::partition(g_XSplit.begin(), g_XSplit.end(), [](auto split) { return !split->empty(); });
    std::for_each(it, g_XSplit.end(), [](auto& split) { xr_delete(split); });
    g_XSplit.erase(it, g_XSplit.end());
}

void CBuild::xrPhase_UVmap_Tbb()
{
    tbb::atomic<size_t> progress { 0 };
    tbb::combinable<UVmapResult> temp;
    size_t grain = std::max(1u, g_XSplit.size() / tbb::task_arena().max_concurrency() / 10);
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, g_XSplit.size(), grain), [&progress, &temp](const tbb::blocked_range<size_t>& r) {
            auto& local = temp.local();
            for (size_t SP = r.begin(); SP != r.end(); ++SP) {
                UVmapSplit(*g_XSplit[SP], local);
                progress.fetch_and_increment();
                Progress(progress * 1.0f / g_XSplit.size());
            }
        });
    UVmapResult add;
    temp.combine_each([&add](UVmapResult x) {
        append(add.newSplits, x.newSplits);
        append(add.deflectors, x.deflectors);
    });
    for (auto split : add.newSplits)
        Detach(split, lc_global_data()->g_vertices());
    IsolateVertices(FALSE);
    RemoveEmptySplits();
    append(g_XSplit, add.newSplits);
    append(lc_global_data()->g_deflectors(), add.deflectors);
}

void CBuild::xrPhase_UVmap()
{
    Status("Processing...");
    if (lc_global_data()->useTbb())
        xrPhase_UVmap_Tbb();
    else
        xrPhase_UVmap_Legacy();
    clMsg("%d subdivisions...", g_XSplit.size());
}

void CBuild::mem_CompactSubdivs()
{
	// Memory compact
	CTimer	dwT;	dwT.Start();
	vecFace			temp;
	for (int SP = 0; SP<int(g_XSplit.size()); SP++) 
	{
		temp.clear			();
		temp.assign			(g_XSplit[SP]->begin(),g_XSplit[SP]->end());
		xr_delete			(g_XSplit[SP]);
		mem_Compact			();
		g_XSplit[SP]		= xr_new<vecFace> ();
		g_XSplit[SP]->assign(temp.begin(),temp.end());
	}
	clMsg		("%d ms for memory compacting...",dwT.GetElapsed_ms());
}
void CBuild::mem_Compact()
{
	Memory.mem_compact	();
	/*
	u32					bytes,blocks_used,blocks_free;
	bytes				= Memory.mem_usage(&blocks_used,&blocks_free);
	LPCSTR h_status		= 0;
	if (HeapValidate	(GetProcessHeap(),0,0))	h_status = "OK";
	else										h_status = "DAMAGED";
	clMsg				("::MEMORY(%s):: %d MB, %d Bused, %d Bfree",
		h_status,bytes/(1024*1024),blocks_used,blocks_free);
	*/
}
