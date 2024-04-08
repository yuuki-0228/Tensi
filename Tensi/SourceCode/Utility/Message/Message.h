#pragma once
#include "..\..\Global.h"

namespace Msg{
	// メッセージタイプ列挙体
	enum class enMessageType {
		// ワンフレームメッセージ.
		//	次のフレームで自動的に削除されるメッセージ.
		OneFrameMsg,
		// チェックフレームメッセージ.
		//	確認した次のフレームで自動的に削除されるメッセージ.
		//	確認後、ワンフレームメッセージに自動的に切り替わるため.
		//	オートメッセージで確認してください.
		CheckFrameMsg,
		// チェックメッセージ.
		//	確認するまで残り続けるメッセージ.
		CheckMsg,
		// ニューメッセージ.
		//	デリートメッセージが届くまで残り続けるメッセージ.
		NewMsg,
		// デリートメッセージ.
		//	ニューメッセージを削除するメッセージ.
		DeleteMsg,
		// オートメッセージ.
		//	確認時メッセージタイプを自動で判断するメッセージ.
		AutoMsg,

		None
	} typedef EMsgType;

	constexpr char ONE_FRAME_MSG[]	 = "FM_";	// ワンフレームメッセージ識別文字.
	constexpr char CHECK_FRAME_MSG[] = "LM_";	// チェックフレームメッセージ識別文字.
	constexpr char CHECK_MSG[]		 = "CM_";	// チェックメッセージ識別文字.
	constexpr char NEW_MSG[]		 = "NM_";	// ニューメッセージ識別文字.
	constexpr char DELETE_MSG[]		 = "DM_";	// デリートメッセージ識別文字.
	constexpr char AUTO_MSG[]		 = "AM_";	// オートメッセージ識別文字.
}

/************************************************
*	メッセージクラス.
**/
class Message final
{
public:
	Message();
	~Message();

	// 更新.
	static void Update( const float& DeltaTime );

	// メッセージを送る.
	static void Send( const std::string& Message, const Msg::EMsgType Type = Msg::EMsgType::CheckFrameMsg );

	// メッセージの確認.
	static bool Check( const std::string& Message, const Msg::EMsgType Type = Msg::EMsgType::AutoMsg );

private:
	// インスタンスの取得.
	static Message* GetInstance();

	// ニューメッセージを削除.
	static bool NewMsgDelete( const std::string& CheckMsg );

	// チェックフレームメッセージの変更.
	static bool CheckFrameMsgChange( const std::string& CheckMsg );

	// チェックメッセージを削除.
	static bool CheckMsgDelete( const std::string& CheckMsg );

	// オートメッセージの確認.
	static bool AutoMsgCheck( const std::string& CheckMsg );

private:
	std::unordered_map<std::string, bool> m_MsgList;
};