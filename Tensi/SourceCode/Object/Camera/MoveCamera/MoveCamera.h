#pragma once
#include "..\CameraBase.h"

/************************************************
*	移動するカメラクラス.
*		﨑田友輝.
**/
class CMoveCamera final
	: public CCameraBase
{
public:
	CMoveCamera();
	~CMoveCamera();

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 移動の開始.
	void MoveStart( const std::vector<D3DXPOSITION3>& Points, const float EndTime );

private:
	D3DXVECTOR3					m_CameraPosition;	// カメラの座標.
	std::vector<D3DXPOSITION3>	m_MovePoint;		// 移動地点.
	float						m_EndTime;			// 何秒で移動を終了させるか.
	float						m_NowPoint;			// 現在の地点(0.0f ~ 1.0f).
	bool						m_IsMove;			// 移動中か.
};
