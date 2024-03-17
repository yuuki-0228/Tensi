#pragma once
#include "..\ColliderBase.h"

/************************************************
*	���b�V���N���X.
*		���c�F�P.
**/
class CMesh
	: public CColliderBase
{
public:
	CMesh();
	~CMesh();

	// ���b�V�����擾.
	LPD3DXMESH GetMesh() const { return m_Model; }
	// ���C�Ƃ̓����蔻��p�̃��b�V�����擾.
	LPD3DXMESH GetMeshForRay() const { return m_ModelForRay; }

	// ���b�V���̐ݒ�.
	void SetMesh( const std::pair<LPD3DXMESH, LPD3DXMESH>& Meshs ) {
		m_Model			= Meshs.first;
		m_ModelForRay	= Meshs.second;
	}
	void SetMesh( const LPD3DXMESH& Meshs ) {
		m_Model			= Meshs;
		m_ModelForRay	= Meshs;
	}

	// �����ʒu�̃|���S���̒��_��������.
	HRESULT FindVerticesOnPoly(
		DWORD			dwPolyIndex,
		D3DXVECTOR3*	pVecVertices );// (out)���_���.

private:
	LPD3DXMESH m_Model;			// ���f���̃��b�V��.
	LPD3DXMESH m_ModelForRay;	// ���C�Ƃ̓����蔻��p�̃��b�V��.
};
