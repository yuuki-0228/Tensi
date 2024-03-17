#include "Const.h"
#include "..\FileManager\FileManager.h"

namespace {
	constexpr int _COMMENT	= 0; // コメントの位置
	constexpr int _DATA		= 1; // データの位置
	constexpr int _X		= 0; // Xの位置
	constexpr int _Y		= 1; // Yの位置
	constexpr int _Z		= 2; // Zの位置
	constexpr int _W		= 2; // Wの位置
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

void ConstStructs::stConstFlower::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\Flower.json" );
	auto This	= const_cast<stConstFlower*>( this );

	// 初期化
	This->COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA];
	This->DELETE_DAY = j["DELETE_DAY"][_DATA];
	This->REACTION_ANIM_SCALE = j["REACTION_ANIM_SCALE"][_DATA];
	This->REACTION_ANIM_SPEED = j["REACTION_ANIM_SPEED"][_DATA];
	This->SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA];
	This->WATERING_ANIM_COOL_TIME_MAX = j["WATERING_ANIM_COOL_TIME_MAX"][_DATA];
	This->WATERING_ANIM_COOL_TIME_MIN = j["WATERING_ANIM_COOL_TIME_MIN"][_DATA];
	This->WATERING_ANIM_EFFECT_ALPHA_SPEED = j["WATERING_ANIM_EFFECT_ALPHA_SPEED"][_DATA];
	This->WATER_OK_NUM = j["WATER_OK_NUM"][_DATA];
	This->WITHER_DAY = j["WITHER_DAY"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\Flower.json 読み込み : 成功" );
}

void ConstStructs::stConstFreeCamera::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\FreeCamera.json" );
	auto This	= const_cast<stConstFreeCamera*>( this );

	// 初期化
	This->MOVE_SPEED = j["MOVE_SPEED"][_DATA];
	This->ROTATION_SPEED = j["ROTATION_SPEED"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\FreeCamera.json 読み込み : 成功" );
}

void ConstStructs::stConstSystem::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\System.json" );
	auto This	= const_cast<stConstSystem*>( this );

	// 初期化
	This->AUTO_SAVE_TIME = j["AUTO_SAVE_TIME"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\System.json 読み込み : 成功" );
}

void ConstStructs::stConstWaterFall::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\WaterFall.json" );
	auto This	= const_cast<stConstWaterFall*>( this );

	// 初期化
	This->WATER_FALL_W = j["WATER_FALL_W"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\WaterFall.json 読み込み : 成功" );
}

void ConstStructs::stConstWateringCan::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\WateringCan.json" );
	auto This	= const_cast<stConstWateringCan*>( this );

	// 初期化
	This->FALL_MAX_VECTOR = j["FALL_MAX_VECTOR"][_DATA];
	This->FULL_ANIM_COOL_TIME = j["FULL_ANIM_COOL_TIME"][_DATA];
	This->FULL_ANIM_POWER_MAX = j["FULL_ANIM_POWER_MAX"][_DATA];
	This->FULL_ANIM_POWER_MIN = j["FULL_ANIM_POWER_MIN"][_DATA];
	This->GAUGE_DISP_ALPHA_SPEED = j["GAUGE_DISP_ALPHA_SPEED"][_DATA];
	This->GAUGE_DISP_TIME_MAX = j["GAUGE_DISP_TIME_MAX"][_DATA];
	This->GAUGE_HIDDEN_ALPHA_SPEED = j["GAUGE_HIDDEN_ALPHA_SPEED"][_DATA];
	This->HIT_MAX_VEC_Y = j["HIT_MAX_VEC_Y"][_DATA];
	This->HIT_VEC_Y = j["HIT_VEC_Y"][_DATA];
	This->HIT_WATER_NUM_MAX = j["HIT_WATER_NUM_MAX"][_DATA];
	This->HIT_WATER_POWER_MAX = j["HIT_WATER_POWER_MAX"][_DATA];
	This->WATERING_POWER_MAX = j["WATERING_POWER_MAX"][_DATA];
	This->WATERING_POWER_MIN = j["WATERING_POWER_MIN"][_DATA];
	This->WATER_VEC_ROT_MAX = j["WATER_VEC_ROT_MAX"][_DATA];
	This->WATER_VEC_ROT_MIN = j["WATER_VEC_ROT_MIN"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\WateringCan.json 読み込み : 成功" );
}

void ConstStructs::stConstWeed::Load() const
{
	json j		= FileManager::JsonLoad( "Data\\Parameter\\Const\\Weed.json" );
	auto This	= const_cast<stConstWeed*>( this );

	// 初期化
	This->COME_OUT_ANIM_ALPHA_SPEED = j["COME_OUT_ANIM_ALPHA_SPEED"][_DATA];
	This->COME_OUT_ANIM_MOVE_SPEED = j["COME_OUT_ANIM_MOVE_SPEED"][_DATA];
	This->COME_OUT_HEIGHT = j["COME_OUT_HEIGHT"][_DATA];
	This->SKIP_SHAKE_SPEED = j["SKIP_SHAKE_SPEED"][_DATA];

	Log::PushLog( "Data\\Parameter\\Const\\Weed.json 読み込み : 成功" );
}

void Const::Load()
{
	Flower.Load();
	FreeCamera.Load();
	System.Load();
	WaterFall.Load();
	WateringCan.Load();
	Weed.Load();
}
