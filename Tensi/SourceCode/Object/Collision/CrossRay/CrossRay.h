#pragma once
#include "..\ColliderBase.h"
#include "..\Ray\Ray.h"

namespace {
	// ���C�̕���.
	enum class enCrossRayDire : unsigned char {
		ZF,	// �O.
		ZB,	// ���.
		XL,	// ��.
		XR,	// �E.

		Max
	} typedef ECrossRayDire;
}

/************************************************
*	�\�����C�N���X.
*		���c�F�P.
**/
class CCrossRay final
	: public CColliderBase
{
public:
	CCrossRay();
	~CCrossRay();

	// �����̎擾.
	inline float GetLength() { return m_Length; }
	// ���C�̎擾.
	inline std::vector<CRay*> GetRay() {
		std::vector<CRay*> OutList;
		OutList.reserve( 4 );
		for ( auto& r : m_pRay ) OutList.emplace_back( r.get() );
		return OutList;
	}
	inline D3DXPOSITION3 GetPosition() const { return m_pRay[0]->GetPosition(); }

	// ���W�ݒ�.
	inline void SetPosition( const D3DXPOSITION3& Pos ) {
		m_Transform.Position = Pos; 
		for ( auto& r : m_pRay ) r->SetPosition( Pos );
	}
	// ��]�ݒ�.
	inline void SetRotation( const D3DXROTATION3& Rot ) {
		m_Transform.Rotation = Rot; 
		for ( auto& r : m_pRay ) r->SetRotation( Rot );
	}
	// �����̐ݒ�.
	inline void SetLength( const float& Length ) {
		m_Length = Length;
		for ( auto& r : m_pRay ) r->SetLength( m_Length );
	}

private:
	std::vector<std::unique_ptr<CRay>>	m_pRay;		// ���C(4����).
	float								m_Length;	// ����.

};
