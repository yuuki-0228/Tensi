#pragma once
#include "..\..\..\Actor.h"

/************************************************
*	�n�ʃN���X.
**/
class CGround
	: public CActor
{
public:
	CGround();
	~CGround();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �n�ʂ̐ݒu
	void Setting();

private:

};