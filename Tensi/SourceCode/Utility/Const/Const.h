#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_CONST
#include "..\..\Global.h"
#include <vector>

/************************************************
*	定数関数
*	「creator」によって自動で作成されています
*/
class Const
{
private:
	struct stConstGameWindow {
		D3DXVECTOR2		SIZE;		// ウィンドウのサイズ(幅,高さ)
		float			FPS;		// フレームレート
		std::string		APP_NAME;	// アプリ名
		std::string		WND_NAME;	// ウィンドウ名
		std::string		VERSION;	// バージョン

		void Load();
		stConstGameWindow() { Load(); }
	} GameWindowData;

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

		void Load();
		stConstFlower() { Load(); }
	} FlowerData; 

	struct stConstFreeCamera {
		float		MOVE_SPEED;	// カメラの移動速度
		float		ROTATION_SPEED;	// カメラの回転速度

		void Load();
		stConstFreeCamera() { Load(); }
	} FreeCameraData; 

	struct stConstSystem {
		float		AUTO_SAVE_TIME;	// オートセーブを行う間隔(秒)

		void Load();
		stConstSystem() { Load(); }
	} SystemData; 

	struct stConstWaterFall {
		int			WATER_FALL_W;	// 滝の幅

		void Load();
		stConstWaterFall() { Load(); }
	} WaterFallData; 

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

		void Load();
		stConstWateringCan() { Load(); }
	} WateringCanData; 

	struct stConstWeed {
		float		COME_OUT_ANIM_ALPHA_SPEED;	// 雑草が抜けた時の透明になっていくアニメーションの速度(秒)
		float		COME_OUT_ANIM_MOVE_SPEED;	// 雑草が抜けた時の移動アニメーションの速度(秒)
		float		COME_OUT_HEIGHT;	// 雑草が抜けたことにする上に移動した高さ
		float		SKIP_SHAKE_SPEED;	// 葉っぱを揺らさないマウスの移動差分値(以下)

		void Load();
		stConstWeed() { Load(); }
	} WeedData; 

public:
	Const();
	~Const();

	// 読み込み
	static void Load();

	// 定数の取得
	static inline stConstGameWindow GameWindow() { return GetInstance()->GameWindowData; }
	static inline stConstFlower Flower() { return GetInstance()->FlowerData; }
	static inline stConstFreeCamera FreeCamera() { return GetInstance()->FreeCameraData; }
	static inline stConstSystem System() { return GetInstance()->SystemData; }
	static inline stConstWaterFall WaterFall() { return GetInstance()->WaterFallData; }
	static inline stConstWateringCan WateringCan() { return GetInstance()->WateringCanData; }
	static inline stConstWeed Weed() { return GetInstance()->WeedData; }

private:
	// インスタンスの取得.
	static Const* GetInstance();
};
#endif