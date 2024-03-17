#pragma once
#include "..\Actor.h"
#include "..\..\Actor\WindowObjectManager\WindowObjectManager.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\Utility\WindowManager\WindowManager.h"

/************************************************
*	�E�B���h�E�̃I�u�W�F�N�g.
**/
class CWindowObject
	: public CActor
{
public:
	CWindowObject();
	virtual ~CWindowObject();

	// �\�����邩�̐ݒ�.
	void SetIsDisp( const bool Flag ) { m_IsDisp = Flag; }

	// �}�E�X�ɐG��Ă��邩
	bool GetIsMouseOver();

	// �E�B���h�E�̒��ɓ����Ă��邩.
	bool GetIsInWindow() { return m_InWndHandle != NULL; }

protected:
	// �E�B���h�E�I�u�W�F�N�g�̏�����.
	void WindowObjectInit();
	// �E�B���h�E�I�u�W�F�N�g�̍X�V.
	void WindowObjectUpdate();
	// �E�B���h�E�I�u�W�F�N�g�̓����蔻���̍X�V.
	void WindowObjectLateUpdate();

	// �͂߂邩�̊m�F.
	virtual bool GrabCheck() { return true; }
	// �͂񂾎��̏�����.
	virtual void GrabInit() {};
	// �݂͂̍X�V.
	virtual void GrabUpdate() {};
	// ���������̏�����.
	virtual void SeparateInit() {};

	// �ړ����̍X�V.
	virtual void MoveUpdate() {};

	// ��ɓ����������̍X�V.
	virtual void HitUpUpdate() {};
	// ���ɓ����������̍X�V.
	virtual void HitDownUpdate() {};
	// ���ɓ����������̍X�V.
	virtual void HitLeftUpdate() {};
	// �E�ɓ����������̍X�V.
	virtual void HitRightUpdate() {};

	// ���n���̏�����.
	virtual void LandingInit() {};
	// ���n�̍X�V.
	virtual void LandingUpdate() {};

private:
	// �͂�.
	void Grab();
	// �͂�ł��鎞�̍X�V.
	void GrabMoveUpdate();
	// �͂�ł��镨�𗣂�.
	void Separate();
	// �E�B���h�E�̒��ɓ���邩�̃`�F�b�N.
	void InWindowCheck();

	// ���ݔ��Ɉړ��ł��邩�m�F.
	void TeleportTrashCanCheck();
	// �E�B���h�E����o��.
	void EscapeWindow();

	// �d�͂̒ǉ�.
	void PushGravity();
	// �ړ��x�N�g���̍쐬.
	void CreateMoveVector();
	// �ړ��x�N�g���̏�����.
	void InitMoveVector();
	// �ړ�.
	void Move();

	// ���E�B���h�E�̓����蔻��.
	void WindowCollision();
	// ���ɓ����Ă���E�B���h�E�Ƃ̓����蔻��.
	void InWindowCollision();
	// �X�N���[���̓����蔻��.
	void ScreenCollision();

	// �㑤�ɓ�������.
	void HitUp( const RECT& HitWnd, const bool IsFromOutside = true );
	// �����ɓ�������.
	void HitDown( const RECT& HitWnd, const bool IsFromOutside = true );
	// �����ɓ�������.
	void HitLeft( const RECT& HitWnd, const bool IsFromOutside = true );
	// �E���ɓ�������.
	void HitRight( const RECT& HitWnd, const bool IsFromOutside = true );

	// ���n.
	void Landing( const RECT& HitWnd );

protected:
	CSprite*					m_pSprite;			// �摜.
	SSpriteRenderState			m_SpriteState;		// �摜�̏��.
	std::vector<D3DXVECTOR3>	m_VectorList;		// �x�N�g�����X�g.
	D3DXVECTOR3					m_OldMoveVector;	// �O��̈ړ��x�N�g��.
	D3DXVECTOR3					m_MoveVector;		// �ړ��x�N�g��.
	D3DXPOSITION2				m_GrabPosition;		// �݂͂̕␳�l.
	D3DXPOSITION3				m_StartPosition;	// �J�n���W.
	D3DXPOSITION3				m_EndPosition;		// �I�����W.
	D3DXVECTOR4					m_DireSpeedRate;	// �������Ƃ̑��x�̕ۑ���( �E,��,��,�� ).
	HWND						m_InWndHandle;		// ���ɓ����Ă���E�B���h�E�̃n���h��.
	RECT						m_LandingWnd;		// ���n���Ă���E�B���h�E���.
	RECT						m_AddWndRect;		// �E�B���h�E�̏��ɒǉ�����␳�l.
	float						m_CollSize;			// �����蔻��̃T�C�Y.
	float						m_GravityPower;		// �d�͂̋���.
	float						m_SpeedRate;		// ���x�̕ۑ���.
	float						m_CeilingPosY;		// �V���Y�ʒu.
	bool						m_IsLanding;		// ���n���Ă��邩.
	bool						m_IsFall;			// �������Ă��邩
	bool						m_IsGrab;			// �͂�ł��邩.
	bool						m_IsInWndSmall;		// ���ɓ����Ă�E�B���h�E���ŏ�������Ă��邩.
	bool						m_IsTrashCan;		// ���ݔ��Ɉړ������邩.
	bool						m_IsDisp;			// �\������邩.

private:
	D3DXPOSITION3				m_AddCenterPosition;// ���S���W�ɕϊ����邽�ߗp�̍��W.
};