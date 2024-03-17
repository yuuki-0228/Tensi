#include "FrameRate.h"
#include <thread>
#include <chrono>

#pragma comment( lib, "winmm.lib" )

namespace {
	constexpr double DEFAULT_FRAME_RATE = 60.0;					// デフォルトFPS.
	constexpr double MIN_FRAME_RATE		= 10.0;					// 最低フレームレート.
	constexpr double MAX_FRAME_TIME		= 1.0 / MIN_FRAME_RATE;	// 最高フレームタイム.
};

double		CFrameRate::m_FrameTime		= 0.0f;
float		CFrameRate::m_Slow			= 1.0f;
D3DXVECTOR4 CFrameRate::m_Time			= { 0.0f, 0.0f, 0.0f, 0.0f };

CFrameRate::CFrameRate()
	: CFrameRate	( DEFAULT_FRAME_RATE )
{
}

CFrameRate::CFrameRate( const double Fps )
	: FRAME_RATE		( Fps )
	, MIN_FRAME_TIME	( 1.0 / Fps )
	, m_FPS				( Fps )
	, m_StartTime		()
	, m_NowTime			()
	, m_FreqTime		()
{
	QueryPerformanceFrequency( &m_FreqTime );
	QueryPerformanceCounter( &m_StartTime );
}

CFrameRate::~CFrameRate()
{
}

//---------------------------.
// 待機関数.
//---------------------------.
bool CFrameRate::Wait()
{
	// 現在の時間を取得.
	QueryPerformanceCounter( &m_NowTime );
	// ( 今の時間 - 前フレームの時間 ) / 周波数 = 経過時間( 秒単位 ).
	m_FrameTime =
		( static_cast<double>( m_NowTime.QuadPart ) - static_cast<double>( m_StartTime.QuadPart ) ) / 
		static_cast<double>( m_FreqTime.QuadPart );

	// 処理時間に余裕がある場合はその分待機.
	if ( m_FrameTime < MIN_FRAME_TIME * m_Slow ) {
		// 待機時間を計算.
		DWORD SleepTime = static_cast<DWORD>( ( MIN_FRAME_TIME * m_Slow - m_FrameTime ) * 1000.0 );

		timeBeginPeriod(1);
		Sleep( SleepTime );
		timeEndPeriod(1); 
		return true;
	}

	if ( m_FrameTime > 0.0				) m_FPS			= ( m_FPS * 0.99 ) + ( 0.01 / m_FrameTime );
	if ( m_FrameTime > MAX_FRAME_TIME	) m_FrameTime	= MAX_FRAME_TIME;

	m_StartTime = m_NowTime;

	m_Time.y += static_cast<float>( m_FrameTime );
	m_Time.x = m_Time.y / 20.0f;
	m_Time.z = m_Time.y * 2.0f;
	m_Time.w = m_Time.y * 3.0f;
	return false;
}