#pragma once
#include "..\CameraBase.h"

/************************************************
*	通常のカメラクラス.
*		﨑田友輝.
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
