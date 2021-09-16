#pragma once

#include "../ECore/ImGui/IM_Window.h"
#include <variant>

class ESceneCustomOTool;
class CCustomObject;

class IM_ObjectListWnd : public IM_Window
{
public:
    void Render() override;

private:
    int m_filterTools = 0;
    int m_showOptions = 0;
    char m_filter[100] = {};

    bool m_needRefresh = true;
    struct Object {
        CCustomObject* Obj;
        bool Selected;
    };
    struct Group {
        ESceneCustomOTool* Tool;
        bool Opened;
        std::vector<Object> Objects;
    };
    std::vector<Group> m_items;
    void refresh();
    void clearAllSel();
    void showSelected(bool vis);

    bool m_needRecalcVis = false;
    std::vector<std::pair<int, int>> m_vis;
    void recalcVis();
};
