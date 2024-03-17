#pragma once
#include "..\..\..\Actor.h"
#include "WaterData.h"

class CWater;

/************************************************
*	水マネージャークラス.
**/
class CWaterManager final
	: public CActor
{
public:
	CWaterManager();
	~CWaterManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 水を表示する
	void Spill( const SWaterData& data, const HWND hWnd );

private:
	// 水リストの削除.
	void WaterListDelete();

private:
	std::vector<std::unique_ptr<CWater>> m_pWaterList;	// 水.
};