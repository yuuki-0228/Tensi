#pragma once
#include "..\..\..\Actor.h"
#include "WaterData.h"

class CWater;

/************************************************
*	���}�l�[�W���[�N���X.
**/
class CWaterManager final
	: public CActor
{
public:
	CWaterManager();
	~CWaterManager();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// ����\������
	void Spill( const SWaterData& data, const HWND hWnd );

private:
	// �����X�g�̍폜.
	void WaterListDelete();

private:
	std::vector<std::unique_ptr<CWater>> m_pWaterList;	// ��.
};