#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_NETWORK
#include "..\Const\Const.h"
#include <string>
#include <vector>
#include <cstring>
#include <type_traits>

/************************************************
*	ネットワーク関係の構造体・列挙体・定数.
*	チューニング可能な設定値( ポート/人数/サイズ/FPS/タイムアウト )は
*	Data\Parameter\Const\Network.json で管理し、Const::Network() から取得する.
**/

// ネットワーク関係の定数( 設定ではないプロトコル上の番兵ID ).
//	チューニング用の設定値は Const::Network() を参照すること.
namespace NetworkConst
{
	constexpr int	ALL_PLAYER			= -1;	// 全プレイヤー宛を表すプレイヤーID.
	constexpr int	INVALID_PLAYER_ID	= -2;	// 無効なプレイヤーID.
	constexpr int	HOST_PLAYER_ID		= 0;	// ホストのプレイヤーID.
}

// 通信モード.
enum class ENetworkMode : unsigned char
{
	None,	// 未接続.
	Host,	// ホスト( 部屋を立てた側 ).
	Client,	// クライアント( 部屋に参加した側 ).
};

// 接続状態.
enum class ENetworkState : unsigned char
{
	Disconnected,	// 未接続.
	Hosting,		// ホストとして受付中.
	Connecting,		// 接続処理中.
	Connected,		// 接続済み.
};

// 参加結果や切断理由.
enum class ENetworkResult : unsigned char
{
	None,			// なし.
	Success,		// 成功.
	Timeout,		// タイムアウト.
	RoomFull,		// 部屋が満員.
	WrongPassword,	// パスワード不一致.
	VersionMismatch,// バージョン不一致.
	RoomClosed,		// 部屋が閉じられた( ホストの退出 ).
	Kicked,			// ホストにキックされた.
	Error,			// その他のエラー.
};

// UPnPによるポート自動開放の状態.
enum class EUPnPState : unsigned char
{
	None,		// 未実行.
	Working,	// 実行中.
	Success,	// 成功( グローバル部屋番号が使用可能 ).
	Failed,		// 失敗( 同一ネットワーク内のみ使用可能 ).
};

// ホスト設定構造体( 部屋を立てる時に渡す詳細設定 ).
struct stNetworkHostState
{
	std::string		RoomName;		// 部屋名( LAN内の部屋検索の一覧に表示される ).
	std::string		PlayerName;		// 自分のプレイヤー名.
	std::string		Password;		// 参加パスワード( 空なら無し ).
	int				MaxPlayerNum;	// 最大参加人数( ホストを含む ).
	unsigned short	TcpPort;		// TCPポート番号.
	unsigned short	UdpPort;		// UDP(状態同期)ポート番号.
	float			SyncFps;		// 状態同期の送信頻度[回/秒].
	float			TimeoutSec;		// 応答が無いクライアントを切断するまでの時間[秒].
	bool			IsLanDiscovery;	// LAN内に部屋情報を公開するか( 部屋検索の一覧に表示されるか ).
	bool			IsUseUPnP;		// UPnPによるポート自動開放を試みるか( 成功するとグローバル部屋番号が使用可能 ).

	stNetworkHostState()
	{
		const auto Net	= Const::Network();
		RoomName		= "Room";
		PlayerName		= "Host";
		Password		= "";
		MaxPlayerNum	= Net.DEFAULT_MAX_PLAYER;
		TcpPort			= static_cast<unsigned short>( Net.DEFAULT_TCP_PORT );
		UdpPort			= static_cast<unsigned short>( Net.DEFAULT_UDP_PORT );
		SyncFps			= Net.DEFAULT_SYNC_FPS;
		TimeoutSec		= Net.DEFAULT_CONNECTION_TIMEOUT_SEC;
		IsLanDiscovery	= true;
		IsUseUPnP		= true;
	}
} typedef SNetworkHostState;

// 参加設定構造体( 部屋に参加する時に渡す詳細設定 ).
struct stNetworkJoinState
{
	std::string		PlayerName;			// 自分のプレイヤー名.
	std::string		Password;			// 参加パスワード.
	float			ConnectTimeoutSec;	// 接続タイムアウト[秒].

	stNetworkJoinState()
	{
		PlayerName			= "Player";
		Password			= "";
		ConnectTimeoutSec	= Const::Network().DEFAULT_CONNECT_TIMEOUT_SEC;
	}
} typedef SNetworkJoinState;

// プレイヤー情報構造体.
struct stNetworkPlayer
{
	std::string			Name;		// プレイヤー名.
	std::vector<char>	SyncData;	// 最新の同期データ( 未受信なら空 ).
	int					Id;			// プレイヤーID( ホストが割り振る. ホストは0 ).
	bool				IsHost;		// ホストかどうか.
	bool				IsLocal;	// 自分自身かどうか.

	stNetworkPlayer()
		: Name		( "" )
		, Id		( NetworkConst::INVALID_PLAYER_ID )
		, IsHost	( false )
		, IsLocal	( false )
	{}

	// 同期データを構造体として取得する( サイズ不一致や未受信なら false ).
	template<class T>
	bool GetSyncData( T* pOut ) const
	{
		static_assert( std::is_trivially_copyable<T>::value, "T is not trivially copyable" );
		if ( pOut == nullptr )								return false;
		if ( SyncData.size() != sizeof( T ) )				return false;
		std::memcpy( pOut, SyncData.data(), sizeof( T ) );
		return true;
	}
} typedef SNetworkPlayer;

// LAN内で見つかった部屋の情報構造体.
struct stNetworkRoomInfo
{
	std::string		RoomName;		// 部屋名.
	std::string		Ip;				// ホストのIPアドレス.
	std::string		RoomCode;		// 部屋番号( そのまま参加に使用できる ).
	unsigned short	TcpPort;		// ホストのTCPポート番号.
	int				NowPlayerNum;	// 現在の参加人数.
	int				MaxPlayerNum;	// 最大参加人数.
	bool			IsNeedPassword;	// 参加にパスワードが必要かどうか.

	stNetworkRoomInfo()
		: RoomName			( "" )
		, Ip				( "" )
		, RoomCode			( "" )
		, TcpPort			( 0 )
		, NowPlayerNum		( 0 )
		, MaxPlayerNum		( 0 )
		, IsNeedPassword	( false )
	{}
} typedef SNetworkRoomInfo;

// 受信メッセージ構造体.
struct stNetworkMessage
{
	std::vector<char>	Data;		// メッセージ本体.
	int					SenderId;	// 送信者のプレイヤーID.
	int					Type;		// ユーザー定義のメッセージ種別.

	stNetworkMessage()
		: SenderId	( NetworkConst::INVALID_PLAYER_ID )
		, Type		( 0 )
	{}

	// メッセージ本体を文字列として取得する.
	std::string GetString() const
	{
		return std::string( Data.begin(), Data.end() );
	}

	// メッセージ本体を構造体として取得する( サイズ不一致なら false ).
	template<class T>
	bool GetData( T* pOut ) const
	{
		static_assert( std::is_trivially_copyable<T>::value, "T is not trivially copyable" );
		if ( pOut == nullptr )						return false;
		if ( Data.size() != sizeof( T ) )			return false;
		std::memcpy( pOut, Data.data(), sizeof( T ) );
		return true;
	}
} typedef SNetworkMessage;

#endif	// #ifdef ENABLE_NETWORK.
