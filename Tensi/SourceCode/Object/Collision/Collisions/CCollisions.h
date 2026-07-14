#pragma once
#include "..\..\..\SystemSetting.h"
#include "..\..\..\Global.h"
#ifdef ENABLE_MESH
#include "..\Sphere\Sphere.h"
#include "..\Cylinder\Cylinder.h"
#include "..\Mesh\Mesh.h"
#include "..\Ray\Ray.h"
#include "..\CrossRay\CrossRay.h"
#endif
#ifdef ENABLE_SPRITE
#include "..\2D\Box2D\Box2D.h"
#include "..\2D\Sphere2D\Sphere2D.h"
#endif
#include <memory>

// 当たり判定のタイプ.
enum class enCollType : unsigned char
{
	None,

#ifdef ENABLE_MESH
	Sphere,		// 球体.
	Cylinder,	// 円柱.
	Ray,		// レイ.
	CrossRay,	// 十字レイ.
	Mesh,		// メッシュ.
#endif
#ifdef ENABLE_SPRITE
	Box2D,		// 2Dのボックス.
	Sphere2D,	// 2Dの円.
#endif

	Max
} typedef ECollType;

/************************************************
*	複数の当たり判定を所持しているクラス.
**/
class CCollisions final
{
public:
	CCollisions();
	~CCollisions();

	// 指定した当たり判定の初期化.
	void InitCollision( const ECollType& Type );

	// 当たり判定の取得.
	template<class T> T*	GetCollision() { return nullptr; }
#ifdef ENABLE_MESH
	// 当たり判定の取得(球体).
	template<> CSphere*		GetCollision() { return m_pSphere.get(); }
	// 当たり判定の取得(円柱).
	template<> CCylinder*	GetCollision() { return m_pCylinder.get(); }
	// 当たり判定の取得(レイ).
	template<> CRay*		GetCollision() { return m_pRay.get(); }
	// 当たり判定の取得(十字レイ).
	template<> CCrossRay*	GetCollision() { return m_pCrossRay.get(); }
	// 当たり判定の取得(メッシュ).
	template<> CMesh*		GetCollision() { return m_pMesh.get(); }
#endif
#ifdef ENABLE_SPRITE
	// 当たり判定の取得(2Dのボックス).
	template<> CBox2D*		GetCollision() { return m_pBox2D.get(); }
	// 当たり判定の取得(2Dの円).
	template<> CSphere2D*	GetCollision() { return m_pSphere2D.get(); }
#endif

private:
#ifdef ENABLE_MESH
	std::shared_ptr<CSphere>	m_pSphere;		// 球体.
	std::shared_ptr<CCylinder>	m_pCylinder;	// 円柱.
	std::shared_ptr<CRay>		m_pRay;			// レイ.
	std::shared_ptr<CCrossRay>	m_pCrossRay;	// 十字レイ.
	std::shared_ptr<CMesh>		m_pMesh;		// メッシュ.
#endif
#ifdef ENABLE_SPRITE
	std::shared_ptr<CBox2D>		m_pBox2D;		// 2Dのボックス.
	std::shared_ptr<CSphere2D>	m_pSphere2D;	// 2Dの円.
#endif
};
