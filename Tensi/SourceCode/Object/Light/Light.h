#pragma once
#include "..\Object.h"
#include "..\..\Global.h"

//ライト情報構造体.
struct stLightState
{
	D3DXVECTOR3	Direction;	// 方向.
	float		Intensity;	// 強度(明るさ).

	stLightState()
		: stLightState( { 1.5f, 1.0f, -1.0f }, 1.0f )
	{}
	stLightState( D3DXVECTOR3 Dir, float Int )
		: Direction	( Dir )
		, Intensity	( Int )
	{}

} typedef SLightState;

/************************************************
*	ライトクラス.
*		﨑田友輝.
**/
class Light final
	: public CObject
{
public:
	Light();
	virtual ~Light();

	// 更新.
	static void Update();

	// 向きの取得.
	static D3DXVECTOR3 GetDirection() { return GetInstance()->m_Direction; }
	// 強弱(明るさ)の取得.
	static float GetIntensity() { return GetInstance()->m_Intensity; }
	// ライトの状態を取得.
	static SLightState GetLightState() { return SLightState( GetInstance()->m_Direction, GetInstance()->m_Intensity ); }

	// 向きの設定.
	static void SetDirection( const D3DXVECTOR3& Dir ) { GetInstance()->m_Direction = Dir; }
	// 強弱(明るさ)の設定.
	static void SetIntensity( const float& Int ) { GetInstance()->m_Intensity = Int; }
	// ライトの状態を設定.
	static void SetLightState( const SLightState& State ) {
		Light* pI = GetInstance();
		pI->m_Direction = State.Direction;
		pI->m_Intensity = State.Intensity;
	}

private:
	// インスタンスの取得.
	static Light* GetInstance();

private:
	D3DXVECTOR3	m_Direction; // 向き.
	float		m_Intensity; // 強弱(明るさ).
};
