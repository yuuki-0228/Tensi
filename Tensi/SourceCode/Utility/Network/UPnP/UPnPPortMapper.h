#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_NETWORK
#include <string>

/************************************************
*	UPnPによるルーターのポート自動開放.
*	ルーターがUPnP(IGD)に対応している場合のみ成功する.
*	( 家庭用ルーターは大抵対応しているが、無効設定の場合は失敗する ).
*
*	注意.
*	・処理に数秒かかることがあるため、必ず別スレッドから呼ぶこと.
*	・開放したポートはアプリ終了前に必ず DeletePortMapping で閉じること.
**/
class UPnPPortMapper final
{
public:
	// ポートマッピングの追加( ブロッキング ).
	//	LocalIp : この端末のLAN内IPアドレス.
	//	成功時は true を返し、pOutExternalIp にルーターの外部IPアドレスが入る.
	static bool AddPortMapping( unsigned short TcpPort, unsigned short UdpPort, const std::string& LocalIp, std::string* pOutExternalIp );

	// ポートマッピングの削除( ブロッキング ).
	static void DeletePortMapping( unsigned short TcpPort, unsigned short UdpPort );

private:
	// 1件分の追加( pCollection は IStaticPortMappingCollection* ).
	static bool Add( void* pCollection, long Port, const wchar_t* pProtocol, const std::wstring& LocalIp, std::wstring* pOutExternalIp );
	// 1件分の削除.
	static void Delete( void* pCollection, long Port, const wchar_t* pProtocol );
};

#endif	// #ifdef ENABLE_NETWORK.
