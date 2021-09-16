#include "stdafx.h"
#include "IM_ObjectListWnd.h"
#include "../ECore/Editor/ui_main.h"
#include "../ECore/ImGui/imgui.h"
#include "../Edit/Scene.h"
#include "../Edit/UI_LevelTools.h"
#include "../Edit/customobject.h"

void IM_ObjectListWnd::Render()
{
    if (m_needRefresh)
        refresh();
    if (m_needRecalcVis)
        recalcVis();
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"Object List", &open)) {
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(1, 150);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 150);
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##filter", m_filter, sizeof(m_filter) - 1))
            m_needRefresh = true;
        ImGui::PopItemWidth();
        ImGui::BeginChild("##items", ImVec2(0, 0), true);
        ImGuiListClipper clipper(m_vis.size(), ImGui::GetTextLineHeightWithSpacing());
        while (clipper.Step()) {
            ImGui::Indent();
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                const auto& vis = m_vis[i];
                if (vis.second == -1) {
                    auto& group = m_items[vis.first];
                    ImGui::Unindent();
                    if (!group.Objects.empty()) {
                        bool opened = ImGui::TreeNodeEx(group.Tool,
                            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, "%ss",
                            group.Tool->ClassDesc());
                        if (opened != group.Opened) {
                            m_needRecalcVis = true;
                            group.Opened = opened;
                        }
                    } else {
                        ImGui::BulletText("%ss", group.Tool->ClassDesc());
                    }
                    ImGui::Indent();
                } else {
                    auto& object = m_items[vis.first].Objects[vis.second];
                    bool vis = object.Obj->Visible();
                    if (!vis)
                        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    if (ImGui::Selectable(object.Obj->GetName(), object.Selected)) {
                        bool sel = !object.Selected;
                        if (sel && !ImGui::GetIO().KeyCtrl)
                            clearAllSel();
                        object.Selected = sel;
                    }
                    if (!vis)
                        ImGui::PopStyleColor();
                }
            }
            ImGui::Unindent();
        }
        ImGui::EndChild();

        ImGui::NextColumn();

        ImGui::BeginChild("options", ImVec2(0, -2.0f * ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text("Tools");
        if (ImGui::RadioButton("All##Tools", &m_filterTools, 0))
            m_needRefresh = true;
        if (ImGui::RadioButton("Current", &m_filterTools, 1))
            m_needRefresh = true;
        ImGui::NewLine();
        ImGui::Text("Show Options");
        if (ImGui::RadioButton("All##Show", &m_showOptions, 0))
            m_needRefresh = true;
        if (ImGui::RadioButton("Visible Only", &m_showOptions, 1))
            m_needRefresh = true;
        if (ImGui::RadioButton("Invisible Only", &m_showOptions, 2))
            m_needRefresh = true;
        ImGui::NewLine();
        if (ImGui::Button("Show Selected", { -1, 0 }))
            showSelected(true);
        if (ImGui::Button("Hide Selected", { -1, 0 }))
            showSelected(false);
        if (ImGui::Button("Show Properties", { -1, 0 }))
            ExecCommand(COMMAND_SHOW_PROPERTIES);
        ImGui::EndChild();

        if (ImGui::Button("Refresh List", { -1, 0 }))
            m_needRefresh = true;
        if (ImGui::Button("Close", { -1, 0 }))
            open = false;
    }
    ImGui::End();
    if (!open) {
        UI->RemoveIMWindow(this);
        auto _this = this;
        xr_delete(_this);
    }
}

void IM_ObjectListWnd::refresh()
{
    m_needRefresh = false;

    m_items.clear();

    // collect and sort tools
    std::vector<ESceneCustomOTool*> tools;
    tools.reserve(OBJCLASS_COUNT);
    if (m_filterTools == 1) {
        ObjClassID i = LTools->CurrentClassID();
        if (i != OBJCLASS_DUMMY)
            tools.push_back(Scene->GetOTool(i));
    } else {
        for (u32 i = 0; i != OBJCLASS_COUNT; i++) {
            auto tool = Scene->GetOTool(i);
            if (!tool)
                continue;
            tools.push_back(tool);
        }
        std::sort(tools.begin(), tools.end(),
            [](auto el1, auto el2) { return stricmp(el1->ClassDesc(), el2->ClassDesc()) < 0; });
    }

    for (auto tool : tools) {
        m_items.push_back(Group{tool, true});
        auto& group = m_items.back(); 
        for (auto obj : tool->GetObjects()) {
            if (!strstr(obj->GetName(), m_filter))
                continue;
            if (m_showOptions == 1 && !obj->Visible())
                continue;
            if (m_showOptions == 2 && obj->Visible())
                continue;
            group.Objects.push_back(Object { obj, (bool)obj->Selected() });
        }
        std::sort(group.Objects.begin(), group.Objects.end(),
            [](const auto& el1, const auto& el2) { return stricmp(el1.Obj->GetName(), el2.Obj->GetName()) < 0; });
    }

    recalcVis();
}

void IM_ObjectListWnd::clearAllSel()
{
    for (auto& group : m_items)
        for (auto& obj : group.Objects)
            obj.Selected = false;
}

void IM_ObjectListWnd::showSelected(bool vis)
{
    for (auto& group : m_items)
        for (auto& obj : group.Objects) {
            if (!obj.Selected)
                continue;
            obj.Obj->Show(vis);
            if (vis)
                obj.Obj->Select(true);
        }
}

void IM_ObjectListWnd::recalcVis()
{
    m_needRecalcVis = false;

    size_t visCount = 0;
    for (const auto& group : m_items) {
        visCount += 1 + (group.Opened ? group.Objects.size() : 0);
    }
    m_vis.resize(visCount);
    size_t vis_i = 0;
    for (size_t i = 0; i != m_items.size(); i++) {
        auto& group = m_items[i];
        m_vis[vis_i++] = { i, -1 };
        if (group.Opened)
            for (u32 j = 0; j != group.Objects.size(); j++)
                m_vis[vis_i++] = { i, j };
    }
}
