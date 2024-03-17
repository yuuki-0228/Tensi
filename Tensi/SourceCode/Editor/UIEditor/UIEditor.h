#pragma once
#include "..\EditorBase.h"
#include "..\..\Scene\SceneManager\SceneManager.h"

class CSceneWidget;

/************************************************
*	UIエディタ.
*		﨑田友輝.
**/
class CUIEditor final
	: public CEditorBase
{
public:
	CUIEditor();
	virtual ~CUIEditor();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// ImGui描画.
	virtual void ImGuiRender() override;
	// モデルの描画.
	virtual void ModelRender() override {};
	// UIの描画.
	virtual void UIRender() override;
	// エフェクト描画.
	virtual void EffectRneder() override {};

private:
	// 表示するシーンの変更.
	void ChangeDispScene( const bool IsSaveStop = false );

	// ログファイルの読み込み.
	HRESULT ZLogLoad();

	// ログファイルの読み込み.
	HRESULT YLogLoad();

	// 保存.
	void Save();
	// 元に戻す.
	void Undo();
	// やり直す.
	void Redu();

private:
	std::unique_ptr<CSceneWidget>	m_pUI;				// UI.
	ESceneList						m_DispScene;		// 表示しているシーン.
	ESceneList						m_UIScene;			// UIの表示するシーン.

	const D3DXCOLOR4				m_OldBackColor;		// 昔の背景画像.
	D3DXCOLOR4						m_BackColor;		// 背景カラー.
	D3DXCOLOR4						m_HitBoxColor;		// 当たり判定カラー.

	int								m_GridSpace;		// グリッド線の間隔.
	CSprite*						m_pGrid;			// グリット線.
	SSpriteRenderState				m_GridState;		// グリッド情報.

	std::string						m_SelectUI;			// 選択しているUI.
	std::vector<std::string>		m_UndoLogList;		// 元に戻すログリスト.
	std::vector<std::string>		m_ReduLogList;		// やり直すログリスト.
	int								m_LogMax;			// ログの上限.

	bool							m_IsAutoSave;		// 自動で保存するか.
	bool							m_IsCreaterLog;		// ログを作成するか.
	bool							m_IsDrag;			// マウスでつかめる.
	bool							m_IsDispHitBox;		// 当たり判定を描画するか.
	bool							m_IsWidgetUpdate;	// UIの更新を行うか.
	bool							m_IsFileDelete;		// ファイルを削除するか.
};