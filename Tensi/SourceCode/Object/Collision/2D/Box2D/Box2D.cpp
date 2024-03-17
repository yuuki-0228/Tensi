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
// 左上の座標を取得.
//---------------------------.
D3DXPOSITION2 CBox2D::GetPosition()
{
	// スプライトを設定されていない場合保存している左上の座標を返す.
	if ( m_pSprite == nullptr ) return m_LUPosition;

	// 現在設定されているスプライトから左上の座標を取得.
	return GetSpriteUpperLeftPos( m_pRenderState );
}

//---------------------------.
// 幅高さを設定.
//---------------------------.
SSize CBox2D::GetSize()
{
	// スプライトを設定されていない場合保存している幅高さを返す.
	if ( m_pSprite == nullptr ) return m_Size;

	// 現在設定されているスプライトから拡縮を考慮したサイズを計算して返す.
	const D3DXSCALE3& Scale = m_pRenderState->Transform.Scale;
	return SSize( m_Size.w * Scale.x, m_Size.h * Scale.y );
}

//---------------------------.
// スプライトを設定.
//---------------------------.
void CBox2D::SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState )
{
	// スプライトを保存.
	m_pSprite		= pSprite;
	m_pRenderState	= pRenderState;
	
	// 座標の取得.
	const D3DXPOSITION3& Pos = pSprite->GetSpriteUpperLeftPos( pRenderState );
	m_LUPosition	= { Pos.x, Pos.y };

	// 幅高さを取得.
	m_Size			= pSprite->GetSpriteState().Disp;
}

//---------------------------.
// 左上の座標を取得する.
//---------------------------.
D3DXPOSITION2 CBox2D::GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState )
{
	const D3DXPOSITION2&	Pos			= { pRenderState->Transform.Position.x, pRenderState->Transform.Position.y };
	const SSize&			Size		= m_Size;
	const D3DXSCALE3&		Scale		= pRenderState->Transform.Scale;

	// 左上の座標を計算して返す.
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
