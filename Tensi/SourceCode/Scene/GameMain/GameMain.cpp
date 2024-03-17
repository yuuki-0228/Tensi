#include "GameMain.h"
#include "..\..\Common\Sprite\Sprite.h"
#include "..\..\Object\Light\Light.h"
#include "..\..\Object\GameObject\Actor\WindowObject\Player\Player.h"
#include "..\..\Object\GameObject\Actor\WindowObject\Ball\Ball.h"
#include "..\..\Object\GameObject\Actor\WindowObject\HeavyBall\HeavyBall.h"
#include "..\..\Object\GameObject\Actor\WindowObject\SuperBall\SuperBall.h"
#include "..\..\Object\GameObject\Actor\WindowObject\WateringCan\WateringCan.h"
#include "..\..\Object\GameObject\Actor\SlimeFrame\SlimeFrame.h"
#include "..\..\Object\GameObject\Actor\Tree\Tree.h"
#include "..\..\Object\GameObject\Actor\WeedManager\CWeedManager.h"
#include "..\..\Object\GameObject\Actor\FlowerManager\FlowerManager.h"
#include "..\..\Object\GameObject\Actor\House\House.h"
#include "..\..\Object\GameObject\Actor\WaterFall\WaterFall.h"
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
	, m_pBall			( nullptr )
	, m_pHeavyBall		( nullptr )
	, m_pSuperBall		( nullptr )
	, m_pSlimeFrame		( nullptr )
	, m_pWateringCan	( nullptr )
	, m_pTree			( nullptr )
	, m_pWeedManager	( nullptr )
	, m_pFlowerManager	( nullptr )
	, m_pHouse			( nullptr )
	, m_pWaterFall		( nullptr )
	, m_GameMainWidget	( nullptr )
{
}

CGameMain::~CGameMain()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CGameMain::Init()
{
	m_pPlayer			= std::make_shared<CPlayer>();
	m_pBall				= std::make_shared<CBall>();
	m_pHeavyBall		= std::make_shared<CHeavyBall>();
	m_pSuperBall		= std::make_shared<CSuperBall>();
	m_pSlimeFrame		= std::make_shared<CSlimeFrame>();
	m_pWateringCan		= std::make_shared<CWateringCan>();
	m_pTree				= std::make_shared<CTree>();
	m_pWeedManager		= std::make_shared<CWeedManager>();
	m_pFlowerManager	= std::make_shared<CFlowerManager>();
	m_pHouse			= std::make_shared<CHouse>();
	m_pWaterFall		= std::make_shared<CWaterFall>();
	m_GameMainWidget	= std::make_unique<CGameMainWidget>();

	m_pPlayer->Init();
	m_pBall->Init();
	m_pHeavyBall->Init();
	m_pSuperBall->Init();
	m_pWateringCan->Init();
	m_pSlimeFrame->Init();
	m_pTree->Init();
	m_pWeedManager->Init();
	m_pFlowerManager->Init();
	m_pHouse->Init();
	m_pWaterFall->Init();
	m_GameMainWidget->Init();

	// ���C�g�̏�����.
	Light::SetDirection( INIT_LIGHT_DIRECTION );
	Light::SetIntensity( INIT_LIGHT_INTENSITY );

	// BGM�̐ݒ�.
	m_BGMName = "12-13";
	return true;
}

//---------------------------.
// ����N�����̏�����.
//---------------------------.
bool CGameMain::FirstPlayInit()
{
	m_pWeedManager->Fill( 100 );	// �G���𐶂₷
	m_pFlowerManager->Fill( 10 );	// �Ԃ�A����
	m_pWaterFall->Setting();		// ��̐ݒ�.
	return true;
}

//---------------------------.
// ���񃍃O�C�����̏�����.
//---------------------------.
bool CGameMain::LoginInit( std::tm lastDay )
{
	const std::tm& now  = TimeManager::GetTime();
	const std::tm& diff = TimeManager::GetTimediff( now, lastDay );

	Log::PushLog( "�O��̃��O�C������" + std::to_string(diff.tm_mday) + "���󂢂Ă��܂�");


	// �G���𐶂₷(���O�C�����Ă��Ȃ����̕������I����)
	for ( int i = 0; i < diff.tm_mday; ++i ) {
		if ( m_pWeedManager->GetNum() < 300 && Random::Probability( 1, 2 ) ) {
			const int Num = Random::GetRand( 5, 10 );
			m_pWeedManager->Fill( Num );
			Log::PushLog( "�G����" + std::to_string( Num ) + "�{���₵�܂���" );
		}
	}

	// �Ԃ�A����
	if ( m_pFlowerManager->GetNum() < 100 && Random::Probability( 1, 10 ) ) {
		const int Num = Random::GetRand( 1, 2 );
		m_pFlowerManager->Fill( Num );
		Log::PushLog( "�Ԃ�" + std::to_string( Num ) + "�{�A���܂���" );
	}

	return false;
}

//---------------------------.
// �X�V.
//---------------------------.
void CGameMain::Update( const float& DeltaTime )
{
	if ( DebugKeyInput::IsKeyDown( 'P' ) ) {
		m_pTree->Fill( Input::GetMousePosition3() );
	}
	if ( DebugKeyInput::IsKeyDown( 'O' ) ) {
		m_pHouse->Setting( Input::GetMousePosition3() );
	}
	if ( DebugKeyInput::IsKeyDown( 'I' ) ) {
		m_pWeedManager->Fill( 50 );
	}

	// �X�V.
	m_pPlayer->Update( DeltaTime );
	m_pBall->Update( DeltaTime );
	m_pHeavyBall->Update( DeltaTime );
	m_pSuperBall->Update( DeltaTime );
	m_pWateringCan->Update( DeltaTime );
	m_pSlimeFrame->Update( DeltaTime );
	m_pHouse->Update( DeltaTime );
	m_pWaterFall->Update( DeltaTime );
	m_pTree->Update( DeltaTime );
	m_pWeedManager->Update( DeltaTime );
	m_pFlowerManager->Update( DeltaTime );

	// �����蔻�菈��.
	ActorCollisionManager::Collision();

	// �����蔻��I����̍X�V.
	m_pPlayer->LateUpdate( DeltaTime );
	m_pBall->LateUpdate( DeltaTime );
	m_pHeavyBall->LateUpdate( DeltaTime );
	m_pSuperBall->LateUpdate( DeltaTime );
	m_pWateringCan->LateUpdate( DeltaTime );
	m_pSlimeFrame->LateUpdate( DeltaTime );
	m_pHouse->LateUpdate( DeltaTime );
	m_pWaterFall->LateUpdate( DeltaTime );
	m_pTree->LateUpdate( DeltaTime );
	m_pWeedManager->LateUpdate( DeltaTime );
	m_pFlowerManager->LateUpdate( DeltaTime );

	// �f�o�b�N�̍X�V.
	m_pPlayer->DebugUpdate( DeltaTime );
	m_pBall->DebugUpdate( DeltaTime );
	m_pHeavyBall->DebugUpdate( DeltaTime );
	m_pSuperBall->DebugUpdate( DeltaTime );
	m_pWateringCan->DebugUpdate( DeltaTime );
	m_pSlimeFrame->DebugUpdate( DeltaTime );
	m_pHouse->DebugUpdate( DeltaTime );
	m_pWaterFall->DebugUpdate( DeltaTime );
	m_pTree->DebugUpdate( DeltaTime );
	m_pWeedManager->DebugUpdate( DeltaTime );
	m_pFlowerManager->DebugUpdate( DeltaTime );

	// UI�̍X�V.
	m_GameMainWidget->Update( DeltaTime );
}

//---------------------------.
// �`��.
//---------------------------.
void CGameMain::ModelRender()
{
}

//---------------------------.
// �X�v���C�g(UI)�̕`��.
//---------------------------.
void CGameMain::SpriteUIRender()
{
	m_GameMainWidget->Render();
	m_pWaterFall->Render();
	m_pWateringCan->Render();
	m_pWeedManager->Render();
	m_pFlowerManager->Render();
	m_pTree->Render();
	m_pHouse->Render();
	m_pBall->Render();
	m_pHeavyBall->Render();
	m_pSuperBall->Render();
	m_pSlimeFrame->Render();
	m_pPlayer->Render();
}
void CGameMain::SubSpriteUIRender()
{
	m_GameMainWidget->SubRender();
	m_pWaterFall->SubRender();
	m_pWateringCan->SubRender();
	m_pWeedManager->SubRender();
	m_pFlowerManager->SubRender();
	m_pTree->SubRender();
	m_pHouse->SubRender();
	m_pBall->SubRender();
	m_pHeavyBall->SubRender();
	m_pSuperBall->SubRender();
	m_pSlimeFrame->SubRender();
	m_pPlayer->SubRender();
}

//---------------------------.
// �X�v���C�g(3D)/Effect�̕`��.
//	_A�F��ɕ\������� / _B�F��ɕ\�������.
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