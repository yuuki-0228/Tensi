#pragma once
#include "..\SceneBase.h"

class CGameMainWidget;
class CPlayer;
class CBall;
class CHeavyBall;
class CSuperBall;
class CWateringCan;
class CSlimeFrame;
class CTree;
class CWeedManager;
class CFlowerManager;
class CHouse;
class CWaterFall;

/************************************************
*	ゲームクラス.
*		﨑田友輝.
**/
class CGameMain final
	: public CSceneBase
{
public:
	CGameMain();
	~CGameMain();

	// 初期化.
	virtual bool Init() override;
	// 初回起動時の初期化.
	virtual bool FirstPlayInit() override;
	// 初回ログイン時の初期化.
	virtual bool LoginInit( std::tm lastDay ) override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// モデルの描画.
	virtual void ModelRender() override;
	// スプライト(UI)の描画.
	virtual void SpriteUIRender() override;
	virtual void SubSpriteUIRender() override;
	// スプライト(3D)/Effectの描画.
	//	_A：後に表示される / _B：先に表示される.
	virtual void Sprite3DRender_A() override;
	virtual void SubSprite3DRender_A() override;
	virtual void Sprite3DRender_B() override;
	virtual void SubSprite3DRender_B() override;

private:
	std::shared_ptr<CPlayer>				m_pPlayer;			// プレイヤー.
	std::shared_ptr<CBall>					m_pBall;			// ボール.
	std::shared_ptr<CHeavyBall>				m_pHeavyBall;		// 重いボール.
	std::shared_ptr<CSuperBall>				m_pSuperBall;		// スーパーボール.
	std::shared_ptr<CWateringCan>			m_pWateringCan;		// ジョウロ.
	std::shared_ptr<CSlimeFrame>			m_pSlimeFrame;		// スライムフレーム.
	std::shared_ptr<CTree>					m_pTree;			// 木.
	std::shared_ptr<CWeedManager>			m_pWeedManager;		// 雑草.
	std::shared_ptr<CFlowerManager>			m_pFlowerManager;	// 花.
	std::shared_ptr<CHouse>					m_pHouse;			// 家.
	std::shared_ptr<CWaterFall>				m_pWaterFall;		// 滝.
	std::unique_ptr<CGameMainWidget>		m_GameMainWidget;	// UI.
};