#pragma once
#include "..\Actor.h"

class CGroundManager;
class CFlowerManager;
class CWeedManager;
class CWaterFall;
//class CTree;
//class CHouse;

/************************************************
*	背景オブジェクトマネージャークラス.
**/
class BackGroundManager
{
public:
	BackGroundManager();
	~BackGroundManager();

	// 初期化
	static bool Init();
	// 初回ログイン時の初期化.
	static bool FirstPlayInit();
	// 初回ログイン時の初期化.
	static bool LoginInit( const std::tm& now, const std::tm& diff );

	// 更新.
	static void Update( const float& DeltaTime );
	// 当たり判定終了後呼び出される更新.
	static void LateUpdate( const float& DeltaTime );
	// デバックの更新.
	static void DebugUpdate( const float& DeltaTime );

	// 描画.
	static void Render();
	static void SubRender();

private:
	// インスタンスの取得.
	static BackGroundManager* GetInstance();

private:
	std::shared_ptr<CWeedManager>			m_pWeedManager;		// 雑草.
	std::shared_ptr<CFlowerManager>			m_pFlowerManager;	// 花.
	std::shared_ptr<CWaterFall>				m_pWaterFall;		// 滝.
//	std::shared_ptr<CTree>					m_pTree;			// 木.
//	std::shared_ptr<CHouse>					m_pHouse;			// 家.

};
