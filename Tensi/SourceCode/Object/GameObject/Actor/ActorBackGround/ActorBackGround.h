#pragma once
#include "..\Actor.h"

/************************************************
*	�w�i�ɕ\������I�u�W�F�N�g�̃x�[�X�N���X.
**/
class CActorBackGround
	: public CActor
{
public:
	CActorBackGround();
	~CActorBackGround();

	// �w�i�I�u�W�F�N�g�͎��"Render()"���g�p����"SubRender()"���g�p����
	virtual void Render() override {};
	virtual void SubRender() override = 0;

private:

};