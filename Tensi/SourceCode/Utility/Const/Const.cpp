#include "Const.h"
#ifdef ENABLE_CONST
#include "..\FileManager\FileManager.h"

namespace {
	constexpr int _COMMENT	= 0; // コメントの位置
	constexpr int _DATA		= 1; // データの位置
	constexpr int _X		= 0; // Xの位置
	constexpr int _Y		= 1; // Yの位置
	constexpr int _Z		= 2; // Zの位置
	constexpr int _W		= 3; // Wの位置
	constexpr int _FIRST	= 0; // 最初の位置
	constexpr int _SECOND	= 1; // 次の位置

	// 配列のサイズを取得
	int GetSize( const json& j ) {
		// 一番最初にコメントがあるため要素数を1減らしておく
		return static_cast<int>( j.size() ) - 1;
	}

	// 文字化けしないようにstd::stringに変換して取得
	std::string GetString( const std::string& string ) {
		// jsonはUTF8なため一度文字列を変換する
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}
}

Const::Const()
{
}

Const::~Const()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
Const* Const::GetInstance()
{
	static std::unique_ptr<Const> pInstance = std::make_unique<Const>();
	return pInstance.get();
}

//----------------------------.
// GameWindow.
//----------------------------.
void Const::stConstGameWindow::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Config\\WindowSetting.json" );

	// 初期化
	SIZE.x		= j["Size"]["w"].Get<float>();
	SIZE.y		= j["Size"]["h"].Get<float>();
	FPS			= j["FPS"].Get<float>();
	APP_NAME	= j["Name"]["App"].Get<std::string>();
	WND_NAME	= j["Name"]["Wnd"].Get<std::string>();
	VERSION		= j["Version"].Get<std::string>();
}

//----------------------------.
// Explore.
//----------------------------.
void Const::stConstExplore::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\Explore.json" );

	// 初期化
	COMPLETE_SEC_PER_MB = j["COMPLETE_SEC_PER_MB"][_DATA].Get<float>();
	COMPLETE_TIME_MAX = j["COMPLETE_TIME_MAX"][_DATA].Get<float>();
	COMPLETE_TIME_MIN = j["COMPLETE_TIME_MIN"][_DATA].Get<float>();
	EXPAND_TIME = j["EXPAND_TIME"][_DATA].Get<float>();
	GAUGE_HEIGHT_RATE = j["GAUGE_HEIGHT_RATE"][_DATA].Get<float>();
	GAUGE_OFFSET_Y_RATE = j["GAUGE_OFFSET_Y_RATE"][_DATA].Get<float>();
	GAUGE_WIDTH_RATE = j["GAUGE_WIDTH_RATE"][_DATA].Get<float>();
	ICON_POLL_INTERVAL = j["ICON_POLL_INTERVAL"][_DATA].Get<float>();
	PEEK_INTERVAL_MAX = j["PEEK_INTERVAL_MAX"][_DATA].Get<float>();
	PEEK_INTERVAL_MIN = j["PEEK_INTERVAL_MIN"][_DATA].Get<float>();
	PEEK_LOOK_COUNT = j["PEEK_LOOK_COUNT"][_DATA].Get<int>();
	PEEK_LOOK_TIME = j["PEEK_LOOK_TIME"][_DATA].Get<float>();
	PEEK_RISE_RATE = j["PEEK_RISE_RATE"][_DATA].Get<float>();
	PEEK_RISE_TIME = j["PEEK_RISE_TIME"][_DATA].Get<float>();
	PEEK_SINK_TIME = j["PEEK_SINK_TIME"][_DATA].Get<float>();
	PEEK_WAIT_TIME = j["PEEK_WAIT_TIME"][_DATA].Get<float>();
	SHRINK_TIME = j["SHRINK_TIME"][_DATA].Get<float>();
	SLIME_ICON_SCALE_RATE = j["SLIME_ICON_SCALE_RATE"][_DATA].Get<float>();
	SMOKE_ALPHA_SPEED = j["SMOKE_ALPHA_SPEED"][_DATA].Get<float>();
	SMOKE_SCALE_MAX = j["SMOKE_SCALE_MAX"][_DATA].Get<float>();
	SMOKE_SCALE_MIN = j["SMOKE_SCALE_MIN"][_DATA].Get<float>();
	SMOKE_SIZE_RATE = j["SMOKE_SIZE_RATE"][_DATA].Get<float>();
	SMOKE_SPAWN_INTERVAL = j["SMOKE_SPAWN_INTERVAL"][_DATA].Get<float>();
	SMOKE_SPREAD_RATE = j["SMOKE_SPREAD_RATE"][_DATA].Get<float>();
	SMOKE_START_ALPHA = j["SMOKE_START_ALPHA"][_DATA].Get<float>();
	TEXT_HEIGHT_RATE = j["TEXT_HEIGHT_RATE"][_DATA].Get<float>();
	TEXT_OFFSET_Y_RATE = j["TEXT_OFFSET_Y_RATE"][_DATA].Get<float>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\Explore.json 読み込み : 成功" );
}

//----------------------------.
// Flower.
//----------------------------.
void Const::stConstFlower::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\Flower.json" );

	// 初期化
	COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA].Get<float>();
	DELETE_DAY = j["DELETE_DAY"][_DATA].Get<int>();
	REACTION_ANIM_SCALE = j["REACTION_ANIM_SCALE"][_DATA].Get<float>();
	REACTION_ANIM_SPEED = j["REACTION_ANIM_SPEED"][_DATA].Get<float>();
	SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA].Get<float>();
	WATERING_ANIM_COOL_TIME_MAX = j["WATERING_ANIM_COOL_TIME_MAX"][_DATA].Get<float>();
	WATERING_ANIM_COOL_TIME_MIN = j["WATERING_ANIM_COOL_TIME_MIN"][_DATA].Get<float>();
	WATERING_ANIM_EFFECT_ALPHA_SPEED = j["WATERING_ANIM_EFFECT_ALPHA_SPEED"][_DATA].Get<float>();
	WATER_OK_NUM = j["WATER_OK_NUM"][_DATA].Get<int>();
	WITHER_DAY = j["WITHER_DAY"][_DATA].Get<int>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\Flower.json 読み込み : 成功" );
}

//----------------------------.
// FreeCamera.
//----------------------------.
void Const::stConstFreeCamera::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\FreeCamera.json" );

	// 初期化
	MOVE_SPEED = j["MOVE_SPEED"][_DATA].Get<float>();
	ROTATION_SPEED = j["ROTATION_SPEED"][_DATA].Get<float>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\FreeCamera.json 読み込み : 成功" );
}

//----------------------------.
// System.
//----------------------------.
void Const::stConstSystem::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\System.json" );

	// 初期化
	AUTO_SAVE_TIME = j["AUTO_SAVE_TIME"][_DATA].Get<float>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\System.json 読み込み : 成功" );
}

//----------------------------.
// WaterFall.
//----------------------------.
void Const::stConstWaterFall::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\WaterFall.json" );

	// 初期化
	WATER_FALL_W = j["WATER_FALL_W"][_DATA].Get<int>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\WaterFall.json 読み込み : 成功" );
}

//----------------------------.
// WateringCan.
//----------------------------.
void Const::stConstWateringCan::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\WateringCan.json" );

	// 初期化
	FALL_MAX_VECTOR = j["FALL_MAX_VECTOR"][_DATA].Get<float>();
	FULL_ANIM_COOL_TIME = j["FULL_ANIM_COOL_TIME"][_DATA].Get<float>();
	FULL_ANIM_POWER_MAX = j["FULL_ANIM_POWER_MAX"][_DATA].Get<float>();
	FULL_ANIM_POWER_MIN = j["FULL_ANIM_POWER_MIN"][_DATA].Get<float>();
	GAUGE_DISP_ALPHA_SPEED = j["GAUGE_DISP_ALPHA_SPEED"][_DATA].Get<float>();
	GAUGE_DISP_TIME_MAX = j["GAUGE_DISP_TIME_MAX"][_DATA].Get<float>();
	GAUGE_HIDDEN_ALPHA_SPEED = j["GAUGE_HIDDEN_ALPHA_SPEED"][_DATA].Get<float>();
	HIT_MAX_VEC_Y = j["HIT_MAX_VEC_Y"][_DATA].Get<float>();
	HIT_VEC_Y = j["HIT_VEC_Y"][_DATA].Get<float>();
	HIT_WATER_NUM_MAX = j["HIT_WATER_NUM_MAX"][_DATA].Get<int>();
	HIT_WATER_POWER_MAX = j["HIT_WATER_POWER_MAX"][_DATA].Get<float>();
	WATERING_POWER_MAX = j["WATERING_POWER_MAX"][_DATA].Get<float>();
	WATERING_POWER_MIN = j["WATERING_POWER_MIN"][_DATA].Get<float>();
	WATER_VEC_ROT_MAX = j["WATER_VEC_ROT_MAX"][_DATA].Get<float>();
	WATER_VEC_ROT_MIN = j["WATER_VEC_ROT_MIN"][_DATA].Get<float>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\WateringCan.json 読み込み : 成功" );
}

//----------------------------.
// Weed.
//----------------------------.
void Const::stConstWeed::Load()
{
	Json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\Weed.json" );

	// 初期化
	COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA].Get<float>();
	COME_OUT_ANIM_MOVE_SPEED = j["COME_OUT_ANIM_MOVE_SPEED"][_DATA].Get<float>();
	COME_OUT_HEIGHT = j["COME_OUT_HEIGHT"][_DATA].Get<float>();
	SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA].Get<float>();

	Log::PushLogInfo( "Data\\Parameter\\Const\\Weed.json 読み込み : 成功" );
}

//----------------------------.
// 読み込み.
//----------------------------.
void Const::Load()
{
	Const* pI = GetInstance();

	pI->ExploreData.Load();
	pI->FlowerData.Load();
	pI->FreeCameraData.Load();
	pI->SystemData.Load();
	pI->WaterFallData.Load();
	pI->WateringCanData.Load();
	pI->WeedData.Load();
}

#endif