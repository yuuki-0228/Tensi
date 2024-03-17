#include "CollisionManager.h"
#include "..\..\Collision\Sphere\Sphere.h"
#include "..\..\Collision\Cylinder\Cylinder.h"
#include "..\..\Collision\Ray\Ray.h"
#include "..\..\Collision\CrossRay\CrossRay.h"
#include "..\..\Collision\Mesh\Mesh.h"

#include "..\..\Collision\2D\Box2D\Box2D.h"
#include "..\..\Collision\2D\Sphere2D\Sphere2D.h"

namespace {
	// 2Dのボックスと円の当たり判定で使用.
	float DistanceSqrf( const float t_x1, const float t_y1, const float t_x2, const float t_y2 ) {
		float dx = t_x2 - t_x1;
		float dy = t_y2 - t_y1;

		return ( dx * dx ) + ( dy * dy );
	}
}

//---------------------------.
// 球体同士の当たり判定.
//	球体が null / 半径が 0 以下の場合処理は行わない.
//---------------------------.
bool Coll::IsSphereToSphere(
	CSphere* pMySphere,		// 自分の球体.
	CSphere* pOpSphere )	// 相手の球体.
{
	if ( pMySphere == nullptr			) return false;
	if ( pOpSphere == nullptr			) return false;
	if ( pMySphere->GetRadius() <= 0.0f ) return false;
	if ( pOpSphere->GetRadius() <= 0.0f ) return false;

	// 初期化.
	pMySphere->SetFlagOff();
	pOpSphere->SetFlagOff();

	// 2つの球体の中心間の距離を求める.
	const D3DXVECTOR3& vLength = pMySphere->GetPosition() - pOpSphere->GetPosition();
	// 上記のベクトルから長さに変換.
	const float& fLength = D3DXVec3Length( &vLength );

	// 2つの球体の半径の合計を求める.
	const float& AddRadius = pMySphere->GetRadius() + pOpSphere->GetRadius();

	// 2つの球体間の距離が2つの球体のそれぞれの半径を足したものより、
	//	小さいということは、球体同士が重なっている(衝突している)ということ.
	if ( fLength <= AddRadius )
	{
		// フラグを立てる.
		pMySphere->SetFlagOn();
		pOpSphere->SetFlagOn();

		return true;	// 衝突.
	}
	return false;		// 衝突してない.
}

//---------------------------.
// 球体と点の当たり判定.
//	球体/座標が null / 半径が 0 以下の場合処理は行わない.
//---------------------------.
bool Coll::IsSphereToPosition(
	CSphere*				pMySphere,	// 自分の球体.
	const D3DXPOSITION3&	pOpPos )	// 相手の座標.
{
	if ( pMySphere	== nullptr			) return false;
	if ( pOpPos		== nullptr			) return false;
	if ( pMySphere->GetRadius() <= 0.0f ) return false;

	// 初期化.
	pMySphere->SetFlagOff();

	// 距離を求める.
	const D3DXVECTOR3& vLength = pMySphere->GetPosition() - pOpPos;
	// 上記のベクトルから長さに変換.
	const float& fLength = D3DXVec3Length( &vLength );

	// 距離が半径より、小さいということは、
	//	球体と点が重なっている(衝突している)ということ.
	if ( fLength <= pMySphere->GetRadius() )
	{
		// フラグを立てる.
		pMySphere->SetFlagOn();

		return true;	// 衝突.
	}
	return false;		// 衝突してない.
}

//---------------------------.
// 円柱と点の当たり判定.
//	円柱/座標が null / 半径/高さが 0 以下の場合処理は行わない.
//---------------------------.
bool Coll::IsCylinderToPosition(
	CCylinder*				pMyCylinder,	// 自分の円柱.
	const D3DXPOSITION3&	pOpPos )		// 相手の座標.
{
	if ( pMyCylinder	== nullptr			) return false;
	if ( pOpPos			== nullptr			) return false;
	if ( pMyCylinder->GetRadius() <= 0.0f	) return false;
	if ( pMyCylinder->GetHeight() <= 0.0f	) return false;

	// 初期化.
	pMyCylinder->SetFlagOff();

	// 距離を求める.
	const D3DXPOSITION3& CheckPos = { pOpPos.x, pMyCylinder->GetPosition().y, pOpPos.z };
	const D3DXVECTOR3&	 vLength  = pMyCylinder->GetPosition() - CheckPos;
	// 上記のベクトルから長さに変換.
	const float& fLength = D3DXVec3Length( &vLength );

	// 距離が半径より小さく、高さの範囲内に座標があるか調べる.
	const float& YMax	= pMyCylinder->GetPosition().y + pMyCylinder->GetHeight();
	const float& YMin	= pMyCylinder->GetPosition().y - pMyCylinder->GetHeight();
	if ( fLength <= pMyCylinder->GetRadius() && (
		 YMax >= pOpPos.y && pOpPos.y >= YMin ) )
	{
		// フラグを立てる.
		pMyCylinder->SetFlagOn();

		return true;	// 衝突.
	}
	return false;		// 衝突してない.
}

//---------------------------.
// レイとメッシュの当たり判定.
//	レイが null / メッシュが null の場合は処理は行わない.
//---------------------------.
bool Coll::IsRayToMesh(
	CRay*			pRay,			// レイ.
	CMesh*			pMesh,			// メッシュ.
	float*			pDistance,		// (out)距離.
	D3DXPOSITION3*	pIntersect,		// (out)交差点.
	D3DXVECTOR3*	pNormal,		// (out)法線.
	bool			IsWallScratch,	// 壁ずりするか.
	float			WallSpace )		// 壁との限界距離.
{
	if ( pRay	== nullptr ) return false;
	if ( pMesh	== nullptr ) return false;

	// 初期化.
	pRay->SetFlagOff();
	pMesh->SetFlagOff();

	// レイの情報を取得.
	const D3DXROTATION3& vRayRot = pRay->GetRotation();
	const D3DXVECTOR3&	 vRayVec = pRay->GetVector();
	const float			 Length	 = pRay->GetLength();

	D3DXVECTOR3 vAxis;		// 軸ベクトル.
	D3DXMATRIX	mRotationY;	// Y軸回転行列.
	D3DXMatrixRotationY( &mRotationY, vRayRot.y );// Y軸回転行列を作成.
	// 軸ベクトルを現在の回転状態に変換する.
	D3DXVec3TransformCoord( &vAxis, &vRayVec, &mRotationY );

	// レイの始点と終点.
	D3DXPOSITION3			vStart, vEnd;
	const D3DXPOSITION3&	RayPos = pRay->GetPosition();
	vStart	= RayPos;						// レイの始点を設定.
	vEnd	= RayPos + ( vAxis * Length );	// レイの終点を設定.

	// レイを当てたいメッシュが移動している場合でも,
	//	対象のworld行列の逆行列を用いれば正しくレイが当たる.
	D3DXMATRIX mWorld, mInvWorld;
	// 逆行列を求める.
	pMesh->SetRotation( INIT_FLOAT3 );
	mWorld = pMesh->GetTransform().GetWorldMatrix();
	D3DXMatrixInverse( &mInvWorld, nullptr, &mWorld );
	// レイの始点,終点に反映.
	D3DXVec3TransformCoord( &vStart, &vStart, &mInvWorld );
	D3DXVec3TransformCoord( &vEnd, &vEnd, &mInvWorld );

	// 方向を求める.
	D3DXVECTOR3 vDirection = vEnd - vStart;

	BOOL bHit = FALSE;			// 命中フラグ.
	DWORD dwIndex = 0;			// インデックス番号.
	D3DXVECTOR3 Vertex[3];		// 頂点座標.
	FLOAT U = 0, V = 0;			// 重心ヒット座標.

	// メッシュとレイの交差を調べる.
	float Distance = 0.0f;
	D3DXIntersect(
		pMesh->GetMesh(),		// 対象メッシュ.
		&vStart,				// レイ始点.
		&vDirection,			// レイの方向.
		&bHit,					// (out)判定結果.
		&dwIndex,				// (out)bHitがTrue時にレイの始点に最も近くの面のインデックス値へのポインタ.
		&U, &V,					// (out)重心ヒット座標.
		&Distance,				// メッシュとの距離.
		nullptr, nullptr );
	if ( pDistance != nullptr ) *pDistance = Distance;

	// 無限に伸びるレイのどこかでメッシュに当たっていたら.
	if ( bHit == TRUE ) {
		// フラグを立てる.
		pRay->SetFlagOn();
		pMesh->SetFlagOn();

		// 命中したとき.
		pMesh->FindVerticesOnPoly( dwIndex, Vertex );
		// 重心座標から交点を算出.
		//	ローカル交点は v0 + U*(v1-v0) + V*(v2-v0) で求まる.
		D3DXPOSITION3 Intersect = { 0.0f, 0.0f, 0.0f };
		Intersect =
			Vertex[0] + U * ( Vertex[1] - Vertex[0] ) + V * ( Vertex[2] - Vertex[0] );
		if ( pIntersect != nullptr ) *pIntersect = Intersect;

		// 法線の出力要求があれば.
		D3DXVECTOR3 Normal;
		if ( pNormal != nullptr || IsWallScratch ) {
			D3DXPLANE	p;
			// その頂点(3点)から「平面の方程式」を得る.
			D3DXPlaneFromPoints( &p, &Vertex[0], &Vertex[1], &Vertex[2] );
			// 「平面の方程式」の係数が法線の成分.
			Normal = D3DXVECTOR3( p.a, p.b, p.c );
			if ( pNormal != nullptr ) *pNormal = Normal;
		}

		// vEndからみた距離で1.0fより小さければ当たっている.
		if ( Distance < 1.0f ) {
			// 壁ずり処理.
			if ( IsWallScratch ) {
				if ( Distance <= WallSpace ) {
					// 法線は壁から真っ直ぐに出ているので、法線とかけ合わせて.
					//	対象を動かすベクトルが得られる.
					D3DXVECTOR3 vOffset = Normal * ( WallSpace - Distance );
					vOffset.y = 0.0f;

					// レイの位置を更新.
					pRay->SetPosition( pRay->GetPosition() + vOffset );
				}
			}
			return true;	// 命中している.
		}
	}
	return false;	// 外れている.
}

//---------------------------.
// 十字レイと壁(メッシュ)の当たり判定.
//	十字レイが null / 壁(メッシュ)が null の場合は処理は行わない.
//---------------------------.
bool Coll::IsCrossRayToWallMesh(
	CCrossRay*			pCrossRay,	// 十字レイ.
	CMesh*				pWallMesh,	// 壁(メッシュ).
	std::vector<int>*	pHitList )	// 当たった方向リスト.
{
	if ( pCrossRay == nullptr	) return false;
	if ( pWallMesh == nullptr	) return false;

	// 壁との限界距離.
	const float WSPACE = 0.8f;

	// レイの向きによる当たる壁までの距離を求める.軸ベクトル(前後左右).
	const int			RayDireMax	= static_cast<int>( ECrossRayDire::Max );
	bool				IsHit		= false;
	std::vector<CRay*>	RayList		= pCrossRay->GetRay();
	if ( pHitList != nullptr ) pHitList->reserve( 4 );
	for ( int i = 0; i < RayDireMax; ++i ) {
		if ( Coll::IsRayToMesh( RayList[i], pWallMesh, nullptr, nullptr, nullptr, true, WSPACE ) ) {
			IsHit = true;

			// 当たった方向を追加.
			if ( pHitList != nullptr ) pHitList->emplace_back( i );

			// レイの位置を更新.
			for ( int j = 0; j < RayDireMax; ++j ) {
				RayList[j]->SetPosition( RayList[i]->GetPosition() );
			}
		}
	}
	return IsHit;
}



//---------------------------.
// 2Dのボックスと点の当たり判定.
//	ボックス2D/点2D が null / サイズが 0 以下の場合処理は行わない.
//---------------------------.
bool Coll2D::IsBox2DToPoint2D( CBox2D* pMyBox, const D3DXPOSITION2& pOpPos )
{
	if ( pMyBox == nullptr ) return false;
	if ( pOpPos == nullptr ) return false;

	const D3DXPOSITION2& mPos	= pMyBox->GetPosition();
	const SSize&		 mSize	= pMyBox->GetSize();

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;

	// ボックスと点が当たっているか調べる.
	if ( ( mPos.x < pOpPos.x ) && ( mPos.x + mSize.w > pOpPos.x ) &&
		 ( mPos.y < pOpPos.y ) && ( mPos.y + mSize.h > pOpPos.y ) )
	{
		return true;	// 命中している.
	}
	return false;	// 外れている.
}

//---------------------------.
// 2Dのボックス同士の当たり判定.
//	ボックス2Dが null / サイズが 0 以下の場合処理は行わない.
//---------------------------.
bool Coll2D::IsBox2DToBox2D( CBox2D* pMyBox, CBox2D* pOpBox )
{
	if ( pMyBox == nullptr ) return false;
	if ( pOpBox == nullptr ) return false;

	const D3DXPOSITION2& mPos	= pMyBox->GetPosition();
	const D3DXPOSITION2& oPos	= pOpBox->GetPosition();
	const SSize&		 mSize	= pMyBox->GetSize();
	const SSize&		 oSize	= pOpBox->GetSize();

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;
	if ( oSize.w <= 0.0f ) return false;
	if ( oSize.h <= 0.0f ) return false;

	// ボックス通しが当たっているか調べる.
	if ( ( mPos.x < oPos.x + oSize.w ) && ( mPos.x + mSize.w > oPos.x ) &&
		 ( mPos.y < oPos.y + oSize.h ) && ( mPos.y + mSize.h > oPos.y ) )
	{
		return true;	// 命中している.
	}
	return false;	// 外れている.
}

//---------------------------.
// 2Dの円と点の当たり判定.
//	円2D/点2Dが null / 半径が 0 以下の場合処理は行わない.
//---------------------------.
bool Coll2D::IsSphere2DToPoint2D( CSphere2D* pMySphere, const D3DXPOSITION2& pOpPos )
{
	if ( pMySphere	== nullptr ) return false;
	if ( pOpPos		== nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMySphere->GetPosition();
	const float			 mRadius	= pMySphere->GetRadius();

	if ( mRadius <= 0.0f ) return false;

	// 円と点が当たっているか調べる.
	const float dx = mPos.x - pOpPos.x;
	const float dy = mPos.y - pOpPos.y;
	const float dr = dx * dx + dy * dy;

	const float dl = mRadius * mRadius;
	if ( dr < dl )
	{
		return true;	// 命中している.
	}
	return false;	// 外れている.

}

//---------------------------.
// 2Dの円同士の当たり判定.
//	円2Dが null / 半径が 0 以下の場合処理は行わない.
//---------------------------.
bool Coll2D::IsSphere2DToSphere2D( CSphere2D* pMySphere, CSphere2D* pOpSphere )
{
	if ( pMySphere == nullptr ) return false;
	if ( pOpSphere == nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMySphere->GetPosition();
	const D3DXPOSITION2& oPos		= pOpSphere->GetPosition();
	const float			 mRadius	= pMySphere->GetRadius();
	const float			 oRadius	= pOpSphere->GetRadius();

	if ( mRadius <= 0.0f ) return false;
	if ( oRadius <= 0.0f ) return false;

	// 円通しが当たっているか調べる.
	const float dx = mPos.x - oPos.x;
	const float dy = mPos.y - oPos.y;
	const float dr = dx * dx + dy * dy;

	const float ar = mRadius + oRadius;
	const float dl = ar * ar;
	if ( dr < dl )
	{
		return true;	// 命中している.
	}
	return false;	// 外れている.
}

//---------------------------.
// 2Dのボックスと円の当たり判定.
//	ボックス/円2Dが null / サイズ/半径が 0 以下のの場合処理は行わない.
//---------------------------.
bool Coll2D::IsBox2DToSphere2D( CBox2D* pMyBox, CSphere2D* pOpSphere )
{
	if ( pMyBox		== nullptr ) return false;
	if ( pOpSphere	== nullptr ) return false;

	const D3DXPOSITION2& mPos		= pMyBox->GetPosition();
	const D3DXPOSITION2& oPos		= pOpSphere->GetPosition();
	const SSize&		 mSize		= pMyBox->GetSize();
	const float			 oRadius	= pOpSphere->GetRadius();
	const float			 mBoxLeft	= mPos.x;
	const float			 mBoxRight	= mPos.x + mSize.w;
	const float			 mBoxTop	= mPos.y;
	const float			 mBoxBottom = mPos.y + mSize.h;

	if ( mSize.w <= 0.0f ) return false;
	if ( mSize.h <= 0.0f ) return false;
	if ( oRadius <= 0.0f ) return false;

	// 四角形の四辺に対して円の半径分だけ足したとき円が重なっていたら.
	if ( ( oPos.x > mBoxLeft	- oRadius ) &&
		 ( oPos.x < mBoxRight	+ oRadius ) &&
		 ( oPos.y > mBoxTop		- oRadius ) &&
		 ( oPos.y < mBoxBottom	+ oRadius ) )
	{
		const float fl = oRadius * oRadius;

		// 左上.
		if ( oPos.x < mBoxLeft &&
			 oPos.y < mBoxTop &&
			 DistanceSqrf( mBoxLeft, mBoxTop, oPos.x, oPos.y ) >= fl )
		{
			return false;	// 外れている.
		}

		// 左下.
		if ( oPos.x < mBoxLeft &&
			 oPos.y > mBoxBottom &&
			 DistanceSqrf( mBoxLeft, mBoxBottom, oPos.x, oPos.y ) >= fl )
		{
			return false;	// 外れている.
		}

		// 右上.
		if ( oPos.x > mBoxRight &&
			 oPos.y < mBoxTop &&
			 DistanceSqrf( mBoxRight, mBoxTop, oPos.x, oPos.y ) >= fl )
		{
			return false;	// 外れている.
		}

		// 右下.
		if ( oPos.x > mBoxRight &&
			 oPos.y > mBoxBottom &&
			 DistanceSqrf( mBoxRight, mBoxBottom, oPos.x, oPos.y ) >= fl )
		{
			return false;	// 外れている.
		}

		return true;	// 命中している.
	}
	return false;		// 外れている.
}
