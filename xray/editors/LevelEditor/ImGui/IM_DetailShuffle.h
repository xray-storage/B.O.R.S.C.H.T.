#pragma once

#include "../ECore/ImGui/IM_Window.h"

class EDetailManager;

class IM_DetailShuffle : public IM_Window
{
public:
    IM_DetailShuffle(EDetailManager* dm);

    void Render() override;

private:
    EDetailManager* m_dm;
    bool m_needRefresh = false;

    void refresh();
};
