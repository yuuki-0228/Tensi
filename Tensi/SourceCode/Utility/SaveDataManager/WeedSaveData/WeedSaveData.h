#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Object\GameObject\Actor\ActorBackGround\WeedManager\Weed\WeedSaveData.h"
#include <functional>

/************************************************
*	草のセーブデータクラス.
**/
class CWeedSaveData final
{
public:
	using SaveFunction = std::function<std::vector<SWeedData>()>;
	using LoadFunction = std::function<void( std::vector<SWeedData> )>;

public:
	CWeedSaveData();
	~CWeedSaveData();

	// 保存時に使用する関数の設定
	void SetSaveFunction( const SaveFunction& Function ) { m_SaveFunction = Function; }
	// 読み込み時に使用する関数の設定
	void SetLoadFunction( const LoadFunction& Function ) { m_LoadFunction = Function; }

	// 保存.
	void Save();
	// 読み込み.
	void Load();

private:
	SaveFunction m_SaveFunction;	// 保存時に使用する関数
	LoadFunction m_LoadFunction;	// 読み込み時に使用する関数
};