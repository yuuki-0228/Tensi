#pragma once
#include "..\SceneWidget.h"

/************************************************
*	ゲームメインUIクラス.
**/
class CGameMainWidget final
	: public CSceneWidget
{
public:
	CGameMainWidget();
	virtual ~CGameMainWidget();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

private:
};
