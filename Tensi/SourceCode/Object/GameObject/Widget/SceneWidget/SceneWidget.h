#pragma once
#include "..\Widget.h"
#include "..\..\..\..\Scene\SceneManager\SceneManager.h"

/************************************************
*	�V�[����UI�x�[�X�N���X.
**/
class CSceneWidget
	: public CWidget
{
public:
	CSceneWidget();
	virtual ~CSceneWidget();

protected:
	ESceneList m_SceneType; // �V�[���^�C�v.
};
