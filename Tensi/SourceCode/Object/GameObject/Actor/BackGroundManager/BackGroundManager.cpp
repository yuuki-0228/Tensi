#include "BackGroundManager.h"
#include "ActorBackGround\Tree\Tree.h"
#include "ActorBackGround\WeedManager\WeedManager.h"
#include "ActorBackGround\FlowerManager\FlowerManager.h"
#include "ActorBackGround\House\House.h"
#include "ActorBackGround\WaterFall\WaterFall.h"
#include "..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\Utility\Input\Key\DebugKeyInput.h"

BackGroundManager::BackGroundManager()
	: m_pWeedManager	( nullptr )
	, m_pFlowerManager	( nullptr )
	, m_pWaterFall		( nullptr )
//	, m_pTree			( nullptr )
//	, m_pHouse			( nullptr )
{
}


BackGroundManager::~BackGroundManager()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
BackGroundManager* BackGroundManager::GetInstance()
{
	static std::unique_ptr<BackGroundManager> pInstance = std::make_unique<BackGroundManager>();
	return pInstance.get();
}

//---------------------------.
// 初期化.
//---------------------------.
bool BackGroundManager::Init()
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWeedManager		= std::make_shared<CWeedManager>();
	pI->m_pFlowerManager	= std::make_shared<CFlowerManager>();
	pI->m_pWaterFall		= std::make_shared<CWaterFall>();
//	pI->m_pTree				= std::make_shared<CTree>();
//	pI->m_pHouse			= std::make_shared<CHouse>();

	pI->m_pWeedManager->Init();
	pI->m_pFlowerManager->Init();
	pI->m_pWaterFall->Init();
//	pI->m_pTree->Init();
//	pI->m_pHouse->Init();
	return true;
}

//---------------------------.
// 初回ログイン時の初期化.
//---------------------------.
bool BackGroundManager::FirstPlayInit()
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWeedManager->Fill( 100 );	// 雑草を生やす
	pI->m_pFlowerManager->Fill( 10 );	// 花を植える
	pI->m_pWaterFall->Setting();		// 滝の設定.
	return true;
}

//---------------------------.
// 初回ログイン時の初期化.
//---------------------------.
bool BackGroundManager::LoginInit( const std::tm& now, const std::tm& diff )
{
	BackGroundManager* pI = GetInstance();

	// 雑草を生やす(ログインしていない日の分も抽選する)
	for ( int i = 0; i < diff.tm_mday; ++i ) {
		if ( pI->m_pWeedManager->GetNum() < 300 && Random::Probability( 1, 2 ) ) {
			const int Num = Random::GetRand( 5, 10 );
			pI->m_pWeedManager->Fill( Num );
			Log::PushLog( "雑草を" + std::to_string( Num ) + "本生やしました" );
		}
	}

	// 花を植える
	if ( pI->m_pFlowerManager->GetNum() < 100 && Random::Probability( 1, 10 ) ) {
		const int Num = Random::GetRand( 1, 2 );
		pI->m_pFlowerManager->Fill( Num );
		Log::PushLog( "花を" + std::to_string( Num ) + "本植えました" );
	}
	return false;
}

//---------------------------.
// 更新.
//---------------------------.
void BackGroundManager::Update( const float& DeltaTime )
{
	BackGroundManager* pI = GetInstance();

	if ( DebugKeyInput::IsKeyDown( 'I' ) ) {
		pI->m_pWeedManager->Fill( 50 );
	}

	pI->m_pWaterFall->Update( DeltaTime );
	pI->m_pWeedManager->Update( DeltaTime );
	pI->m_pFlowerManager->Update( DeltaTime );
//	pI->m_pHouse->Update( DeltaTime );
//	pI->m_pTree->Update( DeltaTime );
}

//---------------------------.
// 当たり判定終了後呼び出される更新.
//---------------------------.
void BackGroundManager::LateUpdate( const float& DeltaTime )
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWaterFall->LateUpdate( DeltaTime );
	pI->m_pWeedManager->LateUpdate( DeltaTime );
	pI->m_pFlowerManager->LateUpdate( DeltaTime );
//	pI->m_pHouse->LateUpdate( DeltaTime );
//	pI->m_pTree->LateUpdate( DeltaTime );
}

//---------------------------.
// デバックの更新.
//---------------------------.
void BackGroundManager::DebugUpdate( const float& DeltaTime )
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWaterFall->DebugUpdate( DeltaTime );
	pI->m_pWeedManager->DebugUpdate( DeltaTime );
	pI->m_pFlowerManager->DebugUpdate( DeltaTime );
//	pI->m_pHouse->DebugUpdate( DeltaTime );
//	pI->m_pTree->DebugUpdate( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void BackGroundManager::Render()
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWaterFall->Render();
	pI->m_pWeedManager->Render();
	pI->m_pFlowerManager->Render();
//	pI->m_pTree->Render();
//	pI->m_pHouse->Render();
}
void BackGroundManager::SubRender()
{
	BackGroundManager* pI = GetInstance();

	pI->m_pWaterFall->SubRender();
	pI->m_pWeedManager->SubRender();
	pI->m_pFlowerManager->SubRender();
//	pI->m_pTree->SubRender();
//	pI->m_pHouse->SubRender();
}


