#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Common\Sprite\Sprite.h"
#include <queue>

class CSphere;
class CCylinder;
class CRay;
class CCrossRay;
class CRayMesh;
class CStaticMesh;
class CCollisionMesh;
class CBox2D;
class CSphere2D;

/************************************************
*	当たり判定描画クラス.
*		﨑田友輝.
**/
class CollisionRenderer final
{
public:
	using Mesh_queue = std::queue<std::pair<std::pair<CStaticMesh*, CCollisionMesh*>, STransform>>;

public:
	CollisionRenderer();
	~CollisionRenderer();

	// 描画.
	static void Render();

	// 点の追加.
	static void PushPoint( const D3DXPOSITION3& pPos );
	// 球体の追加.
	static void PushSphere( CSphere* pSphere );
	// 円柱の追加.
	static void PushCylinder( CCylinder* pCylinder );
	// レイの追加.
	static void PushRay( CRay* pRay );
	// 十字レイの追加.
	static void PushRay( CCrossRay* pRay );
	// メッシュの追加.
	static void PushMesh( CStaticMesh*		pMesh, STransform* pTrans = nullptr );
	static void PushMesh( CCollisionMesh*	pMesh, STransform* pTrans = nullptr );
	// 点2Dの追加.
	static void PushPoint2D( const D3DXPOSITION3& pPos );
	static void PushPoint2D( const D3DXPOSITION2& pPos );
	// ボックス2Dの追加.
	static void PushBox2D( CBox2D* pBox2D );
	// 円2Dの追加.
	static void PushSphere2D( CSphere2D* pSphere2D );

private:
	// インスタンスの取得.
	static CollisionRenderer* GetInstance();

	// 点の描画.
	static void PointRender();
	// 球体の描画.
	static void SphereRender();
	// 円柱の描画.
	static void CylinderRender();
	// レイの描画.
	static void RayRender();
	// メッシュの描画.
	static void MeshRender();
	// 点2Dの描画.
	static void Point2DRender();
	// ボックス2Dの描画.
	static void Box2DRender();
	// 円2Dの描画.
	static void Sphere2DRender();

private:
	std::queue<D3DXPOSITION3>	m_pPointQueue;		// 点のキュー.
	std::queue<CSphere*>		m_pSphereQueue;		// 球体のキュー.
	std::queue<CCylinder*>		m_pCylinderQueue;	// 円柱のキュー.
	std::queue<CRay*>			m_pRayQueue;		// レイのキュー.
	Mesh_queue					m_pMeshQueue;		// メッシュのキュー.
	std::queue<D3DXPOSITION2>	m_pPoint2DQueue;	// 点2Dのキュー.
	std::queue<CBox2D*>			m_pBox2DQueue;		// ボックス2Dのキュー.
	std::queue<CSphere2D*>		m_pSphere2DQueue;	// 円2Dのキュー.
	CCollisionMesh*				m_pSphereMesh;		// 球体メッシュ.
	CCollisionMesh*				m_pCylinderMesh;	// 円柱メッシュ.
	std::unique_ptr<CRayMesh>	m_pRayMesh;			// レイメッシュ.
	CSprite*					m_pBox2DSprite;		// ボックス2Dスプライト.
	CSprite*					m_pSphere2DSprite;	// 円2Dスプライト.
	CSprite*					m_pPoint2DSprite;	// 点2Dスプライト.
	SSpriteRenderState			m_Box2DState;		// ボックス2Dの情報.
	SSpriteRenderState			m_Sphere2DState;	// 円2Dの情報.
	SSpriteRenderState			m_Point2DState;		// 点2Dの情報.

	CBool						m_IsPointRender;	// 点を描画するか.
	CBool						m_IsSphereRender;	// 球体を描画するか.
	CBool						m_IsCylinderRender;	// 円柱を描画するか.
	CBool						m_IsRayRender;		// レイを描画するか.
	CBool						m_IsMeshRender;		// メッシュを描画するか.
	CBool						m_IsPoint2DRender;	// 点2Dを描画するか.
	CBool						m_IsBox2DRender;	// ボックス2Dを描画するか.
	CBool						m_IsSphere2DRender;	// 円2Dを描画するか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CollisionRenderer( const CollisionRenderer & )				= delete;
	CollisionRenderer& operator = ( const CollisionRenderer & )	= delete;
	CollisionRenderer( CollisionRenderer && )						= delete;
	CollisionRenderer& operator = ( CollisionRenderer && )		= delete;
};
