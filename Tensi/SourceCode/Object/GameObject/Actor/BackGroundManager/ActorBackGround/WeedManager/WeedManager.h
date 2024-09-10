#pragma once
#include "..\..\..\Actor.h"
#include "Weed/WeedSaveData.h"

class CWeed;

/************************************************
*	雑草マネージャークラス.
**/
class CWeedManager final
	: public CActor
{
public:
	CWeedManager();
	~CWeedManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 雑草を埋める.
	void Fill( const int Num );

	// 表示している本数の取得
	int GetNum();

private:
	// 保存.
	std::vector<SWeedData> Save();
	//　読み込み.
	void Load( std::vector<SWeedData> Data );

	// 雑草リストの削除.
	void WeedListDelete();

private:
	std::vector<std::unique_ptr<CWeed>> m_pWeedList;	// 雑草リスト.
	float								m_WeedSize;		// 雑草の画像サイズ.
};