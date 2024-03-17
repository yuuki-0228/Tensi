#pragma once
#include "..\..\..\Actor.h"

class CWateringAnim;

/************************************************
*	�Ԃ̐����G�t�F�N�g�}�l�[�W���[�N���X.
**/
class CWateringAnimManager final
	: public CActor
{
public:
	CWateringAnimManager();
	~CWateringAnimManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �Đ�
	void Play( const D3DXPOSITION3& Pos );

private:
	// �����X�g�̍폜.
	void WateringListDelete();

private:
	std::vector<std::unique_ptr<CWateringAnim>> m_pWateringList;	// �����.

};
