#pragma once
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	回転しない円柱クラス.
**/
class CCylinder final
	: public CColliderBase
{
public:
	CCylinder();
	virtual ~CCylinder();

	// 当たり判定の大きさになるScaleを返す.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// 半径(長さ)を取得.
	inline float GetRadius() const {
		return m_Radius;
	}
	// 上下の上限を取得.
	inline float GetHeight() const {
		return m_Height;
	}

	// メッシュに合わして半径を設定.
	HRESULT SetMeshRadius( const LPD3DXMESH& pMesh, const float vScale = 1.0f );
	// 半径(長さ)を設定.
	inline void SetRadius( const float radius ) {
		m_Radius = radius;
	}
	// 高さを設定.
	inline void SetHeight( const float height ) {
		m_Height = height;
	}

private:
	float m_Radius;		// 半径(長さ).
	float m_Height;		// 高さ.
};