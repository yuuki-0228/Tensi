#pragma once
#include <string>
#include <functional>

/************************************************
*	Windowsのメッセージボックスの表示クラス.
**/
namespace WindowsMessageBox {
	// 「はい・いいえ」のメッセージボックスを表示する( 戻り値: はい=true / いいえ=false ).
	bool YesNo( const std::string& Text, const std::string& Caption );

	// OKのみのメッセージボックスを表示する.
	void Info( const std::string& Text, const std::string& Caption );


	// アプリを止めずに「はい・いいえ」のメッセージボックスを表示する( 専用スレッドで表示 ).
	//	応答結果を OnResult に渡します( Yes=true / No=false ).
	//	OnResult はメインスレッド( ThreadManager::Update )で実行されるため、
	//	ゲームオブジェクトの操作など通常の処理をそのまま書いてよい.
	void YesNoAsync( const std::string& Text, const std::string& Caption, std::function<void(bool)> OnResult );

	// アプリを止めずに OK のみのメッセージボックスを表示する( 専用スレッドで表示 ).
	void InfoAsync( const std::string& Text, const std::string& Caption );

	// 表示中のメッセージボックスを全て閉じ、以降の表示を止める.
	//	アプリ終了時、スレッドの終了を待つ前に呼ぶこと.
	//	( メッセージボックスは閉じられるまで戻らないため、閉じずに待つと終了できなくなる ).
	void Release();
}
