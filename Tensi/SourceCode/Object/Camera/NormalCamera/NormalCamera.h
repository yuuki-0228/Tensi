#pragma once
#include "..\CameraBase.h"

/************************************************
*	通常のカメラクラス.
**/
class CNormalCamera final
	: public CCameraBase
{
public:
	CNormalCamera();
	~CNormalCamera();

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

};
