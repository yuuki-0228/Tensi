#pragma once
#include "..\..\..\Global.h"
#include "..\NormalCamera\NormalCamera.h"
#include "..\FreeCamera\FreeCamera.h"
#include "..\MoveCamera\MoveCamera.h"
#include "..\ThirdPersonCamera\ThirdPersonCamera.h"
#include <memory>

// カメラのタイプ.
enum class enCameraType : unsigned char
{
	None,

	NormalCamera,		// 通常のカメラ.
	FreeCamera,			// 自由に動かせるカメラ.
	MoveCamera,			// 移動するカメラ.
	ThirdPersonCamera,	// 三人称カメラ.

	Max
} typedef ECameraType;

/************************************************
*	複数のカメラを所持しているクラス.
*		﨑田友輝.
**/
class CCameras final
{
public:
	CCameras();
	~CCameras();

	// 指定したカメラの初期化.
	void InitCamera( const ECameraType& Type );

	// カメラの取得.
	template<class T> T*			GetCamera() { return nullptr; }
	// カメラの取得(通常のカメラ).
	template<> CNormalCamera*		GetCamera() { return m_pNormalCamera.get(); }
	// カメラの取得(自由に動かせるカメラ).
	template<> CFreeCamera*			GetCamera() { return m_pFreeCamera.get(); }
	// カメラの取得(移動するカメラ).
	template<> CMoveCamera*			GetCamera() { return m_pMoveCamera.get(); }
	// カメラの取得(三人称カメラ).
	template<> CThirdPersonCamera*	GetCamera() { return m_pThirdPersonCamera.get(); }

private:
	std::unique_ptr<CNormalCamera>			m_pNormalCamera;		// 通常のカメラ.
	std::unique_ptr<CFreeCamera>			m_pFreeCamera;			// 自由に動かせるカメラ.
	std::unique_ptr<CMoveCamera>			m_pMoveCamera;			// 移動するカメラ.
	std::unique_ptr<CThirdPersonCamera>		m_pThirdPersonCamera;	// 三人称カメラ.
};
