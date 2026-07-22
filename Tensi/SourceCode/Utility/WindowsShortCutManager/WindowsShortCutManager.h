#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_WINDOWS_SHORT_CUT
#include "..\..\Global.h"

/************************************************
*	Windows のショートカット関連機能をまとめた名前空間.
**/
namespace WindowsShortCutManager {
	// ショートカットを作成する.
	bool CreateShortcut(
		LPCTSTR pszLink,				LPCTSTR pszFile,
		LPCTSTR pszDescription = NULL,	LPCTSTR pszArgs		= NULL,
		LPCTSTR pszWorkingDir  = NULL,	LPCTSTR pszIconPath	= NULL,
		int nIcon = 0, int nShowCmd = SW_SHOWNORMAL );

	// スタートアップフォルダにショートカットを作成する.
	//	pszLinkName     : 作成するショートカットのファイル名( 例: L"Slime.lnk" ).
	//	pszFile         : ショートカットの実行先ファイルパス.
	//	IsEnableStartup : 作成後にスタートアップ項目を有効/無効のどちらに設定するか( 既にある場合はユーザー設定を尊重して何もしない ).
	bool CreateStartupShortcut(
		LPCWSTR pszLinkName,			LPCTSTR pszFile,
		const bool IsEnableStartup,
		LPCTSTR pszDescription = NULL,	LPCTSTR pszArgs		= NULL,
		LPCTSTR pszWorkingDir  = NULL,	LPCTSTR pszIconPath	= NULL,
		int nIcon = 0, int nShowCmd = SW_SHOWNORMAL );

	// スタートアップフォルダにショートカットを作成する.
	//	pszLinkName     : 作成するショートカットのファイル名( 例: L"Slime.lnk" ).
	//	pszFile         : ショートカットの実行先ファイルパス.
	//	IsEnableStartup : 作成後にスタートアップ項目を有効/無効のどちらに設定するか( 強制的に設定する ).
	bool CreateStartupShortcutApprovalForce(
		LPCWSTR pszLinkName,			LPCTSTR pszFile,
		const bool IsEnableStartup,
		LPCTSTR pszDescription = NULL,	LPCTSTR pszArgs		= NULL,
		LPCTSTR pszWorkingDir  = NULL,	LPCTSTR pszIconPath	= NULL,
		int nIcon = 0, int nShowCmd = SW_SHOWNORMAL );

	// スタートアップ項目を有効化する.
	//	pszLinkName : スタートアップフォルダ内のショートカットのファイル名( 例: L"Slime.lnk" ).
	//	レジストリにエントリが無い初回のみ有効を書き込み, 既にある場合はユーザー設定を尊重して何もしない.
	void EnableStartupApproval( LPCWSTR pszLinkName );

	// スタートアップ項目の有効/無効を強制的に設定する( 既存のユーザー設定を上書きする ).
	void SetStartupApprovalForce( LPCWSTR pszLinkName, const bool IsEnable );
}

#endif