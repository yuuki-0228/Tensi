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
// �����ʒu�̃|���S���̒��_��������.
//	�����_���W�̓��[�J�����W.
//----------------------------.
HRESULT CMesh::FindVerticesOnPoly(
	DWORD			dwPolyIndex,
	D3DXVECTOR3*	pVecVertices )
{
	// ���_���Ƃ̃o�C�g�����擾.
	DWORD dwStride		= m_ModelForRay->GetNumBytesPerVertex();
	// ���_�����擾.
	DWORD dwVertexAmt	= m_ModelForRay->GetNumVertices();
	// �ʐ����擾.
	DWORD dwPolyAmt		= m_ModelForRay->GetNumFaces();

	WORD* pwPoly		= nullptr;

	// �C���f�b�N�X�o�b�t�@�����b�N(�ǂݍ��݃��[�h).
	m_ModelForRay->LockIndexBuffer( D3DLOCK_READONLY, (VOID**) &pwPoly );

	BYTE*	pbVertices = nullptr;	// ���_(�o�C�g�^).
	FLOAT*	pfVertices = nullptr;	// ���_(float�^).
	LPDIRECT3DVERTEXBUFFER9 VB = nullptr;	// ���_�o�b�t�@.

	// ���_���̎擾.
	m_ModelForRay->GetVertexBuffer( &VB );

	// ���_�o�b�t�@�̃��b�N.
	if ( SUCCEEDED( VB->Lock( 0, 0, (VOID**) &pbVertices, 0 ) ) )
	{
		// �|���S���̒��_�̂P�ڂ��擾.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3]];
		pVecVertices[0].x = pfVertices[0];
		pVecVertices[0].y = pfVertices[1];
		pVecVertices[0].z = pfVertices[2];

		// �|���S���̒��_�̂Q�ڂ��擾.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3 + 1]];
		pVecVertices[1].x = pfVertices[0];
		pVecVertices[1].y = pfVertices[1];
		pVecVertices[1].z = pfVertices[2];

		// �|���S���̒��_�̂R�ڂ��擾.
		pfVertices = (FLOAT*) &pbVertices[dwStride*pwPoly[dwPolyIndex * 3 + 2]];
		pVecVertices[2].x = pfVertices[0];
		pVecVertices[2].y = pfVertices[1];
		pVecVertices[2].z = pfVertices[2];

		m_ModelForRay->UnlockIndexBuffer();	// ���b�N����.
		VB->Unlock();	// ���b�N����.
	}
	VB->Release();	// �s�v�ɂȂ����̂ŉ��.

	return S_OK;
}
