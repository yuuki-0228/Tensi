#pragma once
#include "..\..\..\Actor.h"

/************************************************
*	地面クラス.
**/
class CGround
	: public CActor
{
public:
	CGround();
	~CGround();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 地面の設置
	void Setting();

private:

};