//---------------------------------------------------------------------------
#ifndef FrameObjectH
#define FrameObjectH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>

#include "ExtBtn.hpp"
#include "multi_edit.hpp"
#include "mxPlacemnt.hpp"

#include "ESceneCustomMTools.h"

// refs
class CEditableObject;
class ESceneObjectTool;
//---------------------------------------------------------------------------
class TfraObject : public TForm
{
__published:	// IDE-managed Components
	TFormStorage *fsStorage;
	TPanel *paSelectObject;
	TLabel *Label1;
	TExtBtn *ebSelectByRefs;
	TExtBtn *ebDeselectByRefs;
	TLabel *APHeadLabel3;
	TExtBtn *ExtBtn5;
	TExtBtn *ebMultiSelectByRefMove;
	TBevel *Bevel3;
	TLabel *Label4;
	TExtBtn *ebMultiSelectByRefAppend;
	TLabel *Label6;
	TPanel *paCommands;
	TExtBtn *ebMultiAppend;
	TLabel *APHeadLabel1;
	TExtBtn *ExtBtn2;
	TBevel *Bevel4;
	TExtBtn *ebRandomAppendMode;
	TExtBtn *ExtBtn8;
	TPanel *paCurrentObject;
	TLabel *APHeadLabel2;
	TBevel *Bevel2;
	TPanel *paItems;
	TPanel *Panel1;
	TExtBtn *ExtBtn4;
	TMultiObjSpinEdit *seSelPercent;
	TExtBtn *ExtBtn9;
	TExtBtn *ebGenerateGarbage;
    void __fastcall PaneMinClick(TObject *Sender);
    void __fastcall ebSelectByRefsClick(TObject *Sender);
    void __fastcall ebDeselectByRefsClick(TObject *Sender);
    void __fastcall ExpandClick(TObject *Sender);
	void __fastcall ebMultiAppendClick(TObject *Sender);
	void __fastcall ebMultiSelectByRefMoveClick(TObject *Sender);
	void __fastcall ebMultiSelectByRefAppendClick(TObject *Sender);
	void __fastcall seSelPercentKeyPress(TObject *Sender, char &Key);
	void __fastcall ExtBtn4Click(TObject *Sender);
	void __fastcall paCurrentObjectResize(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormHide(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ebRandomAppendModeClick(TObject *Sender);
	void __fastcall ExtBtn8Click(TObject *Sender);
	void __fastcall ExtBtn9Click(TObject *Sender);
	void __fastcall ebGenerateGarbageClick(TObject *Sender);
private:	// User declarations
	void 			MultiSelByRefObject ( bool clear_prev );
	void 			SelByRefObject  	( bool flag );
	TItemList* m_Items;
    void __stdcall  OnItemFocused		(ListItemsVec& items);
    LPCSTR m_Current;
	void __stdcall  OnDrawObjectThumbnail(LPCSTR name, HDC hdc, const Irect &R);
public:		// User declarations
	ESceneObjectTool* ParentTools;
    void			RefreshList();
public:		// User declarations
        __fastcall TfraObject(TComponent* Owner,ESceneObjectTool* parent_tools);
    LPCSTR 			Current	(){return m_Current;}
};
//---------------------------------------------------------------------------
#endif
