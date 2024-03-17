#pragma once
#include "..\Widget.h"
#include "..\..\..\..\Scene\SceneManager\SceneManager.h"

/************************************************
*	シーンのUIベースクラス.
*		﨑田友輝.
**/
class CSceneWidget
	: public CWidget
{
public:
	using Render_List	= std::vector<std::pair<std::string, int>>;

public:
	CSceneWidget();
	virtual ~CSceneWidget();

	// UIエディタの更新.
	void UIEdtorUpdate( const bool IsDrag, const bool IsAutoSave );

	// 描画.
	virtual void Render() final;
	// UIエディタのImGuiの描画.
	void UIEdtorImGuiRender( const bool& IsAutoSave );

	// UIスプライトの追加.
	void AddUISpreite( const std::string& Name, CSprite* pSprite );

	// 表示しているスプライト全てのスプライト情報を保存する.
	HRESULT AllSpriteStateDataSave();
	// 表示しているスプライト全てのスプライト情報を読み込む.
	HRESULT AllSpriteStateDataLoad();

	// 当たり判定を描画するか.
	void SetIsDispHitBox( const bool& Flag, const D3DXVECTOR4& Color ) {
		m_IsDispHitBox	= Flag;
		m_HitBoxColor	= Color;
	}
	// ログファイルを作成するか.
	void SetIsCreaterLog( bool* pFlag ) {
		m_pIsCreaterLog = pFlag;
		for ( auto& [Key, List] : m_SpriteList ) {
			for( auto& s : List ) s->SetIsCreaterLog( pFlag );
		}
	}
	// ログリストの設定.
	void SetLogList( std::vector<std::string>* pLogList ) {
		m_pLogList = pLogList;
		for ( auto& [Key, List] : m_SpriteList ) {
			for ( auto& s : List ) s->SetLogList( pLogList );
		}
	}

protected:
	// UIエディタの前に追加で行う描画.
	virtual void FastAddRender() {};
	// UIエディタの後に追加で行う描画.
	virtual void LateAddRender() {};

	// シーン名の設定.
	//	Init関数の初めに呼ぶ.
	void SetSceneName( const ESceneList& NowScene );

	// スプライトリストを取得する.
	//	Init関数の初めに呼ぶ.
	void GetSpriteList();

private:
	// 描画順番のソート.
	void RenderSort();

protected:
	Sprite_map					m_SpriteList;		// スプライトリスト.
	State_map					m_SpriteState;		// スプライト情報.

private:
	std::string					m_SceneName;		// シーン名.
	Render_List					m_RenderList;		// 描画順番リスト.
	std::vector<std::string>	m_DeleteList;		// 削除したスプライトリスト.
	std::vector<std::string>*	m_pLogList;			// ログリスト.
	D3DXVECTOR4					m_HitBoxColor;		// 当たり判定の色.
	bool						m_IsDispHitBox;		// 当たり判定を描画するか.
	bool*						m_pIsCreaterLog;	// ログファイルを作成するか.
};
