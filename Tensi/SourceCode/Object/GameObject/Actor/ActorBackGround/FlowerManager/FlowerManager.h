#pragma once
#include "..\..\Actor.h"
#include "Flower/FlowerSaveData.h"

class CFlower;

/************************************************
*	花マネージャークラス.
**/
class CFlowerManager final
	: public CActor
{
public:
	CFlowerManager();
	~CFlowerManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 花を埋める.
	void Fill( const int Num );

	// 表示している本数の取得
	int GetNum();

private:
	// 保存.
	std::vector<SFlowerData> Save();
	//　読み込み.
	void Load( std::vector<SFlowerData> Data );

	// 花リストの削除.
	void FlowerListDelete();

	// 花のアニメーション用カウントの更新.
	void FlowerAnimCntUpdate();

private:
	std::vector<std::unique_ptr<CFlower>>	m_pFlowerList;		// 花リスト.
	float									m_FlowerSize;		// 花の画像サイズ.
	float									m_FlowerAnimCnt;	// 花のアニメーション用カウント.
};
