#pragma once
#include "..\CameraBase.h"

/************************************************
*	三人称カメラクラス.
*		﨑田友輝.
**/
class CThirdPersonCamera final
	: public CCameraBase
{
public:
	CThirdPersonCamera();
	~CThirdPersonCamera();

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

};
