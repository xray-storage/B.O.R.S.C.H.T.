#ifndef IM_ImageEditorH
#define IM_ImageEditorH

#include "../Editor/EThumbnail.h"
#include "IM_Canvas.h"
#include "IM_PropertyTree.h"
#include "IM_Tree.h"
#include "IM_Window.h"

class IM_ImageEditor : public IM_Window
{
public:
    bool m_open;
    xr_string m_title;
    bool m_importmode;
    bool m_needupdate;

    bool m_showimage;
    bool m_showcube;
    bool m_showbump;
    bool m_shownormal;
    bool m_showterrain;

    IM_Canvas m_thm;
    IM_Tree m_items;
    IM_PropertyTree m_props;

    FS_FileSet texture_map;
    FS_FileSet modif_map;

    DEFINE_VECTOR(ETextureThumbnail*, THMVec, THMIt);
    DEFINE_MAP(shared_str, ETextureThumbnail*, THMMap, THMMapIt);
    THMMap m_THM_Used;
    THMVec m_THM_Current;

    IM_ImageEditor();

    void Open(bool import);
    void Close();
    bool IsOpen() { return m_open; }

    void OnShowTypeClick();

    virtual void Render();

    ETextureThumbnail* FindUsedTHM(shared_str name);
    void SaveUsedTHM();
    void DestroyUsedTHM();

    void OnItemsFocused(ListItemsVec& items);
    void OnTypeChange(PropValue*);

private:
    void UpdateLib();
    void RegisterModifiedTHM();
};

extern ECORE_API IM_ImageEditor imImageEditor;

#endif
