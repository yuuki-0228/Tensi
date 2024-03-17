#pragma once
#include "..\Global.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"
#include <functional>

/************************************************
*	システム設定のベースクラス.
*		﨑田友輝.
**/
class CSystemBase
{
public:
	CSystemBase();
	virtual ~CSystemBase();

	// 描画.
	virtual void Render() final;
	// 表示.
	void Open() { m_IsOpen = true; }

protected:
	// 初期化関数.
	//	m_RenderFuncに描画処理を記述する.
	virtual void Init() {};

	// ウィンドウ名の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitWndName( const std::string& Name ) final;

protected:
	std::function<void()>	m_RenderFunc;	// 描画処理.
	CBool					m_IsOpen;		// 開くかどうか.

private:
	std::string				m_WndName;		// ウィンドウ名.
	bool					m_IsOldOpen;	// 前回の開いているか.
};
