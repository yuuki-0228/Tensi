#pragma once
#include "..\CameraBase.h"

/************************************************
*	�ʏ�̃J�����N���X.
*		���c�F�P.
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
