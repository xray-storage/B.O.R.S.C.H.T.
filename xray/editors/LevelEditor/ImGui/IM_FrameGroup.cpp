#include "stdafx.h"
#pragma hdrstop

#include "../ECore/ImGui/imgui.h"
#include "../ECore/ImGui/IM_ChooseForm.h"

#include "IM_FrameGroup.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/scene.h"
#include "../Edit/ESceneGroupTools.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/UI_LevelMain.h"
#include "../Edit/GroupObject.h"
#include <random>

#pragma package(smart_init)

BOOL bForceInitListBox = FALSE;

void IM_FrameGroup::OnGroupSelected(IM_ChooseForm* form)
{
	m_current = form->GetSelected().c_str();
    m_parent_tool->SetCurrentObject(m_current.c_str());
}

void IM_FrameGroup::MultiSelByRefObject(bool clear_prev)
{
    ObjectList 	objlist;
    LPU32Vec 	sellist;
    if (Scene->GetQueryObjects(objlist,OBJCLASS_GROUP,1,1,-1)){
    	for (ObjectIt it=objlist.begin(); it!=objlist.end(); it++){
	        LPCSTR N = ((CGroupObject*)*it)->RefName();
            ObjectIt _F = Scene->FirstObj(OBJCLASS_GROUP);
            ObjectIt _E = Scene->LastObj(OBJCLASS_GROUP);
            for(;_F!=_E;_F++){
	            CGroupObject *_O = (CGroupObject *)(*_F);
                if((*_F)->Visible()&&_O->RefCompare(N)){
                	if (clear_prev){
                    	_O->Select( false );
	                    sellist.push_back((u32*)_O);
                    }else{
                    	if (!_O->Selected())
                        	sellist.push_back((u32*)_O);
                    }
                }
            }
        }
        std::sort			(sellist.begin(),sellist.end());
        sellist.erase		(std::unique(sellist.begin(),sellist.end()),sellist.end());
        std::shuffle	    (sellist.begin(),sellist.end(), std::random_device());
        int max_k		= iFloor(float(sellist.size())/100.f*float(m_select_percent)+0.5f);
        int k			= 0;
        for (LPU32It o_it=sellist.begin(); k<max_k; o_it++,k++){
            CGroupObject *_O = (CGroupObject *)(*o_it);
            _O->Select( true );
        }
    }
}

void IM_FrameGroup::SelByRefObject(bool flag)
{
	if (m_current.length()){
    	ObjectList objlist;
        LPCSTR N = m_current.c_str();

        ObjectIt _F = Scene->FirstObj(OBJCLASS_GROUP);
        ObjectIt _E = Scene->LastObj(OBJCLASS_GROUP);
        for(;_F!=_E;_F++){
            if((*_F)->Visible() ){
                CGroupObject *_O = (CGroupObject*)(*_F);
                if(_O->RefCompare(N)) _O->Select( flag );
            }
        }
    }
}

void IM_FrameGroup::OnAdd()
{
	m_parent_tool = dynamic_cast<ESceneGroupTool*>(Scene->GetTool(OBJCLASS_GROUP));
	VERIFY(m_parent_tool);

    IM_Storage storage(false, "level.ini", "IM_FrameGroup");

    m_select_percent = storage.GetInt("select_percent", 0);
    m_current = storage.GetString("current", "");
    if(!m_current.empty())
    	m_parent_tool->SetCurrentObject(m_current.c_str());
}

void IM_FrameGroup::OnRemove()
{
	IM_Storage storage(true, "level.ini", "IM_FrameGroup");

    storage.PutInt("select_percent", m_select_percent);
    storage.PutString("current", m_current);
}

void IM_FrameGroup::Render()
{
	if(ImGui::CollapsingHeader("Commands",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Columns(2);

        if(ImGui::MenuItem("Group"))
        	m_parent_tool->GroupObjects();
        if(ImGui::MenuItem("Ungroup"))
        	m_parent_tool->UngroupObjects();

        ImGui::NextColumn();

        if(ImGui::MenuItem("Make Thumbnail"))
        	m_parent_tool->MakeThumbnail();
        if(ImGui::MenuItem("Save AS..."))
        	m_parent_tool->SaveSelectedObject();

        ImGui::Columns(1);
    }

	if(ImGui::CollapsingHeader("Current Object",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Columns(2);

        if(ImGui::MenuItem("Select..."))
        {
			IM_ChooseForm* cf = new IM_ChooseForm(smGroup, 1);
			cf->OnOK.bind(this, &IM_FrameGroup::OnGroupSelected);
            UI->AddIMWindow(cf);
        }

        ImGui::NextColumn();

        if(ImGui::MenuItem("Reload Refs"))
        {
        	extern BOOL bForceInitListBox;
            m_parent_tool->ReloadRefsSelectedObject();
    		bForceInitListBox = TRUE;
    		Tools->UpdateProperties(TRUE);
        }

        ImGui::Columns(1);

        ImGui::Text("Current: %s",
        m_current.length() ? m_current.c_str() : "<empty>");
        if(ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", m_current.c_str());
    }

	if(ImGui::CollapsingHeader("Reference Select",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
    	ImGui::Columns(2, "ref_select", false);

        ImGui::AlignTextToFramePadding();
    	ImGui::TextUnformatted("Select by Current:\t");
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Select by Selected:\t");

        ImGui::NextColumn();

        if(ImGui::Button("+", ImVec2(40,0)))
        	SelByRefObject(true);
        ImGui::SameLine();
        if(ImGui::Button("-", ImVec2(40,0)))
        	SelByRefObject(false);

        if(ImGui::Button("=%", ImVec2(40,0)))
        	MultiSelByRefObject(true);
        ImGui::SameLine();
        if(ImGui::Button("+%", ImVec2(40,0)))
        	MultiSelByRefObject(false);

        ImGui::Columns(1);

        ImGui::SliderInt("%", &m_select_percent, 0, 100);
    }

	if(ImGui::CollapsingHeader("Pivot Alignment",
    ImGuiTreeNodeFlags_Framed|ImGuiTreeNodeFlags_DefaultOpen))
    {
    	if(ImGui::MenuItem("Center to Group"))
        	m_parent_tool->CenterToGroup();
        if(ImGui::MenuItem("Align to Object..."))
        	m_parent_tool->AlignToObject();
    }
}
