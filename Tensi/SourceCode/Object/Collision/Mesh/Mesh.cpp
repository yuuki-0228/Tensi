#include "Mesh.h"

CMesh::CMesh()
	: m_Model		( nullptr )
	, m_ModelForRay	( nullptr )
{
}

CMesh::~CMesh()
{
}

//----------------------------.
// 交差位置のポリゴンの頂点を見つける.
//	※頂点座標はローカル座標.
//----------------------------.
HRESULT CMesh::FindVerticesOnPoly(
	DWORD			dwPolyIndex,
	D3DXVECTOR3*	pVecVertices )
{
	// 頂点ごとのバイト数を取得.
	DWORD dwStride		= m_ModelForRay->GetNumBytesPerVertex();
	// 頂点数を取得.
	DWORD dwVertexAmt	= m_ModelForRay->GetNumVertices();
	// 面数を取得.
	DWORD dwPolyAmt		= m_ModelForRay->GetNumFaces();

	WORD* pwPoly		= nullptr;

	// インデックスバッファをロック(読み込みモード).
	m_ModelForRay->LockIndexBuffer( D3DLOCK_READONLY, (VOID**) &pwPoly );

	BYTE*	pbVertices = nullptr;	// 頂点(バイト型).
	FLOAT*	pfVertices = nullptr;	// 頂点(float型).
	LPDIRECT3DVERTEXBUFFER9 VB = nullptr;	// 頂点バッファ.

	// 頂点情報の取得.
	m_ModelForRay->GetVertexBuffer( &VB );

	// 頂点バッファのロック.
	if ( SUCCEEDED( VB->Lock( 0, 0, (VOID**) &pbVertices, 0 ) ) )
	{
		// ポリゴンの頂点の１つ目を取得.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3]];
		pVecVertices[0].x = pfVertices[0];
		pVecVertices[0].y = pfVertices[1];
		pVecVertices[0].z = pfVertices[2];

		// ポリゴンの頂点の２つ目を取得.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3 + 1]];
		pVecVertices[1].x = pfVertices[0];
		pVecVertices[1].y = pfVertices[1];
		pVecVertices[1].z = pfVertices[2];

		// ポリゴンの頂点の３つ目を取得.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3 + 2]];
		pVecVertices[2].x = pfVertices[0];
		pVecVertices[2].y = pfVertices[1];
		pVecVertices[2].z = pfVertices[2];

		m_ModelForRay->UnlockIndexBuffer();	// ロック解除.
		VB->Unlock();	// ロック解除.
	}
	VB->Release();	// 不要になったので解放.

	return S_OK;
}
