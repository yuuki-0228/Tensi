#pragma once
#include "..\CameraBase.h"

/************************************************
*	�O�l�̃J�����N���X.
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
