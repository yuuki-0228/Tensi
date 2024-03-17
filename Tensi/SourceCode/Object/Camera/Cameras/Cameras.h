#pragma once
#include "..\..\..\Global.h"
#include "..\NormalCamera\NormalCamera.h"
#include "..\FreeCamera\FreeCamera.h"
#include "..\MoveCamera\MoveCamera.h"
#include "..\ThirdPersonCamera\ThirdPersonCamera.h"
#include <memory>

// �J�����̃^�C�v.
enum class enCameraType : unsigned char
{
	None,

	NormalCamera,		// �ʏ�̃J����.
	FreeCamera,			// ���R�ɓ�������J����.
	MoveCamera,			// �ړ�����J����.
	ThirdPersonCamera,	// �O�l�̃J����.

	Max
} typedef ECameraType;

/************************************************
*	�����̃J�������������Ă���N���X.
*		���c�F�P.
**/
class CCameras final
{
public:
	CCameras();
	~CCameras();

	// �w�肵���J�����̏�����.
	void InitCamera( const ECameraType& Type );

	// �J�����̎擾.
	template<class T> T*			GetCamera() { return nullptr; }
	// �J�����̎擾(�ʏ�̃J����).
	template<> CNormalCamera*		GetCamera() { return m_pNormalCamera.get(); }
	// �J�����̎擾(���R�ɓ�������J����).
	template<> CFreeCamera*			GetCamera() { return m_pFreeCamera.get(); }
	// �J�����̎擾(�ړ�����J����).
	template<> CMoveCamera*			GetCamera() { return m_pMoveCamera.get(); }
	// �J�����̎擾(�O�l�̃J����).
	template<> CThirdPersonCamera*	GetCamera() { return m_pThirdPersonCamera.get(); }

private:
	std::unique_ptr<CNormalCamera>			m_pNormalCamera;		// �ʏ�̃J����.
	std::unique_ptr<CFreeCamera>			m_pFreeCamera;			// ���R�ɓ�������J����.
	std::unique_ptr<CMoveCamera>			m_pMoveCamera;			// �ړ�����J����.
	std::unique_ptr<CThirdPersonCamera>		m_pThirdPersonCamera;	// �O�l�̃J����.
};
