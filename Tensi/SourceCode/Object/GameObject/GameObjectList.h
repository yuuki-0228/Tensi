#pragma once

// オブジェクトタグリスト.
enum class enObjectTagList : unsigned int
{
	None,

	Player,			// プレイヤー.
	Ball,			// ボール.
	HeavyBall,		// 重いボール.
	SuperBall,		// スーパーボール.
	Weed,			// 雑草.
	Flower,			// 花.
	WaterFall,		// 滝.
	WateringCan,	// ジョウロ.
	Water,			// 水.

	Max,

} typedef EObjectTag;
