#include "Cameras.h"

CCameras::CCameras()
	: m_pNormalCamera			( nullptr )
	, m_pFreeCamera				( nullptr )
	, m_pMoveCamera				( nullptr )
	, m_pThirdPersonCamera		( nullptr )
{
}

CCameras::~CCameras()
{
}

//---------------------------.
// w’è‚µ‚½“–‚½‚è”»’è‚Ì‰Šú‰».
//---------------------------.
void CCameras::InitCamera( const ECameraType& Type )
{
	switch ( Type ) {
	case ECameraType::NormalCamera:
		m_pNormalCamera			= std::make_unique<CNormalCamera>();
		break;
	case ECameraType::FreeCamera:
		m_pFreeCamera			= std::make_unique<CFreeCamera>();
		break;
	case ECameraType::MoveCamera:
		m_pMoveCamera			= std::make_unique<CMoveCamera>();
		break;
	case ECameraType::ThirdPersonCamera:
		m_pThirdPersonCamera	= std::make_unique<CThirdPersonCamera>();
		break;
	default:
		break;
	}
}
