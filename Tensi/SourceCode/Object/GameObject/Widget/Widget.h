#pragma once
#include "..\GameObject.h"
#include "..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\Common\Font\Font.h"
#include "..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\Resource\FontResource\FontResource.h"

namespace {

}

/************************************************
*	ゲーム内のUIベースクラス.
**/
class CWidget
	: public CGameObject
{
public:
	using Sprite_map	= std::unordered_map<std::string, std::vector<CSprite*>>;
	using State_map		= std::unordered_map<std::string, SSpriteRenderStateList>;

public:
	CWidget();
	virtual ~CWidget();

	// 描画.
	virtual void Sprite3DRender_A() override final {};
	virtual void Sprite3DRender_B() override final {};

protected:
	const float m_Wnd_W;		// ウィンドウの幅.
	const float m_Wnd_H;		// ウィンドウの高さ.
};
