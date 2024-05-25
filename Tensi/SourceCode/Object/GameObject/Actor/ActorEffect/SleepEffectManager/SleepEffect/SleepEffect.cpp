#include "SleepEffect.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"

namespace {
	const		D3DXVECTOR3 MOVE_VECTOR = { 1.0f, -1.0f, 0.0f };	// 移動ベクトル.
	constexpr	float		MOVE_SPEED	= 64.0f;					// 移動速度.
	constexpr	float		END_SCALE	= 1.5f;						// 拡縮の終了.
}

CSleepEffect::CSleepEffect()
{
}

CSleepEffect::~CSleepEffect()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CSleepEffect::Init()
{
	// 画像の取得.
	m_pSprite = SpriteResource::GetSprite( "SleepEffect", &m_SpriteState );

	// 画像の設定.
	m_SpriteState.AnimState.IsSetAnimNumber = true;

	m_IsDisp = false;
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CSleepEffect::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsAnimStop		) return;
	if ( m_IsDisp == false	) return;

	// 現在のアニメーションの位置の更新.
	m_NowAnimPoint += 0.35f * DeltaTime;

	// アニメーションが終了したか.
	if ( m_NowAnimPoint >= 1.0f ) {
		m_NowAnimPoint	= 1.0f;
		m_IsDisp		= false;
		m_IsAnimEnd		= true;
	}

	// 移動.
	m_SpriteState.Transform.Position += MOVE_VECTOR * MOVE_SPEED * DeltaTime;

	// 拡縮の設定.
	const float Scale = END_SCALE * m_NowAnimPoint;
	m_SpriteState.Transform.Scale = { Scale, Scale, Scale };

	// 透明度の設定.
	m_SpriteState.Color.w = Color::ALPHA_MAX - m_NowAnimPoint;
}

//---------------------------.
// 描画.
//---------------------------.
void CSleepEffect::Render()
{
	if ( m_IsAnimEnd		) return;
	if ( m_IsDisp == false	) return;

	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// アニメーションの開始.
//---------------------------.
void CSleepEffect::Play( const D3DXPOSITION3& Pos, const float Size )
{
	if ( m_IsDisp ) return;

	// 再生位置の設定.
	D3DXPOSITION3 PlayPos = Pos;
	PlayPos.x += Random::GetRand( -32.0f,  32.0f );
	PlayPos.y += Random::GetRand( -48.0f, -16.0f );

	// 初期化.
	AnimInit();
	m_SpriteState.AnimState.AnimNumber	= Random::GetRand( 0, m_SpriteState.AnimState.AnimNumberMax );
	m_SpriteState.Transform.Position	= PlayPos;
	m_SpriteState.Transform.Scale		= STransform::SCALE_MIN_VEC3;
	m_SpriteState.Color.w				= Color::ALPHA_MAX;
}
