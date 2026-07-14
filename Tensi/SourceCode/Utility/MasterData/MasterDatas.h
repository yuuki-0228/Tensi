#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_MASTER_DATA
#include "..\..\Global.h"
#include "..\FileManager\FileManager.h"
#include <unordered_map>
#include <any>

/************************************************
*	マスターデータ一覧
*	「creator」によって自動で作成されています
**/
namespace MasterData {
	using ulong = unsigned long;
	struct MasterBulletData
	{
		// ID
		ulong Id;
		// 説明
		std::string Description;
		// HP(最大:100)
		int HP;
		// オブジェクトの色(0:白,1:黒)
		int ObjectColor;
		// オブジェクトに当たった時のダメージ
		int ObjectHitDamage;
		// 反射時のダメージ
		int ReflectionDamage;

		MasterBulletData();
		MasterBulletData( const json& j );
	} typedef MBulletData;

	struct MasterEnemySpawnData
	{
		// ID
		ulong Id;
		// スポーンさせるMEnemyId
		ulong MEnemyId;
		// MWaveId
		ulong MWaveId;
		// スポーンする座標
		D3DXVECTOR3 SpawnPoint;

		MasterEnemySpawnData();
		MasterEnemySpawnData( const json& j );
	} typedef MEnemySpawnData;

	struct MasterPlayerSmokeEffectData
	{
		// ID
		ulong Id;
		// ランダムで加算/減算するサイズ幅
		float AddSize;
		// 説明
		std::string Description;
		// エフェクトの色
		D3DXVECTOR3 EffectColor;
		// 終了時のアニメーション時間
		float EndAnimTime;
		// 最大サイズ
		float Size;
		// 描画順番
		int Sort;
		// 開始時のアニメーション時間
		float StartAnimTime;
		// 待機時間
		float WaitTime;

		MasterPlayerSmokeEffectData();
		MasterPlayerSmokeEffectData( const json& j );
	} typedef MPlayerSmokeEffectData;

	struct MasterStageData
	{
		// ID
		ulong Id;
		// MWaveIdリスト
		std::vector<ulong> MWaveIds;
		// ステージ名
		std::string Name;

		MasterStageData();
		MasterStageData( const json& j );
	} typedef MStageData;

	struct MasterWaveData
	{
		// ID
		ulong Id;
		// Waveの終了条件(どれかを満たしたとき)
		enum class EndTypes {
			Time,
		};
		std::vector<EndTypes> EndType;
		// 終了条件の値
		std::vector<float> EndTypeValue;

		MasterWaveData();
		MasterWaveData( const json& j );
	} typedef MWaveData;

}
using namespace MasterData;

namespace MasterDataUtility {
	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> CreateCache( const std::vector<std::pair<std::string, json>>& data );
	void CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const json& File );
}
#endif
