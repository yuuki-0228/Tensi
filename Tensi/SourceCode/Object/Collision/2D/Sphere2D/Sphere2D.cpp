#include "Sphere2D.h"

CSphere2D::CSphere2D()
	: m_pSprite			( nullptr )
	, m_pRenderState	( nullptr )
	, m_CPosition		( INIT_FLOAT2 )
	, m_Radius			( INIT_FLOAT )
{
}

CSphere2D::~CSphere2D()
{
}

//---------------------------.
// ���S�̍��W���擾.
//---------------------------.
D3DXPOSITION2 CSphere2D::GetPosition()
{
	// �X�v���C�g��ݒ肳��Ă��Ȃ��ꍇ�ۑ����Ă��钆�S�̍��W��Ԃ�.
	if ( m_pSprite == nullptr ) return m_CPosition;

	// ���ݐݒ肳��Ă���X�v���C�g���璆�S�̍��W���擾.
	return GetSpriteCenterPos( m_pRenderState );
}

//---------------------------.
// ���a��ݒ�.
//---------------------------.
float CSphere2D::GetRadius()
{
	// �X�v���C�g��ݒ肳��Ă��Ȃ��ꍇ�ۑ����Ă��锼�a��Ԃ�.
	if ( m_pSprite == nullptr ) return m_Radius;

	// ���ݐݒ肳��Ă���X�v���C�g����g�k���l���������a���v�Z���ĕԂ�.
	const D3DXSCALE3& Scale = m_pRenderState->Transform.Scale;
	return m_Radius * Scale.x;
}

//---------------------------.
// �X�v���C�g��ݒ�.
//---------------------------.
void CSphere2D::SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState )
{
	// �X�v���C�g��ۑ�.
	m_pSprite		= pSprite;
	m_pRenderState	= pRenderState;
}

//---------------------------.
// ���S�̍��W���擾����.
//---------------------------.
D3DXPOSITION2 CSphere2D::GetSpriteCenterPos( SSpriteRenderState* pRenderState )
{
	const D3DXPOSITION2&	Pos			= { pRenderState->Transform.Position.x, pRenderState->Transform.Position.y };
	const SSpriteState&		SpriteState = m_pSprite->GetSpriteState();
	const SSize&			Size		= SpriteState.Disp;
	const D3DXSCALE3&		Scale		= pRenderState->Transform.Scale;

	// ����̍��W���v�Z���ĕԂ�.
	switch ( SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:	return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x,	Pos.y + ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::Left:		return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x,	Pos.y							 );
	case ELocalPosition::LeftDown:	return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x,	Pos.y - ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::Down:		return D3DXPOSITION2( Pos.x,							Pos.y - ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::RightDown:	return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x, Pos.y - ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::Right:		return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x, Pos.y							 );
	case ELocalPosition::RightUp:	return D3DXPOSITION2( Pos.x + ( Size.w / 2 ) * Scale.x, Pos.y + ( Size.h / 2 ) * Scale.y );
	case ELocalPosition::Up:		return D3DXPOSITION2( Pos.x,							Pos.y + ( Size.h / 2 ) * Scale.y );
	default:						return Pos;
	}
}
