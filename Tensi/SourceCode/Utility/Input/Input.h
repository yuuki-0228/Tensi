#pragma once
#include "..\..\Global.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include <unordered_map>

#include "Controller\DirectInput\DirectInput.h"
#include "Controller\XInput\XInput.h"
#include "Key\KeyInput.h"
#include "Key\DebugKeyInput.h"
#include "KeyBind.h"

class CKeyLog;

/************************************************
*	キーバインドによる入力クラス.
*		﨑田友輝.
**/
class Input final
{
public:
	Input();
	~Input();

	// ウィンドウハンドルの設定.
	static void SethWnd( HWND hWnd ) { GetInstance()->m_hWnd = hWnd; }

	// 状態を更新.
	static void Update();

	// ログを出力.
	static void KeyLogOutput();

	// 初期設定.
	static HRESULT Init();

	// キーログを再生中か取得.
	static bool GetLogAuto() { return GetInstance()->m_IsKeyLogPlay; }
	// キーバインドを設定.
	static void SetKeyBind( const KeyBind_map& KeyBind ) { GetInstance()->m_KeyBindList = KeyBind; }

	// 更新を停止させるか設定.
	static void SetIsUpdateStop( const bool Flag ) { GetInstance()->m_IsUpdateStop = Flag; }

	// 押されたことを取得する.
	static bool IsKeyPress	( const std::string& Key );
	// 押した瞬間.
	static bool IsKeyDown	( const std::string& Key );
	// 離した瞬間.
	static bool IsKeyUp		( const std::string& Key );
	// 押し続けている.
	static bool IsKeyRepeat	( const std::string& Key );

	// 複数個の操作のどれかが押されたことを取得する.
	template<typename... T>
	static bool IsORPress( const std::string& Key, T... t );
	// 複数個の操作のどれかを押した瞬間.
	template<typename... T>
	static bool IsORKeyDown( const std::string& Key, T... t );
	// 複数個の操作のどれかを離した瞬間.
	template<typename... T>
	static bool IsORKeyUp( const std::string& Key, T... t );
	// 複数個の操作のどれかを押し続けている.
	template<typename... T>
	static bool IsORKeyRepeat( const std::string& Key, T... t );

	// 複数個の操作が全て押されたことを取得する.
	template<typename... T>
	static bool IsANDPress( const std::string& Key, T... t );
	// 複数個の操作が押した瞬間.
	//	※最後に指定した操作が押された瞬間に全て押しておく必要がある.
	template<typename... T>
	static bool IsANDKeyDown( const std::string& Key, T... t );
	// 複数個の操作が離した瞬間.
	//	※最後に指定した操作が離された瞬間に全て離しておく必要がある.
	template<typename... T>
	static bool IsANDKeyUp( const std::string& Key, T... t );
	// 複数個の操作が全て押し続けている.
	template<typename... T>
	static bool IsANDKeyRepeat( const std::string& Key, T... t );

	//-----------------------------------------------.
	// ↓ マウス系 ↓
	//-----------------------------------------------.

	// 座標の取得.
	static D3DXPOSITION2 GetMousePosition()		{ return D3DXPOSITION2( static_cast<float>( GetInstance()->m_MouseNowPoint.x ), static_cast<float>( GetInstance()->m_MouseNowPoint.y ) ); }
	static D3DXPOSITION3 GetMousePosition3()	{ return D3DXPOSITION3( static_cast<float>( GetInstance()->m_MouseNowPoint.x ), static_cast<float>( GetInstance()->m_MouseNowPoint.y ), 0.0f ); }
	// 前回の座標の取得.
	static D3DXPOSITION2 GetMouseOldPosition()	{ return D3DXPOSITION2( static_cast<float>( GetInstance()->m_MouseOldPoint.x ), static_cast<float>( GetInstance()->m_MouseOldPoint.y ) ); }
	static D3DXPOSITION3 GetMouseOldPosition3()	{ return D3DXPOSITION3( static_cast<float>( GetInstance()->m_MouseOldPoint.x ), static_cast<float>( GetInstance()->m_MouseOldPoint.y ), 0.0f ); }
	// 座標Xの取得.
	static float GetMousePosisionX()	{ return static_cast<float>( GetInstance()->m_MouseNowPoint.x ); }
	// 座標Yの取得.
	static float GetMousePosisionY()	{ return static_cast<float>( GetInstance()->m_MouseNowPoint.y ); }
	// 前回の座標Xの取得.
	static float GetMouseOldPosisionX()	{ return static_cast<float>( GetInstance()->m_MouseOldPoint.x ); }
	// 前回の座標Yの取得.
	static float GetMouseOldPosisionY()	{ return static_cast<float>( GetInstance()->m_MouseOldPoint.y ); }

	// マウスがスクリーンの中か.
	static bool IsScreenMiddleMouse();
	// マウスがつかんでいるか.
	static bool IsMousrGrab() { return GetInstance()->m_IsGrab; }

	// 左ボタンがクリックされたか.
	static bool GetIsLeftClickDown() { return KeyInput::IsKeyDown( VK_LBUTTON ); }
	static bool GetIsLeftClickPress() { return KeyInput::IsKeyPress( VK_LBUTTON ); }
	static bool GetIsLeftClickUp() { return KeyInput::IsKeyUp( VK_LBUTTON ); }
	// 中央ボタンがクリックされたか.
	static bool GetIsCenterClickDown() { return KeyInput::IsKeyDown( VK_MBUTTON ); }
	static bool GetIsCenterClickPress() { return KeyInput::IsKeyPress( VK_MBUTTON ); }
	static bool GetIsCenterClickUp() { return KeyInput::IsKeyUp( VK_MBUTTON ); }
	// 右ボタンがクリックされたか.
	static bool GetIsRightClickDown() { return KeyInput::IsKeyDown( VK_RBUTTON ); }
	static bool GetIsRightClickPress() { return KeyInput::IsKeyPress( VK_RBUTTON ); }
	static bool GetIsRightClickUp() { return KeyInput::IsKeyUp( VK_RBUTTON ); }


	// 左ボタンがダブルクリックされたか.
	static bool GetIsLeftDoubleClick() { return GetInstance()->m_IsLeftDoubleClick; }
	// 中央ボタンがダブルクリックされたか.
	static bool GetIsCenterDoubleClick() { return GetInstance()->m_IsCenterDoubleClick; }
	// 右ボタンがダブルクリックされたか.
	static bool GetIsRightDoubleClick() { return GetInstance()->m_IsRightDoubleClick; }

	// マウスの速度を取得.
	static __int64 SetMouseSpeed() { return GetInstance()->m_StartMouseSpeed; }
	// マウスの速度を変更.
	static void SetMouseSpeed( const __int64& Speed );
	// マウスの速度を元に戻す.
	static void ResetMouseSpeed();

	// マウスがつかんでいるか設定.
	static void SetIsMousrGrab( const int Flag ) { GetInstance()->m_IsGrab = Flag; }
	// マウスホイールの値の設定.
	static void SetMouseWheelDelta( const int delta ) { GetInstance()->m_MouseWheel = delta; }
	// マウスホイールの値の取得.
	//	+ 上方向への移動.
	//	- 下方向への移動.
	static int GetMouseWheelDelta() { return GetInstance()->m_MouseWheel; }

	// 左ボタンがダブルクリックされたか設定.
	static void SetIsLeftDoubleClick(	const bool Flag ) { GetInstance()->m_IsLeftDoubleClick	 = Flag; }
	// 中央ボタンがダブルクリックされたか設定.
	static void SetIsCenterDoubleClick( const bool Flag ) { GetInstance()->m_IsCenterDoubleClick = Flag; }
	// 右ボタンがダブルクリックされたか設定.
	static void SetIsRightDoubleClick(	const bool Flag ) { GetInstance()->m_IsRightDoubleClick	 = Flag; }

private:
	// インスタンスの取得.
	static Input* GetInstance();

	// キーバインドのデータ変換.
	static int	ToKeyData( std::string Key );
	static WORD	ToButData( std::string But );

	// マウスの更新.
	static void UpdateMouse();

private:
	HWND						m_hWnd;					// ウィンドウハンドル.
	KeyBind_map					m_KeyBindList;			// キーバインドのリスト.
//	std::unique_ptr<CKeyLog>	m_pKeyLog;				// キーログ.
	POINT						m_MouseNowPoint;		// 現在のマウス座標.
	POINT						m_MouseOldPoint;		// 前回のマウス座標.
	int							m_MouseWheel;			// マウスホイール値.
	bool						m_IsGrab;				// つかんでいるか.
	bool						m_IsKeyLogPlay;			// キーログを再生するか.
	bool						m_IsNotActiveStop;		// アクティブウィンドウではない時に停止させるか.
	bool						m_IsUpdateStop;			// 更新の停止.
	bool						m_IsLeftDoubleClick;	// 左ボタンがダブルクリックされたか.
	bool						m_IsCenterDoubleClick;	// 中央ボタンがダブルクリックされたか.
	bool						m_IsRightDoubleClick;	// 右ボタンがダブルクリックされたか.
	__int64						m_StartMouseSpeed;		// デフォルトのマウスの速度.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	Input( const Input & )				= delete;
	Input( Input && )						= delete;
	Input& operator = ( const Input & )	= delete;
	Input& operator = ( Input && )		= delete;
};


//----------------------------.
// 複数個の操作のどれかが押されたことを取得する.
//----------------------------.
template<typename... T>
bool Input::IsORPress( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個の操作のどれかを押した瞬間.
//----------------------------.
template<typename... T>
bool Input::IsORKeyDown( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyDown( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個の操作のどれかを離した瞬間.
//----------------------------.
template<typename... T>
bool Input::IsORKeyUp( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyUp( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個の操作のどれかを押し続けている.
//----------------------------.
template<typename... T>
bool Input::IsORKeyRepeat( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) ) return true;
	}
	return false;
}

//----------------------------.
// 複数個の操作が全て押されたことを取得する.
//----------------------------.
template<typename... T>
bool Input::IsANDPress( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個の操作が押した瞬間.
//----------------------------.
template<typename... T>
bool Input::IsANDKeyDown( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	if ( IsKeyDown( KeyList.back() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) == false ) return false;
	}
	return true;
}

//----------------------------.
// 複数個の操作が離した瞬間.
//----------------------------.
template<typename... T>
bool Input::IsANDKeyUp( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	if ( IsKeyUp( KeyList.back() ) == false ) return false;
	for ( auto& k : KeyList ) {
		if ( IsKeyPress( k ) ) return false;
	}
	return true;
}

//----------------------------.
// 複数個の操作が全て押し続けている.
//----------------------------.
template<typename... T>
bool Input::IsANDKeyRepeat( const std::string& Key, T... t )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	std::vector<std::string> KeyList = { Key, t... };
	for ( auto& k : KeyList ) {
		if ( IsKeyRepeat( k ) == false ) return false;
	}
	return true;
}
