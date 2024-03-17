#include "Box2D.h"

CBox2D::CBox2D()
	: m_pSprite			( nullptr )
	, m_pRenderState	( nullptr )
	, m_LUPosition		( INIT_FLOAT2 )
	, m_Size			( INIT_FLOAT2 )
{
}

CBox2D::~CBox2D()
{
}

//---------------------------.
// ����̍��W���擾.
//---------------------------.
D3DXPOSITION2 CBox2D::GetPosition()
{
	// �X�v���C�g��ݒ肳��Ă��Ȃ��ꍇ�ۑ����Ă��鍶��̍��W��Ԃ�.
	if ( m_pSprite == nullptr ) return m_LUPosition;

	// ���ݐݒ肳��Ă���X�v���C�g���獶��̍��W���擾.
	return GetSpriteUpperLeftPos( m_pRenderState );
}

//---------------------------.
// ��������ݒ�.
//---------------------------.
SSize CBox2D::GetSize()
{
	// �X�v���C�g��ݒ肳��Ă��Ȃ��ꍇ�ۑ����Ă��镝������Ԃ�.
	if ( m_pSprite == nullptr ) return m_Size;

	// ���ݐݒ肳��Ă���X�v���C�g����g�k���l�������T�C�Y���v�Z���ĕԂ�.
	const D3DXSCALE3& Scale = m_pRenderState->Transform.Scale;
	return SSize( m_Size.w * Scale.x, m_Size.h * Scale.y );
}

//---------------------------.
// �X�v���C�g��ݒ�.
//---------------------------.
void CBox2D::SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState )
{
	// �X�v���C�g��ۑ�.
	m_pSprite		= pSprite;
	m_pRenderState	= pRenderState;
	
	// ���W�̎擾.
	const D3DXPOSITION3& Pos = pSprite->GetSpriteUpperLeftPos( pRenderState );
	m_LUPosition	= { Pos.x, Pos.y };

	// ���������擾.
	m_Size			= pSprite->GetSpriteState().Disp;
}

//---------------------------.
// ����̍��W���擾����.
//---------------------------.
D3DXPOSITION2 CBox2D::GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState )
{
	const D3DXPOSITION2&	Pos			= { pRenderState->Transform.Position.x, pRenderState->Transform.Position.y };
	const SSize&			Size		= m_Size;
	const D3DXSCALE3&		Scale		= pRenderState->Transform.Scale;

	// ����̍��W���v�Z���ĕԂ�.
	switch ( m_pSprite->GetSpriteState().LocalPosNo ) {
	case ELocalPosition::Left:		return D3DXPOSITION2( Pos.x,							Pos.y - ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::LeftDown:	return D3DXPOSITION2( Pos.x,							Pos.y - Size.h * Scale.y		 );
	case ELocalPosition::Down:		return D3DXPOSITION2( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - Size.h * Scale.y		 );
	case ELocalPosition::RightDown:	return D3DXPOSITION2( Pos.x - Size.w * Scale.x,			Pos.y - Size.h * Scale.y		 );
	case ELocalPosition::Right:		return D3DXPOSITION2( Pos.x - Size.w * Scale.x,			Pos.y - ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::RightUp:	return D3DXPOSITION2( Pos.x - Size.w * Scale.x,			Pos.y							 );
	case ELocalPosition::Up:		return D3DXPOSITION2( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y							 );
	case ELocalPosition::Center:	return D3DXPOSITION2( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - ( Size.h / 2 ) * Scale.y );
	default:						return Pos;
	}
}
