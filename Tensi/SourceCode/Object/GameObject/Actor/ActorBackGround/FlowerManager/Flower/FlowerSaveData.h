#pragma once
#include "..\..\..\..\..\..\Global.h"

struct stFlowerData
{
	STransform	Transform;		// �g�����X�t�H�[��
	D3DXCOLOR4	Color;			// �F.
	std::tm		FillDay;		// ���߂���
	std::tm		WitherDay;		// �͂ꂽ��
	std::tm		WateringDay;	// �ŐV�̐���肵����
	int			FlowerType;		// �Ԃ̃^�C�v
	bool		IsWither;		// �͂�Ă��邩
	bool		IsWatering;		// ��������肵����
} typedef SFlowerData;