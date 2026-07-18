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


	// アプリを止めずに「はい・いいえ」のメッセージボックスを表示する( 別スレッドで表示 ).
	//	応答結果をワーカースレッド上で OnResult に渡します( Yes=true / No=false ).
	//	コールバックは結果をフラグに控える等の軽い処理のみを渡し、実処理は独自の Update() で行うこと.
	//	注意: OnResult で控えるフラグはワーカースレッドから書き込まれメインスレッドで読むため.
	//	std::atomic か volatile にしておくことを推奨.
	// （bool でも可能だが、最適化での取りこぼしを避けるなら std::atomic）.
	void YesNoAsync( const std::string& Text, const std::string& Caption, std::function<void(bool)> OnResult );

	// アプリを止めずに OK のみのメッセージボックスを表示する( 別スレッドで表示 ).
	void InfoAsync( const std::string& Text, const std::string& Caption );
}
