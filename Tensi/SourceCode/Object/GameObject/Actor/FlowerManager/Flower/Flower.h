#pragma once
#include "..\..\Actor.h"
#include "FlowerSaveData.h"

class CWateringAnimManager;

/************************************************
*	�ԃN���X.
**/
class CFlower final
	: public CActor
{
public:
	// �Ԃ̎��
	enum class enFlowerType : unsigned char {
		A, B, C, D, E, F,
		Max
	} typedef EFlowerType;

public:
	CFlower();
	~CFlower();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) override;

	// �Ԃ̃f�[�^���擾.
	SFlowerData GetFlowerData();
	// �Ԃ̃f�[�^��ݒ�.
	void SetFlowerData( const SFlowerData& Data );

	// �Ԃ̃A�j���[�V�����p�J�E���g�̐ݒ�.
	void SetFlowerAnim( const float Cnt ) { m_FlowerAnimCnt = Cnt; }

	// �Ԃ𖄂߂�.
	void Fill( const int Type, const D3DXCOLOR3& Color, const D3DXPOSITION3& Pos );

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

	// �Ԃɉ����������̃��A�N�V�����̐ݒ�.
	void Reaction();
	// �͂ꂳ����.
	void Wither();

	// �Ԃ��N���b�N�������̍X�V
	void FlowerClickUpdate();
	// �Ԃ̃A�j���[�V�����̍X�V
	void FlowerAnimUpdate();
	// �}�E�X�ɂ��Ԃ̃A�j���[�V�����̍X�V.
	void MouseTouchReactionUpdate();
	// �ԂɐG�ꂽ���̃A�j���[�V�����̍X�V.
	void ReactionAnimUpdate();
	// ����芮���A�j���[�V�����̑ҋ@���Ԃ̍X�V.
	void WateringAnimCoolTimeUpdate();
	// ����芮���A�j���[�V�����̍X�V.
	void WateringAnimUpdate();

	// �v���C���[�ƃ{�[���̓����蔻��
	void PlayerBallCollision( CActor* pActor, const EObjectTag Tag );
	// ���̓����蔻��
	void WaterCollision( CActor* pActor, const EObjectTag Tag );

private:
	CSprite*								m_pFlower;				// �Ԃ̉摜.
	SSpriteRenderState						m_LeafState;			// �t���ς̉摜�̏��.
	SSpriteRenderState						m_FlowerState;			// �Ԃ̉摜�̏��.
	SFlowerData								m_FlowerSaveData;		// �Ԃ̃Z�[�u�f�[�^.
	std::unique_ptr<CWateringAnimManager>	m_pWateringAnim;		// �����A�j���[�V����.
	D3DXPOSITION3							m_AddCenterPosition;	// ���S���W�ɕϊ����邽�ߗp�̍��W.
	float									m_FlowerSize;			// �Ԃ̉摜�T�C�Y.
	float									m_FlowerAnimCnt;		// �Ԃ̃A�j���[�V�����p�J�E���g.
	float									m_ReactionAnimCnt;		// �Ԃ̐G�ꂽ���̃A�j���[�V�����p�J�E���g.
	float									m_WateringAnimCnt;		// ������A�j���[�V�����p�J�E���g.
	bool									m_IsReactionAnim;		// �ԂɐG�ꂽ���̃A�j���[�V����.
	int										m_WateringCnt;			// �����J�E���g
};