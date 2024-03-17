#pragma once
#include "..\Object.h"

/************************************************
*	�����蔻��x�[�X�N���X.
*		���c�F�P.
**/
class CColliderBase 
	: public CObject
{
public:
	CColliderBase();
	virtual ~CColliderBase();

	// �t���O�𗧂Ă�.
	inline void SetFlagOn()  { m_HitFlag = true; }
	// �t���O���~�낷.
	inline void SetFlagOff() { m_HitFlag = false; }

	// �������Ă��邩.
	inline bool IsHit() { return m_HitFlag; }

private:
	bool m_HitFlag; // �����������ǂ���.
};
