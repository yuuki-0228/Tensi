#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_NETWORK
#include "NetworkStruct.h"
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

/************************************************
*	ネットワークマネージャー.
*	ローカル通信( 同一ネットワーク )とネットワーク通信( ホストが部屋を立てて参加する形式 )をまとめて扱う.
*	通信はTCP( 接続や重要メッセージ )とUDP( 状態同期 )の併用. 外部サーバーは使用しない.
*
*	使い方.
*	・部屋を立てる : StartHost( SNetworkHostState ) … 設定構造体で人数やパスワードなどを指定する.
*	・部屋番号の取得 : GetRoomCode()( LAN用 ) / GetGlobalRoomCode()( インターネット用. UPnP成功後に有効 ).
*	・部屋に参加 : JoinRoom( 部屋番号 ) または JoinIp( IPアドレス, ポート番号 ).
*		結果は SetOnJoinResult のコールバックで受け取る.
*	・部屋の検索 : StartRoomSearch() で同一ネットワーク内の部屋を検索し、GetFoundRooms() で一覧を取得する.
*	・退出 : Leave()( ホストの場合は部屋の解散 ).
*	・自分の状態を送る : SetSyncData( 構造体 ) … 設定した内容が全員に自動同期される.
*	・他プレイヤーの状態 : GetPlayers() / GetPlayer( ID ) で取得し、SNetworkPlayer::GetSyncData で読み出す.
*	・任意メッセージ : SendData( 種別, データ ) / SendString( 種別, 文字列 ). 受信は SetOnMessage で受け取る.
*	・更新 : Update() をメインスレッドで毎フレーム呼ぶ( 各コールバックはこの中で実行される ).
*
*	注意.
*	・コールバックは全てメインスレッド( Update内 )で実行される.
*	・同期データは1プレイヤーあたり Const::Network().MAX_SYNC_DATA_SIZE まで.
*	・インターネット越しで使う場合、UPnPが失敗した時は手動でのポート開放が必要になる.
**/
class NetworkManager final
{
public:
	// メッセージ受信時の処理.
	using MessageFunc	= std::function<void( const SNetworkMessage& )>;
	// プレイヤーの参加/退出時の処理.
	using PlayerFunc	= std::function<void( const SNetworkPlayer& )>;
	// 参加結果や切断時の処理.
	using ResultFunc	= std::function<void( ENetworkResult )>;
	// UPnPのポート開放完了時の処理.
	using UPnPFunc		= std::function<void( EUPnPState )>;

public:
	NetworkManager();
	~NetworkManager();

	//---------------------------.
	//	部屋の作成 / 参加 / 退出.
	//---------------------------.

	// 部屋を立てる( ホストになる ).
	//	ソケットの準備に失敗した場合は false.
	static bool StartHost( const SNetworkHostState& State = SNetworkHostState() );

	// 部屋番号で参加する.
	//	開始できたかを返し、結果は SetOnJoinResult のコールバックで通知される.
	static bool JoinRoom( const std::string& Code, const SNetworkJoinState& State = SNetworkJoinState() );

	// IPアドレス直接指定で参加する( Port が 0 なら既定値 ).
	//	開始できたかを返し、結果は SetOnJoinResult のコールバックで通知される.
	static bool JoinIp( const std::string& Ip, unsigned short Port = 0, const SNetworkJoinState& State = SNetworkJoinState() );

	// 退出する( ホストの場合は部屋の解散 ).
	static void Leave();

	//---------------------------.
	//	部屋の検索( 同一ネットワーク内 ).
	//---------------------------.

	// 部屋検索の開始.
	static void StartRoomSearch();
	// 部屋検索の停止.
	static void StopRoomSearch();
	// 見つかった部屋一覧の取得.
	static std::vector<SNetworkRoomInfo> GetFoundRooms();

	//---------------------------.
	//	状態の取得.
	//---------------------------.

	// 接続状態の取得.
	static ENetworkState GetState();
	// 通信モードの取得.
	static ENetworkMode GetMode();
	// 接続済みかどうか( ホスト受付中も true ).
	static bool GetIsConnected();
	// ホストかどうか.
	static bool GetIsHost();

	// 部屋番号の取得( 同一ネットワーク用. 未接続なら空文字列 ).
	static std::string GetRoomCode();
	// グローバル部屋番号の取得( インターネット用. UPnP成功前は空文字列 ).
	static std::string GetGlobalRoomCode();
	// UPnPによるポート自動開放の状態取得.
	static EUPnPState GetUPnPState();

	//---------------------------.
	//	プレイヤー情報.
	//---------------------------.

	// 自分のプレイヤーIDの取得.
	static int GetMyPlayerId();
	// 現在の参加人数の取得.
	static int GetPlayerNum();
	// 全プレイヤー情報の取得.
	static std::vector<SNetworkPlayer> GetPlayers();
	// 指定プレイヤー情報の取得( 見つからない場合は false ).
	static bool GetPlayer( int PlayerId, SNetworkPlayer* pOut );

	//---------------------------.
	//	状態の自動同期.
	//---------------------------.

	// 自分の同期データの設定( 設定した内容が全プレイヤーに自動送信される ).
	static void SetSyncData( const void* pData, int Size );
	template<class T>
	static void SetSyncData( const T& Data )
	{
		static_assert( std::is_trivially_copyable<T>::value, "T is not trivially copyable" );
		static_assert( std::is_pointer<T>::value == false, "T is pointer" );
		SetSyncData( &Data, static_cast<int>( sizeof( T ) ) );
	}

	// 指定プレイヤーの同期データの取得( 未受信やサイズ不一致なら false ).
	template<class T>
	static bool GetSyncData( int PlayerId, T* pOut )
	{
		SNetworkPlayer Player;
		if ( GetPlayer( PlayerId, &Player ) == false )	return false;
		return Player.GetSyncData( pOut );
	}

	//---------------------------.
	//	任意メッセージの送信.
	//---------------------------.

	// データの送信( TargetId が ALL_PLAYER なら自分以外の全員へ ).
	static void SendData( int Type, const void* pData, int Size, int TargetId = NetworkConst::ALL_PLAYER );
	template<class T>
	static void SendData( int Type, const T& Data, int TargetId = NetworkConst::ALL_PLAYER )
	{
		static_assert( std::is_trivially_copyable<T>::value, "T is not trivially copyable" );
		static_assert( std::is_pointer<T>::value == false, "T is pointer" );
		SendData( Type, &Data, static_cast<int>( sizeof( T ) ), TargetId );
	}
	// 文字列の送信.
	static void SendString( int Type, const std::string& Text, int TargetId = NetworkConst::ALL_PLAYER );

	//---------------------------.
	//	コールバックの設定( 全てメインスレッドで実行される ).
	//---------------------------.

	// メッセージ受信時.
	static void SetOnMessage( MessageFunc Func );
	// プレイヤー参加時.
	static void SetOnPlayerJoin( PlayerFunc Func );
	// プレイヤー退出時.
	static void SetOnPlayerLeave( PlayerFunc Func );
	// 参加処理の結果通知( JoinRoom / JoinIp 用 ).
	static void SetOnJoinResult( ResultFunc Func );
	// 自分が切断された時( タイムアウトや部屋の解散など ).
	static void SetOnDisconnected( ResultFunc Func );
	// UPnPのポート開放完了時.
	static void SetOnUPnPResult( UPnPFunc Func );

	//---------------------------.
	//	その他.
	//---------------------------.

	// 指定プレイヤーをキックする( ホスト専用 ).
	static void Kick( int PlayerId );

	// 自分のLAN内IPアドレスの取得.
	static std::string GetLocalIp();

	// 更新処理( コールバックの実行. メインスレッドで毎フレーム呼ぶ ).
	static void Update();

	// 終了処理( 切断と全通信スレッドの終了待ち. アプリ終了時に呼ぶ ).
	static void Release();

private:
	// インスタンスの取得.
	static NetworkManager* GetInstance();

private:
	// 内部実装( ソケットなどのWinSock関係を.cpp内に隠蔽する ).
	struct Impl;
	std::unique_ptr<Impl>	m_pImpl;

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	NetworkManager( const NetworkManager& )					= delete;
	NetworkManager& operator = ( const NetworkManager& )	= delete;
	NetworkManager( NetworkManager&& )						= delete;
	NetworkManager& operator = ( NetworkManager&& )			= delete;
};

#endif	// #ifdef ENABLE_NETWORK.
