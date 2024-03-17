#pragma once
#include "..\..\..\Global.h"
#include <Shlwapi.h>

/***********************************
*	フォントスプライト作成クラス.
*		﨑田友輝.
**/
class CFontCreate final
{
public:
	CFontCreate( const std::string& FilePath, const std::string& FileName );
	~CFontCreate();

	// フォント画像の作成.
	HRESULT CreateFontTexture2D( const char* c, ID3D11ShaderResourceView** textur2D );

private:
	// フォントを利用可能にする.
	int FontAvailable();
	// Textur2D_Descを作成.
	D3D11_TEXTURE2D_DESC CreateDesc( UINT width, UINT height );

private:
	ID3D11Device*			m_pDevice;		// デバイス.
	ID3D11DeviceContext*	m_pContext;		// デバイスコンテキスト.
	std::wstring			m_wFilePath;	// ファイルパス.
	std::wstring			m_wFileName;	// ファイル名.

private:
	// デフォルトコンストラクタを禁止する.
	CFontCreate() = delete;
	CFontCreate( const CFontCreate& ) = delete;
	CFontCreate& operator = ( const CFontCreate& rhs ) = default;
};
