#include "stdafx.h"
#include "../Actor.h"
#include "../alife_registry_wrappers.h"
#include "controls/imgui_ext.h"
#include "embedded_editor.h"
#include "embedded_editor_helper.h"

namespace {
struct InfoportionsEditor : public Editor {
    static Editor* create() { return new InfoportionsEditor(); }

    InfoportionsEditor()
        : Editor(Editors::Infoportions)
    {
    }

    bool draw() override
    {
        bool show = true;
        ImGui::SetWindowSize(ImVec2(200.0f, 200.0f), ImGuiCond_FirstUseEver);
        ImguiWnd wnd(u8"Infoportions", &show);
        if (!show)
            return false;
        if (wnd.Collapsed)
            return true;

        auto&& style = ImGui::GetStyle();
        auto h = 2 * (ImGui::GetTextLineHeight() + 2 * style.FramePadding.y) + 2 * style.ItemSpacing.y;
        auto w = ImGui::CalcTextSize(u8"Add").x + 2 * style.FramePadding.x + style.ItemSpacing.x;

        auto&& infos = g_actor->m_known_info_registry->registry().objects();
        ImGui_ListBox(
            "", &m_cur,
            [](void* data, int idx, const char** out_text) -> bool {
                auto infos = (KNOWN_INFO_VECTOR*)data;
                *out_text = (*infos)[infos->size() - idx - 1].info_id.c_str();
                return true;
            },
            &infos, infos.size(), ImVec2(-1.0f, -h));
        if (ImGui::Button(u8"Disable")) {
            if (m_cur != -1) {
                g_actor->TransferInfo(infos[infos.size() - m_cur - 1].info_id, false);
            }
        }
        auto sz = ImGui::GetItemRectSize();
        ImGui::PushItemWidth(-w);
        ImGui::InputText(u8"##new_info", m_newInfo, 100);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button(u8"Add")) {
            g_actor->TransferInfo(m_newInfo, true);
        }
        return true;
    }

    int m_cur = -1;
    char m_newInfo[100] = {};
};

EditorRegistrator simulationRegister(Editors::Infoportions, &InfoportionsEditor::create);

}
