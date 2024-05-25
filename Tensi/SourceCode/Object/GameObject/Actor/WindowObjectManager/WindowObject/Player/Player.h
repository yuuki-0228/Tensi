#pragma once
#include "..\WindowObject.h"

class CSleepEffectManager;
class CNoteEffectManager;

/************************************************
*	プレイヤークラス.
**/
class CPlayer final
	: public CWindowObject
{
public:
	CPlayer();
	~CPlayer();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;
	// 当たり判定終了後呼び出される更新.
	virtual void LateUpdate( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 当たり判定関数.
	virtual void Collision( CActor* pActor ) override;

	// スライムをつぶす.
	void Collapsed();

protected:
	// 当たり判定の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitCollision() override;
	// 当たり判定の更新.
	//	Update関数の最後に呼ぶ.
	virtual void UpdateCollision() override;

	// 掴めるかの確認.
	virtual bool GrabCheck() override;
	// 掴んだ時の初期化.
	virtual void GrabInit() override;

	// 移動時の更新.
	virtual void MoveUpdate() override;

	// 上に当たった時の更新.
	virtual void HitUpUpdate() override;
	// 下に当たった時の更新.
	virtual void HitDownUpdate() override;
	// 左に当たった時の更新.
	virtual void HitLeftUpdate() override;
	// 右に当たった時の更新.
	virtual void HitRightUpdate() override;

	// 着地時の初期化.
	virtual void LandingInit() override;
	// 着地の更新.
	virtual void LandingUpdate() override;

private:
	// エフェクトの更新.
	void EffectUpdate();

	// 行動の終了時の初期化.
	void ActionEnd();
	// 行動の開始の初期化.
	void ActionStart();
	// 行動の更新.
	void ActionUpdate();
	// 待機の行動の更新.
	void ActionWaitUpdate();
	// 移動の行動の更新.
	void ActionMoveUpdate();
	// ボールで遊ぶ行動の更新.
	void ActionPlayWithBallUpdate();
	// ジャンプの行動の更新.
	void ActionJumpUpdate();
	// 寝るの行動の更新.
	void ActionSleepUpdate();
	// 音が鳴ってるときの行動の更新.
	void ActionMusicUpdate();

private:
	CSprite*								m_pCollapsed;		// つぶれた画像.
	SSpriteRenderState						m_CollapsedState;	// つぶれた画像の情報.
	std::unique_ptr<CSleepEffectManager>	m_pSleepEffect;		// 眠りエフェクト.
	std::unique_ptr<CNoteEffectManager>		m_pNoteEffect;		// 音符エフェクト.
	float									m_ActionTime;		// 行動の時間.
	float									m_MoveTimeCnt;		// 移動時間.
	float									m_CoolTime;			// クールタイム.
	int										m_Action;			// 現在の行動.
	bool									m_IsLandingWait;	// 着地硬直中か.
	bool									m_IsOldMoveLeft;	// 前回左方向に移動していたか.
};