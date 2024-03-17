#pragma once
#include "..\Actor.h"
#include "..\..\Actor\WindowObjectManager\WindowObjectManager.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\Utility\WindowManager\WindowManager.h"

/************************************************
*	ウィンドウのオブジェクト.
**/
class CWindowObject
	: public CActor
{
public:
	CWindowObject();
	virtual ~CWindowObject();

	// 表示するかの設定.
	void SetIsDisp( const bool Flag ) { m_IsDisp = Flag; }

	// マウスに触れているか
	bool GetIsMouseOver();

	// ウィンドウの中に入っているか.
	bool GetIsInWindow() { return m_InWndHandle != NULL; }

protected:
	// ウィンドウオブジェクトの初期化.
	void WindowObjectInit();
	// ウィンドウオブジェクトの更新.
	void WindowObjectUpdate();
	// ウィンドウオブジェクトの当たり判定後の更新.
	void WindowObjectLateUpdate();

	// 掴めるかの確認.
	virtual bool GrabCheck() { return true; }
	// 掴んだ時の初期化.
	virtual void GrabInit() {};
	// 掴みの更新.
	virtual void GrabUpdate() {};
	// 離した時の初期化.
	virtual void SeparateInit() {};

	// 移動時の更新.
	virtual void MoveUpdate() {};

	// 上に当たった時の更新.
	virtual void HitUpUpdate() {};
	// 下に当たった時の更新.
	virtual void HitDownUpdate() {};
	// 左に当たった時の更新.
	virtual void HitLeftUpdate() {};
	// 右に当たった時の更新.
	virtual void HitRightUpdate() {};

	// 着地時の初期化.
	virtual void LandingInit() {};
	// 着地の更新.
	virtual void LandingUpdate() {};

private:
	// 掴む.
	void Grab();
	// 掴んでいる時の更新.
	void GrabMoveUpdate();
	// 掴んでいる物を離す.
	void Separate();
	// ウィンドウの中に入れるかのチェック.
	void InWindowCheck();

	// ごみ箱に移動できるか確認.
	void TeleportTrashCanCheck();
	// ウィンドウから出す.
	void EscapeWindow();

	// 重力の追加.
	void PushGravity();
	// 移動ベクトルの作成.
	void CreateMoveVector();
	// 移動ベクトルの初期化.
	void InitMoveVector();
	// 移動.
	void Move();

	// 他ウィンドウの当たり判定.
	void WindowCollision();
	// 中に入っているウィンドウとの当たり判定.
	void InWindowCollision();
	// スクリーンの当たり判定.
	void ScreenCollision();

	// 上側に当たった.
	void HitUp( const RECT& HitWnd, const bool IsFromOutside = true );
	// 下側に当たった.
	void HitDown( const RECT& HitWnd, const bool IsFromOutside = true );
	// 左側に当たった.
	void HitLeft( const RECT& HitWnd, const bool IsFromOutside = true );
	// 右側に当たった.
	void HitRight( const RECT& HitWnd, const bool IsFromOutside = true );

	// 着地.
	void Landing( const RECT& HitWnd );

protected:
	CSprite*					m_pSprite;			// 画像.
	SSpriteRenderState			m_SpriteState;		// 画像の情報.
	std::vector<D3DXVECTOR3>	m_VectorList;		// ベクトルリスト.
	D3DXVECTOR3					m_OldMoveVector;	// 前回の移動ベクトル.
	D3DXVECTOR3					m_MoveVector;		// 移動ベクトル.
	D3DXPOSITION2				m_GrabPosition;		// 掴みの補正値.
	D3DXPOSITION3				m_StartPosition;	// 開始座標.
	D3DXPOSITION3				m_EndPosition;		// 終了座標.
	D3DXVECTOR4					m_DireSpeedRate;	// 方向ごとの速度の保存率( 右,下,左,上 ).
	HWND						m_InWndHandle;		// 中に入っているウィンドウのハンドル.
	RECT						m_LandingWnd;		// 着地しているウィンドウ情報.
	RECT						m_AddWndRect;		// ウィンドウの情報に追加する補正値.
	float						m_CollSize;			// 当たり判定のサイズ.
	float						m_GravityPower;		// 重力の強さ.
	float						m_SpeedRate;		// 速度の保存率.
	float						m_CeilingPosY;		// 天井のY位置.
	bool						m_IsLanding;		// 着地しているか.
	bool						m_IsFall;			// 落下しているか
	bool						m_IsGrab;			// 掴んでいるか.
	bool						m_IsInWndSmall;		// 中に入ってるウィンドウが最小化されているか.
	bool						m_IsTrashCan;		// ごみ箱に移動させるか.
	bool						m_IsDisp;			// 表示されるか.

private:
	D3DXPOSITION3				m_AddCenterPosition;// 中心座標に変換するため用の座標.
};