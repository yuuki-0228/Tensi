#pragma once
#include "..\..\Actor.h"

class CWaterFallTile;

/************************************************
*	滝クラス.
**/
class CWaterFall final
	: public CActor
{
public:
	CWaterFall();
	~CWaterFall();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 滝の設定.
	void Setting();

private:
	// 滝を作る.
	void CreateWaterFall();

private:
	std::vector<std::unique_ptr<CWaterFallTile>>	m_WaterFallTileList;	// 滝のタイルリスト
	float											m_ImageSize;			// 画像のサイズ
};