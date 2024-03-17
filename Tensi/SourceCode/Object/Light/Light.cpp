#include "Light.h"
#include "..\..\Utility\ImGuiManager\DebugWindow\DebugWindow.h"
#include "..\..\Utility\Math\Math.h"

Light::Light()
	: m_Direction		( 0.0, 1.0f, 0.0f )
	, m_Intensity		( 1.0f )
{
}

Light::~Light()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
Light* Light::GetInstance()
{
	static std::unique_ptr<Light> pInstance = std::make_unique<Light>();
	return pInstance.get();
}

//---------------------------.
// 更新.
//---------------------------.
void Light::Update()
{
#ifdef _DEBUG
//	DebugWindow::PushProc( "Light", [&]() {
//		D3DXVECTOR3	Dir = Light::GetDirection();
//		float		Int = Light::GetIntensity();
//
//		ImGuiManager::SliderVec3(  "Direction", &Dir, Math::RADIAN_MIN, Math::RADIAN_MAX );
//		ImGuiManager::SliderFloat( "Intensity", &Int, 0.0f, 10.0f );
//
//		Light::SetDirection( Dir );
//		Light::SetIntensity( Int );
//		} );
#endif	// #ifdef _DEBUG.
}
