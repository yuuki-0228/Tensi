#pragma once
#include "..\Object.h"
#include "..\..\Global.h"

//���C�g���\����.
struct stLightState
{
	D3DXVECTOR3	Direction;	// ����.
	float		Intensity;	// ���x(���邳).

	stLightState()
		: stLightState( { 1.5f, 1.0f, -1.0f }, 1.0f )
	{}
	stLightState( D3DXVECTOR3 Dir, float Int )
		: Direction	( Dir )
		, Intensity	( Int )
	{}

} typedef SLightState;

/************************************************
*	���C�g�N���X.
*		���c�F�P.
**/
class Light final
	: public CObject
{
public:
	Light();
	virtual ~Light();

	// �X�V.
	static void Update();

	// �����̎擾.
	static D3DXVECTOR3 GetDirection() { return GetInstance()->m_Direction; }
	// ����(���邳)�̎擾.
	static float GetIntensity() { return GetInstance()->m_Intensity; }
	// ���C�g�̏�Ԃ��擾.
	static SLightState GetLightState() { return SLightState( GetInstance()->m_Direction, GetInstance()->m_Intensity ); }

	// �����̐ݒ�.
	static void SetDirection( const D3DXVECTOR3& Dir ) { GetInstance()->m_Direction = Dir; }
	// ����(���邳)�̐ݒ�.
	static void SetIntensity( const float& Int ) { GetInstance()->m_Intensity = Int; }
	// ���C�g�̏�Ԃ�ݒ�.
	static void SetLightState( const SLightState& State ) {
		Light* pI = GetInstance();
		pI->m_Direction = State.Direction;
		pI->m_Intensity = State.Intensity;
	}

private:
	// �C���X�^���X�̎擾.
	static Light* GetInstance();

private:
	D3DXVECTOR3	m_Direction; // ����.
	float		m_Intensity; // ����(���邳).
};
