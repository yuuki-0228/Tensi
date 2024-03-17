#pragma once
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	���̃N���X.
*		���c�F�P.
**/
class CSphere final
	: public CColliderBase
{
public:
	CSphere();
	virtual ~CSphere();

	// �����蔻��̑傫���ɂȂ�Scale��Ԃ�.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// ���a(����)���擾.
	inline float GetRadius() const {
		return m_Radius;
	}

	// ���b�V���ɍ��킵�Ĕ��a��ݒ�.
	HRESULT SetMeshRadius( const CStaticMesh& pMesh, const float vScale = 1.0f );
	// ���a(����)��ݒ�.
	inline void SetRadius( const float radius ) {
		m_Radius  = radius;
	}

private:
	float m_Radius;		//���a(����).
};