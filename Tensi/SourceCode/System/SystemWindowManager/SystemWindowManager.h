#pragma once
#include "..\..\Global.h"

class InputSetting;
class CWindowSetting;
class CBugReport;
class CSprite;

/************************************************
*	システム設定をするウィンドウマネージャー.
*		﨑田友輝.
**/
class SystemWindowManager final
{
public:
	SystemWindowManager();
	~SystemWindowManager();

	// 更新.
	static void Update( const float& DeltaTime );
	// 描画.
	static void Render();

private:
	// インスタンスの取得.
	static SystemWindowManager* GetInstance();

private:
	std::unique_ptr<InputSetting>	m_pInputSetting;	// 入力設定を行うウィンドウ.
	std::unique_ptr<CWindowSetting> m_pWindowSetting;	// ウィンドウ設定を行うウィンドウ.
	std::unique_ptr<CBugReport>		m_pBugReport;		// バグ報告を行うウィンドウ.
	CSprite*						m_pButton;			// ボタン.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	SystemWindowManager( const SystemWindowManager & )				= delete;
	SystemWindowManager& operator = ( const SystemWindowManager & )	= delete;
	SystemWindowManager( SystemWindowManager && )						= delete;
	SystemWindowManager& operator = ( SystemWindowManager && )		= delete;
};
