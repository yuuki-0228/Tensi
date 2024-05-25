#pragma once
#include "..\Actor.h"

/************************************************
*	背景に表示するオブジェクトのベースクラス.
**/
class CActorBackGround
	: public CActor
{
public:
	CActorBackGround();
	~CActorBackGround();

	// 背景オブジェクトは主に"Render()"を使用せず"SubRender()"を使用する
	virtual void Render() override {};
	virtual void SubRender() override = 0;

private:

};