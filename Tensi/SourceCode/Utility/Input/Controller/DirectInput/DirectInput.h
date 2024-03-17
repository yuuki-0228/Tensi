#pragma once
#define DIRECTINPUT_VERSION	0x0800	//警告消すためにバージョンを指定.
#include <dinput.h>	//DirectInput用.

//DirectX関係.
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )
#include "..\..\..\Bool\Bool.h"

enum class DIRECT_INPUT_KEY : int	//基底型をintにする.
{
	UP,		RIGHT,	DOWN,	LEFT,
	BTN_00,	BTN_01, BTN_02, BTN_03,
	BTN_04,	BTN_05, BTN_06, BTN_07,
	BTN_08,	BTN_09, BTN_10, BTN_11,

	MAX,
	BTN_FIRST	= BTN_00,
	BTN_LAST	= BTN_11,
	BTN_MAX		= BTN_LAST - BTN_FIRST,
};

/****************************************
*	DirectInputクラス.
**/
class DirectInput final
{
public:
	DirectInput();
	~DirectInput();

	// ウィンドウハンドルの設定.
	static void	SethWnd( HWND hWnd ) { GetInstance()->m_hWnd = hWnd; }
	// 初期化.
	static bool Init();

	// キー入力の更新.
	static HRESULT Update();

	// ボタンを停止させるか.
	static void	IsAllKeyStop( bool Flag ) { GetInstance()->m_ButStop = Flag; }

	// キーが押されたことを取得する.
	static bool IsKeyPress( DIRECT_INPUT_KEY Key );
	// 押した瞬間.
	static bool IsKeyDown( DIRECT_INPUT_KEY Key );
	// 離した瞬間.
	static bool IsKeyUp( DIRECT_INPUT_KEY Key );
	// 押し続けている.
	static bool IsKeyRepeat( DIRECT_INPUT_KEY Key );

	// 複数個のキーのどれかが押されたことを取得する.
	template<typename... T>
	static bool IsORPress( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーのどれかを押した瞬間.
	template<typename... T>
	static bool IsORKeyDown( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーのどれかを離した瞬間.
	template<typename... T>
	static bool IsORKeyUp( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーのどれかを押し続けている.
	template<typename... T>
	static bool IsORKeyRepeat( DIRECT_INPUT_KEY Key, T... t );

	// 複数個のキーが全て押されたことを取得する.
	template<typename... T>
	static bool IsANDPress( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーが押した瞬間.
	//	※最後に指定したキーが押された瞬間に全て押しておく必要がある.
	template<typename... T>
	static bool IsANDKeyDown( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーが離した瞬間.
	//	※最後に指定したキーが離された瞬間に全て離しておく必要がある.
	template<typename... T>
	static bool IsANDKeyUp( DIRECT_INPUT_KEY Key, T... t );
	// 複数個のキーが全て押し続けている.
	template<typename... T>
	static bool IsANDKeyRepeat( DIRECT_INPUT_KEY Key, T... t );

	// 接続チェック.
	static bool		IsConnect();

private:
	// インスタンスの取得.
	static DirectInput* GetInstance();

	// コントローラ接続確認.
	static HRESULT	SettingController();
	// 入力情報回収関数.
	static HRESULT	UpdateKeyInput();
	// 指定キーの入力追加.
	static void		AddKey( DIRECT_INPUT_KEY key );
	// IsDown,IsUp,IsRepeat関数内で使用する関数.
	static bool		IsKeyCore( DIRECT_INPUT_KEY key, unsigned int status );

private:
	HWND					m_hWnd;			// ウィンドウハンドル.
	LPDIRECTINPUT8			m_pDInput;		// DirectInputオブジェクト.
	LPDIRECTINPUTDEVICE8	m_pPad;			// コントローラ.	※複数接続なら数を増やして対応すること.

	DIJOYSTATE2				m_JoyState;		// ジョイスティック情報構造体.
	unsigned int			m_Status;		// 入力情報.
	unsigned int			m_StatusOld;	// 前回フレームの入力情報.

	CBool					m_ButStop;		// ボタンを停止する.
};


//----------------------------.
// 複数個のキーのどれかが押されたことを取得する.
//----------------------------.
template<typename... T>
bool DirectInput::IsORPress( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsORKeyDown( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsORKeyUp( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsORKeyRepeat( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsANDPress( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsANDKeyDown( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsANDKeyUp( DIRECT_INPUT_KEY Key, T... t )
{
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
bool DirectInput::IsANDKeyRepeat( DIRECT_INPUT_KEY Key, T... t )
{
	std::vector<WORD> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
