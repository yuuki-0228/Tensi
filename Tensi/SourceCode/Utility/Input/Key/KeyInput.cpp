#include "KeyInput.h"
#include "..\..\FileManager\FileManager.h"

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
}

KeyInput::KeyInput()
	: m_NowState		()
	, m_OldState		()
#ifdef ENABLE_CLASS_BOOL
	, m_KeyStop			( false, u8"キーボードのキーを停止するか", "Input" )
#else
	, m_KeyStop			( false )
#endif
	, m_IsNotActiveStop	( false )
{
	// ウィンドウの設定の取得.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];
}

KeyInput::~KeyInput()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
KeyInput* KeyInput::GetInstance()
{
	static std::unique_ptr<KeyInput> pInstance = std::make_unique<KeyInput>();
	return pInstance.get();
}

//----------------------------.
// 状態を更新.
//----------------------------.
void KeyInput::Update()
{
	KeyInput* pI = GetInstance();

	// 状態を更新する前に現在の状態をコピー.
	memcpy_s( pI->m_OldState, sizeof( m_OldState ), pI->m_NowState, sizeof( m_NowState ) );

	// 押しているキーを調べる.
	if( !GetKeyboardState( pI->m_NowState ) ) return;
}

//----------------------------.
// キーが押されたことを取得する.
//	キーが停止中なら処理は行わない.
//----------------------------.
bool KeyInput::IsKeyPress( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	if (		pI->m_KeyStop == true ) return false;
	return (	pI->m_NowState[Key] & 0x80 ) != 0;
}

//----------------------------.
// 押した瞬間.
//	キーが停止中なら処理は行わない.
//----------------------------.
bool KeyInput::IsKeyDown( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// 現在入力で前回入力してなければ.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) == 0 &&
			(	pI->m_NowState[Key] & 0x80 ) != 0;
}

//----------------------------.
// 離した瞬間.
//	キーが停止中なら処理は行わない.
//----------------------------.
bool KeyInput::IsKeyUp( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// 現在も入力で前回も入力なら.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) != 0 &&
			(	pI->m_NowState[Key] & 0x80 ) == 0;
}

//----------------------------.
// 押し続けている.
//	キーが停止中なら処理は行わない.
//----------------------------.
bool KeyInput::IsKeyRepeat( const int Key )
{
	KeyInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !DirectX11::IsWindowActive() ) return false;

	// 現在も入力で前回も入力なら.
	if (		pI->m_KeyStop == true ) return false;
	return	(	pI->m_OldState[Key] & 0x80 ) != 0 &&
			(	pI->m_NowState[Key] & 0x80 ) != 0;
}
