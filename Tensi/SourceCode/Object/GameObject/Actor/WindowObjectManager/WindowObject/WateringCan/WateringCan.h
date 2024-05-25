#pragma once
#include "..\WindowObject.h"
#include "..\WaterManager/WaterData.h"

class CWaterManager;

/************************************************
*	�W���E���N���X.
**/
class CWateringCan final
	: public CWindowObject
{
public:
	CWateringCan();
	~CWateringCan();

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

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() override;
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() override;

	// �݂͂̍X�V.
	virtual void GrabUpdate();
	// ���������̏�����.
	virtual void SeparateInit();

	// ��ɓ����������̍X�V.
	virtual void HitUpUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitDownUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitLeftUpdate() override;
	// �E�ɓ����������̍X�V.
	virtual void HitRightUpdate() override;

private:
	// �g�����X�t�H�[���̏�����
	void InitTransform();
	// ���̃f�[�^�̏�����.
	void InitWaterData();

	// �������x�̍X�V.
	void FallSpeedUpdate();
	// �Q�[�W�̍X�V
	void GaugeUpdate();
	// �N�[���^�C���̍X�V.
	void CoolTimeUpdate();
	// �����̍X�V.
	void WateringUpdate();

	// �������炷
	float DecreaseWater( const float rate );
	// ���̗ʂ�ϓ�������
	void ChangeWaterValue( const float value );

	// ���^���̎��̐��̃G�t�F�N�g�̍Đ�.
	void FullWaterPlay();
	// �n�ʂɂԂ��������̐��̃G�t�F�N�g�̍Đ�
	void HitWaterPlay( const float rate, const float decWater );
	// �����̐��̃G�t�F�N�g�̍Đ�
	void WateringWaterPlay();

private:
	CSprite*						m_pWaterGaugeSprite;	// �c��̐��Q�[�W�̉摜
	SSpriteRenderState				m_WaterGaugeBackState;	// �c��̐��Q�[�W�̔w�i���.
	SSpriteRenderState				m_WaterGaugeState;		// �c��̐��Q�[�W�̏��.
	std::unique_ptr<CWaterManager>	m_pWaterEffect;			// ��.
	STransform						m_LeftWaterTransform;	// ���̐������ڂ��ꏊ.
	STransform						m_RightWaterTransform;	// �E�̐������ڂ��ꏊ.
	STransform						m_CenterWaterTransform;	// �^�񒆂̏ꏊ.
	STransform						m_WateringTransform;	// �����̐��̏ꏊ.
	SWaterData						m_LHitWaterData;		// ���̗������̐��̃f�[�^.
	SWaterData						m_RHitWaterData;		// �E�̗������̐��̃f�[�^.
	SWaterData						m_LFullWaterData;		// ���̖��^�����̐��̃f�[�^.
	SWaterData						m_RFullWaterData;		// �E�̖��^�����̐��̃f�[�^.
	SWaterData						m_WateringWaterData;	// ����莞�̐��̃f�[�^.
	float							m_WaterValue;			// �c��̐��̗�.
	float							m_FullAnimCoolTime;		// ���^���̎��̃A�j���[�V�����̑ҋ@����(�b)
	float							m_GaugeDispTime;		// �Q�[�W��\�����Ă��鎞��
	float							m_GaugeAlpha;			// ���Q�[�W�̃A���t�@�l
	bool							m_IsWatering;			// ����蒆��.
	bool							m_IsGaugeDisp;			// �Q�[�W�𖳗����\������.
};
