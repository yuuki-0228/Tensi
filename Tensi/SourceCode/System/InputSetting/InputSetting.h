#pragma once
#include "..\SystemBase.h"
#include "..\..\Utility\Input\KeyBind.h"

/************************************************
*	入力設定クラス.
**/
class InputSetting final
	: public CSystemBase
{
public:
	InputSetting();
	~InputSetting();

protected:
	// 初期化関数.
	virtual void Init() override;

private:
	KeyBind_map	m_KeyBindList;	// キーバインドのリスト.
	std::string	m_ActionName;	// アクション名.
};
