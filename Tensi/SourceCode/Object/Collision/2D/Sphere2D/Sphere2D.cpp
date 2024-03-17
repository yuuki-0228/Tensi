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
// 中心の座標を取得.
//---------------------------.
D3DXPOSITION2 CSphere2D::GetPosition()
{
	// スプライトを設定されていない場合保存している中心の座標を返す.
	if ( m_pSprite == nullptr ) return m_CPosition;

	// 現在設定されているスプライトから中心の座標を取得.
	return GetSpriteCenterPos( m_pRenderState );
}

//---------------------------.
// 半径を設定.
//---------------------------.
float CSphere2D::GetRadius()
{
	// スプライトを設定されていない場合保存している半径を返す.
	if ( m_pSprite == nullptr ) return m_Radius;

	// 現在設定されているスプライトから拡縮を考慮した半径を計算して返す.
	const D3DXSCALE3& Scale = m_pRenderState->Transform.Scale;
	return m_Radius * Scale.x;
}

//---------------------------.
// スプライトを設定.
//---------------------------.
void CSphere2D::SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState )
{
	// スプライトを保存.
	m_pSprite		= pSprite;
	m_pRenderState	= pRenderState;
}

//---------------------------.
// 中心の座標を取得する.
//---------------------------.
D3DXPOSITION2 CSphere2D::GetSpriteCenterPos( SSpriteRenderState* pRenderState )
{
	const D3DXPOSITION2&	Pos			= { pRenderState->Transform.Position.x, pRenderState->Transform.Position.y };
	const SSpriteState&		SpriteState = m_pSprite->GetSpriteState();
	const SSize&			Size		= SpriteState.Disp;
	const D3DXSCALE3&		Scale		= pRenderState->Transform.Scale;

	// 左上の座標を計算して返す.
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
