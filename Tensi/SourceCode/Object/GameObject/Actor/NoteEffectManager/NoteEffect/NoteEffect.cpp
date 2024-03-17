#include "NoteEffect.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Random\Random.h"

CNoteEffect::CNoteEffect()
	: m_pSprite			( nullptr )
	, m_SpriteState		()
	, m_NowPoint		( INIT_FLOAT )
{
}

CNoteEffect::~CNoteEffect()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CNoteEffect::Init()
{
	// �摜�̎擾.
	m_pSprite = SpriteResource::GetSprite( "NoteEffect", &m_SpriteState );

	// �摜�̐ݒ�.
	m_SpriteState.AnimState.IsSetAnimNumber = true;

	m_IsDisp = false;
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CNoteEffect::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// ���݂̃A�j���[�V�����̈ʒu�̍X�V.
	m_NowPoint += 180.0f * DeltaTime;

	// �A�j���[�V�������I��������.
	if ( m_NowPoint >= 180.0f ) {
		m_NowPoint	= 180.0f;
		m_IsDisp	= false;
	}

	// ������h�炷.
	m_SpriteState.Transform.Rotation.z = Math::ToRadian( 20.0f ) * sinf( Math::ToRadian( m_NowPoint * 2.0f ) );

	// �����x�̐ݒ�.
	m_SpriteState.Color.w = sinf( Math::ToRadian( m_NowPoint ) );
}

//---------------------------.
// �`��.
//---------------------------.
void CNoteEffect::Render()
{
	if ( m_IsDisp == false ) return;

	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �A�j���[�V�����̊J�n.
//---------------------------.
void CNoteEffect::Play( const D3DXPOSITION3& Pos, const float Size )
{
	if ( m_IsDisp ) return;

	// �Đ��ʒu�̐ݒ�.
	D3DXPOSITION3 PlayPos = Pos;
	PlayPos.x -= Random::GetRand( -Size, Size );
	PlayPos.y -= Random::GetRand(  0.0f, Size );

	// ������.
	m_NowPoint							= INIT_FLOAT;
	m_IsDisp							= true;
	m_SpriteState.AnimState.AnimNumber	= Random::GetRand( 0, m_SpriteState.AnimState.AnimNumberMax );
	m_SpriteState.Transform.Position	= PlayPos;
	m_SpriteState.Transform.Rotation	= INIT_FLOAT3;
	m_SpriteState.Color.w				= Color::ALPHA_MIN;
}
