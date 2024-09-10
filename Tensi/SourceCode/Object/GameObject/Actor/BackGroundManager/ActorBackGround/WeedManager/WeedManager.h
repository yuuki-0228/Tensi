#pragma once
#include "..\..\..\Actor.h"
#include "Weed/WeedSaveData.h"

class CWeed;

/************************************************
*	�G���}�l�[�W���[�N���X.
**/
class CWeedManager final
	: public CActor
{
public:
	CWeedManager();
	~CWeedManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �G���𖄂߂�.
	void Fill( const int Num );

	// �\�����Ă���{���̎擾
	int GetNum();

private:
	// �ۑ�.
	std::vector<SWeedData> Save();
	//�@�ǂݍ���.
	void Load( std::vector<SWeedData> Data );

	// �G�����X�g�̍폜.
	void WeedListDelete();

private:
	std::vector<std::unique_ptr<CWeed>> m_pWeedList;	// �G�����X�g.
	float								m_WeedSize;		// �G���̉摜�T�C�Y.
};