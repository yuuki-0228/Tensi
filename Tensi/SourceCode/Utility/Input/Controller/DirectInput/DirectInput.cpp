#include "DirectInput.h"
#include <tchar.h>
#include <crtdbg.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <memory>

template <typename T>
T ToRadian( T degree )
{
	return static_cast<T>( degree * M_PI / 180.0 );
}

//解放マクロ.
#define SAFE_RELEASE(p)	if(p!=nullptr){(p)->Release();(p)=nullptr;}

//最終的には複数コントローラ対応したい.
//#define ENABLE_MULTIPLE_GAMEPAD

LPDIRECTINPUT8			g_pDICore = nullptr;	//DirectInputオブジェクト.
LPDIRECTINPUTDEVICE8	g_pPadCore = nullptr;	//コントローラ.	※複数接続なら数を増やして対応すること.

//ジョイスティック列挙関数.
BOOL CALLBACK EnumJoysticksCallBack( const DIDEVICEINSTANCE* pdidi, VOID* pContext );
//オブジェクトの列挙関数.
BOOL CALLBACK EnumObjectsCallBack( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );


DirectInput::DirectInput()
	: m_hWnd		( nullptr )
	, m_pDInput		( nullptr )
	, m_pPad		( nullptr )
	, m_JoyState	()
	, m_Status		( 0 )
	, m_StatusOld	( 0 )
#ifdef ENABLE_CLASS_BOOL
	, m_ButStop		( false, u8"DirectInputのボタンを停止させるか", "Input" )
#else
	, m_ButStop		( false )
#endif
{
}

DirectInput::~DirectInput()
{
	m_hWnd		= nullptr;
	m_pDInput	= nullptr;
	m_pPad		= nullptr;

	//以下のグローバル変数の解放タイミングがないのでここで処理する.
	SAFE_RELEASE( g_pDICore );
	SAFE_RELEASE( g_pPadCore );
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
DirectInput* DirectInput::GetInstance()
{
	static std::unique_ptr<DirectInput> pInstance = std::make_unique<DirectInput>();
	return pInstance.get();
}

//----------------------------.
// DirectInputの初期設定.
//----------------------------.
bool DirectInput::Init()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  //関数復帰値.

	hRslt = DirectInput8Create(
		GetModuleHandle( nullptr ),				//DirectInputを作成するアプリのハンドル.
		DIRECTINPUT_VERSION,					//固定：DirectInputのバージョン.
		IID_IDirectInput8,						//固定：UnicodeとAnsiの区別用.
		reinterpret_cast<LPVOID*>( &g_pDICore ),//(out)DirectInputオブジェクト.
		NULL );									//固定:nullptr.
	if( hRslt != DI_OK ){
		MessageBox( NULL, _T( "DirectInputの作成に失敗" ), _T( "エラー" ), MB_OK );
		return false;
	}

	//メンバー変数へ設定.
	pI->m_pDInput = g_pDICore;

	//利用可能なコントローラを探す(列挙する).
	hRslt = g_pDICore->EnumDevices(
		DI8DEVCLASS_GAMECTRL,	//全てのゲームコントローラ.
		EnumJoysticksCallBack,	//コントローラの列挙関数.
		nullptr,				//コールバック関数に送る情報.
		DIEDFL_ATTACHEDONLY );	//繋がっているモノのみ.
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "コントローラの確認に失敗" ), _T( "エラー" ), MB_OK );
		return false;
	}

	//メンバー変数へ設定.
	pI->m_pPad = g_pPadCore;

	if( FAILED( SettingController() ) ) return false;
	return true;
}

//----------------------------.
// 更新関数.
//	※毎フレーム呼び出されないと入力情報が更新されません.
//----------------------------.
HRESULT DirectInput::Update()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // 関数復帰値.

	// ジョイスティックの接続確認.
	if( pI->m_pPad == nullptr ){
		return E_FAIL;
	}

	// ジョイスティックが変更されているかを確認.
	hRslt = pI->m_pPad->Poll();
	if( hRslt != DI_OK && hRslt != DI_NOEFFECT )    // DI_NOEFFECT:確認不要なデバイス.
	{
		// コントローラへのアクセス権を取得する.
		hRslt = pI->m_pPad->Acquire();
		while( hRslt == DIERR_INPUTLOST ){
			hRslt = pI->m_pPad->Acquire();
		}
		return E_FAIL;
	}

	// コントローラのキー情報を更新.
	UpdateKeyInput();

	return S_OK;
}

//----------------------------.
// キーが押されたことを取得する.
//----------------------------.
bool DirectInput::IsKeyPress( DIRECT_INPUT_KEY Key )
{
	if( IsKeyCore( Key, GetInstance()->m_Status ) == true )
	{
		return true;
	}
	return false;
}

//----------------------------.
// 押した.just=trueにすることで押した瞬間の取得が可能.
//----------------------------.
bool DirectInput::IsKeyDown( DIRECT_INPUT_KEY Key )
{
	DirectInput* pI = GetInstance();

	if(  IsKeyCore( Key, pI->m_Status		) &&
		!IsKeyCore( Key, pI->m_StatusOld	) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// 離した.
//----------------------------.
bool DirectInput::IsKeyUp( DIRECT_INPUT_KEY Key )
{
	DirectInput* pI = GetInstance();

	//前回入力で今回未入力→離した瞬間の判定.
	if(  IsKeyCore( Key, pI->m_StatusOld	) &&
		!IsKeyCore( Key, pI->m_Status		) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// 押し続けている.
//----------------------------.
bool DirectInput::IsKeyRepeat( DIRECT_INPUT_KEY key )
{
	DirectInput* pI = GetInstance();

	//前回入力で今回入力→押し続けてる判定.
	if( IsKeyCore( key, pI->m_StatusOld ) &&
		IsKeyCore( key, pI->m_Status	) )
	{
		return true;
	}
	return false;
}

//----------------------------.
// 接続チェック.
//----------------------------.
bool DirectInput::IsConnect()
{
	return g_pPadCore != nullptr;
}

//----------------------------.
// コントローラ接続確認.
//----------------------------.
HRESULT DirectInput::SettingController()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // 関数復帰値.

	// コントローラの接続されているか.
	if ( g_pPadCore == nullptr ) return S_OK;
	
	// コントローラ構造体のデータフォーマットを作成.
	hRslt = g_pPadCore->SetDataFormat(
		&c_dfDIJoystick2 ); // 固定
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "データフォーマットの作成失敗" ), _T( "エラー" ), MB_OK );
	}
	// (他のデバイスとの)協調レベルの設定.
	hRslt = g_pPadCore->SetCooperativeLevel(
		pI->m_hWnd,
		DISCL_EXCLUSIVE |   // 排他アクセス.
		DISCL_FOREGROUND ); // フォアグラウンドアクセス権.
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "協調レベルの設定失敗" ), _T( "エラー" ), MB_OK );
	}
#if 0	// こっちじゃダメ？
	//	使用可能なオブジェクト(ボタンなど)の列挙.
	hRslt = g_pPadCore->EnumObjects(
		EnumObjectsCallBack,					// オブジェクト列挙関数.
		reinterpret_cast<LPVOID>( g_pPadCore ),	// コールバック関数に送る情報.
		DIDFT_ALL );							// 全てのオブジェクト.
#else
	// 使用可能なオブジェクト(ボタンなど)の列挙.
	hRslt = g_pPadCore->EnumObjects(
		EnumObjectsCallBack,					// オブジェクト列挙関数.
		reinterpret_cast<VOID*>( pI->m_hWnd ),	// コールバック関数に送る情報.
		DIDFT_ALL );							// 全てのオブジェクト.
#endif
	if( hRslt != DI_OK ){
		MessageBox( nullptr, _T( "オブジェクトの列挙に失敗" ), _T( "エラー" ), MB_OK );
	}

	return S_OK;
}

//----------------------------.
// 入力情報回収関数.
//----------------------------.
HRESULT DirectInput::UpdateKeyInput()
{
	DirectInput*	pI = GetInstance();
	HRESULT			hRslt;  // 関数復帰値.

	// コントローラから直接データを取得する.
	hRslt = pI->m_pPad->GetDeviceState(
		sizeof( DIJOYSTATE2 ),	// 取得するサイズ.
		&pI->m_JoyState );		// (out)取得データ.
	if( hRslt != DI_OK ){
		return hRslt;
	}

	// 前回入力情報を退避.
	pI->m_StatusOld = pI->m_Status;
	pI->m_Status	= 0;	// 初期化.

	//======================================.
	// 以降、キーの入力取得.
	//   ボタンとしての処理のみ設定しているので、
	//   スティックの傾きなどを保持したい場合は、
	//   別途メンバ変数を追加して格納することと!!.
	//======================================.

	// ジョイスティック.
	//	js.lX, js.lY の それぞれの範囲は「-1000 ～ 1000」.
	//	【左右】※-500～500の間は未入力と判定する.
	if(	pI->m_JoyState.lX < -500 )		{ AddKey( DIRECT_INPUT_KEY::LEFT );		}	// 左ｷｰ.
	else if( 500 < pI->m_JoyState.lX )	{ AddKey( DIRECT_INPUT_KEY::RIGHT );	}	// 右ｷｰ.
	//	【上下】※-500～500の間は未入力と判定する.
	if( pI->m_JoyState.lY < -500 )		{ AddKey( DIRECT_INPUT_KEY::UP );		}	// 上ｷｰ.
	else if( 500 < pI->m_JoyState.lY )	{ AddKey( DIRECT_INPUT_KEY::DOWN );		}	// 下ｷｰ.

	// 十字キー.
	//	有効値チェック.
	if( pI->m_JoyState.rgdwPOV[0] != 0xFFFFFFFF ) {
#if 1
		float rad = ToRadian( static_cast<float>( pI->m_JoyState.rgdwPOV[0] ) / 100.0f );
		// 本来は、xがcos、yがsinだが、rgdwPOVが0が上から時計回りなので、
		//	cosとsinを逆にした方が都合がよい.
		float x = sinf( rad );
		float y = cosf( rad );
		if( x < -0.01 )		{	AddKey( DIRECT_INPUT_KEY::LEFT );	}
		else if( x > 0.01 )	{	AddKey( DIRECT_INPUT_KEY::RIGHT );	}
		if( y > 0.01 )		{	AddKey( DIRECT_INPUT_KEY::UP );		}
		else if( y <-0.01 )	{	AddKey( DIRECT_INPUT_KEY::DOWN );	}
#else
		// rgdwPOVが0から時計回りで度数の100倍の値.
		switch( js.rgdwPOV[0] ){
		case 0:		AddKey( KEY::UP );								break;
		case 4500:	AddKey( KEY::UP );		AddKey( KEY::RIGHT );	break;
		case 9000:	AddKey( KEY::RIGHT );							break;
		case 13500:	AddKey( KEY::DOWN );	AddKey( KEY::RIGHT );	break;
		case 18000:	AddKey( KEY::DOWN );							break;
		case 22500:	AddKey( KEY::DOWN );	AddKey( KEY::LEFT );	break;
		case 27000:	AddKey( KEY::LEFT );							break;
		case 31500:	AddKey( KEY::UP );		AddKey( KEY::LEFT );	break;
		default:													break;
		}
#endif
	}

	for( int i = 0; i < static_cast<int>( DIRECT_INPUT_KEY::BTN_MAX ) + 1; i++ ) {
		//ボタン.
		if( pI->m_JoyState.rgbButtons[i] & 0x80 ) {
			AddKey( static_cast<DIRECT_INPUT_KEY>( static_cast<int>( DIRECT_INPUT_KEY::BTN_00 ) + i ) );
		}
	}

	return S_OK;
}

//----------------------------.
// 指定キーの入力追加.
//----------------------------.
void DirectInput::AddKey( DIRECT_INPUT_KEY key )
{
	GetInstance()->m_Status |= 1 << static_cast<unsigned int>( key );
}


//----------------------------.
// IsDown,IsUp,IsRepeat関数内で使用する関数.
//----------------------------.
bool DirectInput::IsKeyCore( DIRECT_INPUT_KEY key, unsigned int status )
{
	if( ( status & ( 1 << static_cast<unsigned int>( key ) ) ) != 0 ){
		return true;
	}
	return false;
}

//-------------------------------------------------.
// ジョイスティック列挙関数(ローカル関数).
//-------------------------------------------------.
BOOL CALLBACK EnumJoysticksCallBack(
	const DIDEVICEINSTANCE* pdidi,
	VOID* pContext )
{
	HRESULT hRslt;  //関数復帰値.

	//デバイス(コントローラ)の作成.
	hRslt = g_pDICore->CreateDevice(
		pdidi->guidInstance,		//デバイスの番号.
		&g_pPadCore,	//(out)作成されるデバイスオブジェクト.
		nullptr );
	if( hRslt != DI_OK ){
		return DIENUM_CONTINUE;	//次のデバイスを要求.
	}
	//最初の一つが見つかれば終了.
	return DIENUM_STOP;	//列挙停止.
}

//-------------------------------------------------.
// オブジェクトの列挙関数(ローカル関数).
//-------------------------------------------------.
BOOL CALLBACK EnumObjectsCallBack(
	const DIDEVICEOBJECTINSTANCE* pdidoi,
	VOID* pContext )
{
	HRESULT hr;

	//軸(スティック)を持っているか？.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		//入力情報の範囲を設定する.
		DIPROPRANGE diprg;  //範囲設定構造体.
		ZeroMemory( &diprg, sizeof( diprg ) );

		diprg.diph.dwSize = sizeof( diprg );			//構造体のサイズ.
		diprg.diph.dwHeaderSize = sizeof( diprg.diph );	//ヘッダー情報のサイズ.
		diprg.diph.dwObj = pdidoi->dwType;				//軸(オブジェクト).
		diprg.diph.dwHow = DIPH_BYID;					//dwObjに設定されるものが,
														//オブジェクトの種類か？インスタンス番号か？.
		diprg.lMin = -1000; // 最小値.
		diprg.lMax = +1000; // 最大値.

#if 0	//外部からPadのﾎﾟｲﾝﾀを持ってこれる？.
		//範囲を設定.
		hr = reinterpret_cast<LPDIRECTINPUTDEVICE8>( pContext )->SetProperty(
			DIPROP_RANGE,   // 範囲.
			&diprg.diph );	// 範囲設定構造体.
#else//
		// 範囲を設定.
		hr = g_pPadCore->SetProperty(
			DIPROP_RANGE,   // 範囲.
			&diprg.diph );	// 範囲設定構造体.
#endif//
		if( FAILED( hr ) )
		{
			return DIENUM_STOP;
		}
	}
	return DIENUM_CONTINUE;
}

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// 
//	参考サイト.
// 
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//http://www.eonet.ne.jp/~maeda/directx9/controller2.htm	//２つコントローラ.
//http://www.eonet.ne.jp/~maeda/directx9/directx9.htm#INPUT	//コントローラ.
//http://princess-tiara.biz/directx/?chapter=14				//DirectInput.
//http://www.charatsoft.com/develop/otogema/index.htm		//DirectInput.
//https://yttm-work.jp/directx/directx_0027.html