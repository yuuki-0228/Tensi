#include "Input.h"
#include "InputList.h"
#include "Key\KeyInput.h"
#include "Controller\XInput\XInput.h"
#include "KeyLog\KeyLog.h"
#include "..\Random\Random.h"
#include "..\FileManager\FileManager.h"
#include "..\..\Scene\FadeManager\FadeManager.h"

namespace {
	constexpr char KEY_BIND_FILE_PATH[]			= "Data\\Parameter\\Config\\KeyBind.json";	// キーバインドの保存場所.
	constexpr char WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
}

Input::Input()
	: m_hWnd				()
	, m_KeyBindList			()
//	, m_pKeyLog				( nullptr )
	, m_MouseNowPoint		( { 0, 0 } )
	, m_MouseOldPoint		( { 0, 0 } )
	, m_MouseWheel			( 0 )
	, m_IsGrab				( false )
	, m_IsNotActiveStop		( false )
	, m_IsUpdateStop		( false )
	, m_IsLeftDoubleClick	( false )
	, m_IsCenterDoubleClick	( false )
	, m_IsRightDoubleClick	( false )
	, m_StartMouseSpeed		( INIT_INT )

	// キーログを再生したい場合は( true )にする.
	, m_IsKeyLogPlay	( false )
{
//	m_pKeyLog = std::make_unique<CKeyLog>( m_IsKeyLogPlay );

	// ウィンドウの設定の取得.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];

	// 開始時のマウス速度の取得.
	SystemParametersInfo( SPI_GETMOUSESPEED, NULL, &m_StartMouseSpeed, NULL );
}

Input::~Input()
{
	ResetMouseSpeed();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
Input* Input::GetInstance()
{
	static std::unique_ptr<Input> pInstance = std::make_unique<Input>();
	return pInstance.get();
}

//----------------------------.
// 状態を更新.
//----------------------------.
void Input::Update()
{
	if ( GetInstance()->m_IsUpdateStop || FadeManager::GetIsFade() ) return;

	KeyInput	::Update();
	XInput		::Update();

	// マウスの更新.
	UpdateMouse();
}

//----------------------------.
// ログを出力.
//----------------------------.
void Input::KeyLogOutput()
{
//	Input* pI = GetInstance();
//
//	if( pI->m_IsKeyLogPlay == false )	pI->m_pKeyLog->Output();
//	else								pI->m_pKeyLog->AddFrameCnt();
}

//----------------------------.
// 初期設定.
//----------------------------.
HRESULT Input::Init()
{
	Input* pI = GetInstance();

	// 現在の設定を取得する.
	json KeyBindData = FileManager::JsonLoad( KEY_BIND_FILE_PATH );
	for ( auto& [ActionName, Obj] : KeyBindData.items() ) {
		for ( auto& [KeyType, Value] : Obj.items() ) {
			// キー割り当ての取得.
			if ( KeyType == "Key" ) {
				for ( auto& v : Value ) pI->m_KeyBindList[ActionName].Key.emplace_back( ToKeyData( v ) );
			}
			// ボタン割り当ての取得.
			else if ( KeyType == "But" ) {
				for ( auto& v : Value ) pI->m_KeyBindList[ActionName].But.emplace_back( ToButData( v ) );
			}
		}
	}

	// ログを再生中なら.
	if ( pI->m_IsKeyLogPlay == true ){
		// キーログを読み込む.
//		pI->m_pKeyLog->ReadKeyLog();

		// キーを止める.
		KeyInput	::IsAllKeyStop( true );
		XInput		::IsAllKeyStop( true );

		// 乱数を固定にする.
		Random::IsRandLock( true );
	}

	Log::PushLog( "入力クラスの初期化 : 成功" );
	return S_OK;
}

//----------------------------.
// 押されたことを取得する.
//----------------------------.
bool Input::IsKeyPress( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// キーが押されているか.
	bool IsButPress = false;	// ボタンが押されたか.

	// キーボードが押されているか調べる.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyPress( k );
		if ( IsKeyPress ) break;
	}
	// コントローラが押されているか調べる.
	//	キーボードが押されていた場合調べる必要がないため処理は行わない.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyPress( b );
			if ( IsButPress ) break;
		}
	}

	// 配列番号を取得.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// 押したかどうか.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// 押した瞬間.
//----------------------------.
bool Input::IsKeyDown( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// キーが押されているか.
	bool IsButPress = false;	// ボタンが押されたか.

	// キーボードが押されているか調べる.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyDown( k );
		if ( IsKeyPress ) break;
	}
	// コントローラが押されているか調べる.
	//	キーボードが押されていた場合調べる必要がないため処理は行わない.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyDown( b );
			if ( IsButPress ) break;
		}
	}

	// 配列番号を取得.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// 押したかどうか.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// 離した瞬間.
//----------------------------.
bool Input::IsKeyUp( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// キーが押されているか.
	bool IsButPress = false;	// ボタンが押されたか.

	// キーボードが押されているか調べる.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyUp( k );
		if ( IsKeyPress ) break;
	}
	// コントローラが押されているか調べる.
	//	キーボードが押されていた場合調べる必要がないため処理は行わない.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyUp( b );
			if ( IsButPress ) break;
		}
	}

	// 配列番号を取得.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// 押したかどうか.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//----------------------------.
// 押し続けている.
//----------------------------.
bool Input::IsKeyRepeat( const std::string& Key )
{
	Input* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	bool IsKeyPress = false;	// キーが押されているか.
	bool IsButPress = false;	// ボタンが押されたか.

	// キーボードが押されているか調べる.
	for ( auto& k : pI->m_KeyBindList[Key].Key ) {
		IsKeyPress = KeyInput::IsKeyRepeat( k );
		if ( IsKeyPress ) break;
	}
	// コントローラが押されているか調べる.
	//	キーボードが押されていた場合調べる必要がないため処理は行わない.
	if ( IsKeyPress == false ) {
		for ( auto& b : pI->m_KeyBindList[Key].But ) {
			IsButPress = XInput::IsKeyRepeat( b );
			if ( IsButPress ) break;
		}
	}

	// 配列番号を取得.
	int No = 0;
	for ( auto& [k, v] : pI->m_KeyBindList ) {
		if ( k == Key ) break;
		No++;
	}

	// 押したかどうか.
	if ( IsKeyPress || IsButPress /* ||
		 pI->m_pKeyLog->ReadFrameKey() & 1 << No */ )
	{
//		pI->m_pKeyLog->AddKeyState( No );
		return true;
	}
	return false;
}

//---------------------------.
// マウスがスクリーンの中か.
//---------------------------.
bool Input::IsScreenMiddleMouse()
{
	return false;
}

//---------------------------.
// マウスの速度を変更.
//---------------------------.
void Input::SetMouseSpeed( const __int64& Speed )
{
	SystemParametersInfoA( SPI_SETMOUSESPEED, NULL, ( PVOID ) Speed, NULL );
}

//---------------------------.
// マウスの速度を元に戻す.
//---------------------------.
void Input::ResetMouseSpeed()
{
	Input* pI = GetInstance();

	SystemParametersInfoA( SPI_SETMOUSESPEED, NULL, ( PVOID ) pI->m_StartMouseSpeed, NULL );
}

//---------------------------.
// キーバインドのデータ変換.
//---------------------------.
int Input::ToKeyData( std::string Key )
{
	for ( auto& l : InputList::KeyList ) {
		if ( l.first == Key ) return l.second;
	}
	return 0;
}
WORD Input::ToButData( std::string But )
{
	for ( auto& l : InputList::ButList ) {
		if ( l.first == But ) return l.second;
	}
	return 0;
}

//---------------------------.
// マウスの更新.
//---------------------------.
void Input::UpdateMouse()
{
	Input* pI = GetInstance();

	// 前回を設定.
	pI->m_MouseOldPoint = pI->m_MouseNowPoint;
	// マウスカーソル位置取得.
	GetCursorPos( &pI->m_MouseNowPoint );
	// スクリーン座標→クライアント座標に変換.
	ScreenToClient( pI->m_hWnd, &pI->m_MouseNowPoint );
}