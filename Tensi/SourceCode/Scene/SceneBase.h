#pragma once
#include "SceneManager\SceneManager.h"
#include "..\Common\SoundManeger\SoundManeger.h"
#include "..\Object\Camera\Cameras\Cameras.h"
#include "..\Object\Camera\CameraManager\CameraManager.h"
#include "..\Utility\Input\Input.h"

class CCameras;

/************************************************
*	シーンベースクラス.
**/
class CSceneBase
{
public:
	CSceneBase();
	virtual ~CSceneBase();

	// 初期化.
	virtual bool Init() = 0;
	// 初回起動時の初期化.
	virtual bool FirstPlayInit() = 0;
	// 初回ログイン時の初期化(lastDay:前回のログイン日).
	virtual bool LoginInit( std::tm lastDay ) = 0;

	// 更新.
	virtual void Update( const float& DeltaTime ) = 0;

	// モデルの描画.
	virtual void ModelRender() = 0;
	virtual void SubModelRender() {};
	// スプライト(UI)の描画.
	virtual void SpriteUIRender() = 0;
	virtual void SubSpriteUIRender() {};
	// スプライト(3D)/Effectの描画.
	//	_A：後に表示される / _B：先に表示される.
	virtual void Sprite3DRender_A() = 0;
	virtual void SubSprite3DRender_A() {};
	virtual void Sprite3DRender_B() = 0;
	virtual void SubSprite3DRender_B() {};

protected:
	std::unique_ptr<CCameras>	m_pCameras;		// 複数のカメラを所持するクラス.
	std::string					m_BGMName;		// BGM名.

};