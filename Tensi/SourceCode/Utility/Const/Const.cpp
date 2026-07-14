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
	json j = FileManager::JsonLoad( "Data\\Parameter\\Config\\WindowSetting.json" );

	// 初期化
	SIZE.x		= j["Size"]["w"];
	SIZE.y		= j["Size"]["h"];
	FPS			= j["FPS"];
	APP_NAME	= j["Name"]["App"];
	WND_NAME	= j["Name"]["Wnd"];
	VERSION		= j["Version"];
}

//----------------------------.
// Flower.
//----------------------------.
void Const::stConstFlower::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\Flower.json" );

	// 初期化
	COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA];
	DELETE_DAY = j["DELETE_DAY"][_DATA];
	REACTION_ANIM_SCALE = j["REACTION_ANIM_SCALE"][_DATA];
	REACTION_ANIM_SPEED = j["REACTION_ANIM_SPEED"][_DATA];
	SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA];
	WATERING_ANIM_COOL_TIME_MAX = j["WATERING_ANIM_COOL_TIME_MAX"][_DATA];
	WATERING_ANIM_COOL_TIME_MIN = j["WATERING_ANIM_COOL_TIME_MIN"][_DATA];
	WATERING_ANIM_EFFECT_ALPHA_SPEED = j["WATERING_ANIM_EFFECT_ALPHA_SPEED"][_DATA];
	WATER_OK_NUM = j["WATER_OK_NUM"][_DATA];
	WITHER_DAY = j["WITHER_DAY"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\Flower.json 読み込み : 成功" );
}

//----------------------------.
// FreeCamera.
//----------------------------.
void Const::stConstFreeCamera::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\FreeCamera.json" );

	// 初期化
	MOVE_SPEED = j["MOVE_SPEED"][_DATA];
	ROTATION_SPEED = j["ROTATION_SPEED"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\FreeCamera.json 読み込み : 成功" );
}

//----------------------------.
// System.
//----------------------------.
void Const::stConstSystem::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\System.json" );

	// 初期化
	AUTO_SAVE_TIME = j["AUTO_SAVE_TIME"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\System.json 読み込み : 成功" );
}

//----------------------------.
// WaterFall.
//----------------------------.
void Const::stConstWaterFall::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\WaterFall.json" );

	// 初期化
	WATER_FALL_W = j["WATER_FALL_W"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\WaterFall.json 読み込み : 成功" );
}

//----------------------------.
// WateringCan.
//----------------------------.
void Const::stConstWateringCan::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\WateringCan.json" );

	// 初期化
	FALL_MAX_VECTOR = j["FALL_MAX_VECTOR"][_DATA];
	FULL_ANIM_COOL_TIME = j["FULL_ANIM_COOL_TIME"][_DATA];
	FULL_ANIM_POWER_MAX = j["FULL_ANIM_POWER_MAX"][_DATA];
	FULL_ANIM_POWER_MIN = j["FULL_ANIM_POWER_MIN"][_DATA];
	GAUGE_DISP_ALPHA_SPEED = j["GAUGE_DISP_ALPHA_SPEED"][_DATA];
	GAUGE_DISP_TIME_MAX = j["GAUGE_DISP_TIME_MAX"][_DATA];
	GAUGE_HIDDEN_ALPHA_SPEED = j["GAUGE_HIDDEN_ALPHA_SPEED"][_DATA];
	HIT_MAX_VEC_Y = j["HIT_MAX_VEC_Y"][_DATA];
	HIT_VEC_Y = j["HIT_VEC_Y"][_DATA];
	HIT_WATER_NUM_MAX = j["HIT_WATER_NUM_MAX"][_DATA];
	HIT_WATER_POWER_MAX = j["HIT_WATER_POWER_MAX"][_DATA];
	WATERING_POWER_MAX = j["WATERING_POWER_MAX"][_DATA];
	WATERING_POWER_MIN = j["WATERING_POWER_MIN"][_DATA];
	WATER_VEC_ROT_MAX = j["WATER_VEC_ROT_MAX"][_DATA];
	WATER_VEC_ROT_MIN = j["WATER_VEC_ROT_MIN"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\WateringCan.json 読み込み : 成功" );
}

//----------------------------.
// Weed.
//----------------------------.
void Const::stConstWeed::Load()
{
	json j = FileManager::JsonLoad( "Data\\Parameter\\Const\\Weed.json" );

	// 初期化
	COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA];
	COME_OUT_ANIM_MOVE_SPEED = j["COME_OUT_ANIM_MOVE_SPEED"][_DATA];
	COME_OUT_HEIGHT = j["COME_OUT_HEIGHT"][_DATA];
	SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\Weed.json 読み込み : 成功" );
}

//----------------------------.
// 読み込み.
//----------------------------.
void Const::Load()
{
	Const* pI = GetInstance();

	pI->FlowerData.Load();
	pI->FreeCameraData.Load();
	pI->SystemData.Load();
	pI->WaterFallData.Load();
	pI->WateringCanData.Load();
	pI->WeedData.Load();
}

#endif