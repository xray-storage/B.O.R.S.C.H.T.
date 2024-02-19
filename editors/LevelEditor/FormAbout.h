//---------------------------------------------------------------------------

#ifndef FormAboutH
#define FormAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include "ElCLabel.hpp"
#include "ElURLLabel.hpp"
//---------------------------------------------------------------------------
class TAboutForm : public TForm
{
__published:	// IDE-managed Components
	TImage *Image1;
	TLabel *Label1;
	TLabel *lbBuildID;
	TLabel *Label2;
	TButton *btnOk;
	TLabel *Label3;
	TLabel *lbURLsrc;
	TLabel *lnURLupd;
	TLabel *lbSource;
	TLabel *lbUpdates;
	void __fastcall btnOkClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall lbURLsrcClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TAboutForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutForm *AboutForm;
//---------------------------------------------------------------------------
#endif
