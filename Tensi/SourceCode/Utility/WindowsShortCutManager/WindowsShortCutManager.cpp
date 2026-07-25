#include "WindowsShortCutManager.h"
#ifdef ENABLE_WINDOWS_SHORT_CUT
#include <shlobj.h>
#include "..\Obfuscate\Obfuscate.h"

//---------------------------.
// ショートカットを作成する.
//---------------------------.
bool WindowsShortCutManager::CreateShortcut(
	LPCTSTR pszLink,		LPCTSTR pszFile,
	LPCTSTR pszDescription, LPCTSTR pszArgs,
	LPCTSTR pszWorkingDir,  LPCTSTR pszIconPath,
	int nIcon, int nShowCmd )
{
	HRESULT			hr;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;

	//IShellLinkの作成
	pIShellLink = NULL;
	hr = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( void** )&pIShellLink );
	if ( pIShellLink == NULL || FAILED( hr ) ) return	false;

	//IPersistFileの取得
	pIPersistFile = NULL;
	hr = pIShellLink->QueryInterface( IID_IPersistFile, ( void** )&pIPersistFile );
	if ( pIPersistFile == NULL || FAILED( hr ) )
	{
		pIShellLink->Release();
		return false;
	}

	//ショートカット詳細設定
	hr = pIShellLink->SetPath( pszFile );
	if ( SUCCEEDED( hr ) && pszDescription	) hr = pIShellLink->SetDescription( pszDescription );
	if ( SUCCEEDED( hr ) && pszArgs			) hr = pIShellLink->SetArguments( pszArgs );
	if ( SUCCEEDED( hr ) && pszWorkingDir	) hr = pIShellLink->SetWorkingDirectory( pszWorkingDir );
	if ( SUCCEEDED( hr ) && pszIconPath		) hr = pIShellLink->SetIconLocation( pszIconPath, nIcon );
	if ( SUCCEEDED( hr )					) hr = pIShellLink->SetShowCmd( nShowCmd );

#ifndef UNICODE
	WCHAR*	pwszUnicode;
	int		nLen;

	//Unicode変換
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, NULL, 0 );
	pwszUnicode = new WCHAR[nLen + 1];
	if ( pwszUnicode == NULL )
	{
		pIPersistFile->Release();
		pIShellLink->Release();
		return	false;
	}
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, pwszUnicode, nLen + 1 );
	if ( nLen == 0 ) hr = E_FAIL;

	//ショートカットの保存
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pwszUnicode, TRUE );

	delete	pwszUnicode;
#else
	//ショートカットの保存
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pszLink, TRUE );
#endif

	pIPersistFile->Release();
	pIShellLink->Release();

	return SUCCEEDED( hr ) ? true : false;
}

//---------------------------.
// スタートアップフォルダにショートカットを作成する.
//---------------------------.
bool WindowsShortCutManager::CreateStartupShortcut(
	LPCWSTR pszLinkName, LPCTSTR pszFile,
	const bool IsEnableStartup,
	LPCTSTR pszDescription, LPCTSTR pszArgs,
	LPCTSTR pszWorkingDir, LPCTSTR pszIconPath,
	int nIcon, int nShowCmd )
{
	// スタートアップフォルダのパスを取得.
	PWSTR pStartupDir = nullptr;
	if ( FAILED( SHGetKnownFolderPath( FOLDERID_Startup, 0, nullptr, &pStartupDir ) ) ) {
		if ( pStartupDir != nullptr ) CoTaskMemFree( pStartupDir );
		return false;
	}
	const std::wstring ShortcutPath = std::wstring( pStartupDir ) + L"\\" + pszLinkName;
	CoTaskMemFree( pStartupDir );

	// ショートカットを作成.
	if ( CreateShortcut(
		ShortcutPath.c_str(), pszFile,
		pszDescription, pszArgs,
		pszWorkingDir, pszIconPath,
		nIcon, nShowCmd ) == false ) {
		return false;
	}

	// IsEnableStartup が true の場合のみ有効化する( 初回のみ・既存のユーザー設定は尊重する ).
	if ( IsEnableStartup ) {
		EnableStartupApproval( pszLinkName );
	}
	return true;
}

//---------------------------.
// スタートアップフォルダにショートカットを作成する.
//---------------------------.
bool WindowsShortCutManager::CreateStartupShortcutApprovalForce(
	LPCWSTR pszLinkName, LPCTSTR pszFile,
	const bool IsEnableStartup,
	LPCTSTR pszDescription, LPCTSTR pszArgs,
	LPCTSTR pszWorkingDir, LPCTSTR pszIconPath,
	int nIcon, int nShowCmd )
{
	// スタートアップフォルダのパスを取得.
	PWSTR pStartupDir = nullptr;
	if ( FAILED( SHGetKnownFolderPath( FOLDERID_Startup, 0, nullptr, &pStartupDir ) ) ) {
		if ( pStartupDir != nullptr ) CoTaskMemFree( pStartupDir );
		return false;
	}
	const std::wstring ShortcutPath = std::wstring( pStartupDir ) + L"\\" + pszLinkName;
	CoTaskMemFree( pStartupDir );

	// ショートカットを作成.
	if ( CreateShortcut(
		ShortcutPath.c_str(), pszFile,
		pszDescription, pszArgs,
		pszWorkingDir, pszIconPath,
		nIcon, nShowCmd ) == false ) {
		return false;
	}

	// スタートアップ項目の有効/無効を設定する( 強制的に設定する ).
	SetStartupApprovalForce( pszLinkName, IsEnableStartup );

	return true;
}

//---------------------------.
// スタートアップ項目を有効化する.
//---------------------------.
void WindowsShortCutManager::EnableStartupApproval( LPCWSTR pszLinkName )
{
	// スタートアップの有効/無効を管理するレジストリキー.
	const std::wstring KeyPath =
		OBFW( L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" );

	// キーを開く( 無ければ作成する ).
	HKEY hKey = NULL;
	LONG lResult = ::RegCreateKeyExW(
		HKEY_CURRENT_USER, KeyPath.c_str(), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL );
	if ( lResult != ERROR_SUCCESS ) return;

	// 既にエントリが存在する場合はユーザー設定を尊重して何もしない.
	const bool IsExist = ( ::RegQueryValueExW( hKey, pszLinkName, NULL, NULL, NULL, NULL ) == ERROR_SUCCESS );
	::RegCloseKey( hKey );
	if ( IsExist ) return;

	// エントリが無い場合のみ有効にする.
	SetStartupApprovalForce( pszLinkName, true );
}

//---------------------------.
// スタートアップ項目の有効/無効を強制的に設定する.
//---------------------------.
void WindowsShortCutManager::SetStartupApprovalForce( LPCWSTR pszLinkName, const bool IsEnable )
{
	// スタートアップの有効/無効を管理するレジストリキー.
	const std::wstring KeyPath =
		OBFW( L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\StartupFolder" );

	// キーを開く( 無ければ作成する ).
	HKEY hKey = NULL;
	LONG lResult = ::RegCreateKeyExW(
		HKEY_CURRENT_USER, KeyPath.c_str(), 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL );
	if ( lResult != ERROR_SUCCESS ) return;

	// 有効/無効を表す 12 バイトのデータ( 先頭が 0x02 なら有効, 0x03 なら無効 ).
	BYTE ApprovalData[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	ApprovalData[0] = IsEnable ? 0x02 : 0x03;
	::RegSetValueExW( hKey, pszLinkName, 0, REG_BINARY, ApprovalData, sizeof( ApprovalData ) );

	::RegCloseKey( hKey );
}

#endif