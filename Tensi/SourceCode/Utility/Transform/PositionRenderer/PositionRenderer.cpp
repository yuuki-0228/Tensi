#include "PositionRenderer.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Common\Mesh\StaticMesh\StaticMesh.h"
#include "..\..\..\Resource\MeshResource\MeshResource.h"
#include "..\..\..\Utility\Input\Input.h"

PositionRenderer::PositionRenderer()
	: m_pMesh		( nullptr )
	, m_Transform	()
#ifdef ENABLE_CLASS_BOOL
	, m_IsRender	( false, u8"座標の表示/非表示", u8"System" )
#else
	, m_IsRender	( false )
#endif
{
}

PositionRenderer::~PositionRenderer()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
PositionRenderer* PositionRenderer::GetInstance()
{
	static std::unique_ptr<PositionRenderer> pInstance = std::make_unique<PositionRenderer>();
	return pInstance.get();
}

//---------------------------.
// 更新.
//---------------------------.
void PositionRenderer::Update( const float& DeltaTime )
{
	PositionRenderer* pI = GetInstance();

	// 表示の切り替え.
	if ( DebugKeyInput::IsANDKeyDown( VK_CONTROL, VK_F11 ) ) {
		pI->m_IsRender = !pI->m_IsRender;
	}
}

//---------------------------.
// 描画.
//---------------------------.
void PositionRenderer::Render( const STransform& Transform )
{
#ifdef _DEBUG
	if ( Transform.IsPostionRender == false ) return;

	PositionRenderer* pI = GetInstance();

	// 表示するか.
	if ( pI->m_IsRender == false ) return;

	// モデルの取得.
	if ( pI->m_pMesh == nullptr ) pI->m_pMesh = MeshResource::GetStatic( "XYZModel" );

	// 一番大きいサイズを取得.
	float Scale = Transform.Scale.x;
	if ( Scale < Transform.Scale.y ) Scale = Transform.Scale.y;
	if ( Scale < Transform.Scale.z ) Scale = Transform.Scale.z;

	// トランスフォームの更新.
	pI->m_Transform.Position		= Transform.Position;
	pI->m_Transform.Scale			= { Scale, Scale, Scale };
	pI->m_Transform.IsPostionRender	= false;

	// トランスフォームの描画.
	pI->m_pMesh->Render( &pI->m_Transform );
#endif // #ifdef _DEBUG.
}
