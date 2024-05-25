#pragma once
#include "..\WindowObject.h"

class CSleepEffectManager;
class CNoteEffectManager;

/************************************************
*	�v���C���[�N���X.
**/
class CPlayer final
	: public CWindowObject
{
public:
	CPlayer();
	~CPlayer();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;
	// �����蔻��I����Ăяo�����X�V.
	virtual void LateUpdate( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) override;

	// �X���C�����Ԃ�.
	void Collapsed();

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() override;
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() override;

	// �͂߂邩�̊m�F.
	virtual bool GrabCheck() override;
	// �͂񂾎��̏�����.
	virtual void GrabInit() override;

	// �ړ����̍X�V.
	virtual void MoveUpdate() override;

	// ��ɓ����������̍X�V.
	virtual void HitUpUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitDownUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitLeftUpdate() override;
	// �E�ɓ����������̍X�V.
	virtual void HitRightUpdate() override;

	// ���n���̏�����.
	virtual void LandingInit() override;
	// ���n�̍X�V.
	virtual void LandingUpdate() override;

private:
	// �G�t�F�N�g�̍X�V.
	void EffectUpdate();

	// �s���̏I�����̏�����.
	void ActionEnd();
	// �s���̊J�n�̏�����.
	void ActionStart();
	// �s���̍X�V.
	void ActionUpdate();
	// �ҋ@�̍s���̍X�V.
	void ActionWaitUpdate();
	// �ړ��̍s���̍X�V.
	void ActionMoveUpdate();
	// �{�[���ŗV�ԍs���̍X�V.
	void ActionPlayWithBallUpdate();
	// �W�����v�̍s���̍X�V.
	void ActionJumpUpdate();
	// �Q��̍s���̍X�V.
	void ActionSleepUpdate();
	// �������Ă�Ƃ��̍s���̍X�V.
	void ActionMusicUpdate();

private:
	CSprite*								m_pCollapsed;		// �Ԃꂽ�摜.
	SSpriteRenderState						m_CollapsedState;	// �Ԃꂽ�摜�̏��.
	std::unique_ptr<CSleepEffectManager>	m_pSleepEffect;		// ����G�t�F�N�g.
	std::unique_ptr<CNoteEffectManager>		m_pNoteEffect;		// �����G�t�F�N�g.
	float									m_ActionTime;		// �s���̎���.
	float									m_MoveTimeCnt;		// �ړ�����.
	float									m_CoolTime;			// �N�[���^�C��.
	int										m_Action;			// ���݂̍s��.
	bool									m_IsLandingWait;	// ���n�d������.
	bool									m_IsOldMoveLeft;	// �O�񍶕����Ɉړ����Ă�����.
};