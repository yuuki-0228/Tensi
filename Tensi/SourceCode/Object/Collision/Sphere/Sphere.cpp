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
// �����蔻��̑傫���ɂȂ�Scale��Ԃ�.
//---------------------------.
D3DXSCALE3 CSphere::GetCollisionScale( const LPD3DXMESH& pMesh )
{
	LPDIRECT3DVERTEXBUFFER9 pVB			= nullptr;		// ���_�o�b�t�@.
	void*					pVertices	= nullptr;		// ���_.
	D3DXPOSITION3			Center( 0.0f, 0.0f, 0.0f );	// ���S���W.
	float					Radius		= 0.0f;			// ���a.
	float					OutScale	= 0.0f;			// �{��.

	// ���_�o�b�t�@���擾.
	if ( FAILED(
		pMesh->GetVertexBuffer( &pVB ) ) )
	{
		return D3DXSCALE3( 0.0f, 0.0f, 0.0f );
	}

	// ���b�V���̒��_�o�b�t�@�����b�N����.
	if ( FAILED( pVB->Lock( 0, 0, &pVertices, 0 ) ) )
	{
		SAFE_RELEASE( pVB );
		return D3DXSCALE3( 0.0f, 0.0f, 0.0f );
	}

	// ���b�V���̊O�ډ~�̒��S�Ɣ��a���v�Z����.
	D3DXComputeBoundingSphere(
		static_cast<D3DXVECTOR3*>( pVertices ),
		pMesh->GetNumVertices(),					// ���_�̐�.
		D3DXGetFVFVertexSize( pMesh->GetFVF() ),	// ���_�̏��.
		&Center,									// (out)���S���W.
		&Radius );									// (out)���a.

	// ���b�V���̒��_�o�b�t�@���A�����b�N����.
	if ( pVB != nullptr ) {
		pVB->Unlock();
		SAFE_RELEASE( pVB );
	}

	// �{�����v�Z.
	OutScale	= m_Radius  / Radius;
	return D3DXSCALE3( OutScale, OutScale, OutScale );
}

//---------------------------.
// ���b�V���ɍ��킵�Ĕ��a��ݒ�.
//---------------------------.
HRESULT CSphere::SetMeshRadius( const CStaticMesh& pMesh, const float vScale )
{
	LPDIRECT3DVERTEXBUFFER9 pVB			= nullptr;		// ���_�o�b�t�@.
	void*					pVertices	= nullptr;		// ���_.
	D3DXPOSITION3			Center( 0.0f, 0.0f, 0.0f );	// ���S���W.
	float					Radius		= 0.0f;			// ���a.

	// ���_�o�b�t�@���擾.
	if (FAILED(
		pMesh.GetMesh()->GetVertexBuffer( &pVB )))
	{
		return E_FAIL;
	}
	
	// ���b�V���̒��_�o�b�t�@�����b�N����.
	if (FAILED(	pVB->Lock( 0, 0, &pVertices, 0 )))
	{
		SAFE_RELEASE( pVB );
		return E_FAIL;
	}

	// ���b�V���̊O�ډ~�̒��S�Ɣ��a���v�Z����.
	D3DXComputeBoundingSphere(
		static_cast<D3DXVECTOR3*>(pVertices),
		pMesh.GetMesh()->GetNumVertices(),					// ���_�̐�.
		D3DXGetFVFVertexSize(pMesh.GetMesh()->GetFVF()),	// ���_�̏��.
		&Center,											// (out)���S���W.
		&Radius );											// (out)���a.

	// ���b�V���̒��_�o�b�t�@���A�����b�N����.
	if (pVB != nullptr) {
		pVB->Unlock();
		SAFE_RELEASE( pVB );
	}

	// ���S���W�Ɣ��a��ݒ�.
	m_Radius  = Radius * vScale;
	return S_OK;
}
