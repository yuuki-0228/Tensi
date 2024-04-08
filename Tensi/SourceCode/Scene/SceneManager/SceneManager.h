#pragma once
#include "..\..\Global.h"
#include "..\FadeManager\FadeManager.h"

class CSceneBase;

// シーンリスト.
enum class enSceneList : unsigned char
{
	None,

	GameMain,

	UIEdit,
	Max
} typedef ESceneList;

/************************************************
*	シーンマネージャー.
**/
class SceneManager final
{
public:
	SceneManager();
	~SceneManager();

	// 更新.
	static void	Update( const float& DeltaTime );
	// 描画.
	static void	Render();
	// サブウィンドウの描画.
	static void	SubRender();

	// シーンの変更.
	static void SceneChange( const ESceneList NewScene, const EFadeType FadeType = EFadeType::None, const float FadeSpeed = 0.05f );

	// シーンを停止させるか設定.
	static void SetIsSceneStop( const bool Flag ) { GetInstance()->m_IsSceneStop = Flag; }
	// ポーズしているか設定.
	static void SetIsPause( const bool Flag ) { GetInstance()->m_IsPause = Flag; }

	// シーンを停止させているか取得.
	static bool GetIsSceneStop() { return GetInstance()->m_IsSceneStop; }
	// ポーズしているか取得.
	static bool GetIsPause() { return GetInstance()->m_IsPause; }

private:
	// インスタンスの取得.
	static SceneManager* GetInstance();

	// シーンの変更の更新.
	static void SceneChangeUpdate();

private:
	HWND						m_hWnd;				// ウィンドウハンドル.
	std::unique_ptr<CSceneBase> m_pScene;			// シーン.
	ESceneList					m_NowScene;			// 現在のシーン.
	ESceneList					m_ChangeScene;		// 変更するシーン.
	EFadeType					m_FadeType;			// フェードタイプ.
	float						m_FadeSpeed;		// フェード速度.
	bool						m_IsSceneChange;	// シーン変更しているか.
	bool						m_IsFadeOut;		// フェードアウトか.
	bool						m_UpdateStopFlag;	// 更新を停止させる.
	bool						m_IsSceneStop;		// シーンを停止させているか.
	bool						m_IsPause;			// ポーズ中か.
	CBool						m_IsUIRender;		// UIを表示するか.
	float						m_AutoSaveCnt;		// オートセーブのカウント.
};
