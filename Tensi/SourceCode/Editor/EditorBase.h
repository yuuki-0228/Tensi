#pragma once
#include "..\Global.h"
#include "..\Common\Sprite\Sprite.h"
#include "..\Resource\SpriteResource\SpriteResource.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"

/************************************************
*	エディタベース.
*		﨑田友輝.
**/
class CEditorBase
{
public:
	CEditorBase();
	virtual ~CEditorBase();

	// 初期化.
	virtual bool Init() = 0;

	// 更新.
	virtual void Update( const float& DeltaTime ) = 0;

	// ImGui描画.
	virtual void ImGuiRender() = 0;
	// モデルの描画.
	virtual void ModelRender() = 0;
	// UIの描画.
	virtual void UIRender() = 0;
	// エフェクト描画.
	virtual void EffectRneder() = 0;

private:
};