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
	, m_IsUIRender		( true, u8"UI��\�����邩", "System" )
	, m_AutoSaveCnt		( INIT_FLOAT )
{
	m_pScene	= std::make_unique<CGameMain>();

	// ������.
	m_pScene->Init();
	FadeManager::Init();

	if ( SaveDataManager::Load() ) {
		// ����N�����̏�����.
		m_pScene->FirstPlayInit();
		SaveDataManager::Save();
		Log::PushLog( "����N���ݒ� : ����" );
	}

	std::tm lastDay;
	if ( SaveDataManager::LoginLoad( &lastDay ) ) {
		// ���߂Ẵ��O�C���̐ݒ�.
		m_pScene->LoginInit( lastDay );
		SaveDataManager::LoginSave();
		Log::PushLog( "���񃍃O�C���ݒ� : ����" );
	}
}

SceneManager::~SceneManager()
{
	SaveDataManager::Save();
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
SceneManager* SceneManager::GetInstance()
{
	static std::unique_ptr<SceneManager> pInstance = std::make_unique<SceneManager>();
	return pInstance.get();
}

//---------------------------.
// �X�V.
//---------------------------.
void SceneManager::Update( const float& DeltaTime )
{
	SceneManager* pI = GetInstance();

#ifdef _DEBUG
	PositionRenderer::Update( DeltaTime );
#endif

	// �I�[�g�Z�[�u
	pI->m_AutoSaveCnt += DeltaTime;
	if ( pI->m_AutoSaveCnt >= Const::System.AUTO_SAVE_TIME ) {
		pI->m_AutoSaveCnt = INIT_FLOAT;
		SaveDataManager::Save();
	}

	// �V�[���̕ύX�̍X�V.
	SceneChangeUpdate();

	// �V�[���̍X�V.
	if ( pI->m_UpdateStopFlag == false && pI->m_IsSceneStop == false ) {
		pI->m_pScene->Update( DeltaTime );
	}

	// UI�̕\���̐؂�ւ�.
	if ( DebugKeyInput::IsANDKeyDown( VK_CONTROL, VK_F10 ) ) pI->m_IsUIRender.Inversion();

//	DebugWindow::PushProc( u8"SceneManager",
//	[&] {
//		if ( ImGuiManager::Button( u8"�Q�[�����C���ֈړ�"		) ) SceneChange( ESceneList::GameMain	);
//		if ( ImGuiManager::Button( u8"UI�G�f�B�^�ֈړ�"			) ) SceneChange( ESceneList::UIEdit		);
//	} );
}

//---------------------------.
// �`��.
//---------------------------.
void SceneManager::Render()
{
	SceneManager* pI = GetInstance();

	// �V�[���̕`��.
	pI->m_pScene->Sprite3DRender_B();
	pI->m_pScene->ModelRender();
	pI->m_pScene->Sprite3DRender_A();
	if ( pI->m_IsUIRender == true ) {
		pI->m_pScene->SpriteUIRender();
	}

	// �t�F�[�h�̕`��.
	FadeManager::Render();
}

//---------------------------.
// �T�u�E�B���h�E�̕`��.
//---------------------------.
void SceneManager::SubRender()
{
	SceneManager* pI = GetInstance();

	// �V�[���̕`��.
	pI->m_pScene->SubSprite3DRender_B();
	pI->m_pScene->SubModelRender();
	pI->m_pScene->SubSprite3DRender_A();
	if ( pI->m_IsUIRender == true ) {
		pI->m_pScene->SubSpriteUIRender();
	}
}

//---------------------------.
// �V�[���̕ύX.
//---------------------------.
void SceneManager::SceneChange( const ESceneList NewScene, const EFadeType FadeType, const float FadeSpeed )
{
	SceneManager* pI = GetInstance();

	if ( pI->m_IsSceneChange ) return;

	// �V�[���J�ڏ���ۑ�.
	pI->m_IsSceneChange = true;
	pI->m_ChangeScene	= NewScene;
	pI->m_NowScene		= NewScene;

	// �t�F�[�h����ۑ�.
	pI->m_FadeType		= FadeType;
	pI->m_FadeSpeed		= FadeSpeed;

	// �t�F�[�h�̐ݒ�.
	FadeManager::FadeSetting( FadeType );

	MessageWindow::PushMessage( StringConversion::Enum( NewScene ) + u8"�Ɉړ����܂���", Color4::Yellow );
}

//---------------------------.
// �V�[���̕ύX�̍X�V.
//	�V�[���̕ύX���s���Ă��Ȃ�
//	�X�V���~�߂Ă��Ȃ� �ꍇ�͏����͍s��Ȃ�.
//---------------------------.
void SceneManager::SceneChangeUpdate()
{
	SceneManager* pI = GetInstance();

	if ( pI->m_IsSceneChange == false && pI->m_UpdateStopFlag == false ) return;

	// �X�V���~������.
	pI->m_UpdateStopFlag = true;

	// �t�F�[�h�C�����s��.
	if ( pI->m_IsFadeOut == false && FadeManager::FadeIn( pI->m_FadeType, pI->m_FadeSpeed ) ) {
		// �����蔻�胊�X�g������������.
		ActorCollisionManager::Reset();

		// �V�[����ύX.
		switch ( pI->m_ChangeScene ) {
		case ESceneList::GameMain:
			SoundManager::BGMAllStop();
			pI->m_pScene = std::make_unique<CGameMain>();
			break;
		default:
			break;
		}

		// �V�����V�[���̏�����.
		pI->m_pScene->Init();
		pI->m_pScene->Update( 0.0f );
		pI->m_IsFadeOut = true;

		// �R���g���[���̐U���̒�~.
		XInput::SetVibration( 0.0f, 0.0f );

#ifdef _DEBUG
		// �萔�̏�����.
		Const::Load();
#endif
	}
	// �t�F�[�h�A�E�g���s��.
	else if ( pI->m_IsFadeOut && FadeManager::FadeOut( pI->m_FadeType, pI->m_FadeSpeed ) ) {
		pI->m_IsFadeOut			= false;
		pI->m_UpdateStopFlag	= false;
		pI->m_IsSceneChange		= false;
		pI->m_ChangeScene		= ESceneList::None;
	}
}
