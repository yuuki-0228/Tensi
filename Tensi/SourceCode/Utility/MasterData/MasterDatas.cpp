#include "MasterDatas.h"
#ifdef ENABLE_MASTER_DATA

namespace {
	constexpr int _X		= 0; // Xの位置
	constexpr int _Y		= 1; // Yの位置
	constexpr int _Z		= 2; // Zの位置
	constexpr int _W		= 3; // Wの位置
	constexpr int _FIRST	= 0; // 最初の位置
	constexpr int _SECOND	= 1; // 次の位置

	// 配列のサイズを取得
	int GetSize( const json& j ) {
		return static_cast<int>( j.size() );
	}

	// 文字化けしないようにstd::stringに変換して取得
	std::string GetString( const std::string& string ) {
		// jsonはUTF8なため一度文字列を変換する
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}
}

//----------------------------.
// MasterBulletData.
//----------------------------.
MasterBulletData::MasterBulletData()
	: Id	()
	, Description	()
	, HP	()
	, ObjectColor	()
	, ObjectHitDamage	()
	, ReflectionDamage	()
{
}
MasterBulletData::MasterBulletData( const json& j )
{
	Id = j["Id"];
	Description = GetString( j["Description"] );
	HP = j["HP"];
	ObjectColor = j["ObjectColor"];
	ObjectHitDamage = j["ObjectHitDamage"];
	ReflectionDamage = j["ReflectionDamage"];
}

//----------------------------.
// MasterEnemySpawnData.
//----------------------------.
MasterEnemySpawnData::MasterEnemySpawnData()
	: Id	()
	, MEnemyId	()
	, MWaveId	()
	, SpawnPoint	()
{
}
MasterEnemySpawnData::MasterEnemySpawnData( const json& j )
{
	Id = j["Id"];
	MEnemyId = j["MEnemyId"];
	MWaveId = j["MWaveId"];
	SpawnPoint.x = j["SpawnPoint"][_X];
	SpawnPoint.y = j["SpawnPoint"][_Y];
	SpawnPoint.z = j["SpawnPoint"][_Z];
}

//----------------------------.
// MasterPlayerSmokeEffectData.
//----------------------------.
MasterPlayerSmokeEffectData::MasterPlayerSmokeEffectData()
	: Id	()
	, AddSize	()
	, Description	()
	, EffectColor	()
	, EndAnimTime	()
	, Size	()
	, Sort	()
	, StartAnimTime	()
	, WaitTime	()
{
}
MasterPlayerSmokeEffectData::MasterPlayerSmokeEffectData( const json& j )
{
	Id = j["Id"];
	AddSize = j["AddSize"];
	Description = GetString( j["Description"] );
	EffectColor.x = j["EffectColor"][_X];
	EffectColor.y = j["EffectColor"][_Y];
	EffectColor.z = j["EffectColor"][_Z];
	EndAnimTime = j["EndAnimTime"];
	Size = j["Size"];
	Sort = j["Sort"];
	StartAnimTime = j["StartAnimTime"];
	WaitTime = j["WaitTime"];
}

//----------------------------.
// MasterStageData.
//----------------------------.
MasterStageData::MasterStageData()
	: Id	()
	, MWaveIds	()
	, Name	()
{
}
MasterStageData::MasterStageData( const json& j )
{
	Id = j["Id"];
	const int MWaveIdsSize = GetSize( j["MWaveIds"] );
	MWaveIds.resize( MWaveIdsSize );
	for ( int i = 0; i < MWaveIdsSize; ++i ) {
		MWaveIds[i] = j["MWaveIds"][i];
	}
	Name = GetString( j["Name"] );
}

//----------------------------.
// MasterWaveData.
//----------------------------.
MasterWaveData::MasterWaveData()
	: Id	()
	, EndType	()
	, EndTypeValue	()
{
}
MasterWaveData::MasterWaveData( const json& j )
{
	Id = j["Id"];
	const int EndTypeSize = GetSize( j["EndType"] );
	EndType.resize( EndTypeSize );
	for ( int i = 0; i < EndTypeSize; ++i ) {
		EndType[i] = static_cast<EndTypes>( j["EndType"][i] );
	}
	const int EndTypeValueSize = GetSize( j["EndTypeValue"] );
	EndTypeValue.resize( EndTypeValueSize );
	for ( int i = 0; i < EndTypeValueSize; ++i ) {
		EndTypeValue[i] = j["EndTypeValue"][i];
	}
}

//----------------------------.
// 読み込み
//----------------------------.
std::unordered_map<std::string, std::unordered_map<ulong, std::any>> MasterDataUtility::CreateCache(
	const std::vector<std::pair<std::string, json>>& data )
{
	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> out;

	for ( auto& [Container, File] : data )
	{
		if ( File.is_array() ) {
			for ( auto& Data : File ) {
				CacheSetup( out, Container, Data );
			}
		}
		else {
			CacheSetup( out, Container, File );
		}
	}
	return out;
}
void MasterDataUtility::CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const json& File )
{
	if (	  Container == "MBullet" ) out[typeid( MasterBulletData ).name()][File["Id"]] = MasterBulletData( File );
	else if ( Container == "MEnemySpawn" ) out[typeid( MasterEnemySpawnData ).name()][File["Id"]] = MasterEnemySpawnData( File );
	else if ( Container == "MPlayerSmokeEffect" ) out[typeid( MasterPlayerSmokeEffectData ).name()][File["Id"]] = MasterPlayerSmokeEffectData( File );
	else if ( Container == "MStage" ) out[typeid( MasterStageData ).name()][File["Id"]] = MasterStageData( File );
	else if ( Container == "MWave" ) out[typeid( MasterWaveData ).name()][File["Id"]] = MasterWaveData( File );
}
#endif