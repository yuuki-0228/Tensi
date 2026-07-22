#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_MESH
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	‰ñ“]‚µ‚È‚¢‰~’ŒƒNƒ‰ƒX.
**/
class CCylinder final
	: public CColliderBase
{
public:
	CCylinder();
	virtual ~CCylinder();

	// “–‚½‚è”»’è‚Ì‘å‚«‚³‚É‚È‚éScale‚ğ•Ô‚·.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// ”¼Œa(’·‚³)‚ğæ“¾.
	inline float GetRadius() const {
		return m_Radius;
	}
	// ã‰º‚ÌãŒÀ‚ğæ“¾.
	inline float GetHeight() const {
		return m_Height;
	}

	// ƒƒbƒVƒ…‚É‡‚í‚µ‚Ä”¼Œa‚ğİ’è.
	HRESULT SetMeshRadius( const LPD3DXMESH& pMesh, const float vScale = 1.0f );
	// ”¼Œa(’·‚³)‚ğİ’è.
	inline void SetRadius( const float radius ) {
		m_Radius = radius;
	}
	// ‚‚³‚ğİ’è.
	inline void SetHeight( const float height ) {
		m_Height = height;
	}

private:
	float m_Radius;		// ”¼Œa(’·‚³).
	float m_Height;		// ‚‚³.
};

#endif