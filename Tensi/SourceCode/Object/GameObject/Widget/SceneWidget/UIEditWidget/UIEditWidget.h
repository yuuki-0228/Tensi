#pragma once
#include "..\SceneWidget.h"

/************************************************
*	UIEditのUIクラス.
**/
class CUIEditWidget final
	: public CSceneWidget
{
public:
	CUIEditWidget();
	virtual ~CUIEditWidget();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

private:
};
