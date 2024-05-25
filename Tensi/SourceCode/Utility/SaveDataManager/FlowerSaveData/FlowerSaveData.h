#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Object\GameObject\Actor\ActorBackGround\FlowerManager\Flower\FlowerSaveData.h"
#include <functional>

/************************************************
*	�Ԃ̃Z�[�u�f�[�^�N���X.
**/
class CFlowerSaveData final
{
public:
	using SaveFunction = std::function<std::vector<SFlowerData>()>;
	using LoadFunction = std::function<void( std::vector<SFlowerData> )>;

public:
	CFlowerSaveData();
	~CFlowerSaveData();

	// �ۑ����Ɏg�p����֐��̐ݒ�
	void SetSaveFunction( const SaveFunction& Function ) { m_SaveFunction = Function; }
	// �ǂݍ��ݎ��Ɏg�p����֐��̐ݒ�
	void SetLoadFunction( const LoadFunction& Function ) { m_LoadFunction = Function; }

	// �ۑ�.
	void Save();
	// �ǂݍ���.
	void Load();

private:
	SaveFunction m_SaveFunction;	// �ۑ����Ɏg�p����֐�
	LoadFunction m_LoadFunction;	// �ǂݍ��ݎ��Ɏg�p����֐�
};