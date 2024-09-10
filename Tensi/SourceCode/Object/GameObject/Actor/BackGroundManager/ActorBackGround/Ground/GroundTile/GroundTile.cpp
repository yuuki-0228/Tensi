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
// 初期化.
//---------------------------.
bool CGroundTile::Init()
{
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CGroundTile::Update( const float& DeltaTime )
{
}

//---------------------------.
// 描画.
//---------------------------.
void CGroundTile::SubRender()
{
}

//---------------------------.
// 地面の設置
//---------------------------.
void CGroundTile::Setting( const D3DXPOSITION3& Pos )
{
}
