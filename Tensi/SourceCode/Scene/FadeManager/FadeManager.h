#pragma once
#include "..\..\Global.h"

class CNormalFade;
class CMaskFade;
class CUFOFade;

// フェードタイプ.
enum class enFadeType : unsigned char
{
	None,
	
	NormalFade,	// 普通のフェード.
	MaskFade,	// マスク画像を使用したフェード.
	UFOFade,	// UFO型に切り抜かれたフェード.
} typedef EFadeType;

/************************************************
*	フェードマネージャークラス.
**/
class FadeManager final
{
public:
	FadeManager();
	~FadeManager();

	// 初期化.
	static bool Init();
	// 描画.
	static void Render();

	// フェードの設定.
	static void FadeSetting( const EFadeType Type );

	// フェードイン.
	static bool FadeIn( const EFadeType Type, const float Speed );
	// フェードアウト.
	static bool FadeOut( const EFadeType Type, const float Speed );

	// フェード中か取得.
	static bool GetIsFade() { return GetInstance()->m_NowFadeType != EFadeType::None; }

private:
	// インスタンスの取得.
	static FadeManager* GetInstance();

private:
	EFadeType						m_NowFadeType;	// 現在のフェードタイプ.
	std::unique_ptr<CNormalFade>	m_pNormalFade;	// 普通のフェード.
	std::unique_ptr<CMaskFade>		m_pMaskFade;	// マスク画像を使用したフェード.
	std::unique_ptr<CUFOFade>		m_pUFOFade;		// UFO型に切り抜かれたフェード.
};