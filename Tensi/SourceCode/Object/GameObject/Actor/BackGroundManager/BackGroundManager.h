#pragma once
#include "..\Actor.h"

class CGroundManager;
class CFlowerManager;
class CWeedManager;
class CWaterFall;
//class CTree;
//class CHouse;

/************************************************
*	�w�i�I�u�W�F�N�g�}�l�[�W���[�N���X.
**/
class BackGroundManager
{
public:
	BackGroundManager();
	~BackGroundManager();

	// ������
	static bool Init();
	// ���񃍃O�C�����̏�����.
	static bool FirstPlayInit();
	// ���񃍃O�C�����̏�����.
	static bool LoginInit( const std::tm& now, const std::tm& diff );

	// �X�V.
	static void Update( const float& DeltaTime );
	// �����蔻��I����Ăяo�����X�V.
	static void LateUpdate( const float& DeltaTime );
	// �f�o�b�N�̍X�V.
	static void DebugUpdate( const float& DeltaTime );

	// �`��.
	static void Render();
	static void SubRender();

private:
	// �C���X�^���X�̎擾.
	static BackGroundManager* GetInstance();

private:
	std::shared_ptr<CWeedManager>			m_pWeedManager;		// �G��.
	std::shared_ptr<CFlowerManager>			m_pFlowerManager;	// ��.
	std::shared_ptr<CWaterFall>				m_pWaterFall;		// ��.
//	std::shared_ptr<CTree>					m_pTree;			// ��.
//	std::shared_ptr<CHouse>					m_pHouse;			// ��.

};
