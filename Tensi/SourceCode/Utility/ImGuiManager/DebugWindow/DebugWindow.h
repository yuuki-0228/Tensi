#pragma once
#include "..\..\..\Global.h"
#include "..\ImGuiManager.h"

/************************************************
*	デバッグウィンドウクラス.
*		﨑田友輝.
**/
class DebugWindow final
{
public:
	DebugWindow();
	~DebugWindow();

	// 描画.
	static void Render();

	// ウィンドウに出力する.
	static void PushProc( const char* Name, const std::function<void()>& Proc );

private:
	// インスタンスの取得.
	static DebugWindow* GetInstance();

private:
	std::queue<std::function<void()>>	m_WatchQueue;		// ウォッチ階層用の関数のキュー.
	std::queue<std::function<void()>>	m_MenuQueue;		// デバッグメニュー階層用の関数のキュー.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	DebugWindow( const DebugWindow & )				= delete;
	DebugWindow& operator = ( const DebugWindow & )	= delete;
	DebugWindow( DebugWindow && )						= delete;
	DebugWindow& operator = ( DebugWindow && )		= delete;
};
