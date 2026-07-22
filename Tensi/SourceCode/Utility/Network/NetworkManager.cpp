#include "..\..\SystemSettings.h"
#ifdef ENABLE_NETWORK
// WinSock2 は windows.h( 他ヘッダ経由で含まれる )より先に含める.
//	先に含めないと winsock.h( v1 )が引き込まれ、WinSock2.h と多重定義衝突するため.
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#include "NetworkManager.h"
#ifdef ENABLE_NETWORK
#include "RoomCode\RoomCode.h"
#include "UPnP\UPnPPortMapper.h"
#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>

#pragma comment( lib, "ws2_32.lib" )

namespace
{
	constexpr unsigned int		PROTOCOL_MAGIC			= 0x544E5354;	// パケット識別子.
	constexpr unsigned short	PROTOCOL_VERSION		= 1;			// プロトコルバージョン.
	constexpr float				PING_INTERVAL_SEC		= 2.0f;			// 生存確認の送信間隔[秒].
	constexpr float				SEARCH_SEND_INTERVAL	= 0.5f;			// 部屋検索の送信間隔[秒].
	constexpr float				SEARCH_EXPIRE_SEC		= 3.0f;			// 見つけた部屋情報の有効期限[秒].
	constexpr int				SELECT_WAIT_MS			= 5;			// selectの待機時間[ミリ秒].
	constexpr int				SEND_RETRY_MAX			= 50;			// 送信リトライの最大回数.

	// TCPフレームの最大サイズ( 任意メッセージ + ヘッダの余裕分 ). 設定値は Const::Network() から取得する.
	inline int MaxFrameSize() { return Const::Network().MAX_MESSAGE_SIZE + 64; }

	// TCPパケット種別.
	enum class EPacket : unsigned char
	{
		JoinRequest = 1,	// 参加要求( クライアントからホスト ).
		JoinAccept,			// 参加許可( ホストからクライアント ).
		JoinRefuse,			// 参加拒否( ホストからクライアント ).
		PlayerJoined,		// プレイヤー参加通知( ホストから全員 ).
		PlayerLeft,			// プレイヤー退出通知( ホストから全員 ).
		RoomClosed,			// 部屋の解散通知( ホストから全員 ).
		UserMessage,		// ユーザー定義メッセージ.
		Ping,				// 生存確認.
		Pong,				// 生存確認の応答.
		Bye,				// 退出通知( クライアントからホスト ).
	};
	// UDPパケット種別.
	enum class EUdp : unsigned char
	{
		SyncUp = 1,		// 同期データ( クライアントからホスト ).
		SyncOne,		// 同期データ( ホストからクライアント. 1プレイヤー分 ).
		DiscoverReq,	// 部屋検索の要求.
		DiscoverRes,	// 部屋検索の応答.
	};

	using Clock = std::chrono::steady_clock;

	// 経過秒数の取得.
	float ElapsedSec( const Clock::time_point& From, const Clock::time_point& To )
	{
		return std::chrono::duration<float>( To - From ).count();
	}

	//---------------------------.
	// バイト列書き込みヘルパ.
	//---------------------------.
	void WriteBytes( std::vector<char>& Out, const void* pData, int Size )
	{
		if ( Size <= 0 || pData == nullptr )	return;
		const size_t Pos = Out.size();
		Out.resize( Pos + Size );
		std::memcpy( Out.data() + Pos, pData, Size );
	}
	void WriteU8 ( std::vector<char>& Out, unsigned char  Value )	{ Out.push_back( static_cast<char>( Value ) ); }
	void WriteU16( std::vector<char>& Out, unsigned short Value )	{ WriteBytes( Out, &Value, 2 ); }
	void WriteU32( std::vector<char>& Out, unsigned int   Value )	{ WriteBytes( Out, &Value, 4 ); }
	void WriteI32( std::vector<char>& Out, int            Value )	{ WriteBytes( Out, &Value, 4 ); }
	void WriteStr8( std::vector<char>& Out, const std::string& Text )
	{
		const unsigned char Len = static_cast<unsigned char>( Text.length() > 255 ? 255 : Text.length() );
		WriteU8( Out, Len );
		WriteBytes( Out, Text.data(), Len );
	}

	//---------------------------.
	// バイト列読み取りヘルパ.
	//---------------------------.
	struct ByteReader
	{
		const char*	pData	= nullptr;	// 読み取り元.
		int			Size	= 0;		// 全体サイズ.
		int			Pos		= 0;		// 読み取り位置.
		bool		IsOk	= true;		// 読み取りに失敗していないか.

		ByteReader( const char* p, int s ) : pData( p ), Size( s ) {}

		bool Read( void* pOut, int Num )
		{
			if ( IsOk == false || Pos + Num > Size ) { IsOk = false; return false; }
			std::memcpy( pOut, pData + Pos, Num );
			Pos += Num;
			return true;
		}
		unsigned char  U8()		{ unsigned char  v = 0; Read( &v, 1 ); return v; }
		unsigned short U16()	{ unsigned short v = 0; Read( &v, 2 ); return v; }
		unsigned int   U32()	{ unsigned int   v = 0; Read( &v, 4 ); return v; }
		int            I32()	{ int            v = 0; Read( &v, 4 ); return v; }
		std::string Str8()
		{
			const unsigned char Len = U8();
			if ( IsOk == false || Pos + Len > Size ) { IsOk = false; return ""; }
			std::string Text( pData + Pos, Len );
			Pos += Len;
			return Text;
		}
		std::vector<char> Rest()
		{
			if ( IsOk == false || Pos > Size )	return {};
			std::vector<char> Data( pData + Pos, pData + Size );
			Pos = Size;
			return Data;
		}
	};

	// TCPフレームの作成( [u16 サイズ][u8 種別][本体] ).
	std::vector<char> MakeFrame( EPacket Type, const std::vector<char>& Payload )
	{
		std::vector<char> Frame;
		WriteU16( Frame, static_cast<unsigned short>( Payload.size() + 1 ) );
		WriteU8( Frame, static_cast<unsigned char>( Type ) );
		WriteBytes( Frame, Payload.data(), static_cast<int>( Payload.size() ) );
		return Frame;
	}

	// 全バイト送信( ノンブロッキングソケット用 ).
	bool SendAll( SOCKET Sock, const std::vector<char>& Data )
	{
		if ( Sock == INVALID_SOCKET )	return false;
		int Sent	= 0;
		int Retry	= 0;
		const int Size = static_cast<int>( Data.size() );
		while ( Sent < Size ) {
			const int Result = send( Sock, Data.data() + Sent, Size - Sent, 0 );
			if ( Result > 0 ) { Sent += Result; continue; }
			if ( Result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK ) {
				// 送信バッファが一杯の場合は書き込み可能になるまで少し待つ.
				if ( ++Retry > SEND_RETRY_MAX )	return false;
				fd_set WriteSet;
				FD_ZERO( &WriteSet );
				FD_SET( Sock, &WriteSet );
				timeval Timeout = { 0, 100 * 1000 };
				select( 0, nullptr, &WriteSet, nullptr, &Timeout );
				continue;
			}
			return false;
		}
		return true;
	}

	// ノンブロッキング設定.
	void SetNonBlocking( SOCKET Sock )
	{
		u_long Mode = 1;
		ioctlsocket( Sock, FIONBIO, &Mode );
	}

	// sockaddr_in の作成( 失敗時は false ).
	bool ParseAddr( const std::string& Ip, unsigned short Port, sockaddr_in* pOut )
	{
		sockaddr_in Addr = {};
		Addr.sin_family	= AF_INET;
		Addr.sin_port	= htons( Port );
		if ( inet_pton( AF_INET, Ip.c_str(), &Addr.sin_addr ) != 1 )	return false;
		*pOut = Addr;
		return true;
	}

	// IPアドレス文字列の取得.
	std::string AddrToIp( const sockaddr_in& Addr )
	{
		char Buf[64] = {};
		inet_ntop( AF_INET, &Addr.sin_addr, Buf, sizeof( Buf ) );
		return Buf;
	}

	// ソケットを閉じて無効化する.
	void CloseSocket( SOCKET& Sock )
	{
		if ( Sock == INVALID_SOCKET )	return;
		closesocket( Sock );
		Sock = INVALID_SOCKET;
	}
}

/************************************************
*	内部実装.
*	ソケット関係はネットワークスレッドのみが操作し、
*	共有する状態は StateMutex で保護する.
**/
struct NetworkManager::Impl
{
	// ホストが管理するクライアント接続.
	struct Conn
	{
		std::vector<char>	RecvBuf;										// 受信バッファ.
		std::string			Ip;												// 接続元IPアドレス.
		SOCKET				Sock		= INVALID_SOCKET;					// TCPソケット.
		int					PlayerId	= NetworkConst::INVALID_PLAYER_ID;	// プレイヤーID( 参加完了までは無効ID ).
		sockaddr_in			UdpAddr		= {};								// 同期データの送信先.
		bool				HasUdpAddr	= false;							// UDPの送信先が判明しているか.
		Clock::time_point	LastRecv;										// 最終受信時刻.
		unsigned int		SyncSeq		= 0;								// 受信済み同期データの通し番号.
	};
	// 部屋検索の内部結果.
	struct FoundRoom
	{
		SNetworkRoomInfo	Info;		// 部屋情報.
		Clock::time_point	LastSeen;	// 最終受信時刻.
	};
	// 送信予約( メインスレッドからの送信をネットワークスレッドで処理する ).
	struct PendingSend
	{
		std::vector<char>	Data;										// 送信データ.
		int					TargetId	= NetworkConst::ALL_PLAYER;		// 宛先プレイヤーID.
		int					Type		= 0;							// メッセージ種別.
	};

	// --- 共有状態( StateMutexで保護 ) ---.
	std::mutex						StateMutex;
	ENetworkMode					Mode		= ENetworkMode::None;				// 通信モード.
	ENetworkState					State		= ENetworkState::Disconnected;		// 接続状態.
	SNetworkHostState				HostSetting;									// ホスト時の設定.
	SNetworkJoinState				JoinSetting;									// クライアント時の設定.
	std::string						HostIp;											// ホストのIPアドレス.
	unsigned short					HostTcpPort	= 0;								// ホストのTCPポート番号.
	unsigned short					HostUdpPort	= 0;								// ホストのUDP(同期)ポート番号.
	int								MyPlayerId	= NetworkConst::INVALID_PLAYER_ID;	// 自分のプレイヤーID.
	std::vector<SNetworkPlayer>		Players;										// 全プレイヤー情報.
	std::vector<PendingSend>		SendQueue;										// 送信予約.
	std::vector<int>				KickQueue;										// キック予約( ホスト専用 ).
	std::vector<char>				MySyncData;										// 自分の同期データ.

	// --- コールバック( CallbackMutexで保護 ) ---.
	std::mutex							CallbackMutex;
	std::vector<std::function<void()>>	CallbackQueue;		// メインスレッドで実行する処理.
	MessageFunc							OnMessage;			// メッセージ受信時.
	PlayerFunc							OnPlayerJoin;		// プレイヤー参加時.
	PlayerFunc							OnPlayerLeave;		// プレイヤー退出時.
	ResultFunc							OnJoinResult;		// 参加処理の結果通知.
	ResultFunc							OnDisconnected;		// 自分が切断された時.
	UPnPFunc							OnUPnPResult;		// UPnPのポート開放完了時.

	// --- ソケット( ネットワークスレッドのみが操作 ) ---.
	SOCKET							ListenSock		= INVALID_SOCKET;	// ホスト : TCP待ち受け.
	SOCKET							UdpSock			= INVALID_SOCKET;	// 状態同期用.
	SOCKET							DiscoverySock	= INVALID_SOCKET;	// ホスト : 部屋検索の応答用.
	SOCKET							TcpSock			= INVALID_SOCKET;	// クライアント : ホストへの接続.
	std::vector<Conn>				Conns;								// ホスト : クライアント接続一覧.
	unsigned int					SyncSendSeq		= 0;				// ホスト : 同期配信の通し番号.

	// --- ネットワークスレッド ---.
	std::thread						NetThread;
	std::atomic<bool>				IsStopRequest	= { false };

	// --- 部屋検索( SearchMutexで保護 ) ---.
	std::thread						SearchThread;
	std::atomic<bool>				IsSearchStop	= { false };
	std::mutex						SearchMutex;
	std::unordered_map<std::string, FoundRoom>	FoundRooms;	// 見つかった部屋一覧.

	// --- UPnP( UPnPMutexで保護 ) ---.
	std::thread						UPnPThread;
	std::atomic<EUPnPState>			UPnP			= { EUPnPState::None };
	std::mutex						UPnPMutex;
	std::string						ExternalIp;				// ルーターの外部IPアドレス.
	unsigned short					OpenedTcpPort	= 0;	// 開放済みTCPポート( 0なら未開放 ).
	unsigned short					OpenedUdpPort	= 0;	// 開放済みUDPポート.

	bool							IsWsaInit		= false;	// WSAStartupに成功したか.

	//---------------------------.
	// コンストラクタ / デストラクタ.
	//---------------------------.
	Impl()
	{
		WSADATA Data = {};
		IsWsaInit = ( WSAStartup( MAKEWORD( 2, 2 ), &Data ) == 0 );
	}
	~Impl()
	{
		Leave();
		StopSearch();
		JoinUPnPThread();
		if ( IsWsaInit )	WSACleanup();
	}

	//---------------------------.
	// プレイヤーの検索( StateMutexをロックした状態で呼ぶ ).
	//---------------------------.
	SNetworkPlayer* FindPlayerNoLock( int Id )
	{
		for ( auto& p : Players ) {
			if ( p.Id == Id )	return &p;
		}
		return nullptr;
	}

	//---------------------------.
	// コールバックの積み込み.
	//---------------------------.
	void PushCallback( std::function<void()> Func )
	{
		std::lock_guard<std::mutex> Lock( CallbackMutex );
		CallbackQueue.push_back( std::move( Func ) );
	}
	// メッセージ受信の通知.
	void EnqueueMessage( int SenderId, int Type, std::vector<char>&& Data )
	{
		MessageFunc Func;
		{
			std::lock_guard<std::mutex> Lock( CallbackMutex );
			Func = OnMessage;
		}
		if ( Func == nullptr )	return;
		SNetworkMessage Msg;
		Msg.SenderId	= SenderId;
		Msg.Type		= Type;
		Msg.Data		= std::move( Data );
		PushCallback( [Func, Msg]() { Func( Msg ); } );
	}
	// プレイヤーの参加/退出の通知.
	void EnqueuePlayer( bool IsJoin, const SNetworkPlayer& Player )
	{
		PlayerFunc Func;
		{
			std::lock_guard<std::mutex> Lock( CallbackMutex );
			Func = IsJoin ? OnPlayerJoin : OnPlayerLeave;
		}
		if ( Func == nullptr )	return;
		PushCallback( [Func, Player]() { Func( Player ); } );
	}
	// 参加結果/切断の通知.
	void EnqueueResult( bool IsJoinResult, ENetworkResult Result )
	{
		ResultFunc Func;
		{
			std::lock_guard<std::mutex> Lock( CallbackMutex );
			Func = IsJoinResult ? OnJoinResult : OnDisconnected;
		}
		if ( Func == nullptr )	return;
		PushCallback( [Func, Result]() { Func( Result ); } );
	}

	//---------------------------.
	// ホストの開始.
	//---------------------------.
	bool StartHost( const SNetworkHostState& Setting )
	{
		if ( IsWsaInit == false )	return false;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			if ( Mode != ENetworkMode::None )	return false;
		}
		StopSearch();

		// 設定値の補正.
		SNetworkHostState Fixed = Setting;
		if ( Fixed.MaxPlayerNum < 1 )								Fixed.MaxPlayerNum	= 1;
		if ( Fixed.MaxPlayerNum > Const::Network().MAX_PLAYER_LIMIT )	Fixed.MaxPlayerNum	= Const::Network().MAX_PLAYER_LIMIT;
		if ( Fixed.TcpPort == 0 )									Fixed.TcpPort		= Const::Network().DEFAULT_TCP_PORT;
		if ( Fixed.UdpPort == 0 )									Fixed.UdpPort		= Const::Network().DEFAULT_UDP_PORT;
		if ( Fixed.SyncFps <= 0.0f )								Fixed.SyncFps		= Const::Network().DEFAULT_SYNC_FPS;
		if ( Fixed.TimeoutSec <= 0.0f )								Fixed.TimeoutSec	= Const::Network().DEFAULT_CONNECTION_TIMEOUT_SEC;

		// TCP待ち受けソケットの作成.
		SOCKET Listen = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( Listen == INVALID_SOCKET )	return false;
		sockaddr_in Addr = {};
		Addr.sin_family			= AF_INET;
		Addr.sin_addr.s_addr	= htonl( INADDR_ANY );
		Addr.sin_port			= htons( Fixed.TcpPort );
		if ( bind( Listen, reinterpret_cast<sockaddr*>( &Addr ), sizeof( Addr ) ) != 0 ||
			 listen( Listen, SOMAXCONN ) != 0 ) {
			closesocket( Listen );
			return false;
		}
		SetNonBlocking( Listen );

		// 状態同期用UDPソケットの作成.
		SOCKET Udp = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( Udp == INVALID_SOCKET ) {
			closesocket( Listen );
			return false;
		}
		Addr.sin_port = htons( Fixed.UdpPort );
		if ( bind( Udp, reinterpret_cast<sockaddr*>( &Addr ), sizeof( Addr ) ) != 0 ) {
			closesocket( Listen );
			closesocket( Udp );
			return false;
		}
		SetNonBlocking( Udp );

		// 部屋検索の応答用ソケットの作成( 失敗しても検索に出ないだけなので続行する ).
		SOCKET Discovery = INVALID_SOCKET;
		if ( Fixed.IsLanDiscovery ) {
			Discovery = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if ( Discovery != INVALID_SOCKET ) {
				BOOL Reuse = TRUE;
				setsockopt( Discovery, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>( &Reuse ), sizeof( Reuse ) );
				Addr.sin_port = htons( Const::Network().DISCOVERY_PORT );
				if ( bind( Discovery, reinterpret_cast<sockaddr*>( &Addr ), sizeof( Addr ) ) != 0 ) {
					closesocket( Discovery );
					Discovery = INVALID_SOCKET;
				}
				else {
					SetNonBlocking( Discovery );
				}
			}
		}

		// 状態の初期化.
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			Mode		= ENetworkMode::Host;
			State		= ENetworkState::Hosting;
			HostSetting	= Fixed;
			HostIp		= NetworkManager::GetLocalIp();
			HostTcpPort	= Fixed.TcpPort;
			HostUdpPort	= Fixed.UdpPort;
			MyPlayerId	= NetworkConst::HOST_PLAYER_ID;
			Players.clear();
			SendQueue.clear();
			KickQueue.clear();
			MySyncData.clear();

			SNetworkPlayer Me;
			Me.Id		= NetworkConst::HOST_PLAYER_ID;
			Me.Name		= Fixed.PlayerName;
			Me.IsHost	= true;
			Me.IsLocal	= true;
			Players.push_back( Me );
		}
		ListenSock		= Listen;
		UdpSock			= Udp;
		DiscoverySock	= Discovery;
		Conns.clear();
		SyncSendSeq		= 0;

		// ネットワークスレッドの開始.
		IsStopRequest = false;
		NetThread = std::thread( &Impl::HostLoop, this );

		// UPnPによるポート自動開放.
		if ( Fixed.IsUseUPnP )	StartUPnP( Fixed.TcpPort, Fixed.UdpPort );
		return true;
	}

	//---------------------------.
	// 参加の開始.
	//---------------------------.
	bool StartJoin( const std::string& Ip, unsigned short Port, const SNetworkJoinState& Setting )
	{
		if ( IsWsaInit == false )	return false;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			if ( Mode != ENetworkMode::None )	return false;
		}
		StopSearch();

		// 接続先の確認.
		sockaddr_in Addr = {};
		if ( ParseAddr( Ip, Port, &Addr ) == false )	return false;

		// 設定値の補正.
		SNetworkJoinState Fixed = Setting;
		if ( Fixed.ConnectTimeoutSec <= 0.0f )	Fixed.ConnectTimeoutSec = Const::Network().DEFAULT_CONNECT_TIMEOUT_SEC;

		// TCPソケットの作成.
		SOCKET Tcp = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if ( Tcp == INVALID_SOCKET )	return false;
		SetNonBlocking( Tcp );

		// 状態同期用UDPソケットの作成( ポートは自動割り当て ).
		SOCKET Udp = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( Udp == INVALID_SOCKET ) {
			closesocket( Tcp );
			return false;
		}
		SetNonBlocking( Udp );

		// 接続の開始( ノンブロッキングのため完了はスレッド側で待つ ).
		connect( Tcp, reinterpret_cast<sockaddr*>( &Addr ), sizeof( Addr ) );

		// 状態の初期化.
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			Mode		= ENetworkMode::Client;
			State		= ENetworkState::Connecting;
			JoinSetting	= Fixed;
			HostIp		= Ip;
			HostTcpPort	= Port;
			HostUdpPort	= 0;
			MyPlayerId	= NetworkConst::INVALID_PLAYER_ID;
			Players.clear();
			SendQueue.clear();
			KickQueue.clear();
			MySyncData.clear();
		}
		TcpSock	= Tcp;
		UdpSock	= Udp;

		// ネットワークスレッドの開始.
		IsStopRequest = false;
		NetThread = std::thread( &Impl::ClientLoop, this );
		return true;
	}

	//---------------------------.
	// 退出.
	//---------------------------.
	void Leave()
	{
		// ネットワークスレッドの停止.
		IsStopRequest = true;
		if ( NetThread.joinable() )	NetThread.join();
		IsStopRequest = false;

		// 状態のリセット.
		ResetState();

		// 開放したポートを閉じる.
		StopUPnP();
	}

	// 状態のリセット.
	void ResetState()
	{
		std::lock_guard<std::mutex> Lock( StateMutex );
		Mode		= ENetworkMode::None;
		State		= ENetworkState::Disconnected;
		HostIp.clear();
		HostTcpPort	= 0;
		HostUdpPort	= 0;
		MyPlayerId	= NetworkConst::INVALID_PLAYER_ID;
		Players.clear();
		SendQueue.clear();
		KickQueue.clear();
		MySyncData.clear();
	}

	//---------------------------.
	// UPnPによるポート自動開放.
	//---------------------------.
	void StartUPnP( unsigned short TcpPort, unsigned short UdpPort )
	{
		JoinUPnPThread();
		UPnP = EUPnPState::Working;
		UPnPThread = std::thread( [this, TcpPort, UdpPort]() {
			std::string Ip;
			const bool IsOpened = UPnPPortMapper::AddPortMapping( TcpPort, UdpPort, NetworkManager::GetLocalIp(), &Ip );

			// 外部IPがIPv4形式で取得できた場合のみグローバル部屋番号が使用可能.
			const bool IsGlobal = IsOpened && ( RoomCode::Encode( Ip, 1 ).empty() == false );
			{
				std::lock_guard<std::mutex> Lock( UPnPMutex );
				ExternalIp = IsGlobal ? Ip : "";
				if ( IsOpened ) {
					OpenedTcpPort = TcpPort;
					OpenedUdpPort = UdpPort;
				}
			}
			const EUPnPState Result = IsGlobal ? EUPnPState::Success : EUPnPState::Failed;
			UPnP = Result;

			// 完了の通知.
			UPnPFunc Func;
			{
				std::lock_guard<std::mutex> Lock( CallbackMutex );
				Func = OnUPnPResult;
			}
			if ( Func != nullptr )	PushCallback( [Func, Result]() { Func( Result ); } );
		} );
	}
	// 開放したポートを閉じる.
	void StopUPnP()
	{
		JoinUPnPThread();
		unsigned short TcpPort = 0;
		unsigned short UdpPort = 0;
		{
			std::lock_guard<std::mutex> Lock( UPnPMutex );
			TcpPort = OpenedTcpPort;
			UdpPort = OpenedUdpPort;
			OpenedTcpPort = 0;
			OpenedUdpPort = 0;
			ExternalIp.clear();
		}
		UPnP = EUPnPState::None;
		if ( TcpPort != 0 ) {
			// 削除には時間がかかるため別スレッドで実行する( Releaseで終了待ちする ).
			UPnPThread = std::thread( [TcpPort, UdpPort]() {
				UPnPPortMapper::DeletePortMapping( TcpPort, UdpPort );
			} );
		}
	}
	// UPnPスレッドの終了待ち.
	void JoinUPnPThread()
	{
		if ( UPnPThread.joinable() )	UPnPThread.join();
	}

	//---------------------------.
	// 部屋検索.
	//---------------------------.
	void StartSearch()
	{
		if ( IsWsaInit == false )		return;
		if ( SearchThread.joinable() )	return;	// 既に検索中.
		{
			std::lock_guard<std::mutex> Lock( SearchMutex );
			FoundRooms.clear();
		}
		IsSearchStop = false;
		SearchThread = std::thread( &Impl::SearchLoop, this );
	}
	void StopSearch()
	{
		IsSearchStop = true;
		if ( SearchThread.joinable() )	SearchThread.join();
	}
	void SearchLoop()
	{
		SOCKET Sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if ( Sock == INVALID_SOCKET )	return;
		BOOL Broadcast = TRUE;
		setsockopt( Sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>( &Broadcast ), sizeof( Broadcast ) );
		SetNonBlocking( Sock );

		// 検索要求パケットの作成.
		std::vector<char> Request;
		WriteU32( Request, PROTOCOL_MAGIC );
		WriteU8( Request, static_cast<unsigned char>( EUdp::DiscoverReq ) );
		WriteU16( Request, PROTOCOL_VERSION );

		sockaddr_in BroadcastAddr = {};
		BroadcastAddr.sin_family		= AF_INET;
		BroadcastAddr.sin_addr.s_addr	= htonl( INADDR_BROADCAST );
		BroadcastAddr.sin_port			= htons( Const::Network().DISCOVERY_PORT );

		bool				IsFirst		= true;
		Clock::time_point	LastSend	= Clock::now();
		while ( IsSearchStop == false ) {
			// 検索要求の送信.
			const Clock::time_point Now = Clock::now();
			if ( IsFirst || ElapsedSec( LastSend, Now ) >= SEARCH_SEND_INTERVAL ) {
				IsFirst		= false;
				LastSend	= Now;
				sendto( Sock, Request.data(), static_cast<int>( Request.size() ), 0,
					reinterpret_cast<sockaddr*>( &BroadcastAddr ), sizeof( BroadcastAddr ) );
			}

			// 応答の受信.
			fd_set ReadSet;
			FD_ZERO( &ReadSet );
			FD_SET( Sock, &ReadSet );
			timeval Wait = { 0, 100 * 1000 };
			if ( select( 0, &ReadSet, nullptr, nullptr, &Wait ) > 0 && FD_ISSET( Sock, &ReadSet ) ) {
				while ( true ) {
					char		Buf[2048];
					sockaddr_in	From	= {};
					int			FromLen	= sizeof( From );
					const int Len = recvfrom( Sock, Buf, sizeof( Buf ), 0, reinterpret_cast<sockaddr*>( &From ), &FromLen );
					if ( Len == SOCKET_ERROR ) {
						if ( WSAGetLastError() == WSAEWOULDBLOCK )	break;
						continue;	// ICMP由来のエラーなどは無視する.
					}
					// 応答の解析.
					ByteReader Reader( Buf, Len );
					if ( Reader.U32() != PROTOCOL_MAGIC )											continue;
					if ( Reader.U8() != static_cast<unsigned char>( EUdp::DiscoverRes ) )			continue;
					if ( Reader.U16() != PROTOCOL_VERSION )											continue;
					SNetworkRoomInfo Info;
					Info.TcpPort		= Reader.U16();
					Info.NowPlayerNum	= Reader.U8();
					Info.MaxPlayerNum	= Reader.U8();
					Info.IsNeedPassword	= ( Reader.U8() != 0 );
					Info.RoomName		= Reader.Str8();
					if ( Reader.IsOk == false )	continue;
					Info.Ip			= AddrToIp( From );
					Info.RoomCode	= RoomCode::Encode( Info.Ip, Info.TcpPort );

					std::lock_guard<std::mutex> Lock( SearchMutex );
					FoundRooms[Info.Ip + ":" + std::to_string( Info.TcpPort )] = { Info, Clock::now() };
				}
			}

			// 応答が途絶えた部屋を一覧から削除する.
			{
				std::lock_guard<std::mutex> Lock( SearchMutex );
				for ( auto it = FoundRooms.begin(); it != FoundRooms.end(); ) {
					if ( ElapsedSec( it->second.LastSeen, Now ) >= SEARCH_EXPIRE_SEC )	it = FoundRooms.erase( it );
					else																++it;
				}
			}
		}
		closesocket( Sock );
	}

	//---------------------------.
	// ホストのメインループ.
	//---------------------------.
	void HostLoop()
	{
		float SyncInterval	= 1.0f / Const::Network().DEFAULT_SYNC_FPS;
		float TimeoutSec	= Const::Network().DEFAULT_CONNECTION_TIMEOUT_SEC;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			SyncInterval	= 1.0f / HostSetting.SyncFps;
			TimeoutSec		= HostSetting.TimeoutSec;
		}
		Clock::time_point LastSyncSend	= Clock::now();
		Clock::time_point LastPing		= Clock::now();

		while ( IsStopRequest == false ) {
			// 受信可能なソケットを待つ.
			fd_set ReadSet;
			FD_ZERO( &ReadSet );
			FD_SET( ListenSock, &ReadSet );
			FD_SET( UdpSock, &ReadSet );
			if ( DiscoverySock != INVALID_SOCKET )	FD_SET( DiscoverySock, &ReadSet );
			for ( const auto& c : Conns ) {
				if ( c.Sock != INVALID_SOCKET )	FD_SET( c.Sock, &ReadSet );
			}
			timeval Wait = { 0, SELECT_WAIT_MS * 1000 };
			select( 0, &ReadSet, nullptr, nullptr, &Wait );

			const Clock::time_point Now = Clock::now();

			// 新規接続の受け付け.
			if ( FD_ISSET( ListenSock, &ReadSet ) )	AcceptNewConns( Now );
			// クライアントからの受信.
			for ( auto& c : Conns ) {
				if ( c.Sock != INVALID_SOCKET && FD_ISSET( c.Sock, &ReadSet ) )	RecvConn( c, Now );
			}
			// UDP同期データの受信.
			if ( FD_ISSET( UdpSock, &ReadSet ) )	RecvSyncUp();
			// 部屋検索要求への応答.
			if ( DiscoverySock != INVALID_SOCKET && FD_ISSET( DiscoverySock, &ReadSet ) )	RecvDiscovery();

			// メインスレッドからの送信予約の処理.
			ProcessSendQueueHost();
			// キック予約の処理.
			ProcessKickQueue();

			// 同期データの配信.
			if ( ElapsedSec( LastSyncSend, Now ) >= SyncInterval ) {
				LastSyncSend = Now;
				SendSyncAll();
			}
			// 生存確認の送信.
			if ( ElapsedSec( LastPing, Now ) >= PING_INTERVAL_SEC ) {
				LastPing = Now;
				const std::vector<char> Frame = MakeFrame( EPacket::Ping, {} );
				for ( auto& c : Conns )	SendAll( c.Sock, Frame );
			}
			// 応答が無いクライアントの切断.
			for ( auto& c : Conns ) {
				if ( c.Sock != INVALID_SOCKET && ElapsedSec( c.LastRecv, Now ) >= TimeoutSec ) {
					DisconnectConn( c, ENetworkResult::Timeout );
				}
			}
			// 切断済み接続の削除.
			for ( auto it = Conns.begin(); it != Conns.end(); ) {
				if ( it->Sock == INVALID_SOCKET )	it = Conns.erase( it );
				else								++it;
			}
		}

		// 終了処理( 部屋の解散通知 ).
		std::vector<char> Payload;
		WriteU8( Payload, static_cast<unsigned char>( ENetworkResult::RoomClosed ) );
		const std::vector<char> Frame = MakeFrame( EPacket::RoomClosed, Payload );
		for ( auto& c : Conns ) {
			SendAll( c.Sock, Frame );
			CloseSocket( c.Sock );
		}
		Conns.clear();
		CloseSocket( ListenSock );
		CloseSocket( UdpSock );
		CloseSocket( DiscoverySock );
	}

	// 新規接続の受け付け.
	void AcceptNewConns( const Clock::time_point& Now )
	{
		while ( true ) {
			sockaddr_in	Addr	= {};
			int			AddrLen	= sizeof( Addr );
			SOCKET Sock = accept( ListenSock, reinterpret_cast<sockaddr*>( &Addr ), &AddrLen );
			if ( Sock == INVALID_SOCKET )	break;
			SetNonBlocking( Sock );
			BOOL NoDelay = TRUE;
			setsockopt( Sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>( &NoDelay ), sizeof( NoDelay ) );

			Conn NewConn;
			NewConn.Sock		= Sock;
			NewConn.Ip			= AddrToIp( Addr );
			NewConn.LastRecv	= Now;
			Conns.push_back( std::move( NewConn ) );
		}
	}

	// クライアントからの受信.
	void RecvConn( Conn& c, const Clock::time_point& Now )
	{
		while ( true ) {
			char Buf[4096];
			const int Len = recv( c.Sock, Buf, sizeof( Buf ), 0 );
			if ( Len > 0 ) {
				c.RecvBuf.insert( c.RecvBuf.end(), Buf, Buf + Len );
				c.LastRecv = Now;
				continue;
			}
			if ( Len == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK )	break;
			// 切断された.
			DisconnectConn( c, ENetworkResult::Error );
			return;
		}
		ParseConnFrames( c );
	}

	// 受信バッファからフレームを取り出して処理する.
	void ParseConnFrames( Conn& c )
	{
		while ( c.Sock != INVALID_SOCKET ) {
			if ( c.RecvBuf.size() < 3 )	break;
			unsigned short Size = 0;
			std::memcpy( &Size, c.RecvBuf.data(), 2 );
			if ( Size < 1 || Size > MaxFrameSize() ) {
				DisconnectConn( c, ENetworkResult::Error );
				return;
			}
			if ( c.RecvBuf.size() < static_cast<size_t>( 2 + Size ) )	break;

			const EPacket Type = static_cast<EPacket>( static_cast<unsigned char>( c.RecvBuf[2] ) );
			ByteReader Reader( c.RecvBuf.data() + 3, Size - 1 );
			HandleHostPacket( c, Type, Reader );
			if ( c.Sock == INVALID_SOCKET )	return;
			c.RecvBuf.erase( c.RecvBuf.begin(), c.RecvBuf.begin() + 2 + Size );
		}
	}

	// ホスト側のパケット処理.
	void HandleHostPacket( Conn& c, EPacket Type, ByteReader& Reader )
	{
		switch ( Type ) {
		case EPacket::JoinRequest:	HandleJoinRequest( c, Reader );	break;
		case EPacket::UserMessage:
		{
			Reader.I32();	// 送信者ID( なりすまし防止のため接続情報のIDを使用する ).
			const int			TargetId	= Reader.I32();
			const int			MsgType		= Reader.I32();
			std::vector<char>	Data		= Reader.Rest();
			if ( Reader.IsOk == false || c.PlayerId == NetworkConst::INVALID_PLAYER_ID )	break;
			RouteUserMessage( c.PlayerId, TargetId, MsgType, std::move( Data ) );
			break;
		}
		case EPacket::Ping:	SendAll( c.Sock, MakeFrame( EPacket::Pong, {} ) );	break;
		case EPacket::Pong:	break;	// 受信時刻は更新済み.
		case EPacket::Bye:	DisconnectConn( c, ENetworkResult::Success );		break;
		default:			break;
		}
	}

	// 参加要求の処理.
	void HandleJoinRequest( Conn& c, ByteReader& Reader )
	{
		const unsigned short	Version	= Reader.U16();
		const std::string		Name	= Reader.Str8();
		const std::string		Pass	= Reader.Str8();
		if ( Reader.IsOk == false ) {
			DisconnectConn( c, ENetworkResult::Error );
			return;
		}

		// 参加できるかの確認とID割り振り.
		ENetworkResult	Result	= ENetworkResult::Success;
		int				NewId	= NetworkConst::INVALID_PLAYER_ID;
		unsigned short	UdpPort	= 0;
		std::vector<char> AcceptPayload;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			if ( Version != PROTOCOL_VERSION )											Result = ENetworkResult::VersionMismatch;
			else if ( static_cast<int>( Players.size() ) >= HostSetting.MaxPlayerNum )	Result = ENetworkResult::RoomFull;
			else if ( HostSetting.Password.empty() == false && HostSetting.Password != Pass )	Result = ENetworkResult::WrongPassword;
			else {
				// 空いているIDを割り振る( ホストは0のため1から順 ).
				NewId = 1;
				while ( FindPlayerNoLock( NewId ) != nullptr )	++NewId;

				SNetworkPlayer NewPlayer;
				NewPlayer.Id	= NewId;
				NewPlayer.Name	= Name;
				Players.push_back( NewPlayer );

				// 参加許可( 自分のIDと全プレイヤー一覧付き ).
				UdpPort = HostSetting.UdpPort;
				WriteI32( AcceptPayload, NewId );
				WriteU16( AcceptPayload, UdpPort );
				WriteU8( AcceptPayload, static_cast<unsigned char>( Players.size() ) );
				for ( const auto& p : Players ) {
					WriteI32( AcceptPayload, p.Id );
					WriteU8( AcceptPayload, p.IsHost ? 1 : 0 );
					WriteStr8( AcceptPayload, p.Name );
				}
			}
		}

		// 参加拒否.
		if ( Result != ENetworkResult::Success ) {
			std::vector<char> Payload;
			WriteU8( Payload, static_cast<unsigned char>( Result ) );
			SendAll( c.Sock, MakeFrame( EPacket::JoinRefuse, Payload ) );
			CloseSocket( c.Sock );
			return;
		}

		// 参加許可の送信.
		c.PlayerId = NewId;
		SendAll( c.Sock, MakeFrame( EPacket::JoinAccept, AcceptPayload ) );

		// 他のクライアントへ参加通知.
		std::vector<char> Notify;
		WriteI32( Notify, NewId );
		WriteU8( Notify, 0 );
		WriteStr8( Notify, Name );
		const std::vector<char> NotifyFrame = MakeFrame( EPacket::PlayerJoined, Notify );
		for ( auto& Other : Conns ) {
			if ( &Other == &c || Other.PlayerId == NetworkConst::INVALID_PLAYER_ID )	continue;
			SendAll( Other.Sock, NotifyFrame );
		}

		// 参加コールバック.
		SNetworkPlayer Joined;
		Joined.Id	= NewId;
		Joined.Name	= Name;
		EnqueuePlayer( true, Joined );
	}

	// ユーザーメッセージの配送( ホスト側 ).
	void RouteUserMessage( int SenderId, int TargetId, int MsgType, std::vector<char>&& Data )
	{
		if ( static_cast<int>( Data.size() ) > Const::Network().MAX_MESSAGE_SIZE )	return;

		// 自分( ホスト )宛の配信.
		if ( TargetId == NetworkConst::ALL_PLAYER || TargetId == NetworkConst::HOST_PLAYER_ID ) {
			std::vector<char> Copy = Data;
			EnqueueMessage( SenderId, MsgType, std::move( Copy ) );
		}
		if ( TargetId == NetworkConst::HOST_PLAYER_ID )	return;

		// 各クライアントへ転送.
		std::vector<char> Payload;
		WriteI32( Payload, SenderId );
		WriteI32( Payload, TargetId );
		WriteI32( Payload, MsgType );
		WriteBytes( Payload, Data.data(), static_cast<int>( Data.size() ) );
		const std::vector<char> Frame = MakeFrame( EPacket::UserMessage, Payload );
		for ( auto& c : Conns ) {
			if ( c.PlayerId == NetworkConst::INVALID_PLAYER_ID || c.PlayerId == SenderId )	continue;
			if ( TargetId != NetworkConst::ALL_PLAYER && c.PlayerId != TargetId )			continue;
			SendAll( c.Sock, Frame );
		}
	}

	// クライアント接続の切断.
	void DisconnectConn( Conn& c, ENetworkResult Reason )
	{
		if ( c.Sock == INVALID_SOCKET )	return;
		CloseSocket( c.Sock );
		if ( c.PlayerId == NetworkConst::INVALID_PLAYER_ID )	return;
		const int Id = c.PlayerId;
		c.PlayerId = NetworkConst::INVALID_PLAYER_ID;

		// プレイヤー一覧から削除する.
		SNetworkPlayer Left;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			for ( auto it = Players.begin(); it != Players.end(); ++it ) {
				if ( it->Id != Id )	continue;
				Left = *it;
				Players.erase( it );
				break;
			}
		}

		// 全クライアントへ退出通知.
		std::vector<char> Payload;
		WriteI32( Payload, Id );
		WriteU8( Payload, static_cast<unsigned char>( Reason ) );
		const std::vector<char> Frame = MakeFrame( EPacket::PlayerLeft, Payload );
		for ( auto& Other : Conns ) {
			if ( Other.PlayerId == NetworkConst::INVALID_PLAYER_ID )	continue;
			SendAll( Other.Sock, Frame );
		}

		// 退出コールバック.
		EnqueuePlayer( false, Left );
	}

	// UDP同期データの受信( ホスト側 ).
	void RecvSyncUp()
	{
		while ( true ) {
			char		Buf[2048];
			sockaddr_in	From	= {};
			int			FromLen	= sizeof( From );
			const int Len = recvfrom( UdpSock, Buf, sizeof( Buf ), 0, reinterpret_cast<sockaddr*>( &From ), &FromLen );
			if ( Len == SOCKET_ERROR ) {
				if ( WSAGetLastError() == WSAEWOULDBLOCK )	break;
				continue;	// ICMP由来のエラーなどは無視する.
			}

			ByteReader Reader( Buf, Len );
			if ( Reader.U32() != PROTOCOL_MAGIC )								continue;
			if ( Reader.U8() != static_cast<unsigned char>( EUdp::SyncUp ) )	continue;
			const int				Id		= Reader.I32();
			const unsigned int		Seq		= Reader.U32();
			const unsigned short	Size	= Reader.U16();
			if ( Reader.IsOk == false || Size > Const::Network().MAX_SYNC_DATA_SIZE )	continue;

			// 該当プレイヤーの接続を探して送信元を確認する.
			Conn* pConn = nullptr;
			for ( auto& c : Conns ) {
				if ( c.PlayerId == Id ) { pConn = &c; break; }
			}
			if ( pConn == nullptr )					continue;
			if ( pConn->Ip != AddrToIp( From ) )	continue;	// なりすまし対策の簡易チェック.

			// 同期データの返信先を記録する.
			pConn->UdpAddr		= From;
			pConn->HasUdpAddr	= true;
			if ( Size == 0 )	continue;	// データ無し( ハートビートのみ ).

			// 新しい通し番号の場合のみ保存する( UDPの順序入れ替わり対策 ).
			if ( pConn->SyncSeq != 0 && static_cast<int>( Seq - pConn->SyncSeq ) <= 0 )	continue;
			pConn->SyncSeq = Seq;

			std::vector<char> Data( Size );
			if ( Reader.Read( Data.data(), Size ) == false )	continue;
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				SNetworkPlayer* pPlayer = FindPlayerNoLock( Id );
				if ( pPlayer != nullptr )	pPlayer->SyncData = std::move( Data );
			}
		}
	}

	// 部屋検索要求への応答( ホスト側 ).
	void RecvDiscovery()
	{
		while ( true ) {
			char		Buf[256];
			sockaddr_in	From	= {};
			int			FromLen	= sizeof( From );
			const int Len = recvfrom( DiscoverySock, Buf, sizeof( Buf ), 0, reinterpret_cast<sockaddr*>( &From ), &FromLen );
			if ( Len == SOCKET_ERROR ) {
				if ( WSAGetLastError() == WSAEWOULDBLOCK )	break;
				continue;
			}

			ByteReader Reader( Buf, Len );
			if ( Reader.U32() != PROTOCOL_MAGIC )									continue;
			if ( Reader.U8() != static_cast<unsigned char>( EUdp::DiscoverReq ) )	continue;
			if ( Reader.U16() != PROTOCOL_VERSION )									continue;

			// 部屋情報を返信する.
			std::vector<char> Response;
			WriteU32( Response, PROTOCOL_MAGIC );
			WriteU8( Response, static_cast<unsigned char>( EUdp::DiscoverRes ) );
			WriteU16( Response, PROTOCOL_VERSION );
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				WriteU16( Response, HostSetting.TcpPort );
				WriteU8( Response, static_cast<unsigned char>( Players.size() ) );
				WriteU8( Response, static_cast<unsigned char>( HostSetting.MaxPlayerNum ) );
				WriteU8( Response, HostSetting.Password.empty() ? 0 : 1 );
				WriteStr8( Response, HostSetting.RoomName );
			}
			sendto( DiscoverySock, Response.data(), static_cast<int>( Response.size() ), 0,
				reinterpret_cast<sockaddr*>( &From ), FromLen );
		}
	}

	// 送信予約の処理( ホスト側 ).
	void ProcessSendQueueHost()
	{
		std::vector<PendingSend> Queue;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			Queue.swap( SendQueue );
		}
		for ( auto& Send : Queue ) {
			std::vector<char> Payload;
			WriteI32( Payload, NetworkConst::HOST_PLAYER_ID );
			WriteI32( Payload, Send.TargetId );
			WriteI32( Payload, Send.Type );
			WriteBytes( Payload, Send.Data.data(), static_cast<int>( Send.Data.size() ) );
			const std::vector<char> Frame = MakeFrame( EPacket::UserMessage, Payload );
			for ( auto& c : Conns ) {
				if ( c.PlayerId == NetworkConst::INVALID_PLAYER_ID )						continue;
				if ( Send.TargetId != NetworkConst::ALL_PLAYER && c.PlayerId != Send.TargetId )	continue;
				SendAll( c.Sock, Frame );
			}
		}
	}

	// キック予約の処理( ホスト側 ).
	void ProcessKickQueue()
	{
		std::vector<int> Queue;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			Queue.swap( KickQueue );
		}
		for ( const int Id : Queue ) {
			for ( auto& c : Conns ) {
				if ( c.PlayerId != Id )	continue;
				// 本人へキック通知を送ってから切断する.
				std::vector<char> Payload;
				WriteI32( Payload, Id );
				WriteU8( Payload, static_cast<unsigned char>( ENetworkResult::Kicked ) );
				SendAll( c.Sock, MakeFrame( EPacket::PlayerLeft, Payload ) );
				DisconnectConn( c, ENetworkResult::Kicked );
				break;
			}
		}
	}

	// 同期データの配信( ホスト側 ).
	void SendSyncAll()
	{
		// 全プレイヤーの最新データを1人分ずつのパケットにまとめる.
		//	( 1つの大きなパケットにするとMTUを超えて欠落しやすくなるため ).
		std::vector<std::pair<int, std::vector<char>>> Datagrams;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			// 自分( ホスト )の同期データを反映する.
			SNetworkPlayer* pMe = FindPlayerNoLock( NetworkConst::HOST_PLAYER_ID );
			if ( pMe != nullptr )	pMe->SyncData = MySyncData;

			++SyncSendSeq;
			for ( const auto& p : Players ) {
				if ( p.SyncData.empty() )	continue;
				std::vector<char> Data;
				WriteU32( Data, PROTOCOL_MAGIC );
				WriteU8( Data, static_cast<unsigned char>( EUdp::SyncOne ) );
				WriteI32( Data, p.Id );
				WriteU32( Data, SyncSendSeq );
				WriteU16( Data, static_cast<unsigned short>( p.SyncData.size() ) );
				WriteBytes( Data, p.SyncData.data(), static_cast<int>( p.SyncData.size() ) );
				Datagrams.emplace_back( p.Id, std::move( Data ) );
			}
		}
		for ( auto& c : Conns ) {
			if ( c.HasUdpAddr == false || c.Sock == INVALID_SOCKET )	continue;
			for ( const auto& Datagram : Datagrams ) {
				// 本人のデータは送り返さない.
				if ( Datagram.first == c.PlayerId )	continue;
				sendto( UdpSock, Datagram.second.data(), static_cast<int>( Datagram.second.size() ), 0,
					reinterpret_cast<const sockaddr*>( &c.UdpAddr ), sizeof( c.UdpAddr ) );
			}
		}
	}

	//---------------------------.
	// クライアントのメインループ.
	//---------------------------.
	void ClientLoop()
	{
		// 設定の取得.
		float		ConnectTimeout	= Const::Network().DEFAULT_CONNECT_TIMEOUT_SEC;
		std::string	Name;
		std::string	Pass;
		std::string	HostIpCopy;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			ConnectTimeout	= JoinSetting.ConnectTimeoutSec;
			Name			= JoinSetting.PlayerName;
			Pass			= JoinSetting.Password;
			HostIpCopy		= HostIp;
		}

		// --- 接続完了待ち ---.
		const Clock::time_point Start = Clock::now();
		bool IsConnectDone = false;
		while ( IsStopRequest == false ) {
			fd_set WriteSet;
			fd_set ExceptSet;
			FD_ZERO( &WriteSet );
			FD_ZERO( &ExceptSet );
			FD_SET( TcpSock, &WriteSet );
			FD_SET( TcpSock, &ExceptSet );
			timeval Wait = { 0, 50 * 1000 };
			select( 0, nullptr, &WriteSet, &ExceptSet, &Wait );
			if ( FD_ISSET( TcpSock, &ExceptSet ) )	break;	// 接続失敗.
			if ( FD_ISSET( TcpSock, &WriteSet ) ) {
				IsConnectDone = true;
				break;
			}
			if ( ElapsedSec( Start, Clock::now() ) >= ConnectTimeout )	break;
		}
		if ( IsConnectDone == false || IsStopRequest ) {
			FinishClient( IsStopRequest ? ENetworkResult::None : ENetworkResult::Timeout, true );
			return;
		}
		BOOL NoDelay = TRUE;
		setsockopt( TcpSock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>( &NoDelay ), sizeof( NoDelay ) );

		// --- 参加要求の送信 ---.
		{
			std::vector<char> Payload;
			WriteU16( Payload, PROTOCOL_VERSION );
			WriteStr8( Payload, Name );
			WriteStr8( Payload, Pass );
			if ( SendAll( TcpSock, MakeFrame( EPacket::JoinRequest, Payload ) ) == false ) {
				FinishClient( ENetworkResult::Error, true );
				return;
			}
		}

		// --- 受信ループ ---.
		std::vector<char>						RecvBuf;
		std::unordered_map<int, unsigned int>	SyncSeqs;		// プレイヤーIDごとの受信済み通し番号.
		bool			IsJoined		= false;
		bool			IsEnd			= false;
		bool			IsFirstSync		= true;
		ENetworkResult	EndReason		= ENetworkResult::Error;
		sockaddr_in		HostUdpAddr		= {};
		bool			HasHostUdpAddr	= false;
		unsigned int	MySeq			= 0;
		const float		SyncInterval	= 1.0f / Const::Network().DEFAULT_SYNC_FPS;
		Clock::time_point LastRecv		= Clock::now();
		Clock::time_point LastSyncSend	= Clock::now();
		Clock::time_point LastPing		= Clock::now();

		while ( IsStopRequest == false && IsEnd == false ) {
			fd_set ReadSet;
			FD_ZERO( &ReadSet );
			FD_SET( TcpSock, &ReadSet );
			FD_SET( UdpSock, &ReadSet );
			timeval Wait = { 0, SELECT_WAIT_MS * 1000 };
			select( 0, &ReadSet, nullptr, nullptr, &Wait );

			const Clock::time_point Now = Clock::now();

			// TCP受信.
			if ( FD_ISSET( TcpSock, &ReadSet ) ) {
				bool IsClosed = false;
				while ( true ) {
					char Buf[4096];
					const int Len = recv( TcpSock, Buf, sizeof( Buf ), 0 );
					if ( Len > 0 ) {
						RecvBuf.insert( RecvBuf.end(), Buf, Buf + Len );
						LastRecv = Now;
						continue;
					}
					if ( Len == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK )	break;
					// ホストとの接続が切れた.
					EndReason	= ( Len == 0 ) ? ENetworkResult::RoomClosed : ENetworkResult::Error;
					IsEnd		= true;
					IsClosed	= true;
					break;
				}
				// フレームの処理.
				while ( IsClosed == false && IsEnd == false ) {
					if ( RecvBuf.size() < 3 )	break;
					unsigned short Size = 0;
					std::memcpy( &Size, RecvBuf.data(), 2 );
					if ( Size < 1 || Size > MaxFrameSize() ) {
						EndReason	= ENetworkResult::Error;
						IsEnd		= true;
						break;
					}
					if ( RecvBuf.size() < static_cast<size_t>( 2 + Size ) )	break;

					const EPacket Type = static_cast<EPacket>( static_cast<unsigned char>( RecvBuf[2] ) );
					ByteReader Reader( RecvBuf.data() + 3, Size - 1 );
					HandleClientPacket( Type, Reader, HostIpCopy, &IsJoined, &IsEnd, &EndReason, &HostUdpAddr, &HasHostUdpAddr );
					RecvBuf.erase( RecvBuf.begin(), RecvBuf.begin() + 2 + Size );
				}
			}

			// UDP受信( 同期データ ).
			if ( FD_ISSET( UdpSock, &ReadSet ) )	RecvSyncOne( SyncSeqs );

			// 参加完了待ちのタイムアウト.
			if ( IsJoined == false ) {
				if ( IsEnd == false && ElapsedSec( Start, Now ) >= ConnectTimeout ) {
					EndReason	= ENetworkResult::Timeout;
					IsEnd		= true;
				}
				continue;
			}

			// 送信予約の処理.
			ProcessSendQueueClient();
			// 同期データの送信( ホストへのUDP経路確保を兼ねるため未設定でも送る ).
			if ( HasHostUdpAddr && ( IsFirstSync || ElapsedSec( LastSyncSend, Now ) >= SyncInterval ) ) {
				IsFirstSync		= false;
				LastSyncSend	= Now;
				SendSyncUp( HostUdpAddr, ++MySeq );
			}
			// 生存確認の送信.
			if ( ElapsedSec( LastPing, Now ) >= PING_INTERVAL_SEC ) {
				LastPing = Now;
				SendAll( TcpSock, MakeFrame( EPacket::Ping, {} ) );
			}
			// ホストからの応答が無い場合は切断する.
			if ( ElapsedSec( LastRecv, Now ) >= Const::Network().DEFAULT_CONNECTION_TIMEOUT_SEC ) {
				EndReason	= ENetworkResult::Timeout;
				IsEnd		= true;
			}
		}

		// --- 終了処理 ---.
		if ( IsStopRequest ) {
			// 自分から退出する場合は通知を送る( コールバックは呼ばない ).
			SendAll( TcpSock, MakeFrame( EPacket::Bye, {} ) );
			FinishClient( ENetworkResult::None, false );
			return;
		}
		FinishClient( EndReason, IsJoined == false );
	}

	// クライアント側のパケット処理.
	void HandleClientPacket(
		EPacket Type, ByteReader& Reader, const std::string& HostIpCopy,
		bool* pIsJoined, bool* pIsEnd, ENetworkResult* pEndReason,
		sockaddr_in* pHostUdpAddr, bool* pHasHostUdpAddr )
	{
		switch ( Type ) {
		case EPacket::JoinAccept:
		{
			const int				NewId	= Reader.I32();
			const unsigned short	UdpPort	= Reader.U16();
			const int				Num		= Reader.U8();
			std::vector<SNetworkPlayer> NewPlayers;
			for ( int i = 0; i < Num; ++i ) {
				SNetworkPlayer Player;
				Player.Id		= Reader.I32();
				Player.IsHost	= ( Reader.U8() != 0 );
				Player.Name		= Reader.Str8();
				Player.IsLocal	= ( Player.Id == NewId );
				NewPlayers.push_back( Player );
			}
			if ( Reader.IsOk == false ) {
				*pEndReason	= ENetworkResult::Error;
				*pIsEnd		= true;
				break;
			}
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				MyPlayerId	= NewId;
				HostUdpPort	= UdpPort;
				Players		= std::move( NewPlayers );
				State		= ENetworkState::Connected;
			}
			// 同期データの送信先を作成する.
			*pHasHostUdpAddr = ParseAddr( HostIpCopy, UdpPort, pHostUdpAddr );
			*pIsJoined = true;
			EnqueueResult( true, ENetworkResult::Success );
			break;
		}
		case EPacket::JoinRefuse:
		{
			const ENetworkResult Reason = static_cast<ENetworkResult>( Reader.U8() );
			*pEndReason	= Reader.IsOk ? Reason : ENetworkResult::Error;
			*pIsEnd		= true;
			break;
		}
		case EPacket::PlayerJoined:
		{
			SNetworkPlayer Player;
			Player.Id		= Reader.I32();
			Player.IsHost	= ( Reader.U8() != 0 );
			Player.Name		= Reader.Str8();
			if ( Reader.IsOk == false )	break;
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				if ( FindPlayerNoLock( Player.Id ) == nullptr )	Players.push_back( Player );
			}
			EnqueuePlayer( true, Player );
			break;
		}
		case EPacket::PlayerLeft:
		{
			const int				Id		= Reader.I32();
			const ENetworkResult	Reason	= static_cast<ENetworkResult>( Reader.U8() );
			if ( Reader.IsOk == false )	break;

			// 自分宛の場合はキックなどによる切断.
			bool IsMe = false;
			SNetworkPlayer Left;
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				IsMe = ( Id == MyPlayerId );
				if ( IsMe == false ) {
					for ( auto it = Players.begin(); it != Players.end(); ++it ) {
						if ( it->Id != Id )	continue;
						Left = *it;
						Players.erase( it );
						break;
					}
				}
			}
			if ( IsMe ) {
				*pEndReason	= Reason;
				*pIsEnd		= true;
				break;
			}
			EnqueuePlayer( false, Left );
			break;
		}
		case EPacket::RoomClosed:
		{
			*pEndReason	= ENetworkResult::RoomClosed;
			*pIsEnd		= true;
			break;
		}
		case EPacket::UserMessage:
		{
			const int			SenderId	= Reader.I32();
			Reader.I32();	// 宛先ID( ホストが選別済み ).
			const int			MsgType		= Reader.I32();
			std::vector<char>	Data		= Reader.Rest();
			if ( Reader.IsOk == false )	break;
			EnqueueMessage( SenderId, MsgType, std::move( Data ) );
			break;
		}
		case EPacket::Ping:	SendAll( TcpSock, MakeFrame( EPacket::Pong, {} ) );	break;
		case EPacket::Pong:	break;
		default:			break;
		}
	}

	// UDP同期データの受信( クライアント側 ).
	void RecvSyncOne( std::unordered_map<int, unsigned int>& SyncSeqs )
	{
		while ( true ) {
			char		Buf[2048];
			sockaddr_in	From	= {};
			int			FromLen	= sizeof( From );
			const int Len = recvfrom( UdpSock, Buf, sizeof( Buf ), 0, reinterpret_cast<sockaddr*>( &From ), &FromLen );
			if ( Len == SOCKET_ERROR ) {
				if ( WSAGetLastError() == WSAEWOULDBLOCK )	break;
				continue;	// ICMP由来のエラーなどは無視する.
			}

			ByteReader Reader( Buf, Len );
			if ( Reader.U32() != PROTOCOL_MAGIC )								continue;
			if ( Reader.U8() != static_cast<unsigned char>( EUdp::SyncOne ) )	continue;
			const int				Id		= Reader.I32();
			const unsigned int		Seq		= Reader.U32();
			const unsigned short	Size	= Reader.U16();
			if ( Reader.IsOk == false || Size == 0 || Size > Const::Network().MAX_SYNC_DATA_SIZE )	continue;

			// 新しい通し番号の場合のみ保存する.
			const auto it = SyncSeqs.find( Id );
			if ( it != SyncSeqs.end() && static_cast<int>( Seq - it->second ) <= 0 )	continue;
			SyncSeqs[Id] = Seq;

			std::vector<char> Data( Size );
			if ( Reader.Read( Data.data(), Size ) == false )	continue;
			{
				std::lock_guard<std::mutex> Lock( StateMutex );
				if ( Id == MyPlayerId )	continue;	// 自分のデータはローカルの値を使用する.
				SNetworkPlayer* pPlayer = FindPlayerNoLock( Id );
				if ( pPlayer != nullptr )	pPlayer->SyncData = std::move( Data );
			}
		}
	}

	// 同期データの送信( クライアント側 ).
	void SendSyncUp( const sockaddr_in& HostUdpAddr, unsigned int Seq )
	{
		std::vector<char> Data;
		WriteU32( Data, PROTOCOL_MAGIC );
		WriteU8( Data, static_cast<unsigned char>( EUdp::SyncUp ) );
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			WriteI32( Data, MyPlayerId );
			WriteU32( Data, Seq );
			WriteU16( Data, static_cast<unsigned short>( MySyncData.size() ) );
			WriteBytes( Data, MySyncData.data(), static_cast<int>( MySyncData.size() ) );
		}
		sendto( UdpSock, Data.data(), static_cast<int>( Data.size() ), 0,
			reinterpret_cast<const sockaddr*>( &HostUdpAddr ), sizeof( HostUdpAddr ) );
	}

	// 送信予約の処理( クライアント側 ).
	void ProcessSendQueueClient()
	{
		std::vector<PendingSend>	Queue;
		int							MyId = NetworkConst::INVALID_PLAYER_ID;
		{
			std::lock_guard<std::mutex> Lock( StateMutex );
			Queue.swap( SendQueue );
			MyId = MyPlayerId;
		}
		for ( auto& Send : Queue ) {
			std::vector<char> Payload;
			WriteI32( Payload, MyId );
			WriteI32( Payload, Send.TargetId );
			WriteI32( Payload, Send.Type );
			WriteBytes( Payload, Send.Data.data(), static_cast<int>( Send.Data.size() ) );
			SendAll( TcpSock, MakeFrame( EPacket::UserMessage, Payload ) );
		}
	}

	// クライアントの終了処理.
	//	IsJoinPhase : 参加完了前の失敗かどうか( 通知するコールバックが変わる ).
	void FinishClient( ENetworkResult Reason, bool IsJoinPhase )
	{
		CloseSocket( TcpSock );
		CloseSocket( UdpSock );
		ResetState();
		if ( Reason == ENetworkResult::None )	return;
		EnqueueResult( IsJoinPhase, Reason );
	}
};

//=================================================
//	NetworkManager本体.
//=================================================
NetworkManager::NetworkManager()
	: m_pImpl	( std::make_unique<Impl>() )
{
}

NetworkManager::~NetworkManager()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
NetworkManager* NetworkManager::GetInstance()
{
	static NetworkManager Instance;
	return &Instance;
}

//---------------------------.
// 部屋を立てる.
//---------------------------.
bool NetworkManager::StartHost( const SNetworkHostState& State )
{
	return GetInstance()->m_pImpl->StartHost( State );
}

//---------------------------.
// 部屋番号で参加する.
//---------------------------.
bool NetworkManager::JoinRoom( const std::string& Code, const SNetworkJoinState& State )
{
	std::string		Ip;
	unsigned short	Port = 0;
	if ( RoomCode::Decode( Code, &Ip, &Port ) == false )	return false;
	return JoinIp( Ip, Port, State );
}

//---------------------------.
// IPアドレス直接指定で参加する.
//---------------------------.
bool NetworkManager::JoinIp( const std::string& Ip, unsigned short Port, const SNetworkJoinState& State )
{
	if ( Port == 0 )	Port = Const::Network().DEFAULT_TCP_PORT;
	return GetInstance()->m_pImpl->StartJoin( Ip, Port, State );
}

//---------------------------.
// 退出する.
//---------------------------.
void NetworkManager::Leave()
{
	GetInstance()->m_pImpl->Leave();
}

//---------------------------.
// 部屋検索の開始.
//---------------------------.
void NetworkManager::StartRoomSearch()
{
	GetInstance()->m_pImpl->StartSearch();
}

//---------------------------.
// 部屋検索の停止.
//---------------------------.
void NetworkManager::StopRoomSearch()
{
	GetInstance()->m_pImpl->StopSearch();
}

//---------------------------.
// 見つかった部屋一覧の取得.
//---------------------------.
std::vector<SNetworkRoomInfo> NetworkManager::GetFoundRooms()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::vector<SNetworkRoomInfo> Rooms;
	std::lock_guard<std::mutex> Lock( pImpl->SearchMutex );
	for ( const auto& Pair : pImpl->FoundRooms ) {
		Rooms.push_back( Pair.second.Info );
	}
	return Rooms;
}

//---------------------------.
// 状態の取得.
//---------------------------.
ENetworkState NetworkManager::GetState()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	return pImpl->State;
}
ENetworkMode NetworkManager::GetMode()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	return pImpl->Mode;
}
bool NetworkManager::GetIsConnected()
{
	const ENetworkState State = GetState();
	return State == ENetworkState::Connected || State == ENetworkState::Hosting;
}
bool NetworkManager::GetIsHost()
{
	return GetMode() == ENetworkMode::Host;
}

//---------------------------.
// 部屋番号の取得.
//---------------------------.
std::string NetworkManager::GetRoomCode()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	if ( pImpl->Mode == ENetworkMode::None )	return "";
	return RoomCode::Encode( pImpl->HostIp, pImpl->HostTcpPort );
}
std::string NetworkManager::GetGlobalRoomCode()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	if ( pImpl->UPnP != EUPnPState::Success )	return "";

	std::string ExternalIp;
	{
		std::lock_guard<std::mutex> Lock( pImpl->UPnPMutex );
		ExternalIp = pImpl->ExternalIp;
	}
	if ( ExternalIp.empty() )	return "";

	unsigned short Port = 0;
	{
		std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
		if ( pImpl->Mode != ENetworkMode::Host )	return "";
		Port = pImpl->HostTcpPort;
	}
	return RoomCode::Encode( ExternalIp, Port );
}
EUPnPState NetworkManager::GetUPnPState()
{
	return GetInstance()->m_pImpl->UPnP;
}

//---------------------------.
// プレイヤー情報の取得.
//---------------------------.
int NetworkManager::GetMyPlayerId()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	return pImpl->MyPlayerId;
}
int NetworkManager::GetPlayerNum()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	return static_cast<int>( pImpl->Players.size() );
}
std::vector<SNetworkPlayer> NetworkManager::GetPlayers()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	// 自分の同期データはローカルの最新値を反映して返す.
	std::vector<SNetworkPlayer> Result = pImpl->Players;
	for ( auto& p : Result ) {
		if ( p.IsLocal )	p.SyncData = pImpl->MySyncData;
	}
	return Result;
}
bool NetworkManager::GetPlayer( int PlayerId, SNetworkPlayer* pOut )
{
	if ( pOut == nullptr )	return false;
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	SNetworkPlayer* pPlayer = pImpl->FindPlayerNoLock( PlayerId );
	if ( pPlayer == nullptr )	return false;
	*pOut = *pPlayer;
	if ( pOut->IsLocal )	pOut->SyncData = pImpl->MySyncData;
	return true;
}

//---------------------------.
// 自分の同期データの設定.
//---------------------------.
void NetworkManager::SetSyncData( const void* pData, int Size )
{
	if ( pData == nullptr || Size <= 0 || Size > Const::Network().MAX_SYNC_DATA_SIZE )	return;
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	const char* pBytes = static_cast<const char*>( pData );
	pImpl->MySyncData.assign( pBytes, pBytes + Size );
}

//---------------------------.
// データの送信.
//---------------------------.
void NetworkManager::SendData( int Type, const void* pData, int Size, int TargetId )
{
	if ( Size < 0 || Size > Const::Network().MAX_MESSAGE_SIZE )	return;
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	if ( pImpl->State != ENetworkState::Connected && pImpl->State != ENetworkState::Hosting )	return;
	if ( TargetId == pImpl->MyPlayerId )	return;	// 自分宛は不要.

	Impl::PendingSend Send;
	Send.TargetId	= TargetId;
	Send.Type		= Type;
	if ( pData != nullptr && Size > 0 ) {
		const char* pBytes = static_cast<const char*>( pData );
		Send.Data.assign( pBytes, pBytes + Size );
	}
	pImpl->SendQueue.push_back( std::move( Send ) );
}

//---------------------------.
// 文字列の送信.
//---------------------------.
void NetworkManager::SendString( int Type, const std::string& Text, int TargetId )
{
	SendData( Type, Text.data(), static_cast<int>( Text.length() ), TargetId );
}

//---------------------------.
// コールバックの設定.
//---------------------------.
void NetworkManager::SetOnMessage( MessageFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnMessage = std::move( Func );
}
void NetworkManager::SetOnPlayerJoin( PlayerFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnPlayerJoin = std::move( Func );
}
void NetworkManager::SetOnPlayerLeave( PlayerFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnPlayerLeave = std::move( Func );
}
void NetworkManager::SetOnJoinResult( ResultFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnJoinResult = std::move( Func );
}
void NetworkManager::SetOnDisconnected( ResultFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnDisconnected = std::move( Func );
}
void NetworkManager::SetOnUPnPResult( UPnPFunc Func )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->OnUPnPResult = std::move( Func );
}

//---------------------------.
// 指定プレイヤーをキックする.
//---------------------------.
void NetworkManager::Kick( int PlayerId )
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	std::lock_guard<std::mutex> Lock( pImpl->StateMutex );
	if ( pImpl->Mode != ENetworkMode::Host )				return;
	if ( PlayerId == NetworkConst::HOST_PLAYER_ID )			return;
	pImpl->KickQueue.push_back( PlayerId );
}

//---------------------------.
// 自分のLAN内IPアドレスの取得.
//---------------------------.
std::string NetworkManager::GetLocalIp()
{
	if ( GetInstance()->m_pImpl->IsWsaInit == false )	return "127.0.0.1";

	// 外部宛のUDPソケットを作成して自分のIPアドレスを取得する( 実際に送信はされない ).
	SOCKET Sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( Sock == INVALID_SOCKET )	return "127.0.0.1";

	std::string Result = "127.0.0.1";
	sockaddr_in Addr = {};
	if ( ParseAddr( "8.8.8.8", 53, &Addr ) &&
		 connect( Sock, reinterpret_cast<sockaddr*>( &Addr ), sizeof( Addr ) ) == 0 ) {
		sockaddr_in	Local		= {};
		int			LocalLen	= sizeof( Local );
		if ( getsockname( Sock, reinterpret_cast<sockaddr*>( &Local ), &LocalLen ) == 0 ) {
			Result = AddrToIp( Local );
		}
	}
	closesocket( Sock );
	return Result;
}

//---------------------------.
// 更新処理.
//---------------------------.
void NetworkManager::Update()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();

	// 溜まっているコールバックをメインスレッドで実行する.
	std::vector<std::function<void()>> Queue;
	{
		std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
		Queue.swap( pImpl->CallbackQueue );
	}
	for ( const auto& Func : Queue )	Func();
}

//---------------------------.
// 終了処理.
//---------------------------.
void NetworkManager::Release()
{
	Impl* pImpl = GetInstance()->m_pImpl.get();
	pImpl->Leave();
	pImpl->StopSearch();
	pImpl->JoinUPnPThread();

	// 溜まっているコールバックを破棄する.
	std::lock_guard<std::mutex> Lock( pImpl->CallbackMutex );
	pImpl->CallbackQueue.clear();
}

#endif	// #ifdef ENABLE_NETWORK.
