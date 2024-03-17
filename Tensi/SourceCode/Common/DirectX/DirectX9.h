#pragma once
#include "..\..\Global.h"

/************************************************
*	DirectX9 セットアップ.
**/
class DirectX9 final
{
public:
	DirectX9();
	~DirectX9();

	// DirectX9構築.
	static HRESULT Create( HWND hWnd );
	// DirectX9解放.
	static void Release();

	// デバイスオブジェクトを取得.
	static LPDIRECT3DDEVICE9 GetDevice() { return GetInstance()->m_pDevice9; }

private:
	// インスタンスの取得.
	static DirectX9* GetInstance();

private:
	LPDIRECT3DDEVICE9	m_pDevice9;	// デバイスオブジェクト.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	DirectX9( const DirectX9 & ) = delete;
	DirectX9& operator = ( const DirectX9 & ) = delete;
	DirectX9( DirectX9 && ) = delete;
	DirectX9& operator = ( DirectX9 && ) = delete;
};