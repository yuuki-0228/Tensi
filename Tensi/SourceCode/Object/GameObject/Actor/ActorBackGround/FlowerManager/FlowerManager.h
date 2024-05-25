#pragma once
#include "..\..\Actor.h"
#include "Flower/FlowerSaveData.h"

class CFlower;

/************************************************
*	�ԃ}�l�[�W���[�N���X.
**/
class CFlowerManager final
	: public CActor
{
public:
	CFlowerManager();
	~CFlowerManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �Ԃ𖄂߂�.
	void Fill( const int Num );

	// �\�����Ă���{���̎擾
	int GetNum();

private:
	// �ۑ�.
	std::vector<SFlowerData> Save();
	//�@�ǂݍ���.
	void Load( std::vector<SFlowerData> Data );

	// �ԃ��X�g�̍폜.
	void FlowerListDelete();

	// �Ԃ̃A�j���[�V�����p�J�E���g�̍X�V.
	void FlowerAnimCntUpdate();

private:
	std::vector<std::unique_ptr<CFlower>>	m_pFlowerList;		// �ԃ��X�g.
	float									m_FlowerSize;		// �Ԃ̉摜�T�C�Y.
	float									m_FlowerAnimCnt;	// �Ԃ̃A�j���[�V�����p�J�E���g.
};
