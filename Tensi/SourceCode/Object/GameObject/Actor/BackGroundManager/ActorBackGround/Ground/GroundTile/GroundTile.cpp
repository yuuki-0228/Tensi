#include "GroundTile.h"

CGroundTile::CGroundTile()
	: m_pGround		( nullptr )
	, m_GroundState	()
{
}

CGroundTile::~CGroundTile()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CGroundTile::Init()
{
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CGroundTile::Update( const float& DeltaTime )
{
}

//---------------------------.
// �`��.
//---------------------------.
void CGroundTile::SubRender()
{
}

//---------------------------.
// �n�ʂ̐ݒu
//---------------------------.
void CGroundTile::Setting( const D3DXPOSITION3& Pos )
{
}
