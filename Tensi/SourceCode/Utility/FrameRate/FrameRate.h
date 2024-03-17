#pragma once
#include "..\..\Global.h"

/********************************************
*	フレームレートの計測クラス.
*		﨑田友輝.
**/
class CFrameRate final
{
public:
	CFrameRate();
	CFrameRate( const double Fps = 60.0 );
	~CFrameRate();

	// 待機関数.
	//	true の場合そのフレームを終了する.
	bool Wait();

	// FPSの取得.
	inline double GetFPS() const { return m_FPS; }
	// デルタタイムの取得.
	static inline double GetDeltaTime() { return m_FrameTime / m_Slow; }
	// 経過時間の取得.
	//	ステージロードからの経過時間(t/20,t,t*2,t*3).
	static inline D3DXVECTOR4 GetElapsedTime() { return m_Time; }
	// 経過時間の初期化.
	//	シーンのロード終了時などに呼ぶ.
	static inline void ClearElapsedTime() { m_Time = { 0.0f, 0.0f, 0.0f, 0.0f }; }

	// スローの設定( 速度を何分の一にするか ).
	static inline void SetSlow( const float Slow ) { m_Slow = Slow; }
	// 速度を元に戻す.
	static inline void ResetSpeed() { m_Slow = 1.0f; }

private:
	const double		FRAME_RATE;		// フレームレート.
	const double		MIN_FRAME_TIME;	// 最小フレームタイム.

private:
	static double		m_FrameTime;	// フレームタイム(デルタタイム).
	static float		m_Slow;			// スロー.
	static D3DXVECTOR4	m_Time;			// ステージロードからの経過時間(t/20,t,t*2,t*3).
	double				m_FPS;			// 現在のFPS.
	LARGE_INTEGER		m_StartTime;	// 開始時間.
	LARGE_INTEGER		m_NowTime;		// 現在の時間.
	LARGE_INTEGER		m_FreqTime;		// 起動した時間.
};

// デルタタイムの取得.
template<class T = float>
static inline T GetDeltaTime()
{
	return static_cast<T>( CFrameRate::GetDeltaTime() );
}

// 経過時間の取得.
//	ステージロードからの経過時間(t/20,t,t*2,t*3).
template<class T = D3DXVECTOR4>
static inline T GetElapsedTime()
{
	return static_cast<T>( CFrameRate::GetElapsedTime() );
}
