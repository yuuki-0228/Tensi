#pragma once
#include "..\..\Global.h"

//---------------------------.
//	�Z�[�u�f�[�^���X�g.
//	�ucreator�v�œǂݍ��݁uCCommonSaveData�v���쐬���܂�
//	��Ɠ����ɂȂ�悤�ɋL�����Ă�������
//---------------------------.
namespace SaveData{
	struct stSetSaveDataList {
	//	( �^ ) *HogeHoge;					// ��
		STransform* PlayerTransform;		// �v���C���[�̃g�����X�t�H�[��.
		STransform* BallTransform;			// �{�[���̃g�����X�t�H�[��.
		STransform* WaterFallTransform;		// ��̃g�����X�t�H�[��.
		STransform* WateringCanTransform;	// �W���E���̃g�����X�t�H�[��.
		float* WateringCanWaterValue;		// �W���E���̐��̗�.
	} typedef SSetSaveDataList;
}