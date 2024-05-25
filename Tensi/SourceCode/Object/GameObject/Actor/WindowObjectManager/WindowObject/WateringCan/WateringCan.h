#pragma once
#include "..\WindowObject.h"
#include "..\WaterManager/WaterData.h"

class CWaterManager;

/************************************************
*	ジョウロクラス.
**/
class CWateringCan final
	: public CWindowObject
{
public:
	CWateringCan();
	~CWateringCan();

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

protected:
	// 当たり判定の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitCollision() override;
	// 当たり判定の更新.
	//	Update関数の最後に呼ぶ.
	virtual void UpdateCollision() override;

	// 掴みの更新.
	virtual void GrabUpdate();
	// 離した時の初期化.
	virtual void SeparateInit();

	// 上に当たった時の更新.
	virtual void HitUpUpdate() override;
	// 下に当たった時の更新.
	virtual void HitDownUpdate() override;
	// 左に当たった時の更新.
	virtual void HitLeftUpdate() override;
	// 右に当たった時の更新.
	virtual void HitRightUpdate() override;

private:
	// トランスフォームの初期化
	void InitTransform();
	// 水のデータの初期化.
	void InitWaterData();

	// 落下速度の更新.
	void FallSpeedUpdate();
	// ゲージの更新
	void GaugeUpdate();
	// クールタイムの更新.
	void CoolTimeUpdate();
	// 水やりの更新.
	void WateringUpdate();

	// 水を減らす
	float DecreaseWater( const float rate );
	// 水の量を変動させる
	void ChangeWaterValue( const float value );

	// 満タンの時の水のエフェクトの再生.
	void FullWaterPlay();
	// 地面にぶつかった時の水のエフェクトの再生
	void HitWaterPlay( const float rate, const float decWater );
	// 水やりの水のエフェクトの再生
	void WateringWaterPlay();

private:
	CSprite*						m_pWaterGaugeSprite;	// 残りの水ゲージの画像
	SSpriteRenderState				m_WaterGaugeBackState;	// 残りの水ゲージの背景情報.
	SSpriteRenderState				m_WaterGaugeState;		// 残りの水ゲージの情報.
	std::unique_ptr<CWaterManager>	m_pWaterEffect;			// 水.
	STransform						m_LeftWaterTransform;	// 左の水がこぼれる場所.
	STransform						m_RightWaterTransform;	// 右の水がこぼれる場所.
	STransform						m_CenterWaterTransform;	// 真ん中の場所.
	STransform						m_WateringTransform;	// 水やりの水の場所.
	SWaterData						m_LHitWaterData;		// 左の落下時の水のデータ.
	SWaterData						m_RHitWaterData;		// 右の落下時の水のデータ.
	SWaterData						m_LFullWaterData;		// 左の満タン時の水のデータ.
	SWaterData						m_RFullWaterData;		// 右の満タン時の水のデータ.
	SWaterData						m_WateringWaterData;	// 水やり時の水のデータ.
	float							m_WaterValue;			// 残りの水の量.
	float							m_FullAnimCoolTime;		// 満タンの時のアニメーションの待機時間(秒)
	float							m_GaugeDispTime;		// ゲージを表示している時間
	float							m_GaugeAlpha;			// 水ゲージのアルファ値
	bool							m_IsWatering;			// 水やり中か.
	bool							m_IsGaugeDisp;			// ゲージを無理やり表示する.
};
