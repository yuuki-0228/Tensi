#pragma once
#include "..\..\..\Global.h"
#include "..\Sphere\Sphere.h"
#include "..\Cylinder\Cylinder.h"
#include "..\Ray\Ray.h"
#include "..\CrossRay\CrossRay.h"
#include "..\2D\Box2D\Box2D.h"
#include "..\2D\Sphere2D\Sphere2D.h"
#include "..\Mesh\Mesh.h"
#include <memory>

// 当たり判定のタイプ.
enum class enCollType : unsigned char
{
	None,

	Sphere,		// 球体.
	Cylinder,	// 円柱.
	Ray,		// レイ.
	CrossRay,	// 十字レイ.
	Mesh,		// メッシュ.

	Box2D,		// 2Dのボックス.
	Sphere2D,	// 2Dの円.

	Max
} typedef ECollType;

/************************************************
*	複数の当たり判定を所持しているクラス.
*		﨑田友輝.
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
	// 当たり判定の取得(2Dのボックス).
	template<> CBox2D*		GetCollision() { return m_pBox2D.get(); }
	// 当たり判定の取得(2Dの円).
	template<> CSphere2D*	GetCollision() { return m_pSphere2D.get(); }

private:
	std::shared_ptr<CSphere>	m_pSphere;		// 球体.
	std::shared_ptr<CCylinder>	m_pCylinder;	// 円柱.
	std::shared_ptr<CRay>		m_pRay;			// レイ.
	std::shared_ptr<CCrossRay>	m_pCrossRay;	// 十字レイ.
	std::shared_ptr<CMesh>		m_pMesh;		// メッシュ.
	std::shared_ptr<CBox2D>		m_pBox2D;		// 2Dのボックス.
	std::shared_ptr<CSphere2D>	m_pSphere2D;	// 2Dの円.
};
