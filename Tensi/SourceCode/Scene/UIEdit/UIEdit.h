#pragma once
#include "..\SceneBase.h"

class CSprite;
class CUIEditor;

/************************************************
*	UIエディタシーン.
**/
class CUIEdit final
	: public CSceneBase
{
public:
	CUIEdit();
	~CUIEdit();

	// 初期化.
	virtual bool Init() override;
	// 初回起動時の初期化.
	virtual bool FirstPlayInit() override { return true; }
	// 初回ログイン時の初期化.
	virtual bool LoginInit( std::tm lastDay ) override { return true; }

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// モデルの描画.
	virtual void ModelRender() override {};
	// スプライト(UI)の描画.
	virtual void SpriteUIRender() override;
	// スプライト(3D)/Effectの描画.
	//	_A：後に表示される / _B：先に表示される.
	virtual void Sprite3DRender_A() override {};
	virtual void Sprite3DRender_B() override {};

private:
	std::unique_ptr<CUIEditor>	m_pUIEditor;		// UIエディタ.

};