#include "SleepEffect.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Random\Random.h"

namespace {
	const		D3DXVECTOR3 MOVE_VECTOR = { 1.0f, -1.0f, 0.0f };	// �ړ��x�N�g��.
	constexpr	float		MOVE_SPEED	= 64.0f;					// �ړ����x.
	constexpr	float		END_SCALE	= 1.5f;						// �g�k�̏I��.
}

CSleepEffect::CSleepEffect()
	: m_pSprite			( nullptr )
	, m_SpriteState		()
	, m_NowPoint		( INIT_FLOAT )
{
}

CSleepEffect::~CSleepEffect()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CSleepEffect::Init()
{
	// �摜�̎擾.
	m_pSprite = SpriteResource::GetSprite( "SleepEffect", &m_SpriteState );

	// �摜�̐ݒ�.
	m_SpriteState.AnimState.IsSetAnimNumber = true;

	m_IsDisp = false;
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CSleepEffect::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// ���݂̃A�j���[�V�����̈ʒu�̍X�V.
	m_NowPoint += 0.35f * DeltaTime;

	// �A�j���[�V�������I��������.
	if ( m_NowPoint >= 1.0f ) {
		m_NowPoint	= 1.0f;
		m_IsDisp	= false;
	}

	// �ړ�.
	m_SpriteState.Transform.Position += MOVE_VECTOR * MOVE_SPEED * DeltaTime;

	// �g�k�̐ݒ�.
	const float Scale = END_SCALE * m_NowPoint;
	m_SpriteState.Transform.Scale = { Scale, Scale, Scale };

	// �����x�̐ݒ�.
	m_SpriteState.Color.w = Color::ALPHA_MAX - m_NowPoint;
}

//---------------------------.
// �`��.
//---------------------------.
void CSleepEffect::Render()
{
	if ( m_IsDisp == false ) return;

	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �A�j���[�V�����̊J�n.
//---------------------------.
void CSleepEffect::Play( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	// �Đ��ʒu�̐ݒ�.
	D3DXPOSITION3 PlayPos = Pos;
	PlayPos.x += Random::GetRand( -32.0f,  32.0f );
	PlayPos.y += Random::GetRand( -48.0f, -16.0f );

	// ������.
	m_NowPoint							= INIT_FLOAT;
	m_IsDisp							= true;
	m_SpriteState.AnimState.AnimNumber	= Random::GetRand( 0, m_SpriteState.AnimState.AnimNumberMax );
	m_SpriteState.Transform.Position	= PlayPos;
	m_SpriteState.Transform.Scale		= STransform::SCALE_MIN_VEC3;
	m_SpriteState.Color.w				= Color::ALPHA_MAX;
}
