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
// 初期化.
//---------------------------.
bool CUIEdit::Init()
{
	// UIエディタの初期化.
	m_pUIEditor	= std::make_unique<CUIEditor>();
	m_pUIEditor->Init();

	// カメラの設定.
	m_pCameras = std::make_unique<CCameras>();
	m_pCameras->InitCamera( ECameraType::NormalCamera );
	CameraManager::SetCamera( m_pCameras->GetCamera<CNormalCamera>() );

	// BGMの設定.
	m_BGMName = "12-13";
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CUIEdit::Update( const float& DeltaTime )
{
	// UIエディタの更新.
	m_pUIEditor->Update( DeltaTime );

	// BGMの再生.
	SoundManager::PlayBGM( m_BGMName );

	// シーン移行.
	if ( Input::IsKeyDown( "Back" ) ) {
		SoundManager::PlaySE( "Decision" );
		SceneManager::SceneChange( ESceneList::GameMain, EFadeType::MaskFade );
	}
}

//---------------------------.
// スプライト(UI)の描画.
//---------------------------.
void CUIEdit::SpriteUIRender()
{
	m_pUIEditor->ImGuiRender();
	m_pUIEditor->UIRender();
}
