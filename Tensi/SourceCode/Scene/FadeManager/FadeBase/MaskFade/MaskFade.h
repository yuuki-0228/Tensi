#pragma once
#include "..\FadeBase.h"

/************************************************
*	マスク画像を使用したフェードクラス.
*		﨑田友輝.
**/
class CMaskFade final
	: public CFadeBase
{
public:
	CMaskFade();
	virtual ~CMaskFade();

	// 初期化.
	virtual bool Init() override;
	// 描画.
	virtual void Render() override;

	// フェードの設定.
	virtual void FadeSetting() override;

	// フェードイン.
	virtual bool FadeIn( const float Speed ) override;
	// フェードアウト.
	virtual bool FadeOut( const float Speed ) override;

};