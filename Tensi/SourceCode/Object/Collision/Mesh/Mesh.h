#pragma once
#include "..\ColliderBase.h"

/************************************************
*	メッシュクラス.
*		﨑田友輝.
**/
class CMesh
	: public CColliderBase
{
public:
	CMesh();
	~CMesh();

	// メッシュを取得.
	LPD3DXMESH GetMesh() const { return m_Model; }
	// レイとの当たり判定用のメッシュを取得.
	LPD3DXMESH GetMeshForRay() const { return m_ModelForRay; }

	// メッシュの設定.
	void SetMesh( const std::pair<LPD3DXMESH, LPD3DXMESH>& Meshs ) {
		m_Model			= Meshs.first;
		m_ModelForRay	= Meshs.second;
	}
	void SetMesh( const LPD3DXMESH& Meshs ) {
		m_Model			= Meshs;
		m_ModelForRay	= Meshs;
	}

	// 交差位置のポリゴンの頂点を見つける.
	HRESULT FindVerticesOnPoly(
		DWORD			dwPolyIndex,
		D3DXVECTOR3*	pVecVertices );// (out)頂点情報.

private:
	LPD3DXMESH m_Model;			// モデルのメッシュ.
	LPD3DXMESH m_ModelForRay;	// レイとの当たり判定用のメッシュ.
};
