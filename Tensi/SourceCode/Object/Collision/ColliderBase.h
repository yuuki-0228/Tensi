#pragma once
#include "..\Object.h"

/************************************************
*	当たり判定ベースクラス.
*		﨑田友輝.
**/
class CColliderBase 
	: public CObject
{
public:
	CColliderBase();
	virtual ~CColliderBase();

	// フラグを立てる.
	inline void SetFlagOn()  { m_HitFlag = true; }
	// フラグを降ろす.
	inline void SetFlagOff() { m_HitFlag = false; }

	// 当たっているか.
	inline bool IsHit() { return m_HitFlag; }

private:
	bool m_HitFlag; // 当たったかどうか.
};
