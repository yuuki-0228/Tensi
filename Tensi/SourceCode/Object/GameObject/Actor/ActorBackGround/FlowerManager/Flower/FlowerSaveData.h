#pragma once
#include "..\..\..\..\..\..\Global.h"

struct stFlowerData
{
	STransform	Transform;		// トランスフォーム
	D3DXCOLOR4	Color;			// 色.
	std::tm		FillDay;		// 埋めた日
	std::tm		WitherDay;		// 枯れた日
	std::tm		WateringDay;	// 最新の水やりした日
	int			FlowerType;		// 花のタイプ
	bool		IsWither;		// 枯れているか
	bool		IsWatering;		// 今日水やりしたか
} typedef SFlowerData;