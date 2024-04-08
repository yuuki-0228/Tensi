#pragma once
#include "SceneManager\SceneManager.h"
#include "..\Common\SoundManeger\SoundManeger.h"
#include "..\Object\Camera\Cameras\Cameras.h"
#include "..\Object\Camera\CameraManager\CameraManager.h"
#include "..\Utility\Input\Input.h"

class CCameras;

/************************************************
*	�V�[���x�[�X�N���X.
**/
class CSceneBase
{
public:
	CSceneBase();
	virtual ~CSceneBase();

	// ������.
	virtual bool Init() = 0;
	// ����N�����̏�����.
	virtual bool FirstPlayInit() = 0;
	// ���񃍃O�C�����̏�����(lastDay:�O��̃��O�C����).
	virtual bool LoginInit( std::tm lastDay ) = 0;

	// �X�V.
	virtual void Update( const float& DeltaTime ) = 0;

	// ���f���̕`��.
	virtual void ModelRender() = 0;
	virtual void SubModelRender() {};
	// �X�v���C�g(UI)�̕`��.
	virtual void SpriteUIRender() = 0;
	virtual void SubSpriteUIRender() {};
	// �X�v���C�g(3D)/Effect�̕`��.
	//	_A�F��ɕ\������� / _B�F��ɕ\�������.
	virtual void Sprite3DRender_A() = 0;
	virtual void SubSprite3DRender_A() {};
	virtual void Sprite3DRender_B() = 0;
	virtual void SubSprite3DRender_B() {};

protected:
	std::unique_ptr<CCameras>	m_pCameras;		// �����̃J��������������N���X.
	std::string					m_BGMName;		// BGM��.

};