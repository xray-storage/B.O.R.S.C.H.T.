//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "FormAbout.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;

extern XRCORE_API u32 build_id;
extern XRCORE_API LPCSTR build_date;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::btnOkClick(TObject *Sender)
{
	AboutForm->Close();
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormShow(TObject *Sender)
{
	lbBuildID->Caption = AnsiString("Build ") + build_id + ", " + build_date;
}
//---------------------------------------------------------------------------

void __fastcall TAboutForm::lbURLsrcClick(TObject *Sender)
{
	TLabel* label = dynamic_cast<TLabel*>(Sender);
	if(label) {
		ShellExecute(Handle, "open", label->Caption.c_str(), "", "", SW_SHOWNORMAL);
		label->Font->Color = clHotLight;
	}
}
//---------------------------------------------------------------------------

