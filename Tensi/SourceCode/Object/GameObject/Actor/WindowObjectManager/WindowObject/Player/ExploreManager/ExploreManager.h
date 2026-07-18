#pragma once
#include "..\..\..\..\..\..\..\Global.h"
#include "..\..\..\..\..\..\..\Common\Sprite\SpriteStruct.h"
#include "..\..\..\..\..\..\..\Common\Font\FontStruct.h"
#include "..\..\..\..\..\..\..\Utility\SaveDataManager\ExploreSaveData.h"
#include <memory>
#include <string>
#include <atomic>

class CSprite;
class CFont;
class CIconSmokeEffectManager;

/************************************************
*	スライムの探索マネージャークラス.
*	スライムをフォルダに潜らせて探索させる一連の処理を管理する.
**/
class CExploreManager final
{
public:
	// 探索の状態.
	enum class enState : unsigned char {
		None,		// 探索していない( 通常 ).
		ShrinkIn,	// アイコンに縮小して入っていく.
		Ask,		// 冒険に行かせるか確認する.
		Exploring,	// 探索中.
		ExpandOut,	// アイコンから拡大して飛び出す.
	};

	CExploreManager();
	~CExploreManager();

	// 初期化( スライムの画像と状態、見た目基準サイズ( 当たり判定サイズ等 )を受け取る ).
	void Init( CSprite* pSlimeSprite, SSpriteRenderState* pSlimeState, const float SlimeRefSize );

	// 更新.
	void Update( const float& DeltaTime );

	// サブウィンドウ( アイコンの後ろ )に描画.
	void SubRender();

	// 探索を開始できるか確認し、できるなら開始する( 戻り値: 開始したか ).
	//	スライムを離した瞬間に呼ぶ.
	bool TryStart( const D3DXPOSITION3& SlimeCenter );

	// 探索に関する処理中か( スライムを乗っ取っているか ).
	bool GetIsActive() const { return m_State != enState::None; }

	// スライムをサブウィンドウに描画するフェーズか.
	bool IsSlimeInSub() const;

private:
	// 各状態の更新.
	void ShrinkInUpdate( const float& DeltaTime );
	void AskUpdate();
	void ExploringUpdate( const float& DeltaTime );
	void ExpandOutUpdate( const float& DeltaTime );

	// 探索を開始する( はいを押した時 ).
	void StartExplore();
	// アイコンから飛び出す演出を開始する( いいえ・完了・中止時 ).
	void StartExpandOut();
	// 探索の情報を初期化してセーブデータを解除する.
	void ClearSaveData();

	// 進捗がMAXになった時の処理( 特定の処理 ).
	void OnExploreComplete();

	// 読み込み時の復帰処理.
	void OnLoaded();

	// 対象フォルダのアイコンを探し情報を更新する( 見つからなければ false ).
	bool LocateTargetIcon();
	// アイコンの矩形から目標位置・目標スケールを求める.
	void UpdateTarget();

	// ゲージと残り時間の描画.
	void GaugeSubRender();
	// 残り時間の文字列を作成する.
	std::string MakeRemainText( const long long RemainSec ) const;
	// 進捗率( 0.0～1.0 )を取得.
	float GetProgress() const;

private:
	CSprite*								m_pSlimeSprite;		// スライムの画像.
	SSpriteRenderState*						m_pSlimeState;		// スライムの画像の状態.
	std::unique_ptr<CIconSmokeEffectManager>m_pSmoke;			// 煙エフェクト.
	CSprite*								m_pGaugeSprite;		// ゲージの画像.
	SSpriteRenderState						m_GaugeBackState;	// ゲージの背景( 白 )の状態.
	SSpriteRenderState						m_GaugeState;		// ゲージ( 黄 )の状態.
	CFont*									m_pFont;			// 残り時間の文字.
	SFontRenderState						m_FontState;		// 残り時間の文字の状態.
	SExploreSaveData						m_SaveData;			// 探索のセーブデータ.
	std::string								m_FolderPath;		// 対象フォルダのフルパス.
	enState									m_State;			// 現在の状態.
	RECT									m_IconClickRect;	// 対象アイコンのクリック範囲.
	RECT									m_IconDrawRect;		// 対象アイコンの描画範囲.
	int										m_IconIndex;		// 対象アイコンのインデックス.
	D3DXPOSITION3							m_StartPos;			// 縮小・拡大の開始位置.
	D3DXPOSITION3							m_TargetCenter;		// 対象アイコンの中心位置.
	float									m_StartScale;		// 縮小・拡大の開始スケール.
	float									m_TargetScale;		// アイコンサイズのスケール.
	float									m_Timer;			// 状態の経過時間.
	float									m_PeekCoolTime;		// 頭出しをするまでの時間.
	float									m_PeekTime;			// 頭出ししている残り時間.
	float									m_IconPollCoolTime;	// デスクトップのアイコン情報を再取得するまでの時間.
	float									m_SlimeDispW;		// スライム画像の幅.
	float									m_SlimeDispH;		// スライム画像の高さ.
	float									m_SlimeRefSize;		// スライムの見た目基準サイズ( 当たり判定サイズ等. 画像の余白を含まない ).
	float									m_SmokeDispW;		// 煙画像の幅.
	float									m_GaugeDispW;		// ゲージ画像の幅.
	float									m_GaugeDispH;		// ゲージ画像の高さ.
	float									m_FontDispH;		// フォント1文字の高さ.
	bool									m_IsPeeking;		// 頭出し中か.
	bool									m_IsAskShown;		// 確認ダイアログを表示済みか.
	std::atomic<int>						m_AskAnswer;		// 確認ダイアログの回答( 0=未回答 / 1=はい / 2=いいえ ).
};
