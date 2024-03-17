#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Common\Sprite\Sprite.h"

/************************************************
*	フェードクラス.
*		﨑田友輝.
**/
class CFadeBase
{
protected:
	// フェードの状態.
	enum class enFadeState : unsigned char
	{
		None,
		FadeIn,		// フェードイン中.
		FadeOut		// フェードアウト中.
	} typedef EFadeState;

public:
	CFadeBase();
	virtual ~CFadeBase();

	// 初期化.
	virtual bool Init() = 0;
	// 描画.
	virtual void Render() = 0;

	// フェードの設定.
	virtual void FadeSetting() = 0;

	// フェードイン.
	virtual bool FadeIn( const float Speed ) = 0;
	// フェードアウト.
	virtual bool FadeOut( const float Speed ) = 0;


protected:
	CSprite*					m_pFadeSprite;	// フェード用の画像.
	ID3D11ShaderResourceView*	m_pMaskTexture;	// マスクテクスチャ.
	SSpriteRenderState			m_FadeState;	// フェード用の画像の情報.
	EFadeState					m_NowFade;		// 現在のフェードの状態.
	float						m_FadeSpeed;	// フェード速度.
};