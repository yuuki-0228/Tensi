#pragma once
#include "..\..\Global.h"

//---------------------------.
//	共通のセーブデータ.
//---------------------------.
namespace CommonSaveDatas{
	//---------------------------.
	//	探索の保存データ.
	//	アプリを終了して再度立ち上げても復帰できる情報.
	//---------------------------.
	struct stExploreSaveData {
		bool		IsExploring;		// 探索中か.
		long long	StartTime;			// 探索の開始時刻( time_t ).
		long long	EndTime;			// 探索の完了予定時刻( time_t ).
		char		FolderPath[260];	// 探索中のフォルダのフルパス.
	} typedef SExploreSaveData;
}

//---------------------------.
//	セーブデータリスト.
//	「creator」で読み込み「CCommonSaveData」を作成します
//	例と同じになるように記入してください
//---------------------------.
namespace SaveData{
	struct stSetSaveDataList {
	//	( 型 ) *HogeHoge;					// 例
		STransform* PlayerTransform;		// プレイヤーのトランスフォーム.
		STransform* BallTransform;			// ボールのトランスフォーム.
		STransform* WaterFallTransform;		// 滝のトランスフォーム.
		STransform* WateringCanTransform;	// ジョウロのトランスフォーム.
		float* WateringCanWaterValue;		// ジョウロの水の量.
		CommonSaveDatas::SExploreSaveData* ExploreData;	// 探索の保存データ.
	} typedef SSetSaveDataList;
}
