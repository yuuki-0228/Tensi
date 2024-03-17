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
// 初期化.
//---------------------------.
bool CNoteEffect::Init()
{
	// 画像の取得.
	m_pSprite = SpriteResource::GetSprite( "NoteEffect", &m_SpriteState );

	// 画像の設定.
	m_SpriteState.AnimState.IsSetAnimNumber = true;

	m_IsDisp = false;
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CNoteEffect::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// 現在のアニメーションの位置の更新.
	m_NowPoint += 180.0f * DeltaTime;

	// アニメーションが終了したか.
	if ( m_NowPoint >= 180.0f ) {
		m_NowPoint	= 180.0f;
		m_IsDisp	= false;
	}

	// 音符を揺らす.
	m_SpriteState.Transform.Rotation.z = Math::ToRadian( 20.0f ) * sinf( Math::ToRadian( m_NowPoint * 2.0f ) );

	// 透明度の設定.
	m_SpriteState.Color.w = sinf( Math::ToRadian( m_NowPoint ) );
}

//---------------------------.
// 描画.
//---------------------------.
void CNoteEffect::Render()
{
	if ( m_IsDisp == false ) return;

	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// アニメーションの開始.
//---------------------------.
void CNoteEffect::Play( const D3DXPOSITION3& Pos, const float Size )
{
	if ( m_IsDisp ) return;

	// 再生位置の設定.
	D3DXPOSITION3 PlayPos = Pos;
	PlayPos.x -= Random::GetRand( -Size, Size );
	PlayPos.y -= Random::GetRand(  0.0f, Size );

	// 初期化.
	m_NowPoint							= INIT_FLOAT;
	m_IsDisp							= true;
	m_SpriteState.AnimState.AnimNumber	= Random::GetRand( 0, m_SpriteState.AnimState.AnimNumberMax );
	m_SpriteState.Transform.Position	= PlayPos;
	m_SpriteState.Transform.Rotation	= INIT_FLOAT3;
	m_SpriteState.Color.w				= Color::ALPHA_MIN;
}
