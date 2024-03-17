#pragma once
#include "..\SceneWidget.h"

/************************************************
*	ゲームメインUIクラス.
*		井谷凌.
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

private:
};
