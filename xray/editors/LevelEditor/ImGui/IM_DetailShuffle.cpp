#include "stdafx.h"
#include "IM_DetailShuffle.h"
#include "../ECore/ImGui/imgui.h"
#include "../ECore/Editor/ui_main.h"
#include "../Edit/ESceneDOTools.h"

IM_DetailShuffle::IM_DetailShuffle(EDetailManager* dm) : m_dm(dm)
{
}

void IM_DetailShuffle::Render()
{
    if (m_needRefresh)
        refresh();
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(u8"Detail Objects", &open)) {
        ImGui::Columns(2, nullptr, false);
        ImGui::SetColumnWidth(0, 250);
        ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() - 250);
        ImGui::BeginChild("##left", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        ImGui::EndChild();
        if (ImGui::Button("Load")) {
            xr_string fileName;
            if (EFS.GetOpenName(_detail_objects_, fileName) && m_dm->ImportColorIndices(fileName.c_str())) {
                m_dm->InvalidateSlots();
                m_needRefresh = true;
            }
        }

        ImGui::NextColumn();

        ImGui::BeginChild("##right", ImVec2(0, 0));
        ImGui::EndChild();
    }
    ImGui::End();
    if (!open) {
        UI->RemoveIMWindow(this);
        auto _this = this;
        xr_delete(_this);
    }
}

void IM_DetailShuffle::refresh()
{
    m_needRefresh = false;
}
