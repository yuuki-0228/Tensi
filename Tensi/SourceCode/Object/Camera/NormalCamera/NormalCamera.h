#pragma once
#include "..\CameraBase.h"

/************************************************
*	�ʏ�̃J�����N���X.
**/
class CNormalCamera final
	: public CCameraBase
{
public:
	CNormalCamera();
	~CNormalCamera();

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

};
