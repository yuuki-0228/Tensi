#include "GameObject.h"

CGameObject::CGameObject()
	: m_DeltaTime	( INIT_FLOAT )
	, m_ObjectTag	( EObjectTag::None )
{
}

CGameObject::~CGameObject()
{
}