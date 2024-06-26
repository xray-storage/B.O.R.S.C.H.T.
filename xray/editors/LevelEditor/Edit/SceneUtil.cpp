//----------------------------------------------------
// file: SceneUtil.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include "Scene.h"
#include "ELight.h"
#include "SceneObject.h"
#include "ui_leveltools.h"

//----------------------------------------------------
CCustomObject* EScene::FindObjectByName( LPCSTR name, ObjClassID classfilter )
{
	if(!name)
    return NULL;
    
	CCustomObject* object = 0;
    if (classfilter==OBJCLASS_DUMMY)
    {
        SceneToolsMapPairIt _I = m_SceneTools.begin();
        SceneToolsMapPairIt _E = m_SceneTools.end();
        for (; _I!=_E; ++_I)
        {
            ESceneCustomOTool* mt = dynamic_cast<ESceneCustomOTool*>(_I->second);

            if (mt&&(0!=(object=mt->FindObjectByName(name))))
            	return object;
        }
    }else{
        ESceneCustomOTool* mt = GetOTool(classfilter); VERIFY(mt);
        if (mt&&(0!=(object=mt->FindObjectByName(name)))) return object;
    }
    return object;
}

CCustomObject* EScene::FindObjectByName( LPCSTR name, CCustomObject* pass_object )
{
	CCustomObject* object = 0;
    SceneToolsMapPairIt _I = m_SceneTools.begin();
    SceneToolsMapPairIt _E = m_SceneTools.end();
    for (; _I!=_E; _I++){
        ESceneCustomOTool* mt = dynamic_cast<ESceneCustomOTool*>(_I->second);
        if (mt&&(0!=(object=mt->FindObjectByName(name,pass_object)))) return object;
    }
    return 0;
}

bool EScene::FindDuplicateName()
{
// find duplicate name
    SceneToolsMapPairIt _I = m_SceneTools.begin();
    SceneToolsMapPairIt _E = m_SceneTools.end();
    for (; _I!=_E; _I++){
        ESceneCustomOTool* mt = dynamic_cast<ESceneCustomOTool*>(_I->second);
        if (mt){
        	ObjectList& lst = mt->GetObjects(); 
            for(ObjectIt _F = lst.begin();_F!=lst.end();_F++)
                if (FindObjectByName((*_F)->GetName(), *_F)){
                    ELog.DlgMsg(mtError,"Duplicate object name already exists: '%s'",(*_F)->GetName());
                    return true;
                }
        }
    }
    return false;
}

void EScene::GenObjectName( ObjClassID cls_id, char *buffer, const char* pref )
{
	ESceneCustomOTool* ot = GetOTool(cls_id); VERIFY(ot); 
//    AnsiString result	= FHelper.GenerateName(pref&&pref[0]?pref:ot->ClassName(),4,fastdelegate::bind<TFindObjectByName>(this,&EScene::FindObjectByNameCB),true,true);
//    strcpy				(buffer,result.c_str());

	if(!pref||!pref[0])
		pref = ot->ClassName();

	if(!FindObjectByName(pref, OBJCLASS_DUMMY))
	{
		strcpy(buffer, pref);
		return;
    }

    xr_string s(pref);
	size_t len = s.length();

	if(
		len >= 5 &&
		s[len-5] == '_' &&
		isdigit(s[len-4]) &&
		isdigit(s[len-3]) &&
		isdigit(s[len-2]) &&
		isdigit(s[len-1])
	)
		s = s.substr(0,len-5);

	int& cnt = m_CountByName[s];
	do
	{
		sprintf(buffer, "%s_%04d", pref, cnt++);
	}
	while(FindObjectByName(buffer, OBJCLASS_DUMMY) != NULL);
}
//------------------------------------------------------------------------------


