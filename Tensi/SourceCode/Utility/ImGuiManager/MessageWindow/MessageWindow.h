#pragma once
#include "..\..\..\Global.h"
#include "..\ImGuiManager.h"

/************************************************
*	メッセージウィンドウクラス.
*		﨑田友輝.
**/
class MessageWindow final
{
public:
	MessageWindow();
	~MessageWindow();

	// 描画.
	static void Render();

	// メッセージウィンドウに追加.
	static void PushMessage( const std::string& Message, const D3DXCOLOR3& Color );
	static void PushMessage( const std::string& Message, const D3DXCOLOR4& Color = Color4::White );
	
private:
	// インスタンスの取得.
	static MessageWindow* GetInstance();

	static void MessageFind();		// メッセージ検索処理.
	static void ColorBlock();		// カラーブロック処理.
	void SetColorTable();			// カラーテーブルの設定.

private:
	std::vector<std::function<void()>>	m_MessageList;		// メッセージウィンドウ用.
	std::vector<bool>					m_ColorBlockFlags;	// 各カラーをブロックしないかのフラグ.
	std::vector<D3DXCOLOR4>				m_ColorTable;		// 色のテーブル.
	std::string							m_FindString;		// メッセージウィンドウの検索する文字.
	bool								m_IsFindTime;		// メッセージウィンドウの検索に時間を含ませるか.
	bool								m_IsNewMsg;			// メッセージ追加されたか.
	int									m_ColorResetCnt;	// 色を元に戻すカウント.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	MessageWindow( const MessageWindow & )				= delete;
	MessageWindow& operator = ( const MessageWindow & )	= delete;
	MessageWindow( MessageWindow && )						= delete;
	MessageWindow& operator = ( MessageWindow && )		= delete;
};
