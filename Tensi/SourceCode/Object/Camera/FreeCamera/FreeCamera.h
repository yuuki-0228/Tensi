#pragma once
#include "..\CameraBase.h"

/************************************************
*	自由に動かせるカメラクラス.
*		﨑田友輝.
**/
class CFreeCamera final
	: public CCameraBase
{
public:
	CFreeCamera();
	~CFreeCamera();

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

private:
	D3DXPOSITION3	m_CameraPosition;	// カメラの座標.
	D3DXVECTOR3		m_MoveVec;			// 移動ベクトル.
	D3DXVECTOR3		m_SideMoveVec;		// 横移動ベクトル.
};
