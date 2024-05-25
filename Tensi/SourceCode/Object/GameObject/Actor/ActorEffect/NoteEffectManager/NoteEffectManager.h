#pragma once
#include "..\..\Actor.h"

class CNoteEffect;

/************************************************
*	�����G�t�F�N�g�}�l�[�W���[.
**/
class CNoteEffectManager final
	: public CActor
{
public:
	CNoteEffectManager();
	~CNoteEffectManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �A�j���[�V�����̊J�n.
	void Play( const float Size );
	// �ꎞ��~.
	void Pause( const bool Flag );
	// �A�j���[�V�����̒�~.
	void Stop();

private:
	// �����G�t�F�N�g�̍Đ�.
	void NoteEffectPlay();

	// �����G�t�F�N�g���X�g�̍폜.
	void NoteEffectListDelete();

private:
	std::vector<std::unique_ptr<CNoteEffect>>	m_pNoteEffectList;	// �����G�t�F�N�g���X�g.
	float										m_CoolTime;			// �N�[���^�C��.
	float										m_EffectSize;		// �G�t�F�N�g�̃T�C�Y.
	bool										m_IsAnimation;		// �A�j���[�V��������.
	bool										m_IsPause;			// �|�[�Y����.
};