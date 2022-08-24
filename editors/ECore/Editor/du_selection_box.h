//---------------------------------------------------------------------------

#ifndef du_selection_boxH
#define du_selection_boxH
//---------------------------------------------------------------------------

#define DU_SELECTION_BOX_NUMVERTEX 32
#define DU_SELECTION_BOX_NUMLINES  24

extern ECORE_API Fvector du_selection_box_vertices[DU_SELECTION_BOX_NUMVERTEX];
extern ECORE_API WORD du_selection_box_lines[DU_SELECTION_BOX_NUMLINES*2];

#endif
