#include "UIEdit.h"
#include "..\..\Common\Sprite\Sprite.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Editor\UIEditor\UIEditor.h"
#include "..\..\Common\DirectX\DirectX11.h"

CUIEdit::CUIEdit()
	: m_pUIEditor		( nullptr )
{
}

CUIEdit::~CUIEdit()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CUIEdit::Init()
{
	// UI�G�f�B�^�̏�����.
	m_pUIEditor	= std::make_unique<CUIEditor>();
	m_pUIEditor->Init();

	// �J�����̐ݒ�.
	m_pCameras = std::make_unique<CCameras>();
	m_pCameras->InitCamera( ECameraType::NormalCamera );
	CameraManager::SetCamera( m_pCameras->GetCamera<CNormalCamera>() );

	// BGM�̐ݒ�.
	m_BGMName = "12-13";
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CUIEdit::Update( const float& DeltaTime )
{
	// UI�G�f�B�^�̍X�V.
	m_pUIEditor->Update( DeltaTime );

	// BGM�̍Đ�.
	SoundManager::PlayBGM( m_BGMName );

	// �V�[���ڍs.
	if ( Input::IsKeyDown( "Back" ) ) {
		SoundManager::PlaySE( "Decision" );
		SceneManager::SceneChange( ESceneList::GameMain, EFadeType::MaskFade );
	}
}

//---------------------------.
// �X�v���C�g(UI)�̕`��.
//---------------------------.
void CUIEdit::SpriteUIRender()
{
	m_pUIEditor->ImGuiRender();
	m_pUIEditor->UIRender();
}
