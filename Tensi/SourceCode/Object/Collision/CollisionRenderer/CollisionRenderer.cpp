#include "CollisionRenderer.h"
#include "..\Sphere\Sphere.h"
#include "..\Cylinder\Cylinder.h"
#include "..\Ray\Ray.h"
#include "..\CrossRay\CrossRay.h"
#include "..\2D\Box2D\Box2D.h"
#include "..\2D\Sphere2D\Sphere2D.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Common\Mesh\RayMesh\RayMesh.h"
#include "..\..\..\Common\Mesh\StaticMesh\StaticMesh.h"
#include "..\..\..\Common\Mesh\CollisionMesh\CollisionMesh.h"
#include "..\..\..\Resource\MeshResource\MeshResource.h"
#include "..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\Utility\Input\Input.h"

CollisionRenderer::CollisionRenderer()
	: m_pPointQueue			()
	, m_pSphereQueue		()
	, m_pRayQueue			()
	, m_pMeshQueue			()
	, m_pPoint2DQueue		()
	, m_pBox2DQueue			()
	, m_pSphere2DQueue		()
	, m_pSphereMesh			( nullptr )
	, m_pCylinderMesh		( nullptr )
	, m_pRayMesh			( nullptr )
	, m_pBox2DSprite		( nullptr )
	, m_pSphere2DSprite		( nullptr )
	, m_pPoint2DSprite		( nullptr )
	, m_Box2DState			()
	, m_Sphere2DState		()
	, m_Point2DState		()
	, m_IsPointRender		( false, u8"当たり判定(     点     )を描画するか", u8"Collision" )
	, m_IsSphereRender		( false, u8"当たり判定(    球体    )を描画するか", u8"Collision" )
	, m_IsCylinderRender	( false, u8"当たり判定(    円柱    )を描画するか", u8"Collision" )
	, m_IsRayRender			( false, u8"当たり判定(    レイ    )を描画するか", u8"Collision" )
	, m_IsMeshRender		( false, u8"当たり判定(  メッシュ  )を描画するか", u8"Collision" )
	, m_IsPoint2DRender		( false, u8"当たり判定(    点2D    )を描画するか", u8"Collision" )
	, m_IsBox2DRender		( false, u8"当たり判定( ボックス2D )を描画するか", u8"Collision" )
	, m_IsSphere2DRender	( false, u8"当たり判定(    円2D    )を描画するか", u8"Collision" )
{
}

CollisionRenderer::~CollisionRenderer()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
CollisionRenderer* CollisionRenderer::GetInstance()
{
	static std::unique_ptr<CollisionRenderer> pInstance = std::make_unique<CollisionRenderer>();
	return pInstance.get();
}

//---------------------------.
// 描画.
//	Releaseの場合は処理は行わない.
//---------------------------.
void CollisionRenderer::Render()
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	// 当たり判定を表示するか.
	if ( DebugKeyInput::IsKeyPress( VK_CONTROL ) ) {
		if ( DebugKeyInput::IsKeyDown( VK_F1 ) ) pI->m_IsPointRender.Inversion();		// 点の表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F2 ) ) pI->m_IsSphereRender.Inversion();		// 球体の表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F3 ) ) pI->m_IsCylinderRender.Inversion();	// 円柱の表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F4 ) ) pI->m_IsRayRender.Inversion();			// レイの表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F5 ) ) pI->m_IsMeshRender.Inversion();		// メッシュの表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F6 ) ) pI->m_IsPoint2DRender.Inversion();		// 点2Dの表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F7 ) ) pI->m_IsBox2DRender.Inversion();		// ボックス2Dの表示/非表示.
		if ( DebugKeyInput::IsKeyDown( VK_F8 ) ) pI->m_IsSphere2DRender.Inversion();	// 円2Dの表示/非表示.
	}

	PointRender();		// 点の描画.
	SphereRender();		// 球体の描画.
	CylinderRender();	// 円柱の描画.
	RayRender();		// レイの描画.
	MeshRender();		// メッシュの描画.
	Point2DRender();	// 点2Dの描画.
	Box2DRender();		// ボックス2Dの描画.
	Sphere2DRender();	// 円2Dの描画.
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 点の追加.
//---------------------------.
void CollisionRenderer::PushPoint( const D3DXPOSITION3& pPos )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsPointRender == false ) return;
	pI->m_pPointQueue.push( pPos );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 球体の追加.
//	Release / 球体を描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushSphere( CSphere* pSphere )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsSphereRender == false ) return;
	pI->m_pSphereQueue.push( pSphere );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 円柱の追加.
//	Release / 円柱を描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushCylinder( CCylinder* pCylinder )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsCylinderRender == false ) return;
	pI->m_pCylinderQueue.push( pCylinder );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// レイの追加.
//	Release / レイを描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushRay( CRay* pRay )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsRayRender == false ) return;
	pI->m_pRayQueue.push( pRay );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 十字レイの追加.
//	Release / レイを描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushRay( CCrossRay* pRay )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsRayRender == false ) return;
	std::vector<CRay*> RayList = pRay->GetRay();
	for( auto& r : RayList ) pI->m_pRayQueue.push( r );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// メッシュの追加.
//	Release / メッシュを描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushMesh( CStaticMesh* pMesh, STransform* pTrans )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsMeshRender == false ) return;
	pI->m_pMeshQueue.push(
		std::make_pair( std::make_pair( pMesh, nullptr ),
		pTrans != nullptr ?
		*pTrans :
		pMesh->GetTransform() )
	);
#endif // #ifdef _DEBUG.
}
void CollisionRenderer::PushMesh( CCollisionMesh* pMesh, STransform* pTrans )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsMeshRender == false ) return;
	pI->m_pMeshQueue.push(
		std::make_pair( std::make_pair( nullptr, pMesh ),
		pTrans != nullptr ?
		*pTrans :
		pMesh->GetTransform() )
	);
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 点2Dの追加.
//	点を描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushPoint2D( const D3DXPOSITION3& pPos )
{
	PushPoint2D( { pPos.x, pPos.y } );
}
void CollisionRenderer::PushPoint2D( const D3DXPOSITION2& pPos )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsPoint2DRender == false ) return;
	pI->m_pPoint2DQueue.push( pPos );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// ボックス2Dの追加.
//	ボックス2Dを描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushBox2D( CBox2D* pBox2D )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsBox2DRender == false ) return;
	pI->m_pBox2DQueue.push( pBox2D );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 円2Dの追加.
//	円2Dを描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PushSphere2D( CSphere2D* pSphere2D )
{
#ifdef _DEBUG
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsSphere2DRender == false ) return;
	pI->m_pSphere2DQueue.push( pSphere2D );
#endif // #ifdef _DEBUG.
}

//---------------------------.
// 点の描画.
//	点を描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::PointRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsPointRender == false	) return;
	if ( pI->m_pPointQueue.empty()		) return;

	// 球体メッシュの取得.
	if ( pI->m_pSphereMesh == nullptr ) {
		pI->m_pSphereMesh = MeshResource::GetCollision( "Sphere_c" );
	}

	// 当たり判定を描画していく.
	D3DXPOSITION3 pRenderPoint = { 0.0f, 0.0f, 0.0f };
	const int Size = static_cast<int>( pI->m_pPointQueue.size() );
	DirectX11::SetDepth( false );
	for ( int i = 0; i < Size; i++ ) {
		pRenderPoint = pI->m_pPointQueue.front();

		// 球体の描画.
		pI->m_pSphereMesh->SetColor( Color::White );
		pI->m_pSphereMesh->SetPosition( pRenderPoint );
		pI->m_pSphereMesh->SetScale( { 0.1f, 0.1f, 0.1f } );
		pI->m_pSphereMesh->Render();

		pI->m_pPointQueue.pop();
	}
	DirectX11::SetDepth( true );
}

//---------------------------.
// 球体の描画.
//	球体を描画しない場合は処理は行わない.
//---------------------------.
void CollisionRenderer::SphereRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsSphereRender == false	) return;
	if ( pI->m_pSphereQueue.empty()		) return;

	// 球体メッシュの取得.
	if ( pI->m_pSphereMesh == nullptr ) {
		pI->m_pSphereMesh = MeshResource::GetCollision( "Sphere_c" );
	}

	// 当たり判定を描画していく.
	DirectX11::SetRasterizerState( ERS_STATE::Wire );
	CSphere* pRenderSphere = nullptr;
	const int Size = static_cast<int>( pI->m_pSphereQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		pRenderSphere = pI->m_pSphereQueue.front();

		// 球体の描画.
		pI->m_pSphereMesh->SetColor( pRenderSphere->IsHit() ? Color::Red : Color::White );
		pI->m_pSphereMesh->SetPosition( pRenderSphere->GetPosition() );
		pI->m_pSphereMesh->SetScale( pRenderSphere->GetCollisionScale( pI->m_pSphereMesh->GetMesh() ) );
		pI->m_pSphereMesh->Render();

		pI->m_pSphereQueue.pop();
	}
	DirectX11::SetRasterizerState( ERS_STATE::None );
}

//---------------------------.
// 円柱の描画.
//	円柱を描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::CylinderRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsCylinderRender == false	) return;
	if ( pI->m_pCylinderQueue.empty()		) return;

	// 円柱メッシュの取得.
	if ( pI->m_pCylinderMesh == nullptr ) {
		pI->m_pCylinderMesh = MeshResource::GetCollision( "Cylinder_c" );
	}

	// 当たり判定を描画していく.
	DirectX11::SetRasterizerState( ERS_STATE::Wire );
	CCylinder* pRenderCylinder = nullptr;
	const int Size = static_cast<int>( pI->m_pCylinderQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		pRenderCylinder = pI->m_pCylinderQueue.front();

		// 円柱の描画.
		pI->m_pCylinderMesh->SetColor( pRenderCylinder->IsHit() ? Color::Red : Color::White );
		pI->m_pCylinderMesh->SetPosition( pRenderCylinder->GetPosition() );
		pI->m_pCylinderMesh->SetScale( pRenderCylinder->GetCollisionScale( pI->m_pCylinderMesh->GetMesh() ) );
		pI->m_pCylinderMesh->Render();

		pI->m_pCylinderQueue.pop();
	}
	DirectX11::SetRasterizerState( ERS_STATE::None );
}

//---------------------------.
// レイの描画.
//	レイを描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::RayRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsRayRender == false	) return;
	if ( pI->m_pRayQueue.empty()	) return;

	// レイメッシュの取得.
	if ( pI->m_pRayMesh == nullptr ) {
		pI->m_pRayMesh = std::make_unique<CRayMesh>();
		if ( FAILED( pI->m_pRayMesh->Init() ) ) return;
	}

	// 当たり判定を描画していく.
	CRay* pRenderRay = nullptr;
	const int Size = static_cast<int>( pI->m_pRayQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		pRenderRay = pI->m_pRayQueue.front();

		// レイの描画.
		pI->m_pRayMesh->SetColor( pRenderRay->IsHit() ? Color::Red : Color::White );
		pI->m_pRayMesh->Render( pRenderRay );

		pI->m_pRayQueue.pop();
	}
}

//---------------------------.
// メッシュの描画.
//	メッシュを描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::MeshRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsMeshRender == false	) return;
	if ( pI->m_pMeshQueue.empty()		) return;

	// 当たり判定を描画していく.
	DirectX11::SetRasterizerState( ERS_STATE::Wire );
	const int Size = static_cast<int>( pI->m_pMeshQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		// 表示するトランスフォームを取得.
		STransform RenderTransform	= pI->m_pMeshQueue.front().second;
		RenderTransform.Rotation	= INIT_FLOAT3;

		// メッシュの描画.
		if ( pI->m_pMeshQueue.front().first.first != nullptr ) {
			pI->m_pMeshQueue.front().first.first->Render( &RenderTransform );
		}
		else {
			pI->m_pMeshQueue.front().first.second->Render( &RenderTransform );
		}
		pI->m_pMeshQueue.pop();
	}
	DirectX11::SetRasterizerState( ERS_STATE::None );
}

//---------------------------.
// 点2Dの描画.
//	点2Dを描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::Point2DRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsPoint2DRender == false	) return;
	if ( pI->m_pPoint2DQueue.empty()	) return;

	// 球体メッシュの取得.
	if ( pI->m_pPoint2DSprite == nullptr ) {
		pI->m_pPoint2DSprite = SpriteResource::GetSprite( "PointColl2D", &pI->m_Point2DState );
	}

	// 当たり判定を描画していく.
	D3DXPOSITION2 pRenderPoint2D = { 0.0f, 0.0f };
	const int Size = static_cast<int>( pI->m_pPoint2DQueue.size() );
	DirectX11::SetDepth( false );
	for ( int i = 0; i < Size; i++ ) {
		pRenderPoint2D = pI->m_pPoint2DQueue.front();

		// 円2Dの座標の設定.
		pI->m_Point2DState.Transform.Position.x = pRenderPoint2D.x;
		pI->m_Point2DState.Transform.Position.y = pRenderPoint2D.y;

		// 円2Dの描画.
		pI->m_pPoint2DSprite->RenderUI( &pI->m_Point2DState );

		pI->m_pPoint2DQueue.pop();
	}
	DirectX11::SetDepth( true );

}

//---------------------------.
// ボックス2Dの描画.
//	ボックス2Dを描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::Box2DRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsBox2DRender == false	) return;
	if ( pI->m_pBox2DQueue.empty()		) return;

	// ボックス2Dスプライトの取得.
	if ( pI->m_pBox2DSprite == nullptr ) {
		pI->m_pBox2DSprite = SpriteResource::GetSprite( "BoxColl2D", &pI->m_Box2DState );
	}

	// 当たり判定を描画していく.
	DirectX11::SetDepth( false );
	CBox2D* pRenderBox2D = nullptr;
	const int Size = static_cast< int >( pI->m_pBox2DQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		pRenderBox2D = pI->m_pBox2DQueue.front();

		// ボックス2Dの座標の設定.
		const D3DXPOSITION2& BoxPos = pRenderBox2D->GetPosition();
		pI->m_Box2DState.Transform.Position.x = BoxPos.x;
		pI->m_Box2DState.Transform.Position.y = BoxPos.y;

		// ボックス2Dの大きさの設定.
		const SSize& BoxSize = pRenderBox2D->GetSize();
		pI->m_Box2DState.Transform.Scale.x = BoxSize.w / 64.0f;
		pI->m_Box2DState.Transform.Scale.y = BoxSize.h / 64.0f;

		// 色の設定.
		if ( pRenderBox2D->IsHit() ) {
			pI->m_Box2DState.Color = Color4::Red;
		}
		else {
			pI->m_Box2DState.Color = Color4::White;
		}

		// ボックス2Dの描画.
		pI->m_pBox2DSprite->RenderUI( &pI->m_Box2DState );

		pI->m_pBox2DQueue.pop();
	}
	DirectX11::SetDepth( true );
}

//---------------------------.
// 円2Dの描画.
//	円2Dを描画しない場合処理は行わない.
//---------------------------.
void CollisionRenderer::Sphere2DRender()
{
	CollisionRenderer* pI = GetInstance();

	if ( pI->m_IsSphere2DRender == false	) return;
	if ( pI->m_pSphere2DQueue.empty()		) return;

	// 円2Dスプライトの取得.
	if ( pI->m_pSphere2DSprite == nullptr ) {
		pI->m_pSphere2DSprite = SpriteResource::GetSprite( "SphereColl2D", &pI->m_Sphere2DState );
	}

	// 当たり判定を描画していく.
	DirectX11::SetDepth( false );
	CSphere2D* pRenderSphere2D = nullptr;
	const int Size = static_cast< int >( pI->m_pSphere2DQueue.size() );
	for ( int i = 0; i < Size; i++ ) {
		pRenderSphere2D = pI->m_pSphere2DQueue.front();

		// 円2Dの座標の設定.
		const D3DXPOSITION2& SpherePos = pRenderSphere2D->GetPosition();
		pI->m_Sphere2DState.Transform.Position.x = SpherePos.x;
		pI->m_Sphere2DState.Transform.Position.y = SpherePos.y;

		// 円2Dの大きさの設定.
		const float SphereSize = pRenderSphere2D->GetRadius();
		pI->m_Sphere2DState.Transform.Scale.x = SphereSize * 2.0f / 64.0f;
		pI->m_Sphere2DState.Transform.Scale.y = SphereSize * 2.0f / 64.0f;

		// 色の設定.
		if ( pRenderSphere2D->IsHit() ) {
			pI->m_Sphere2DState.Color = Color4::Red;
		}
		else {
			pI->m_Sphere2DState.Color = Color4::White;
		}

		// 円2Dの描画.
		pI->m_pSphere2DSprite->RenderUI( &pI->m_Sphere2DState );

		pI->m_pSphere2DQueue.pop();
	}
	DirectX11::SetDepth( true );
}
