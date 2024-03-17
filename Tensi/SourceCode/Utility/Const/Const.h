#pragma once
#include "..\..\Global.h"
#include <vector>

/************************************************
*	�萔�֐�
*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�
*/
namespace ConstStructs {
	struct stConstFlower {
		float		COME_OUT_ANIM_ALPHA_SPEED;	// �G�������������̓����ɂȂ��Ă����A�j���[�V�����̑��x(�b)
		int			DELETE_DAY;	// ����肵�Ă��Ȃ�������������������ł��邩
		float		REACTION_ANIM_SCALE;	// �ԂɐG�ꂽ���̃A�j���[�V�����̊g�嗦
		float		REACTION_ANIM_SPEED;	// �ԂɐG�ꂽ���̃A�j���[�V�����̑��x
		float		SKIP_SHAKE_SPEED;	// �ԂɐG�ꂽ�A�j���[�V�������s��Ȃ��}�E�X�̈ړ������l(�ȉ�)
		float		WATERING_ANIM_COOL_TIME_MAX;	// ����芮�����̃A�j���[�V�����̍ő�̑҂�����(�b)
		float		WATERING_ANIM_COOL_TIME_MIN;	// ����芮�����̃A�j���[�V�����̍ŏ��̑҂�����(�b)
		float		WATERING_ANIM_EFFECT_ALPHA_SPEED;	// ����芮�����̃G�t�F�N�g�̓����ɂȂ��Ă������x(�b)
		int			WATER_OK_NUM;	// ����肪�ł������Ƃɂ����
		int			WITHER_DAY;	// ����肵�Ă��Ȃ�����������������͂�邩

		void Load() const;
		stConstFlower() { Load(); }
	};

	struct stConstFreeCamera {
		float		MOVE_SPEED;	// �J�����̈ړ����x
		float		ROTATION_SPEED;	// �J�����̉�]���x

		void Load() const;
		stConstFreeCamera() { Load(); }
	};

	struct stConstSystem {
		float		AUTO_SAVE_TIME;	// �I�[�g�Z�[�u���s���Ԋu(�b)

		void Load() const;
		stConstSystem() { Load(); }
	};

	struct stConstWaterFall {
		int			WATER_FALL_W;	// ��̕�

		void Load() const;
		stConstWaterFall() { Load(); }
	};

	struct stConstWateringCan {
		float		FALL_MAX_VECTOR;	// �������x�̉������I������ړ��x�N�g���̒l
		float		FULL_ANIM_COOL_TIME;	// ���^���̎��̃A�j���[�V�����̑ҋ@����
		float		FULL_ANIM_POWER_MAX;	// ���^���̎��̃A�j���[�V�����̍ő�̋���
		float		FULL_ANIM_POWER_MIN;	// ���^���̎��̃A�j���[�V�����̍ŏ��̋���
		float		GAUGE_DISP_ALPHA_SPEED;	// ���Q�[�W�̕\�����鑬�x(�b)
		float		GAUGE_DISP_TIME_MAX;	// ���Q�[�W���ϓ��������ɕ\�����鎞��(�b)
		float		GAUGE_HIDDEN_ALPHA_SPEED;	// ���Q�[�W�̔�\���ɂȂ鑬�x(�b)
		float		HIT_MAX_VEC_Y;	// �n�ʂɂԂ��������̍ő�Ƃ���x�N�g���̒l
		float		HIT_VEC_Y;	// �n�ʂɂԂ������Ƃ���x�N�g���̒l
		int			HIT_WATER_NUM_MAX;	// �n�ʂɂԂ������Ƃ��̋����̍ő�
		float		HIT_WATER_POWER_MAX;	// �n�ʂɂԂ������Ƃ��̋����̍ő�
		float		WATERING_POWER_MAX;	// �����̍ő�̋���
		float		WATERING_POWER_MIN;	// �����̍ŏ��̋���
		float		WATER_VEC_ROT_MAX;	// �A�j���[�V�����̃x�N�g���̍ő�̉�]
		float		WATER_VEC_ROT_MIN;	// �A�j���[�V�����̃x�N�g���̍ŏ��̉�]

		void Load() const;
		stConstWateringCan() { Load(); }
	};

	struct stConstWeed {
		float		COME_OUT_ANIM_ALPHA_SPEED;	// �G�������������̓����ɂȂ��Ă����A�j���[�V�����̑��x(�b)
		float		COME_OUT_ANIM_MOVE_SPEED;	// �G�������������̈ړ��A�j���[�V�����̑��x(�b)
		float		COME_OUT_HEIGHT;	// �G�������������Ƃɂ����Ɉړ���������
		float		SKIP_SHAKE_SPEED;	// �t���ς�h�炳�Ȃ��}�E�X�̈ړ������l(�ȉ�)

		void Load() const;
		stConstWeed() { Load(); }
	};

}

namespace Const{
	const static ConstStructs::stConstFlower Flower;
	const static ConstStructs::stConstFreeCamera FreeCamera;
	const static ConstStructs::stConstSystem System;
	const static ConstStructs::stConstWaterFall WaterFall;
	const static ConstStructs::stConstWateringCan WateringCan;
	const static ConstStructs::stConstWeed Weed;
	void Load();
}
