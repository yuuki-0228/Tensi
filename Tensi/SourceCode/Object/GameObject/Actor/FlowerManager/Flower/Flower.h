#pragma once
#include "..\..\Actor.h"
#include "FlowerSaveData.h"

class CWateringAnimManager;

/************************************************
*	花クラス.
**/
class CFlower final
	: public CActor
{
public:
	// 花の種類
	enum class enFlowerType : unsigned char {
		A, B, C, D, E, F,
		Max
	} typedef EFlowerType;

public:
	CFlower();
	~CFlower();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 当たり判定関数.
	virtual void Collision( CActor* pActor ) override;

	// 花のデータを取得.
	SFlowerData GetFlowerData();
	// 花のデータを設定.
	void SetFlowerData( const SFlowerData& Data );

	// 花のアニメーション用カウントの設定.
	void SetFlowerAnim( const float Cnt ) { m_FlowerAnimCnt = Cnt; }

	// 花を埋める.
	void Fill( const int Type, const D3DXCOLOR3& Color, const D3DXPOSITION3& Pos );

protected:
	// 当たり判定の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitCollision() override;
	// 当たり判定の更新.
	//	Update関数の最後に呼ぶ.
	virtual void UpdateCollision() override;

private:
	// マウスに触れているか.
	bool GetIsTouchMouse();

	// 花に何かした時のリアクションの設定.
	void Reaction();
	// 枯れさせる.
	void Wither();

	// 花をクリックした時の更新
	void FlowerClickUpdate();
	// 花のアニメーションの更新
	void FlowerAnimUpdate();
	// マウスによる花のアニメーションの更新.
	void MouseTouchReactionUpdate();
	// 花に触れた時のアニメーションの更新.
	void ReactionAnimUpdate();
	// 水やり完了アニメーションの待機時間の更新.
	void WateringAnimCoolTimeUpdate();
	// 水やり完了アニメーションの更新.
	void WateringAnimUpdate();

	// プレイヤーとボールの当たり判定
	void PlayerBallCollision( CActor* pActor, const EObjectTag Tag );
	// 水の当たり判定
	void WaterCollision( CActor* pActor, const EObjectTag Tag );

private:
	CSprite*								m_pFlower;				// 花の画像.
	SSpriteRenderState						m_LeafState;			// 葉っぱの画像の情報.
	SSpriteRenderState						m_FlowerState;			// 花の画像の情報.
	SFlowerData								m_FlowerSaveData;		// 花のセーブデータ.
	std::unique_ptr<CWateringAnimManager>	m_pWateringAnim;		// 水やりアニメーション.
	D3DXPOSITION3							m_AddCenterPosition;	// 中心座標に変換するため用の座標.
	float									m_FlowerSize;			// 花の画像サイズ.
	float									m_FlowerAnimCnt;		// 花のアニメーション用カウント.
	float									m_ReactionAnimCnt;		// 花の触れた時のアニメーション用カウント.
	float									m_WateringAnimCnt;		// 水やり後アニメーション用カウント.
	bool									m_IsReactionAnim;		// 花に触れた時のアニメーション.
	int										m_WateringCnt;			// 水やりカウント
};