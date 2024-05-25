#pragma once
#include "..\..\WindowObject.h"

/************************************************
*	�d���{�[���N���X.
**/
class CHeavyBall final
	: public CWindowObject
{
public:
	CHeavyBall();
	~CHeavyBall();

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

};