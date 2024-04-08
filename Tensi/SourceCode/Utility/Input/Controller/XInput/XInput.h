#pragma once
#include "..\..\..\..\Global.h"
#include "..\..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\KeyBind.h"

#pragma comment( lib, "xinput.lib" )

namespace{
	// トリガー入力範囲.
	static constexpr BYTE	INPUT_TRIGGER_MAX	= 255;
	// スティック入力範囲.
	static constexpr SHORT	INPUT_THUMB_MIN		=-32768;
	static constexpr SHORT	INPUT_THUMB_MAX		= 32767;
	// バイブレーション値範囲.
	static constexpr WORD	INPUT_VIBRATION_MIN	= 0;
	static constexpr WORD	INPUT_VIBRATION_MAX	= 65535;
}

/************************************************
*	Xinputコントローラー関係クラス.
**/
class XInput final
{
public:
	XInput();
	~XInput();

	// キー入力の更新.
	static bool Update();

	// ボタンを停止させるか.
	static void	IsAllKeyStop( const bool Flag ) { GetInstance()->m_ButStop = Flag; }

	// キーが押されたことを取得する.
	static bool IsKeyPress( const WORD GamePad );
	// 押した瞬間.
	static bool IsKeyDown( const WORD GamePad );
	// 離した瞬間.
	static bool IsKeyUp( const WORD GamePad );
	// 押し続けている.
	static bool IsKeyRepeat( const WORD GamePad );

	// 複数個のキーのどれかが押されたことを取得する.
	template<typename... T>
	static bool IsORPress( WORD Key, T... t );
	// 複数個のキーのどれかを押した瞬間.
	template<typename... T>
	static bool IsORKeyDown( WORD Key, T... t );
	// 複数個のキーのどれかを離した瞬間.
	template<typename... T>
	static bool IsORKeyUp( WORD Key, T... t );
	// 複数個のキーのどれかを押し続けている.
	template<typename... T>
	static bool IsORKeyRepeat( WORD Key, T... t );

	// 複数個のキーが全て押されたことを取得する.
	template<typename... T>
	static bool IsANDPress( WORD Key, T... t );
	// 複数個のキーが押した瞬間.
	//	※最後に指定したキーが押された瞬間に全て押しておく必要がある.
	template<typename... T>
	static bool IsANDKeyDown( WORD Key, T... t );
	// 複数個のキーが離した瞬間.
	//	※最後に指定したキーが離された瞬間に全て離しておく必要がある.
	template<typename... T>
	static bool IsANDKeyUp( WORD Key, T... t );
	// 複数個のキーが全て押し続けている.
	template<typename... T>
	static bool IsANDKeyRepeat( WORD Key, T... t );


	// 左トリガー取得.
	static float GetLTrigger( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.bLeftTrigger		) / static_cast<float>( INPUT_TRIGGER_MAX / Max ); }
	// 右トリガー取得.
	static float GetRTrigger( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.bRightTrigger	) / static_cast<float>( INPUT_TRIGGER_MAX / Max ); }

	// 左スティックX軸取得.
	static float GetLThumbX( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbLX			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }
	// 左スティックY軸取得.
	static float GetLThumbY( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbLY			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }

	// 右スティックX軸取得.
	static float GetRThumbX( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbRX			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }
	// 右スティックY軸取得.
	static float GetRThumbY( const float Max = 1.0f ) { return static_cast<float>( GetInstance()->m_state.Gamepad.sThumbRY			) / static_cast<float>( INPUT_THUMB_MAX / Max	); }

	static void SetIsVibration( const bool Flag ) { GetInstance()->m_IsVibration = Flag; }
	// 振動設定(0.0f ~ 1.0f).
	static bool SetVibration( const float LMotorSpd, const float RMotorSpd );

	// 接続チェック.
	static bool IsConnect() { return GetInstance()->m_connect;	}
	// パッド番号取得.
	static DWORD GetPadID() { return GetInstance()->m_padId;	}

private:
	// インスタンスの取得.
	static XInput* GetInstance();

	// 終了処理.
	static void EndProc();

	// キー入力の更新.
	bool UpdateStatus();

	//--------------------.
	//	ボタンの入力状態を取得する.
	//--------------------.
	// ボタンが押されたことを取得する.
	static bool IsButPressed( WORD GamePad );
	// ボタンを押した瞬間.
	static bool IsButDown( WORD GamePad );
	// ボタンを離した瞬間.
	static bool IsButUp( WORD GamePad );
	// ボタンを押し続けている.
	static bool IsButRepeat( WORD GamePad );

	//--------------------.
	//	スティックの状態を取得する.
	//--------------------.
	// スティックが倒したことを取得する.
	static bool IsStiPressed( WORD GamePad );
	// スティックを倒した瞬間.
	static bool IsStiDown( WORD GamePad );
	// スティックを戻した瞬間.
	static bool IsStiUp( WORD GamePad );
	// スティックを倒し続けている.
	static bool IsStiRepeat( WORD GamePad );

	//--------------------.
	//	トリガーの状態を取得する.
	//--------------------.
	// トリガーが押されたことを取得する.
	static bool IsTriPressed( WORD GamePad );
	// トリガーを押した瞬間.
	static bool IsTriDown( WORD GamePad );
	// トリガーを離した瞬間.
	static bool IsTriUp( WORD GamePad );
	// トリガーを押し続けている.
	static bool IsTriRepeat( WORD GamePad );

private:
	DWORD				m_padId;			// パッド番号(0~3).
	XINPUT_STATE		m_state;			// キー入力情報.
	XINPUT_STATE		m_stateOld;			// キー入力情報(キーストローク判定用).
	XINPUT_VIBRATION	m_vibration;		// 振動.
	CBool				m_IsVibration;		// 振動を使用するか.
	CBool				m_ButStop;			// ボタンを停止する.
	bool				m_connect;			// 接続判定.
	bool				m_IsNotActiveStop;	// アクティブウィンドウではない時に停止させるか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	XInput( const XInput& )				= delete;
	XInput( XInput&& )					= delete;
	XInput& operator = ( const XInput& )	= delete;
	XInput& operator = ( XInput&& )		= delete;
};


//----------------------------.
// 複数個のキーのどれかが押されたことを取得する.
//----------------------------.
template<typename... T>
bool XInput::IsORPress( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを押した瞬間.
//----------------------------.
template<typename... T>
bool XInput::IsORKeyDown( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyDown( Key ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを離した瞬間.
//----------------------------.
template<typename... T>
bool XInput::IsORKeyUp( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyUp( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを押し続けている.
//----------------------------.
template<typename... T>
bool XInput::IsORKeyRepeat( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーが全て押されたことを取得する.
//----------------------------.
template<typename... T>
bool XInput::IsANDPress( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが押した瞬間.
//----------------------------.
template<typename... T>
bool XInput::IsANDKeyDown( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	if ( IsKeyDown( KeyList.end() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが離した瞬間.
//----------------------------.
template<typename... T>
bool XInput::IsANDKeyUp( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	if ( IsKeyUp( KeyList.end() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが全て押し続けている.
//----------------------------.
template<typename... T>
bool XInput::IsANDKeyRepeat( WORD Key, T... t )
{
	XInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
