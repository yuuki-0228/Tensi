#include "SceneManager.h"
#include "..\GameMain\GameMain.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include "..\..\Common\SoundManeger\SoundManeger.h"
#include "..\..\Object\GameObject\ActorCollisionManager\ActorCollisionManager.h"
#include "..\..\Utility\ImGuiManager\DebugWindow\DebugWindow.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"
#include "..\..\Utility\StringConversion\StringConversion.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"
#include "..\..\Utility\SaveDataManager\SaveDataManager.h"
#include "..\..\Utility\Const\Const.h"

SceneManager::SceneManager()
	: m_pScene			( nullptr )
	, m_NowScene		( ESceneList::GameMain )
	, m_ChangeScene		( ESceneList::None )
	, m_IsSceneChange	( false )
	, m_IsFadeOut		( false )
	, m_UpdateStopFlag	( false )
	, m_IsSceneStop		( false )
	, m_IsPause			( false )
	, m_IsUIRender		( true, u8"UIを表示するか", "System" )
	, m_AutoSaveCnt		( INIT_FLOAT )
{
	m_pScene	= std::make_unique<CGameMain>();

	// 初期化.
	m_pScene->Init();
	FadeManager::Init();

	if ( SaveDataManager::Load() ) {
		// 初回起動時の初期化.
		m_pScene->FirstPlayInit();
		SaveDataManager::Save();
		Log::PushLog( "初回起動設定 : 成功" );
	}

	std::tm lastDay;
	if ( SaveDataManager::LoginLoad( &lastDay ) ) {
		// 初めてのログインの設定.
		m_pScene->LoginInit( lastDay );
		SaveDataManager::LoginSave();
		Log::PushLog( "初回ログイン設定 : 成功" );
	}
}

SceneManager::~SceneManager()
{
	SaveDataManager::Save();
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
SceneManager* SceneManager::GetInstance()
{
	static std::unique_ptr<SceneManager> pInstance = std::make_unique<SceneManager>();
	return pInstance.get();
}

//---------------------------.
// 更新.
//---------------------------.
void SceneManager::Update( const float& DeltaTime )
{
	SceneManager* pI = GetInstance();

#ifdef _DEBUG
	PositionRenderer::Update( DeltaTime );
#endif

	// オートセーブ
	pI->m_AutoSaveCnt += DeltaTime;
	if ( pI->m_AutoSaveCnt >= Const::System.AUTO_SAVE_TIME ) {
		pI->m_AutoSaveCnt = INIT_FLOAT;
		SaveDataManager::Save();
	}

	// シーンの変更の更新.
	SceneChangeUpdate();

	// シーンの更新.
	if ( pI->m_UpdateStopFlag == false && pI->m_IsSceneStop == false ) {
		pI->m_pScene->Update( DeltaTime );
	}

	// UIの表示の切り替え.
	if ( DebugKeyInput::IsANDKeyDown( VK_CONTROL, VK_F10 ) ) pI->m_IsUIRender.Inversion();

//	DebugWindow::PushProc( u8"SceneManager",
//	[&] {
//		if ( ImGuiManager::Button( u8"ゲームメインへ移動"		) ) SceneChange( ESceneList::GameMain	);
//		if ( ImGuiManager::Button( u8"UIエディタへ移動"			) ) SceneChange( ESceneList::UIEdit		);
//	} );
}

//---------------------------.
// 描画.
//---------------------------.
void SceneManager::Render()
{
	SceneManager* pI = GetInstance();

	// シーンの描画.
	pI->m_pScene->Sprite3DRender_B();
	pI->m_pScene->ModelRender();
	pI->m_pScene->Sprite3DRender_A();
	if ( pI->m_IsUIRender == true ) {
		pI->m_pScene->SpriteUIRender();
	}

	// フェードの描画.
	FadeManager::Render();
}

//---------------------------.
// サブウィンドウの描画.
//---------------------------.
void SceneManager::SubRender()
{
	SceneManager* pI = GetInstance();

	// シーンの描画.
	pI->m_pScene->SubSprite3DRender_B();
	pI->m_pScene->SubModelRender();
	pI->m_pScene->SubSprite3DRender_A();
	if ( pI->m_IsUIRender == true ) {
		pI->m_pScene->SubSpriteUIRender();
	}
}

//---------------------------.
// シーンの変更.
//---------------------------.
void SceneManager::SceneChange( const ESceneList NewScene, const EFadeType FadeType, const float FadeSpeed )
{
	SceneManager* pI = GetInstance();

	if ( pI->m_IsSceneChange ) return;

	// シーン遷移情報を保存.
	pI->m_IsSceneChange = true;
	pI->m_ChangeScene	= NewScene;
	pI->m_NowScene		= NewScene;

	// フェード情報を保存.
	pI->m_FadeType		= FadeType;
	pI->m_FadeSpeed		= FadeSpeed;

	// フェードの設定.
	FadeManager::FadeSetting( FadeType );

	MessageWindow::PushMessage( StringConversion::Enum( NewScene ) + u8"に移動しました", Color4::Yellow );
}

//---------------------------.
// シーンの変更の更新.
//	シーンの変更を行っていなく
//	更新も止めていない 場合は処理は行わない.
//---------------------------.
void SceneManager::SceneChangeUpdate()
{
	SceneManager* pI = GetInstance();

	if ( pI->m_IsSceneChange == false && pI->m_UpdateStopFlag == false ) return;

	// 更新を停止させる.
	pI->m_UpdateStopFlag = true;

	// フェードインを行う.
	if ( pI->m_IsFadeOut == false && FadeManager::FadeIn( pI->m_FadeType, pI->m_FadeSpeed ) ) {
		// 当たり判定リストを初期化する.
		ActorCollisionManager::Reset();

		// シーンを変更.
		switch ( pI->m_ChangeScene ) {
		case ESceneList::GameMain:
			SoundManager::BGMAllStop();
			pI->m_pScene = std::make_unique<CGameMain>();
			break;
		default:
			break;
		}

		// 新しいシーンの初期化.
		pI->m_pScene->Init();
		pI->m_pScene->Update( 0.0f );
		pI->m_IsFadeOut = true;

		// コントローラの振動の停止.
		XInput::SetVibration( 0.0f, 0.0f );

#ifdef _DEBUG
		// 定数の初期化.
		Const::Load();
#endif
	}
	// フェードアウトを行う.
	else if ( pI->m_IsFadeOut && FadeManager::FadeOut( pI->m_FadeType, pI->m_FadeSpeed ) ) {
		pI->m_IsFadeOut			= false;
		pI->m_UpdateStopFlag	= false;
		pI->m_IsSceneChange		= false;
		pI->m_ChangeScene		= ESceneList::None;
	}
}
