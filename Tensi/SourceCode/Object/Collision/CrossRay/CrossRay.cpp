#include "CrossRay.h"
#include "..\..\..\Utility\Math\Math.h"

CCrossRay::CCrossRay()
	: m_pRay	()
	, m_Length	( 0.0f )
{
	// レイの作成.
	m_pRay.resize( static_cast<int>( ECrossRayDire::Max ) );
	for ( auto& r : m_pRay ) r = std::make_unique<CRay>();

	// レイの方向の設定.
	m_pRay[static_cast<int>( ECrossRayDire::ZF )]->SetVector( Math::_Z_VECTOR );
	m_pRay[static_cast<int>( ECrossRayDire::ZB )]->SetVector( Math::Z_VECTOR  );
	m_pRay[static_cast<int>( ECrossRayDire::XL )]->SetVector( Math::X_VECTOR );
	m_pRay[static_cast<int>( ECrossRayDire::XR )]->SetVector( Math::_X_VECTOR  );
}

CCrossRay::~CCrossRay()
{
}
