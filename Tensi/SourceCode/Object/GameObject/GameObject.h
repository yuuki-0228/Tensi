#pragma once
#include "..\Object.h"
#include "GameObjectList.h"

/************************************************
*	�Q�[���I�u�W�F�N�g�N���X.
**/
class CGameObject
	: public CObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	// ������.
	virtual bool Init() = 0;

	// �X�V.
	virtual void Update( const float& DeltaTime ) = 0;
	// �����蔻��I����Ăяo�����X�V.
	virtual void LateUpdate( const float& DeltaTime ) {}
	// �f�o�b�N�̍X�V.
	virtual void DebugUpdate( const float& DeltaTime ) {}

	// �`��.
	virtual void Render() = 0;
	virtual void SubRender() {};

	// �X�v���C�g(3D)�̕`��.
	//	_A�F��ɕ\������� / _B�F��ɕ\�������.
	virtual void Sprite3DRender_A() {}
	virtual void Sprite3DRender_B() {}
	// �X�v���C�g(2D)�̕`��.
	virtual void Sprite2DRender() {}

	// �I�u�W�F�N�g�^�O�̎擾.
	EObjectTag GetObjectTag() const { return m_ObjectTag; }

protected:
	float		m_DeltaTime;	// �f���^�^�C��(�K�v�ȏꍇUptate�̍ŏ��ɑ������).
	EObjectTag	m_ObjectTag;	// �I�u�W�F�N�g�^�O.
};