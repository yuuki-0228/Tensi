#pragma once
#include "..\..\Actor.h"

class CSleepEffect;

/************************************************
*	�����G�t�F�N�g�}�l�[�W���[�N���X.
**/
class CSleepEffectManager final
	: public CActor
{
public:
	CSleepEffectManager();
	~CSleepEffectManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �A�j���[�V�����̊J�n.
	void Play();
	// �ꎞ��~.
	void Pause( const bool Flag );
	// �A�j���[�V�����̒�~.
	void Stop();

private:
	// �т�����G�t�F�N�g�̍Đ�.
	void SurprisedEffect();
	// ����G�t�F�N�g�̍Đ�.
	void SleepEffectPlay();

	// ����G�t�F�N�g���X�g�̍폜.
	void SleepEffectListDelete();

private:
	CSprite*									m_pSurprisedEffect;		// �т�����G�t�F�N�g.
	SSpriteRenderState							m_SurprisedEffectState;	// �т�����G�t�F�N�g�̏��.
	std::vector<std::unique_ptr<CSleepEffect>>	m_pSleepEffectList;		// �����G�t�F�N�g���X�g.
	float										m_CoolTime;				// �N�[���^�C��.
	float										m_EffectAnimAng;		// �т�����G�t�F�N�g�A�j���[�V�����p�̊p�x.
	bool										m_IsAnimation;			// �A�j���[�V��������.
	bool										m_IsPause;				// �|�[�Y����.
};