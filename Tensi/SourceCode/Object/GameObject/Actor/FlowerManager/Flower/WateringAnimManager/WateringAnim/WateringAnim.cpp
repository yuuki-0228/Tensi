#include "WateringAnim.h"
#include "..\..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\..\Utility\Const\Const.h"

CWateringAnim::CWateringAnim()
	: m_pSprite		( nullptr )
	, m_SpriteState	()
	, m_IsDispAnim	( false )
{
}

CWateringAnim::~CWateringAnim()
{
}

//---------------------------.
// èâä˙âª.
//---------------------------.
bool CWateringAnim::Init()
{
	m_pSprite = SpriteResource::GetSprite( "WateringAnim", &m_SpriteState );
	m_IsDisp = false;
	return true;
}

//---------------------------.
// çXêV.
//---------------------------.
void CWateringAnim::Update( const float& DeltaTime )
{
	if ( m_IsDisp == false ) return;

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
			m_IsDisp = false;
		}
	}
}

//---------------------------.
// ï`âÊ.
//---------------------------.
void CWateringAnim::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// çƒê∂.
//---------------------------.
void CWateringAnim::Play( const D3DXPOSITION3& Pos )
{
	m_SpriteState.Transform.Position = Pos;
	m_IsDisp						 = true;
	m_IsDispAnim					 = true;
	m_SpriteState.Color.w			 = Color::ALPHA_MIN;

	const auto s = Random::GetRand( STransform::NORMAL_SCALE / 2.0f, STransform::NORMAL_SCALE );
	m_SpriteState.Transform.Scale = { s, s, s };
}
