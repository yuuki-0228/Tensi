#pragma once
#include "..\SystemBase.h"

/************************************************
*	ウィンドウ設定クラス.
*		﨑田友輝.
**/
class CWindowSetting final
	: public CSystemBase
{
public:
	CWindowSetting();
	~CWindowSetting();

protected:
	// 初期化関数.
	virtual void Init() override;

private:
	D3DXVECTOR3		m_Color;				// 背景の色.
	D3DXVECTOR2		m_Size;					// ウィンドウサイズ.
	D3DXPOSITION2	m_Pos;					// ウィンドウの表示位置.
	std::string		m_AppName;				// アプリ名.
	std::string		m_WndName;				// ウィンドウ名.
	bool			m_IsDispCenter;			// 真ん中に表示するか.
	bool			m_IsDispMouseCursor;	// マウスカーソルを表示するか.
	bool			m_IsFPSRender;			// 画面にFPSを描画するか.
	bool			m_IsFullScreenLock;		// フルスクリーンを無効化するか.
	bool			m_IsSizeLock;			// サイズを固定化する.
	bool			m_IsStartFullScreen;	// フルスクリーンで起動するか.
	bool			m_IsMaxLock;			// 最大化ボタンを無効化する.
	bool			m_IsMinLock;			// 最小化ボタンを無効化する.
	bool			m_IsPopUpWnd;			// ポップアップウィンドウ(枠無し)にするか.
	bool			m_IsShowOnTop;			// 最前面で固定化する.
	bool			m_IsLogStop;			// ログファイルを停止するか.
	bool			m_IsInputNotActiveStop;	// アクティブウィンドウでは無い場合操作を停止させるか.
	bool			m_IsDispCloseMessage;	// ウィンドウを閉じるメッセージボックスを表示するか.
	std::string		m_Version;				// アプリバージョン.
};
