#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Object\GameObject\Actor\ActorBackGround\WeedManager\Weed\WeedSaveData.h"
#include <functional>

/************************************************
*	���̃Z�[�u�f�[�^�N���X.
**/
class CWeedSaveData final
{
public:
	using SaveFunction = std::function<std::vector<SWeedData>()>;
	using LoadFunction = std::function<void( std::vector<SWeedData> )>;

public:
	CWeedSaveData();
	~CWeedSaveData();

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