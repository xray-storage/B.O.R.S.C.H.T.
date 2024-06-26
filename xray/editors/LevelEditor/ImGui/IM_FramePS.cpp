#include "stdafx.h"
#pragma hdrstop

#include "../ECore/ImGui/imgui.h"
#include "../ECore/ImGui/utf8.h"

#include "IM_FramePS.h"
#include "../ECore/Editor/render.h"
#include <Layers/xrRender/PSLibrary.h>
#include <Layers/xrRender/ParticleGroup.h>
#include <Layers/xrRender/ParticleEffectDef.h>
#include "../Edit/Scene.h"
#include "../Edit/EParticlesObject.h"

#pragma package(smart_init)

LPCSTR IM_FramePS::Current()
{
	if(m_particles.GetSelectedCount())
    	return m_particles.GetSelected().c_str();
    else
    	return NULL;
}

void IM_FramePS::SelectByRef(bool flag)
{
	if(Current()){
		ObjectIt _F = Scene->FirstObj(OBJCLASS_PS);
        ObjectIt _E = Scene->LastObj(OBJCLASS_PS);
		for(;_F!=_E;_F++){
			if( (*_F)->Visible() ){
				EParticlesObject *_O = (EParticlesObject *)(*_F);
				if(_O->RefCompare(Current()))
                	_O->Select( flag );
			}
		}
	}
}

void IM_FramePS::RefreshList()
{
	m_particles.Clear();

	PS::PEDIt pe, pe_end;
    PS::PGDIt pg, pg_end;

	pe = ::Render->PSLibrary.FirstPED();
    pe_end = ::Render->PSLibrary.LastPED();
	for(; pe != pe_end; pe++)
    {
    	LPCSTR particle_name = (*pe)->m_Name.c_str();
        m_particles.Add(codepage2utf8(particle_name).c_str(), particle_name);
    }

    pg = ::Render->PSLibrary.FirstPGD();
    pg_end = ::Render->PSLibrary.LastPGD();
	for(; pg != pg_end; pg++)
    {
    	LPCSTR particle_name = (*pg)->m_Name.c_str();
        m_particles.Add(codepage2utf8(particle_name).c_str(), particle_name);
    }
}

void IM_FramePS::OnAdd()
{
	RefreshList();

    IM_Storage storage(false, "level.ini", "IM_FramePS");
    m_particles.Select(storage.GetString("current").c_str(), true);
}

void IM_FramePS::OnRemove()
{
	IM_Storage storage(true, "level.ini", "IM_FramePS");
    LPCSTR current = Current();
    storage.PutString("current", current ? current : "");
}

void IM_FramePS::Render()
{
    if(ImGui::CollapsingHeader("PS Commands",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextUnformatted("Ref's select:\t");
        ImGui::SameLine();
        if(ImGui::Button("+", ImVec2(80.f, 0.0f)))
        	SelectByRef(true);
        ImGui::SameLine();
        if(ImGui::Button("-", ImVec2(80.f, 0.0f)))
        	SelectByRef(false);

        ImGui::TextUnformatted("Selected:\t\t");
        ImGui::SameLine();
        if(ImGui::Button("Play", ImVec2(80.f, 0.0f)))
        {
            ObjectIt _F = Scene->FirstObj(OBJCLASS_PS);
    		ObjectIt _E = Scene->LastObj(OBJCLASS_PS);
    		for(;_F!=_E;_F++){
        		if( (*_F)->Visible() && (*_F)->Selected())
            		((EParticlesObject *)(*_F))->Play();
    		}
    	}
        ImGui::SameLine();
        if(ImGui::Button("Stop", ImVec2(80.f, 0.0f)))
        {
            ObjectIt _F = Scene->FirstObj(OBJCLASS_PS);
    		ObjectIt _E = Scene->LastObj(OBJCLASS_PS);
    		for(;_F!=_E;_F++){
        		if( (*_F)->Visible() && (*_F)->Selected())
            		((EParticlesObject *)(*_F))->Stop();
    		}
    	}
    }

	if(ImGui::CollapsingHeader("Current PS",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::BeginChild("particles", ImVec2(0,0), true);
    	m_particles.Render();
        ImGui::EndChild();
    }
}
