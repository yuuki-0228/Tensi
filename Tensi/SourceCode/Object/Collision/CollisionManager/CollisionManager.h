#pragma once
#include "..\..\..\Global.h"

class CSphere;
class CCylinder;
class CRay;
class CCrossRay;
class CMesh;

class CBox2D;
class CSphere2D;

/************************************************
*	当たり判定一覧.
**/
namespace Coll {
	// 球体同士の当たり判定.
	bool IsSphereToSphere(
		CSphere* pMySphere,		// 自分の球体.
		CSphere* pOpSphere );	// 相手の球体.

	// 球体と点の当たり判定.
	bool IsSphereToPosition(
		CSphere*				pMySphere,		// 自分の球体.
		const D3DXPOSITION3&	pOpPos );		// 相手の座標.

	// 円柱と点の当たり判定.
	bool IsCylinderToPosition(
		CCylinder*				pMyCylinder,	// 自分の円柱.
		const D3DXPOSITION3&	pOpPos);		// 相手の座標.

	// レイとメッシュの当たり判定.
	bool IsRayToMesh(
		CRay*				pRay,						// レイ.
		CMesh*				pMesh,						// メッシュ.
		float*				pDistance		= nullptr,	// (out)距離.
		D3DXPOSITION3*		pIntersect		= nullptr,	// (out)交差点.
		D3DXVECTOR3*		pNormal			= nullptr,	// (out)法線.
		bool				IsWallScratch	= false,	// 壁ずりするか.
		float				WallSpace		= 0.8f );	// 壁との限界距離.

	// 十字レイと壁(メッシュ)の当たり判定.
	bool IsCrossRayToWallMesh( 
		CCrossRay*			pCrossRay,					// 十字レイ.
		CMesh*				pWallMesh,					// 壁(メッシュ).
		std::vector<int>*	pHitList = nullptr );		// 当たった方向リスト.
}

/************************************************
*	2Dの当たり判定一覧.
**/
namespace Coll2D {
	// 2Dのボックスと点の当たり判定.
	bool IsBox2DToPoint2D(
		CBox2D*					pMyBox,			// 自分のボックス.
		const D3DXPOSITION2&	pOpPos );		// 相手の座標.

	// 2Dのボックス同士の当たり判定.
	bool IsBox2DToBox2D(
		CBox2D*					pMyBox,			// 自分のボックス.
		CBox2D*					pOpBox );		// 相手のボックス.

	// 2Dの円と点の当たり判定.
	bool IsSphere2DToPoint2D(
		CSphere2D*				pMySphere,		// 自分の円.
		const D3DXPOSITION2&	pOpPos );		// 相手の座標.

	// 2Dの円同士の当たり判定.
	bool IsSphere2DToSphere2D(
		CSphere2D*				pMySphere,		// 自分の円.
		CSphere2D*				pOpSphere );	// 相手の円.

	// 2Dのボックスと円の当たり判定.
	bool IsBox2DToSphere2D(
		CBox2D*					pMyBox,			// 自分のボックス.
		CSphere2D*				pOpSphere );	// 相手の円.
}