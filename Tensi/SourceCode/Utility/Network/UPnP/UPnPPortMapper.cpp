#include "UPnPPortMapper.h"
#ifdef ENABLE_NETWORK
#include <windows.h>
#include <natupnp.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

namespace
{
	constexpr int		GET_COLLECTION_RETRY	= 3;		// コレクション取得のリトライ回数.
	constexpr DWORD		GET_COLLECTION_WAIT_MS	= 1000;		// リトライ間隔[ミリ秒].
	constexpr wchar_t	MAPPING_DESCRIPTION[]	= L"Tensi";	// ルーターに登録される説明文.

	// COMの初期化( 呼び出し側で終了処理が必要かを返す ).
	bool InitCom( bool* pOutIsNeedUninit )
	{
		const HRESULT hr = CoInitializeEx( nullptr, COINIT_MULTITHREADED );
		*pOutIsNeedUninit = SUCCEEDED( hr );
		// 既に別モードで初期化済みの場合はそのまま使用する.
		return SUCCEEDED( hr ) || hr == RPC_E_CHANGED_MODE;
	}

	// ポートマッピングコレクションの取得( 失敗時は nullptr ).
	//	ルーターの応答待ちで失敗することがあるため数回リトライする.
	IStaticPortMappingCollection* GetCollection( IUPnPNAT* pNat )
	{
		for ( int i = 0; i < GET_COLLECTION_RETRY; ++i ) {
			IStaticPortMappingCollection* pCollection = nullptr;
			if ( SUCCEEDED( pNat->get_StaticPortMappingCollection( &pCollection ) ) && pCollection != nullptr ) {
				return pCollection;
			}
			Sleep( GET_COLLECTION_WAIT_MS );
		}
		return nullptr;
	}
}

//---------------------------.
// ポートマッピングの追加.
//---------------------------.
bool UPnPPortMapper::AddPortMapping( unsigned short TcpPort, unsigned short UdpPort, const std::string& LocalIp, std::string* pOutExternalIp )
{
	bool IsNeedUninit = false;
	if ( InitCom( &IsNeedUninit ) == false )	return false;

	bool Result = false;
	IUPnPNAT* pNat = nullptr;
	if ( SUCCEEDED( CoCreateInstance( __uuidof( UPnPNAT ), nullptr, CLSCTX_ALL, __uuidof( IUPnPNAT ), reinterpret_cast<void**>( &pNat ) ) ) && pNat != nullptr ) {
		IStaticPortMappingCollection* pCollection = GetCollection( pNat );
		if ( pCollection != nullptr ) {
			const std::wstring	WideLocalIp( LocalIp.begin(), LocalIp.end() );
			std::wstring		WideExternalIp;

			// TCPとUDPの両方を開放する.
			const bool IsTcp = Add( pCollection, static_cast<long>( TcpPort ), L"TCP", WideLocalIp, &WideExternalIp );
			const bool IsUdp = Add( pCollection, static_cast<long>( UdpPort ), L"UDP", WideLocalIp, nullptr );
			if ( IsTcp && IsUdp ) {
				Result = true;
				if ( pOutExternalIp != nullptr ) {
					// 外部IPはASCIIのため1文字ずつ変換する.
					pOutExternalIp->clear();
					for ( const wchar_t Char : WideExternalIp ) {
						pOutExternalIp->push_back( static_cast<char>( Char ) );
					}
				}
			}
			else {
				// 片方だけ成功した場合は掃除する.
				Delete( pCollection, static_cast<long>( TcpPort ), L"TCP" );
				Delete( pCollection, static_cast<long>( UdpPort ), L"UDP" );
			}
			pCollection->Release();
		}
		pNat->Release();
	}

	if ( IsNeedUninit )	CoUninitialize();
	return Result;
}

//---------------------------.
// ポートマッピングの削除.
//---------------------------.
void UPnPPortMapper::DeletePortMapping( unsigned short TcpPort, unsigned short UdpPort )
{
	bool IsNeedUninit = false;
	if ( InitCom( &IsNeedUninit ) == false )	return;

	IUPnPNAT* pNat = nullptr;
	if ( SUCCEEDED( CoCreateInstance( __uuidof( UPnPNAT ), nullptr, CLSCTX_ALL, __uuidof( IUPnPNAT ), reinterpret_cast<void**>( &pNat ) ) ) && pNat != nullptr ) {
		IStaticPortMappingCollection* pCollection = GetCollection( pNat );
		if ( pCollection != nullptr ) {
			Delete( pCollection, static_cast<long>( TcpPort ), L"TCP" );
			Delete( pCollection, static_cast<long>( UdpPort ), L"UDP" );
			pCollection->Release();
		}
		pNat->Release();
	}

	if ( IsNeedUninit )	CoUninitialize();
}

//---------------------------.
// 1件分の追加.
//---------------------------.
bool UPnPPortMapper::Add( void* pCollection, long Port, const wchar_t* pProtocol, const std::wstring& LocalIp, std::wstring* pOutExternalIp )
{
	IStaticPortMappingCollection* pCol = static_cast<IStaticPortMappingCollection*>( pCollection );

	BSTR Protocol	= SysAllocString( pProtocol );
	BSTR Client		= SysAllocString( LocalIp.c_str() );
	BSTR Desc		= SysAllocString( MAPPING_DESCRIPTION );

	IStaticPortMapping* pMapping = nullptr;
	const HRESULT hr = pCol->Add( Port, Protocol, Port, Client, VARIANT_TRUE, Desc, &pMapping );

	const bool Result = SUCCEEDED( hr ) && pMapping != nullptr;
	if ( Result && pOutExternalIp != nullptr ) {
		// ルーターの外部IPアドレスを取得する.
		BSTR ExternalIp = nullptr;
		if ( SUCCEEDED( pMapping->get_ExternalIPAddress( &ExternalIp ) ) && ExternalIp != nullptr ) {
			*pOutExternalIp = ExternalIp;
			SysFreeString( ExternalIp );
		}
	}

	if ( pMapping != nullptr )	pMapping->Release();
	SysFreeString( Desc );
	SysFreeString( Client );
	SysFreeString( Protocol );
	return Result;
}

//---------------------------.
// 1件分の削除.
//---------------------------.
void UPnPPortMapper::Delete( void* pCollection, long Port, const wchar_t* pProtocol )
{
	IStaticPortMappingCollection* pCol = static_cast<IStaticPortMappingCollection*>( pCollection );

	BSTR Protocol = SysAllocString( pProtocol );
	pCol->Remove( Port, Protocol );
	SysFreeString( Protocol );
}

#endif	// #ifdef ENABLE_NETWORK.
