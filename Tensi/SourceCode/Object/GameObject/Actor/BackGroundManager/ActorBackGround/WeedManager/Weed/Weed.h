#pragma once
#include "..\..\ActorBackGround.h"
#include "WeedSaveData.h"

/************************************************
*	�G���N���X.
**/
class CWeed final
	: public CActorBackGround
{
public:
	CWeed();
	~CWeed();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void SubRender() override;

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) override;

	// ���̃f�[�^�̎擾.
	SWeedData GetWeedData();
	// ���̃f�[�^�̐ݒ�.
	void SetWeedData( const SWeedData& t );

	// �G���𖄂߂�.
	void Fill( const D3DXPOSITION3& Pos );

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() override;
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() override;

private:
	// �}�E�X�ɐG��Ă��邩.
	bool GetIsTouchMouse();

	// �G����h�炷.
	void Shake( const D3DXPOSITION3& Pos, const D3DXPOSITION3& OldPos );

	// �G����͂ނ̍X�V.
	void GrabUptate();
	// �}�E�X�ɂ��G���̗h��̍X�V.
	void MouseShakeUpdate();
	// �G����͂�ł��鎞�̍X�V.
	void GrabUpdate();
	// �G����������A�j���[�V�����̍X�V.
	void ComeOutAnimUptate();

private:
	CSprite*			m_pWeed;				// �G���̉摜.
	SSpriteRenderState	m_WeedState;			// �G���̉摜�̏��.
	D3DXPOSITION3		m_AddCenterPosition;	// ���S���W�ɕϊ����邽�ߗp�̍��W.
	float				m_WeedSize;				// �G���̉摜�T�C�Y.
	bool				m_IsGrab;				// ����ł��邩.
	bool				m_IsComeOut;			// �G���������邩.
};