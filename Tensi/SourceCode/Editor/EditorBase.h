#pragma once
#include "..\Global.h"
#include "..\Common\Sprite\Sprite.h"
#include "..\Resource\SpriteResource\SpriteResource.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"

/************************************************
*	�G�f�B�^�x�[�X.
*		���c�F�P.
**/
class CEditorBase
{
public:
	CEditorBase();
	virtual ~CEditorBase();

	// ������.
	virtual bool Init() = 0;

	// �X�V.
	virtual void Update( const float& DeltaTime ) = 0;

	// ImGui�`��.
	virtual void ImGuiRender() = 0;
	// ���f���̕`��.
	virtual void ModelRender() = 0;
	// UI�̕`��.
	virtual void UIRender() = 0;
	// �G�t�F�N�g�`��.
	virtual void EffectRneder() = 0;

private:
};