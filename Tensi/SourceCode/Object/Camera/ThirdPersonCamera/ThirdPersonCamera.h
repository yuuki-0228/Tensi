#pragma once
#include "..\CameraBase.h"

/************************************************
*	�O�l�̃J�����N���X.
*		���c�F�P.
**/
class CThirdPersonCamera final
	: public CCameraBase
{
public:
	CThirdPersonCamera();
	~CThirdPersonCamera();

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

};
