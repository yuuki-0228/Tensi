#include "CCollisions.h"

CCollisions::CCollisions()
	: m_pSphere		( nullptr )
	, m_pRay		( nullptr )
	, m_pCrossRay	( nullptr )
	, m_pMesh		( nullptr )
	, m_pBox2D		( nullptr )
	, m_pSphere2D	( nullptr )
{
}

CCollisions::~CCollisions()
{
}

//---------------------------.
// éwíËÇµÇΩìñÇΩÇËîªíËÇÃèâä˙âª.
//---------------------------.
void CCollisions::InitCollision( const ECollType& Type )
{
	switch ( Type ) {
	case ECollType::Sphere:
		m_pSphere	= std::make_shared<CSphere>();
		break;
	case ECollType::Cylinder:
		m_pCylinder = std::make_shared<CCylinder>();
		break;
	case ECollType::Ray:
		m_pRay		= std::make_shared<CRay>();
		break;
	case ECollType::CrossRay:
		m_pCrossRay = std::make_shared<CCrossRay>();
		break;
	case ECollType::Mesh:
		m_pMesh		= std::make_shared<CMesh>();
		break;
	case ECollType::Box2D:
		m_pBox2D	= std::make_shared<CBox2D>();
		break;
	case ECollType::Sphere2D:
		m_pSphere2D	= std::make_shared<CSphere2D>();
		break;
	default:
		break;
	}
}
