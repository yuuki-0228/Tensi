#pragma once
#include "..\SceneWidget.h"

/************************************************
*	�Q�[�����C��UI�N���X.
*		��J��.
**/
class CGameMainWidget final
	: public CSceneWidget
{
public:
	CGameMainWidget();
	virtual ~CGameMainWidget();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

private:
};
