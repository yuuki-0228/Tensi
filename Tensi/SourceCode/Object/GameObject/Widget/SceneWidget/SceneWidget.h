#pragma once
#include "..\Widget.h"
#include "..\..\..\..\Scene\SceneManager\SceneManager.h"

/************************************************
*	シーンのUIベースクラス.
**/
class CSceneWidget
	: public CWidget
{
public:
	CSceneWidget();
	virtual ~CSceneWidget();

protected:
	ESceneList m_SceneType; // シーンタイプ.
};
