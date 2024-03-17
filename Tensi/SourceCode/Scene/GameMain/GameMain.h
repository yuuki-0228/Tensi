#pragma once
#include "..\SceneBase.h"

class CGameMainWidget;
class CPlayer;
class CBall;
class CHeavyBall;
class CSuperBall;
class CWateringCan;
class CSlimeFrame;
class CTree;
class CWeedManager;
class CFlowerManager;
class CHouse;
class CWaterFall;

/************************************************
*	�Q�[���N���X.
*		���c�F�P.
**/
class CGameMain final
	: public CSceneBase
{
public:
	CGameMain();
	~CGameMain();

	// ������.
	virtual bool Init() override;
	// ����N�����̏�����.
	virtual bool FirstPlayInit() override;
	// ���񃍃O�C�����̏�����.
	virtual bool LoginInit( std::tm lastDay ) override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// ���f���̕`��.
	virtual void ModelRender() override;
	// �X�v���C�g(UI)�̕`��.
	virtual void SpriteUIRender() override;
	virtual void SubSpriteUIRender() override;
	// �X�v���C�g(3D)/Effect�̕`��.
	//	_A�F��ɕ\������� / _B�F��ɕ\�������.
	virtual void Sprite3DRender_A() override;
	virtual void SubSprite3DRender_A() override;
	virtual void Sprite3DRender_B() override;
	virtual void SubSprite3DRender_B() override;

private:
	std::shared_ptr<CPlayer>				m_pPlayer;			// �v���C���[.
	std::shared_ptr<CBall>					m_pBall;			// �{�[��.
	std::shared_ptr<CHeavyBall>				m_pHeavyBall;		// �d���{�[��.
	std::shared_ptr<CSuperBall>				m_pSuperBall;		// �X�[�p�[�{�[��.
	std::shared_ptr<CWateringCan>			m_pWateringCan;		// �W���E��.
	std::shared_ptr<CSlimeFrame>			m_pSlimeFrame;		// �X���C���t���[��.
	std::shared_ptr<CTree>					m_pTree;			// ��.
	std::shared_ptr<CWeedManager>			m_pWeedManager;		// �G��.
	std::shared_ptr<CFlowerManager>			m_pFlowerManager;	// ��.
	std::shared_ptr<CHouse>					m_pHouse;			// ��.
	std::shared_ptr<CWaterFall>				m_pWaterFall;		// ��.
	std::unique_ptr<CGameMainWidget>		m_GameMainWidget;	// UI.
};