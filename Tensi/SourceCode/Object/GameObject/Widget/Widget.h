#pragma once
#include "..\GameObject.h"
#include "..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\Common\Font\Font.h"
#include "..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\Resource\FontResource\FontResource.h"

namespace {
	// 選択肢列挙体.
	enum class enChoice : unsigned char {
		StartChoice,	// 最初の選択肢.
		EndChoice,		// 最後の選択肢.

		Max,			// 最大要素数.
	} typedef EChoice;

	// タイトルの選択肢列挙体.
	enum class enTitleChoice : unsigned char
	{
		GameStart,			// ゲーム開始.
		MiniGame,			// ミニゲーム.
		PizzaEdit,			// ピザエディタ.
		RankingDelete,		// ランキングの削除.
		GameEnd,			// ゲーム終了.

		Max,				// 最大要素数.
		Start	= 0,		// 一番最初の選択肢.
		End		= Max - 1,	// 一番最後の選択肢.
	} typedef ETitleChoice;

	// 拡縮管理列挙体.
	enum class enScaling : unsigned char
	{
		None,			// 待機中.
		Expand,			// 拡大中.
		Shrink,			// 縮小中.
	} typedef EScaling;
}

/************************************************
*	ゲーム内のUIベースクラス.
*		﨑田友輝.
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
