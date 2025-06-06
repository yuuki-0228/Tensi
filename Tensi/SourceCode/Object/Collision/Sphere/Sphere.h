#pragma once
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	球体クラス.
**/
class CSphere final
	: public CColliderBase
{
public:
	CSphere();
	virtual ~CSphere();

	// 当たり判定の大きさになるScaleを返す.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// 半径(長さ)を取得.
	inline float GetRadius() const {
		return m_Radius;
	}

	// メッシュに合わして半径を設定.
	HRESULT SetMeshRadius( const CStaticMesh& pMesh, const float vScale = 1.0f );
	// 半径(長さ)を設定.
	inline void SetRadius( const float radius ) {
		m_Radius  = radius;
	}

private:
	float m_Radius;		//半径(長さ).
};