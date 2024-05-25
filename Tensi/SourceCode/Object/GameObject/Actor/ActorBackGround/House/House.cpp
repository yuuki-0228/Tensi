#include "House.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Input\Input.h"

CHouse::CHouse()
	: m_pTent		( nullptr )
	, m_TentState	()
{
}

CHouse::~CHouse()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CHouse::Init()
{
	// �摜�̎擾.
	m_pTent = SpriteResource::GetSprite( "Tent", &m_TentState );
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CHouse::Update( const float& DeltaTime )
{
	// �}�E�X�̈ʒu�̎擾.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// ����̍��W���擾.
	const D3DXPOSITION3& Pos = m_pTent->GetSpriteUpperLeftPos( &m_TentState );

	// �T�C�Y�̎擾.
	const SSize& Size = m_pTent->GetSpriteState().Disp;

	// �}�E�X���摜�̏�ɂ��邩.
	if ( Pos.x <= MousePos.x && MousePos.x <= Pos.x + Size.w &&
		 Pos.y <= MousePos.y && MousePos.y <= Pos.y + Size.h )
	{
		// �_�u���N���b�N���ꂽ��.
		if ( Input::GetIsLeftDoubleClick() == false ) return;

		// �G�N�X�v���[���[���J��.
		std::filesystem::path result = std::filesystem::absolute( "Data\\�X���C���̂���" );
		std::string command = "explorer.exe " + result.string();
		system( command.c_str() );

		// �_�u���N���b�N���I��.
		Input::SetIsLeftDoubleClick( false );
	}
}

//---------------------------.
// �`��.
//---------------------------.
void CHouse::SubRender()
{
	m_pTent->RenderUI( &m_TentState );
}

//---------------------------.
// �ݒu����.
//---------------------------.
void CHouse::Setting( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	m_IsDisp = true;

	// �^�X�N�o�[�̃T�C�Y���擾.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// �e���g�̈ʒu�̐ݒ�.
	m_TentState.Transform.Position		= Pos;
	m_TentState.Transform.Position.y	= static_cast<float>( Rect.top );
}
