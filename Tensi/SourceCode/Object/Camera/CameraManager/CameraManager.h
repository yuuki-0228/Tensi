#pragma once
#include "..\..\..\Global.h"
#include "..\..\Camera\CameraBase.h"
#include "..\..\Collision\Ray\Ray.h"
#include <functional>

class CFreeCamera;

/************************************************
*	�J�����}�l�[�W���[�N���X.
*		���c�F�P.
**/
class CameraManager final
{
public:
	CameraManager();
	~CameraManager();

	// �X�V.
	static void Update( float DeltaTime );

	// �J�����̐ݒ�.
	static void SetCamera( CCameraBase* pCamera );
	// �J�����̕ύX���֎~�����邩.
	static void SetIsCameraChangeLock( const bool Flag ) { GetInstance()->m_IsCameraChangeLock = Flag; }

	// �����x�N�g���̎擾.
	static D3DXVECTOR3 GetDireVector() { return GetInstance()->m_DireVec; }
	// ���_�̎擾.
	static D3DXPOSITION3 GetPosition() { return GetInstance()->m_pNowCamera->GetPosition(); }
	// ���_���W�̎擾.
	static D3DXPOSITION3 GetLookPosition() { return GetInstance()->m_pNowCamera->GetLookPosition(); }
	// ��]�̎擾.
	static D3DXROTATION3 GetRotation() { return GetInstance()->m_pNowCamera->GetRotation(); }
	// �r���[�s��擾.
	static D3DXMATRIX GetViewMatrix() { return GetInstance()->m_pNowCamera->GetViewMatrix(); }
	// �v���W�F�N�V�����s��擾.
	static D3DXMATRIX GetProjMatrix() { return GetInstance()->m_pNowCamera->GetProjMatrix(); }
	// �r���[�E�v���W�F�N�V�����̍����s��擾.
	static D3DXMATRIX GetViewProjMatrix() { return GetInstance()->m_pNowCamera->GetViewProjMatrix(); }

	// ���C�̎擾.
	static CRay* GetRay() { return GetInstance()->m_pRay.get(); }

	// �J�����̐U��(�c�h��)�̐ݒ�.
	static void SetPitch( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );
	// �J�����̐U��(���h��)�̐ݒ�.
	static void SetRolling( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );
	// �J�����̐U��(�~�b�N�X)�̐ݒ�.
	static void SetMixShaking( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );

	// �q�b�g�X�g�b�v�̐ݒ�.
	static void HitStop( const float Time, const bool IsSlow = false, const std::function<void()>& EndProc = {}, const std::function<void()>& SlowProc = {} );

private:
	// �C���X�^���X�̎擾.
	static CameraManager* GetInstance();

	// �t���[�J�����ɕύX.
	static void ChangeFreeCamera();
	// �ݒ肳��Ă���J�����ɕύX.
	static void ChangeSetCamera();

	// �h��̍X�V.
	static void ShakingUpdate();
	// �c�h��̍X�V.
	static D3DXPOSITION3 PitchUpdate();
	// ���h��̍X�V.
	static D3DXPOSITION3 RollingUpdate();

private:
	CCameraBase*					m_pNowCamera;			// ���݂̃J����.
	CCameraBase*					m_pTmpCamera;			// �J������ύX���Ɉꎞ�J������ۊǂ���.
	std::unique_ptr<CFreeCamera>	m_pFreeCamera;			// ���R�ɓ������J����.
	std::unique_ptr<CRay>			m_pRay;					// �J�����̌����Ă�������̃��C.
	SCameraState					m_FreeCameraState;		// �t���[�J�����̏��.
	SCameraState					m_TmpCameraState;		// �J�����̕ύX���Ɉꎞ�I�ɃJ�����̏�Ԃ�ۊǂ���.
	D3DXVECTOR3						m_DireVec;				// �J�����̌����x�N�g��.
	std::function<void()>			m_HitStopEndProc;		// �q�b�g�X�g�b�v���I���������ɍs���֐�.
	std::function<void()>			m_HitStopSlowProc;		// �q�b�g�X�g�b�v���X���[�ɐ؂�ւ��鎞�ɍs���֐�.
	int								m_ShakingNo;			// �J�����̐U���̎��.
	int								m_ShakingNum;			// �J�����̐U����.
	float							m_DeltaTime;			// �f���^�^�C��.
	float							m_ShakingTimeCnt;		// �J�����̐U���̎��ԗp�̃J�E���g.
	float							m_ShakingTimeMax;		// �J�����̐U������.
	float							m_ShakingMax;			// �J�����̗h��̍ő�l.
	float							m_ShakingMin;			// �J�����̗h��̍ŏ��l.
	bool							m_IsHitStop;			// �q�b�g�X�g�b�v����.
	bool							m_IsHitStopSlow;		// �q�b�g�X�g�b�v���X���[�������邩.
	bool							m_IsFreeCamera;			// �t���[�J�������ǂ���.
	bool							m_IsCameraChangeLock;	// �J�����̕ύX���֎~���邩.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	CameraManager( const CameraManager & ) = delete;
	CameraManager& operator = ( const CameraManager & ) = delete;
	CameraManager( CameraManager && ) = delete;
	CameraManager& operator = ( CameraManager && ) = delete;
};
