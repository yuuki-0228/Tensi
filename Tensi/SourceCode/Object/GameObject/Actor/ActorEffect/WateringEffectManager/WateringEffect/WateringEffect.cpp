#include "WateringEffect.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\Utility\Const\Const.h"

CWateringEffect::CWateringEffect()
	: m_IsDispAnim	( false )
{
}

CWateringEffect::~CWateringEffect()
{
}

//---------------------------.
// èâä˙âª.
//---------------------------.
bool CWateringEffect::Init()
{
	m_pSprite	= SpriteResource::GetSprite( "WateringAnim", &m_SpriteState );
	m_IsDisp	= false;
	return true;
}

//---------------------------.
// çXêV.
//---------------------------.
void CWateringEffect::Update( const float& DeltaTime )
{
	if ( m_IsAnimStop		) return;
	if ( m_IsDisp == false	) return;

	auto* pColor = &m_SpriteState.Color;
	if ( m_IsDispAnim ) {
		pColor->w += Const::Flower.WATERING_ANIM_EFFECT_ALPHA_SPEED * DeltaTime;
		if ( pColor->w >= Color::ALPHA_MAX ) {
			pColor->w	 = Color::ALPHA_MAX;
			m_IsDispAnim = false;
		}
	}
	else {
		pColor->w -= Const::Flower.WATERING_ANIM_EFFECT_ALPHA_SPEED * DeltaTime;
		if ( pColor->w <= Color::ALPHA_MIN ) {
			pColor->w = Color::ALPHA_MIN;
			m_IsDisp		= false;
			m_IsAnimStop	= true;
		}
	}
}

//---------------------------.
// ï`âÊ.
//---------------------------.
void CWateringEffect::Render()
{
	if ( m_IsAnimStop		) return;
	if ( m_IsDisp == false	) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// çƒê∂.
//---------------------------.
void CWateringEffect::Play( const D3DXPOSITION3& Pos, const float Size )
{
	AnimInit();
	m_IsDispAnim					 = true;
	m_SpriteState.Transform.Position = Pos;
	m_SpriteState.Color.w			 = Color::ALPHA_MIN;

	const auto s = Random::GetRand( STransform::NORMAL_SCALE / 2.0f, STransform::NORMAL_SCALE );
	m_SpriteState.Transform.Scale = { s, s, s };
}
