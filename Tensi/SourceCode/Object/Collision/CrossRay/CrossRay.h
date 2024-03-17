#pragma once
#include "..\ColliderBase.h"
#include "..\Ray\Ray.h"

namespace {
	// レイの方向.
	enum class enCrossRayDire : unsigned char {
		ZF,	// 前.
		ZB,	// 後ろ.
		XL,	// 左.
		XR,	// 右.

		Max
	} typedef ECrossRayDire;
}

/************************************************
*	十字レイクラス.
*		﨑田友輝.
**/
class CCrossRay final
	: public CColliderBase
{
public:
	CCrossRay();
	~CCrossRay();

	// 長さの取得.
	inline float GetLength() { return m_Length; }
	// レイの取得.
	inline std::vector<CRay*> GetRay() {
		std::vector<CRay*> OutList;
		OutList.reserve( 4 );
		for ( auto& r : m_pRay ) OutList.emplace_back( r.get() );
		return OutList;
	}
	inline D3DXPOSITION3 GetPosition() const { return m_pRay[0]->GetPosition(); }

	// 座標設定.
	inline void SetPosition( const D3DXPOSITION3& Pos ) {
		m_Transform.Position = Pos; 
		for ( auto& r : m_pRay ) r->SetPosition( Pos );
	}
	// 回転設定.
	inline void SetRotation( const D3DXROTATION3& Rot ) {
		m_Transform.Rotation = Rot; 
		for ( auto& r : m_pRay ) r->SetRotation( Rot );
	}
	// 長さの設定.
	inline void SetLength( const float& Length ) {
		m_Length = Length;
		for ( auto& r : m_pRay ) r->SetLength( m_Length );
	}

private:
	std::vector<std::unique_ptr<CRay>>	m_pRay;		// レイ(4方向).
	float								m_Length;	// 長さ.

};
