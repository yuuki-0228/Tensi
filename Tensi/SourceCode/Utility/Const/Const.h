#pragma once
#include "..\..\Global.h"
#include <vector>

/************************************************
*	定数関数
*	「creator」によって自動で作成されています
*/
namespace ConstStructs {
	struct stConstFlower {
		float		COME_OUT_ANIM_ALPHA_SPEED;	// 雑草が抜けた時の透明になっていくアニメーションの速度(秒)
		int			DELETE_DAY;	// 水やりしていない日が何日続いたら消滅するか
		float		REACTION_ANIM_SCALE;	// 花に触れた時のアニメーションの拡大率
		float		REACTION_ANIM_SPEED;	// 花に触れた時のアニメーションの速度
		float		SKIP_SHAKE_SPEED;	// 花に触れたアニメーションを行わないマウスの移動差分値(以下)
		float		WATERING_ANIM_COOL_TIME_MAX;	// 水やり完了時のアニメーションの最大の待ち時間(秒)
		float		WATERING_ANIM_COOL_TIME_MIN;	// 水やり完了時のアニメーションの最小の待ち時間(秒)
		float		WATERING_ANIM_EFFECT_ALPHA_SPEED;	// 水やり完了時のエフェクトの透明になっていく速度(秒)
		int			WATER_OK_NUM;	// 水やりができたことにする回数
		int			WITHER_DAY;	// 水やりしていない日が何日続いたら枯れるか

		void Load() const;
		stConstFlower() { Load(); }
	};

	struct stConstFreeCamera {
		float		MOVE_SPEED;	// カメラの移動速度
		float		ROTATION_SPEED;	// カメラの回転速度

		void Load() const;
		stConstFreeCamera() { Load(); }
	};

	struct stConstSystem {
		float		AUTO_SAVE_TIME;	// オートセーブを行う間隔(秒)

		void Load() const;
		stConstSystem() { Load(); }
	};

	struct stConstWaterFall {
		int			WATER_FALL_W;	// 滝の幅

		void Load() const;
		stConstWaterFall() { Load(); }
	};

	struct stConstWateringCan {
		float		FALL_MAX_VECTOR;	// 落下速度の加速を終了する移動ベクトルの値
		float		FULL_ANIM_COOL_TIME;	// 満タンの時のアニメーションの待機時間
		float		FULL_ANIM_POWER_MAX;	// 満タンの時のアニメーションの最大の強さ
		float		FULL_ANIM_POWER_MIN;	// 満タンの時のアニメーションの最小の強さ
		float		GAUGE_DISP_ALPHA_SPEED;	// 水ゲージの表示する速度(秒)
		float		GAUGE_DISP_TIME_MAX;	// 水ゲージが変動した時に表示する時間(秒)
		float		GAUGE_HIDDEN_ALPHA_SPEED;	// 水ゲージの非表示になる速度(秒)
		float		HIT_MAX_VEC_Y;	// 地面にぶつかった時の最大とするベクトルの値
		float		HIT_VEC_Y;	// 地面にぶつかったとするベクトルの値
		int			HIT_WATER_NUM_MAX;	// 地面にぶつかったときの強さの最大
		float		HIT_WATER_POWER_MAX;	// 地面にぶつかったときの強さの最大
		float		WATERING_POWER_MAX;	// 水やりの最大の強さ
		float		WATERING_POWER_MIN;	// 水やりの最小の強さ
		float		WATER_VEC_ROT_MAX;	// アニメーションのベクトルの最大の回転
		float		WATER_VEC_ROT_MIN;	// アニメーションのベクトルの最小の回転

		void Load() const;
		stConstWateringCan() { Load(); }
	};

	struct stConstWeed {
		float		COME_OUT_ANIM_ALPHA_SPEED;	// 雑草が抜けた時の透明になっていくアニメーションの速度(秒)
		float		COME_OUT_ANIM_MOVE_SPEED;	// 雑草が抜けた時の移動アニメーションの速度(秒)
		float		COME_OUT_HEIGHT;	// 雑草が抜けたことにする上に移動した高さ
		float		SKIP_SHAKE_SPEED;	// 葉っぱを揺らさないマウスの移動差分値(以下)

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
