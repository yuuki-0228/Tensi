#pragma once
#include "..\ColliderBase.h"

/************************************************
*	���C�N���X.
*		���c�F�P.
**/
class CRay final
	: public CColliderBase
{
public:
	CRay();
	~CRay();

	// �����x�N�g���̎擾.
	inline D3DXVECTOR3 GetVector() { return m_Vector; }
	// �����̎擾.
	inline float GetLength() { return m_Length; }

	// �����̒P�ʃx�N�g���̐ݒ�.
	inline void SetVector( const D3DXVECTOR3& Vec ) { m_Vector = Vec; }
	// �����̐ݒ�( Vector�ɒ������܂܂�Ă���ꍇ�ݒ肵�Ȃ��đ��v ).
	inline void SetLength( const float& Length ) { m_Length = Length; }

private:
	D3DXVECTOR3 m_Vector;	// �����x�N�g��.
	float		m_Length;	// ����.

};
