#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"

/************************************************
*	シェイク( 振動 )アニメーションクラス.
*	時間とともに減衰するランダムな2Dオフセットを生成する.
*	衝突・被弾・画面揺れなどの瞬間的な演出に利用する.
**/
class CShakeAnimation
{
public:
	CShakeAnimation();

	// 振動を開始する.
	//	Strength : 最大の揺れ幅( ピクセルなどの単位 ).
	//	Duration : 揺れが収まるまでの秒数.
	void Start( const float Strength, const float Duration );

	// 更新.
	void Update( const float DeltaTime );

	// 揺れを止めてオフセットを0へ戻す.
	void Stop();

	// 現在の揺れオフセットを取得.
	const D3DXVECTOR2& GetOffset() const { return m_Offset; }

	// 揺れている最中かどうか.
	bool IsShaking() const { return m_Elapsed < m_Duration; }

private:
	D3DXVECTOR2	m_Offset;		// 現在の揺れオフセット.
	float		m_Strength;		// 最大の揺れ幅.
	float		m_Duration;		// 揺れが収まるまでの秒数.
	float		m_Elapsed;		// 経過秒数.
};

#endif
