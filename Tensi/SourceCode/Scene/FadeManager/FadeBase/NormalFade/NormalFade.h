#pragma once
#include "..\FadeBase.h"

/************************************************
*	普通のフェードクラス.
*		﨑田友輝.
**/
class CNormalFade final
	: public CFadeBase
{
public:
	CNormalFade();
	virtual ~CNormalFade();

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