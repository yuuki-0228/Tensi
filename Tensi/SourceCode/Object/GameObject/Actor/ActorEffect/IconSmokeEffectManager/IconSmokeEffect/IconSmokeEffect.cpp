#include "IconSmokeEffect.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\Utility\Const\Const.h"
#include "..\..\..\..\..\..\Utility\Color\Color.h"
#include <algorithm>

namespace {
	const int SMOKE_ANIM_MIN = 0;	// 煙のアニメーション番号の最小.
	const int SMOKE_ANIM_MAX = 3;	// 煙のアニメーション番号の最大( 4分割画像 ).
}

CIconSmokeEffect::CIconSmokeEffect()
	: m_Size( 1.0f )
{
}

CIconSmokeEffect::~CIconSmokeEffect()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CIconSmokeEffect::Init()
{
	m_pSprite								= SpriteResource::GetSprite( "IconSmoke", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber	= true;
	m_IsDisp								= false;
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CIconSmokeEffect::Update( const float& DeltaTime )
{
	if ( m_IsAnimStop		) return;
	if ( m_IsDisp == false	) return;

	// 透明にしていく.
	auto* pColor = &m_SpriteState.Color;
	pColor->w -= Const::Explore().SMOKE_ALPHA_SPEED * DeltaTime;

	// アルファ値から進行度を求める.
	const float StartAlpha = Const::Explore().SMOKE_START_ALPHA;
	float Progress = ( StartAlpha <= 0.0f ) ? 1.0f : 1.0f - ( pColor->w / StartAlpha );
	Progress = std::clamp( Progress, 0.0f, 1.0f );

	// 進行度に応じて拡大する.
	const float MinScale = Const::Explore().SMOKE_SCALE_MIN;
	const float MaxScale = Const::Explore().SMOKE_SCALE_MAX;
	const float Scale	 = ( MinScale + ( MaxScale - MinScale ) * Progress ) * m_Size;
	m_SpriteState.Transform.Scale = { Scale, Scale, Scale };

	// 透明になりきったら終了.
	if ( pColor->w <= Color::ALPHA_MIN ) {
		pColor->w		= Color::ALPHA_MIN;
		m_IsDisp		= false;
		m_IsAnimStop	= true;
		m_IsAnimEnd		= true;
	}
}

//---------------------------.
// サブウィンドウ( アイコンの後ろ )に描画.
//---------------------------.
void CIconSmokeEffect::SubRender()
{
	if ( m_IsAnimStop		) return;
	if ( m_IsDisp == false	) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// 再生.
//---------------------------.
void CIconSmokeEffect::Play( const D3DXPOSITION3& Pos, const float Size )
{
	AnimInit();
	m_Size = Size;

	// ランダムでアニメーション番号を選ぶ.
	m_SpriteState.AnimState.IsSetAnimNumber	= true;
	m_SpriteState.AnimState.AnimNumber		= Random::GetRand( SMOKE_ANIM_MIN, SMOKE_ANIM_MAX );

	// 位置・色・大きさの初期化.
	m_SpriteState.Transform.Position	= Pos;
	m_SpriteState.Color.w				= Const::Explore().SMOKE_START_ALPHA;

	const float Scale = Const::Explore().SMOKE_SCALE_MIN * m_Size;
	m_SpriteState.Transform.Scale = { Scale, Scale, Scale };
}
