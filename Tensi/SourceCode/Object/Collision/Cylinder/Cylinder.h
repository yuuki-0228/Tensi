#pragma once
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	��]���Ȃ��~���N���X.
*		���c�F�P.
**/
class CCylinder final
	: public CColliderBase
{
public:
	CCylinder();
	virtual ~CCylinder();

	// �����蔻��̑傫���ɂȂ�Scale��Ԃ�.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// ���a(����)���擾.
	inline float GetRadius() const {
		return m_Radius;
	}
	// �㉺�̏�����擾.
	inline float GetHeight() const {
		return m_Height;
	}

	// ���b�V���ɍ��킵�Ĕ��a��ݒ�.
	HRESULT SetMeshRadius( const LPD3DXMESH& pMesh, const float vScale = 1.0f );
	// ���a(����)��ݒ�.
	inline void SetRadius( const float radius ) {
		m_Radius = radius;
	}
	// ������ݒ�.
	inline void SetHeight( const float height ) {
		m_Height = height;
	}

private:
	float m_Radius;		// ���a(����).
	float m_Height;		// ����.
};