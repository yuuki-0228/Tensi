#pragma once
#include "..\..\SystemSettings.h"
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

	struct stConstExplore {
		float		COMPLETE_SEC_PER_MB;	// 容量1MBあたりの探索完了秒数
		float		COMPLETE_TIME_MAX;	// 探索完了時間の最大(秒)
		float		COMPLETE_TIME_MIN;	// 探索完了時間の最小(秒)
		float		EXPAND_TIME;	// スライムがアイコンから飛び出すまでの時間(秒)
		float		GAUGE_HEIGHT_RATE;	// ゲージの高さ(アイコン高さに対する割合)
		float		GAUGE_OFFSET_Y_RATE;	// クリック範囲の下端からゲージまでのYオフセット(アイコン高さに対する割合)
		float		GAUGE_WIDTH_RATE;	// アイコンのクリック範囲幅に対するゲージ幅の割合
		float		ICON_POLL_INTERVAL;	// 探索中に追跡中のフォルダの情報を再取得する間隔(秒)(負荷軽減のため間引く)
		float		PEEK_INTERVAL_MAX;	// スライムが頭を出す間隔の最大(秒)
		float		PEEK_INTERVAL_MIN;	// スライムが頭を出す間隔の最小(秒)
		int			PEEK_LOOK_COUNT;	// 左右きょろきょろの回数
		float		PEEK_LOOK_TIME;	// 左右きょろきょろの1方向あたりの時間(秒)
		float		PEEK_RISE_RATE;	// 頭出しで上に移動する量(アイコン高さに対する割合)
		float		PEEK_RISE_TIME;	// 頭出しで上に出るまでの時間(秒)
		float		PEEK_SINK_TIME;	// 正面に戻って潜るまでの時間(秒)
		float		PEEK_WAIT_TIME;	// 頭を出してから待機する時間(秒)
		float		SHRINK_TIME;	// スライムがアイコンに縮小して入るまでの時間(秒)
		float		SLIME_ICON_SCALE_RATE;	// スライムをアイコンに入れる時の大きさの倍率(アイコン幅に対する割合)
		float		SMOKE_ALPHA_SPEED;	// 煙が透明になっていく速度(秒)
		float		SMOKE_SCALE_MAX;	// 煙の終了時のスケール
		float		SMOKE_SCALE_MIN;	// 煙の開始時のスケール
		float		SMOKE_SIZE_RATE;	// 煙の大きさの倍率
		float		SMOKE_SPAWN_INTERVAL;	// 煙を発生させる間隔(秒)
		float		SMOKE_SPREAD_RATE;	// 煙の発生位置の上下左右のランダムずれ幅(アイコン幅に対する割合)
		float		SMOKE_START_ALPHA;	// 煙の開始時のアルファ値
		float		TEXT_HEIGHT_RATE;	// 残り時間テキストの高さ(アイコン高さに対する割合)
		float		TEXT_OFFSET_Y_RATE;	// ゲージから残り時間テキストまでのYオフセット(アイコン高さに対する割合)

		void Load();
		stConstExplore() { Load(); }
	} ExploreData; 

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

	struct stConstNetwork {
		float		DEFAULT_CONNECTION_TIMEOUT_SEC;	// 既定の応答無し切断時間(秒)
		float		DEFAULT_CONNECT_TIMEOUT_SEC;	// 既定の接続タイムアウト(秒)
		int			DEFAULT_MAX_PLAYER;	// 既定の最大参加人数(ホストを含む)
		float		DEFAULT_SYNC_FPS;	// 既定の状態同期の送信頻度(回/秒)
		int			DEFAULT_TCP_PORT;	// 既定のTCPポート番号(接続・重要メッセージ用)
		int			DEFAULT_UDP_PORT;	// 既定のUDPポート番号(状態同期用)
		int			DISCOVERY_PORT;	// 同一ネットワーク内の部屋検索に使うUDPポート番号
		int			MAX_MESSAGE_SIZE;	// 任意メッセージの最大サイズ(byte)
		int			MAX_PLAYER_LIMIT;	// 最大参加人数の上限
		int			MAX_SYNC_DATA_SIZE;	// 1プレイヤーあたりの同期データの最大サイズ(byte)

		void Load();
		stConstNetwork() { Load(); }
	} NetworkData;

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
	static inline stConstExplore Explore() { return GetInstance()->ExploreData; }
	static inline stConstFlower Flower() { return GetInstance()->FlowerData; }
	static inline stConstFreeCamera FreeCamera() { return GetInstance()->FreeCameraData; }
	static inline stConstNetwork Network() { return GetInstance()->NetworkData; }
	static inline stConstSystem System() { return GetInstance()->SystemData; }
	static inline stConstWaterFall WaterFall() { return GetInstance()->WaterFallData; }
	static inline stConstWateringCan WateringCan() { return GetInstance()->WateringCanData; }
	static inline stConstWeed Weed() { return GetInstance()->WeedData; }

private:
	// インスタンスの取得.
	static Const* GetInstance();
};
#endif