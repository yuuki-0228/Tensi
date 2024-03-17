#pragma once
#include "..\..\Global.h"

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
	} typedef SSetSaveDataList;
}