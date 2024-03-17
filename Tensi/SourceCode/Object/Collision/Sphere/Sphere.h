#pragma once
#include "..\ColliderBase.h"

class CStaticMesh;

/**************************************
*	‹…‘ÌƒNƒ‰ƒX.
*		ú±“c—F‹P.
**/
class CSphere final
	: public CColliderBase
{
public:
	CSphere();
	virtual ~CSphere();

	// “–‚½‚è”»’è‚Ì‘å‚«‚³‚É‚È‚éScale‚ğ•Ô‚·.
	D3DXSCALE3 GetCollisionScale( const LPD3DXMESH& pMesh );

	// ”¼Œa(’·‚³)‚ğæ“¾.
	inline float GetRadius() const {
		return m_Radius;
	}

	// ƒƒbƒVƒ…‚É‡‚í‚µ‚Ä”¼Œa‚ğİ’è.
	HRESULT SetMeshRadius( const CStaticMesh& pMesh, const float vScale = 1.0f );
	// ”¼Œa(’·‚³)‚ğİ’è.
	inline void SetRadius( const float radius ) {
		m_Radius  = radius;
	}

private:
	float m_Radius;		//”¼Œa(’·‚³).
};