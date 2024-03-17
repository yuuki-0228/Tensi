#pragma once
#include "..\ColliderBase.h"

/************************************************
*	レイクラス.
*		﨑田友輝.
**/
class CRay final
	: public CColliderBase
{
public:
	CRay();
	~CRay();

	// 方向ベクトルの取得.
	inline D3DXVECTOR3 GetVector() { return m_Vector; }
	// 長さの取得.
	inline float GetLength() { return m_Length; }

	// 方向の単位ベクトルの設定.
	inline void SetVector( const D3DXVECTOR3& Vec ) { m_Vector = Vec; }
	// 長さの設定( Vectorに長さが含まれている場合設定しなくて大丈夫 ).
	inline void SetLength( const float& Length ) { m_Length = Length; }

private:
	D3DXVECTOR3 m_Vector;	// 方向ベクトル.
	float		m_Length;	// 長さ.

};
