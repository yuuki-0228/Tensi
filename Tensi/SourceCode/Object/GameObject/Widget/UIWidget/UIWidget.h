#pragma once
#include "..\Widget.h"

/************************************************
*	UIベースクラス.
*		﨑田友輝.
**/
class CUIWidget
	: public CWidget
{
public:
	using pState_map = std::unordered_map<std::string, std::vector<SSpriteRenderState*>>;

public:
	CUIWidget();
	virtual ~CUIWidget();

	// スプライトリストの取得.
	void SetSpriteList( const Sprite_map& pState ) { m_pSpriteList = pState; }
	// スプライト情報の取得.
	void SetSpriteState( State_map& pState );

protected:
	Sprite_map	m_pSpriteList;	// スプライトリスト.
	pState_map	m_pSpriteState;	// スプライト情報.
};
