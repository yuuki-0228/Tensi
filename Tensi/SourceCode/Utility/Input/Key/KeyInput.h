#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Common\DirectX\DirectX11.h"

/************************************************
*	キーボードのキー入力クラス.
**/
class KeyInput final
{
private:
	static constexpr int KEY_MAX = 256;	// キーの最大値.

public:
	KeyInput();
	~KeyInput();

	// 状態を更新.
	static void Update();

	// 全てのキーを停止するか.
	inline static void IsAllKeyStop( const bool Flag ) { GetInstance()->m_KeyStop = Flag; };

	// キーが押されたことを取得する.
	static bool IsKeyPress( const int Key );
	// 押した瞬間.
	static bool IsKeyDown( const int Key );
	// 離した瞬間.
	static bool IsKeyUp( const int Key );
	// 押し続けている.
	static bool IsKeyRepeat( const int Key );

	// 複数個のキーのどれかが押されたことを取得する.
	template<typename... T>
	static bool IsORPress( const int Key, T... t );
	// 複数個のキーのどれかを押した瞬間.
	template<typename... T>
	static bool IsORKeyDown( const int Key, T... t );
	template<typename... T>
	// 複数個のキーのどれかを離した瞬間.
	static bool IsORKeyUp( const int Key, T... t );
	template<typename... T>
	// 複数個のキーのどれかを押し続けている.
	static bool IsORKeyRepeat( const int Key, T... t );

	// 複数個のキーが全て押されたことを取得する.
	template<typename... T>
	static bool IsANDPress( const int Key, T... t );
	// 複数個のキーが押した瞬間.
	//	※最後に指定したキーが押された瞬間に全て押しておく必要がある.
	template<typename... T>
	static bool IsANDKeyDown( const int Key, T... t );
	// 複数個のキーが離した瞬間.
	//	※最後に指定したキーが離された瞬間に全て離しておく必要がある.
	template<typename... T>
	static bool IsANDKeyUp( const int Key, T... t );
	// 複数個のキーが全て押し続けている.
	template<typename... T>
	static bool IsANDKeyRepeat( const int Key, T... t );

private:
	// インスタンスの取得.
	static KeyInput* GetInstance();

private:
	BYTE	m_NowState[KEY_MAX];	// 現在のキーの状態.
	BYTE	m_OldState[KEY_MAX];	// 前回のキーの状態.
#ifdef ENABLE_CLASS_BOOL
	CBool	m_KeyStop;				// 全てのキーを停止する.
#else
	bool	m_KeyStop;				// 全てのキーを停止する.
#endif
	bool	m_IsNotActiveStop;		// アクティブウィンドウではない時に停止させるか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	KeyInput( const KeyInput& )				= delete;
	KeyInput( KeyInput&& )					= delete;
	KeyInput& operator = ( const KeyInput& )	= delete;
	KeyInput& operator = ( KeyInput&& )		= delete;
};


//----------------------------.
// 複数個のキーのどれかが押されたことを取得する.
//----------------------------.
template<typename... T>
bool KeyInput::IsORPress( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを押した瞬間.
//----------------------------.
template<typename... T>
bool KeyInput::IsORKeyDown( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyDown( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを離した瞬間.
//----------------------------.
template<typename... T>
bool KeyInput::IsORKeyUp( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyUp( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーのどれかを押し続けている.
//----------------------------.
template<typename... T>
bool KeyInput::IsORKeyRepeat( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個のキーが全て押されたことを取得する.
//----------------------------.
template<typename... T>
bool KeyInput::IsANDPress( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが押した瞬間.
//----------------------------.
template<typename... T>
bool KeyInput::IsANDKeyDown( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	if ( IsKeyDown( KeyList.back() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが離した瞬間.
//----------------------------.
template<typename... T>
bool KeyInput::IsANDKeyUp( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	if ( IsKeyUp( KeyList.back() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return false;
	}
	return true;
}

//----------------------------.
// 複数個のキーが全て押し続けている.
//----------------------------.
template<typename... T>
bool KeyInput::IsANDKeyRepeat( const int Key, T... t )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<int> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
