#ifndef IM_ChooseFormH
#define IM_ChooseFormH

#include "IM_Window.h"
#include "ChooseTypes.H"
#include "IM_Tree.h"
#include "IM_PropertyTree.h"
#include "IM_Canvas.h"

typedef fastdelegate::FastDelegate1<class IM_ChooseForm*> IM_CFCallback;

ECORE_API class IM_ChooseForm : public IM_Window
{
	public:
    SChooseEvents* m_events;

	IM_CFCallback OnOK;
	IM_CFCallback OnCancel;
    IM_CFCallback OnClose;

    IM_Tree m_items_tree;
    IM_PropertyTree m_props;

    bool m_show_thumbnail;
    IM_Canvas m_thm;

    xr_vector<shared_str> m_selected;
    int m_current_item; // for selection list

    bool m_open;

	public:
    IM_ChooseForm(EChooseMode mode, int max_select,
	ChooseItemVec* items = NULL, TOnChooseFillItems fill_items = NULL, void* fill_param = NULL);

    private:
    void OnNodeSelected(shared_str path, shared_str value, bool select);

    public:
    void Close();

    void SetSelected(shared_str selected);
    shared_str GetSelected();

    virtual void Render();
};

#endif
