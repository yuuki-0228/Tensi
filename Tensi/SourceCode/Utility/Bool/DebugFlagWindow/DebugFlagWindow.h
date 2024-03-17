#pragma once
#include "..\..\..\Global.h"

/************************************************
*	メッセージウィンドウクラス.
*		﨑田友輝.
**/
class DebugFlagWindow final
{
public:
	DebugFlagWindow();
	~DebugFlagWindow();

	// 描画.
	static void Render();

private:
	// インスタンスの取得.
	static DebugFlagWindow* GetInstance();

	static void FlagFind(); // フラグ検索処理.

private:
	std::string	m_FindString;			// フラグウィンドウの検索する文字.
	CBool		m_IsDispNameNoneFlag;	// 無名のフラグを表示させるか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	DebugFlagWindow( const DebugFlagWindow & )				= delete;
	DebugFlagWindow& operator = ( const DebugFlagWindow & )	= delete;
	DebugFlagWindow( DebugFlagWindow && )						= delete;
	DebugFlagWindow& operator = ( DebugFlagWindow && )		= delete;
};
