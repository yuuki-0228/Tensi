#pragma once
#include "..\..\ActorBackGround.h"
#include "WeedSaveData.h"

/************************************************
*	雑草クラス.
**/
class CWeed final
	: public CActorBackGround
{
public:
	CWeed();
	~CWeed();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void SubRender() override;

	// 当たり判定関数.
	virtual void Collision( CActor* pActor ) override;

	// 草のデータの取得.
	SWeedData GetWeedData();
	// 草のデータの設定.
	void SetWeedData( const SWeedData& t );

	// 雑草を埋める.
	void Fill( const D3DXPOSITION3& Pos );

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

	// 雑草を揺らす.
	void Shake( const D3DXPOSITION3& Pos, const D3DXPOSITION3& OldPos );

	// 雑草を掴むの更新.
	void GrabUptate();
	// マウスによる雑草の揺れの更新.
	void MouseShakeUpdate();
	// 雑草を掴んでいる時の更新.
	void GrabUpdate();
	// 雑草が抜けるアニメーションの更新.
	void ComeOutAnimUptate();

private:
	CSprite*			m_pWeed;				// 雑草の画像.
	SSpriteRenderState	m_WeedState;			// 雑草の画像の情報.
	D3DXPOSITION3		m_AddCenterPosition;	// 中心座標に変換するため用の座標.
	float				m_WeedSize;				// 雑草の画像サイズ.
	bool				m_IsGrab;				// つかんでいるか.
	bool				m_IsComeOut;			// 雑草が抜けるか.
};