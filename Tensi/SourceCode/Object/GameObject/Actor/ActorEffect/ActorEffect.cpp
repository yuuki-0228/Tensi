#include "ActorEffect.h"

CActorEffect::CActorEffect()
	: m_pSprite			( nullptr )
	, m_SpriteState		()
	, m_IsAnimStop		( false )
	, m_IsAnimEnd		( false )
	, m_NowAnimPoint	( INIT_FLOAT )
{
}

CActorEffect::~CActorEffect()
{
}

//---------------------------.
// アニメーションの表示の初期化
//---------------------------.
void CActorEffect::AnimInit()
{
	m_NowAnimPoint	= INIT_FLOAT;
	m_IsDisp		= true;
	m_IsAnimEnd		= false;
	m_IsAnimStop	= false;
}