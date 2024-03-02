#include "stdAfx.h"
#include "embedded_editor_hud.h"
#include "../../xrEngine/device.h"
#include "../player_hud.h"
#include "embedded_editor_helper.h"
#include <addons/ImGuizmo/ImGuizmo.h>

Fvector attach0[2];
Fvector attach1[2];
bool first = true;

void ShowHudEditor(bool& show)
{
    ImguiWnd wnd("HUD Editor", &show);
    if (wnd.Collapsed) {
        first = true;
        return;
    }

    if (!g_player_hud)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::OPERATION mode = io.KeyCtrl ? ImGuizmo::ROTATE : ImGuizmo::TRANSLATE;
    bool showSeparator = false;
    auto item = g_player_hud->attached_item(0);
    if (item) {
        if (first) {
            attach0[0] = item->m_measures.m_item_attach[0];
            attach0[1] = item->m_measures.m_item_attach[1];
        }
        if (showSeparator)
            ImGui::Separator();
        showSeparator = true;
        ImGui::Text("Item 0");
        ImGui::InputFloat3("item_position 0", (float*)&item->m_measures.m_item_attach[0], 3);
        ImGui::InputFloat3("item_orientation 0", (float*)&item->m_measures.m_item_attach[1], 3);
        ImGuizmo::SetID(0);
        ImGuizmo::Manipulate(
            (float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::LOCAL, (float*)&item->m_item_transform);
        Fvector ypr;
        item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
        ypr.mul(180.f / PI);
        item->m_measures.m_item_attach[1] = ypr;
        item->m_measures.m_item_attach[0] = item->m_attach_offset.c;
        if (ImGui::Button("Reset##0")) {
            item->m_measures.m_item_attach[0] = attach0[0];
            item->m_measures.m_item_attach[1] = attach0[1];
        }
    }
    item = g_player_hud->attached_item(1);
    if (item) {
        if (first) {
            attach1[0] = item->m_measures.m_item_attach[0];
            attach1[1] = item->m_measures.m_item_attach[1];
        }
        if (showSeparator)
            ImGui::Separator();
        ImGui::Text("Item 1");
        ImGui::InputFloat3("item_position 1", (float*)&item->m_measures.m_item_attach[0], 3);
        ImGui::InputFloat3("item_orientation 1", (float*)&item->m_measures.m_item_attach[1], 3);
        ImGuizmo::SetID(1);
        ImGuizmo::Manipulate(
            (float*)&Device.mView, (float*)&Device.mProject, mode, ImGuizmo::LOCAL, (float*)&item->m_attach_offset);
        if (ImGuizmo::IsUsing()) {
            Fvector ypr;
            item->m_attach_offset.getHPB(ypr.x, ypr.y, ypr.z);
            ypr.mul(180.f / PI);
            item->m_measures.m_item_attach[1] = ypr;
            item->m_measures.m_item_attach[0] = item->m_attach_offset.c;
        }
        if (ImGui::Button("Reset##1")) {
            item->m_measures.m_item_attach[0] = attach1[0];
            item->m_measures.m_item_attach[1] = attach1[1];
        }
    }
    first = false;
}
