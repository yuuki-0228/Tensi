#include "GameMain.h"
#include "..\..\Common\Sprite\Sprite.h"
#include "..\..\Object\Light\Light.h"
#include "..\..\Object\GameObject\Actor\BackGroundManager\BackGroundManager.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\Player\Player.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\Ball\NormalBall\NormalBall.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\Ball\HeavyBall\HeavyBall.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\Ball\SuperBall\SuperBall.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\WateringCan\WateringCan.h"
#include "..\..\Object\GameObject\Actor\SlimeFrame\SlimeFrame.h"
#include "..\..\Object\GameObject\Widget\SceneWidget\GameMainWidget\GameMainWidget.h"
#include "..\..\Object\GameObject\ActorCollisionManager\ActorCollisionManager.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\Random\Random.h"
#include "..\..\Utility\TimeManager\TimeManager.h"

namespace {
	constexpr float INIT_LIGHT_DIRECTION[3] = { 0.0, 1.0f, 0.0f };
	constexpr float INIT_LIGHT_INTENSITY	= 1.0f;
}

CGameMain::CGameMain()
	: m_pPlayer			( nullptr )
	, m_pNormalBall		( nullptr )
	, m_pHeavyBall		( nullptr )
	, m_pSuperBall		( nullptr )
	, m_pSlimeFrame		( nullptr )
	, m_pWateringCan	( nullptr )
	, m_GameMainWidget	( nullptr )
{
}

CGameMain::~CGameMain()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CGameMain::Init()
{
	BackGroundManager::Init();

	m_pPlayer			= std::make_shared<CPlayer>();
	m_pNormalBall		= std::make_shared<CNormalBall>();
	m_pHeavyBall		= std::make_shared<CHeavyBall>();
	m_pSuperBall		= std::make_shared<CSuperBall>();
	m_pSlimeFrame		= std::make_shared<CSlimeFrame>();
	m_pWateringCan		= std::make_shared<CWateringCan>();
	m_GameMainWidget	= std::make_unique<CGameMainWidget>();

	m_pPlayer->Init();
	m_pNormalBall->Init();
	m_pHeavyBall->Init();
	m_pSuperBall->Init();
	m_pWateringCan->Init();
	m_pSlimeFrame->Init();
	m_GameMainWidget->Init();

	// ライトの初期化.
	Light::SetDirection( INIT_LIGHT_DIRECTION );
	Light::SetIntensity( INIT_LIGHT_INTENSITY );

	// BGMの設定.
	m_BGMName = "12-13";
	return true;
}

//---------------------------.
// 初回起動時の初期化.
//---------------------------.
bool CGameMain::FirstPlayInit()
{
	BackGroundManager::FirstPlayInit();
	return true;
}

//---------------------------.
// 初回ログイン時の初期化.
//---------------------------.
bool CGameMain::LoginInit( std::tm lastDay )
{
	const std::tm& now  = TimeManager::GetTime();
	const std::tm& diff = TimeManager::GetTimediff( now, lastDay );

	Log::PushLog( "前回のログインから" + std::to_string(diff.tm_mday) + "日空いています");

	BackGroundManager::LoginInit( now, diff );
	return false;
}

//---------------------------.
// 更新.
//---------------------------.
void CGameMain::Update( const float& DeltaTime )
{
	// 更新.
	BackGroundManager::Update( DeltaTime );
	m_pPlayer->Update( DeltaTime );
	m_pNormalBall->Update( DeltaTime );
	m_pHeavyBall->Update( DeltaTime );
	m_pSuperBall->Update( DeltaTime );
	m_pWateringCan->Update( DeltaTime );
	m_pSlimeFrame->Update( DeltaTime );

	// 当たり判定処理.
	ActorCollisionManager::Collision();

	// 当たり判定終了後の更新.
	BackGroundManager::LateUpdate( DeltaTime );
	m_pPlayer->LateUpdate( DeltaTime );
	m_pNormalBall->LateUpdate( DeltaTime );
	m_pHeavyBall->LateUpdate( DeltaTime );
	m_pSuperBall->LateUpdate( DeltaTime );
	m_pWateringCan->LateUpdate( DeltaTime );
	m_pSlimeFrame->LateUpdate( DeltaTime );

	// デバックの更新.
	BackGroundManager::DebugUpdate( DeltaTime );
	m_pPlayer->DebugUpdate( DeltaTime );
	m_pNormalBall->DebugUpdate( DeltaTime );
	m_pHeavyBall->DebugUpdate( DeltaTime );
	m_pSuperBall->DebugUpdate( DeltaTime );
	m_pWateringCan->DebugUpdate( DeltaTime );
	m_pSlimeFrame->DebugUpdate( DeltaTime );

	// UIの更新.
	m_GameMainWidget->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CGameMain::ModelRender()
{
}

//---------------------------.
// スプライト(UI)の描画.
//---------------------------.
void CGameMain::SpriteUIRender()
{
	m_GameMainWidget->Render();
	BackGroundManager::Render();
	m_pWateringCan->Render();
	m_pNormalBall->Render();
	m_pHeavyBall->Render();
	m_pSuperBall->Render();
	m_pSlimeFrame->Render();
	m_pPlayer->Render();
}
void CGameMain::SubSpriteUIRender()
{
	m_GameMainWidget->SubRender();
	BackGroundManager::SubRender();
	m_pWateringCan->SubRender();
	m_pNormalBall->SubRender();
	m_pHeavyBall->SubRender();
	m_pSuperBall->SubRender();
	m_pSlimeFrame->SubRender();
	m_pPlayer->SubRender();
}

//---------------------------.
// スプライト(3D)/Effectの描画.
//	_A：後に表示される / _B：先に表示される.
//---------------------------.
void CGameMain::Sprite3DRender_A()
{
}
void CGameMain::SubSprite3DRender_A()
{
}
void CGameMain::Sprite3DRender_B()
{
}
void CGameMain::SubSprite3DRender_B()
{
}