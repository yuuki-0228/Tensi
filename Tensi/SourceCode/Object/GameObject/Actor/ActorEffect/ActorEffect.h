#pragma once
#include "../Actor.h"

/************************************************
*	�G�t�F�N�g�N���X.
**/
class CActorEffect
	: public CActor
{
public:
	CActorEffect();
	~CActorEffect();

	// �A�j���[�V�����̊J�n
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) {}
	// �A�j���[�V�����̒�~
	virtual void Stop() { m_IsAnimStop = true; }
	// �A�j���[�V�����̍ĊJ
	virtual void ReStart() { m_IsAnimStop = false; }

	// �A�j���[�V�������Đ�����
	bool GetIsAnimPlay() { return m_IsAnimEnd == false; }
	// �A�j���[�V�����̍Đ����I��������
	bool GetIsAnimEnd() { return m_IsAnimEnd == true; }

protected:
	// �A�j���[�V�����̏�����
	void AnimInit();

protected:
	CSprite*			m_pSprite;		// �摜.
	SSpriteRenderState	m_SpriteState;	// �摜�̏��.
	bool				m_IsAnimStop;	// ��~����
	bool				m_IsAnimEnd;	// �A�j���[�V�������I��������
	float				m_NowAnimPoint;	// ���݂̃A�j���[�V�����̈ʒu.

};