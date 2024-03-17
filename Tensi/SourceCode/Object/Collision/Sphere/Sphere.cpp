#include "Sphere.h"
#include "..\..\..\Common\Mesh\StaticMesh\StaticMesh.h"

CSphere::CSphere()
	: m_Radius	()
{
}

CSphere::~CSphere()
{
}

//---------------------------.
// 当たり判定の大きさになるScaleを返す.
//---------------------------.
D3DXSCALE3 CSphere::GetCollisionScale( const LPD3DXMESH& pMesh )
{
	LPDIRECT3DVERTEXBUFFER9 pVB			= nullptr;		// 頂点バッファ.
	void*					pVertices	= nullptr;		// 頂点.
	D3DXPOSITION3			Center( 0.0f, 0.0f, 0.0f );	// 中心座標.
	float					Radius		= 0.0f;			// 半径.
	float					OutScale	= 0.0f;			// 倍率.

	// 頂点バッファを取得.
	if ( FAILED(
		pMesh->GetVertexBuffer( &pVB ) ) )
	{
		return D3DXSCALE3( 0.0f, 0.0f, 0.0f );
	}

	// メッシュの頂点バッファをロックする.
	if ( FAILED( pVB->Lock( 0, 0, &pVertices, 0 ) ) )
	{
		SAFE_RELEASE( pVB );
		return D3DXSCALE3( 0.0f, 0.0f, 0.0f );
	}

	// メッシュの外接円の中心と半径を計算する.
	D3DXComputeBoundingSphere(
		static_cast<D3DXVECTOR3*>( pVertices ),
		pMesh->GetNumVertices(),					// 頂点の数.
		D3DXGetFVFVertexSize( pMesh->GetFVF() ),	// 頂点の情報.
		&Center,									// (out)中心座標.
		&Radius );									// (out)半径.

	// メッシュの頂点バッファをアンロックする.
	if ( pVB != nullptr ) {
		pVB->Unlock();
		SAFE_RELEASE( pVB );
	}

	// 倍率を計算.
	OutScale	= m_Radius  / Radius;
	return D3DXSCALE3( OutScale, OutScale, OutScale );
}

//---------------------------.
// メッシュに合わして半径を設定.
//---------------------------.
HRESULT CSphere::SetMeshRadius( const CStaticMesh& pMesh, const float vScale )
{
	LPDIRECT3DVERTEXBUFFER9 pVB			= nullptr;		// 頂点バッファ.
	void*					pVertices	= nullptr;		// 頂点.
	D3DXPOSITION3			Center( 0.0f, 0.0f, 0.0f );	// 中心座標.
	float					Radius		= 0.0f;			// 半径.

	// 頂点バッファを取得.
	if (FAILED(
		pMesh.GetMesh()->GetVertexBuffer( &pVB )))
	{
		return E_FAIL;
	}
	
	// メッシュの頂点バッファをロックする.
	if (FAILED(	pVB->Lock( 0, 0, &pVertices, 0 )))
	{
		SAFE_RELEASE( pVB );
		return E_FAIL;
	}

	// メッシュの外接円の中心と半径を計算する.
	D3DXComputeBoundingSphere(
		static_cast<D3DXVECTOR3*>(pVertices),
		pMesh.GetMesh()->GetNumVertices(),					// 頂点の数.
		D3DXGetFVFVertexSize(pMesh.GetMesh()->GetFVF()),	// 頂点の情報.
		&Center,											// (out)中心座標.
		&Radius );											// (out)半径.

	// メッシュの頂点バッファをアンロックする.
	if (pVB != nullptr) {
		pVB->Unlock();
		SAFE_RELEASE( pVB );
	}

	// 中心座標と半径を設定.
	m_Radius  = Radius * vScale;
	return S_OK;
}
