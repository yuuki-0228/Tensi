#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_FONT
#include "..\..\..\Global.h"
#include "..\FontStruct.h"
#include <Shlwapi.h>

/***********************************
*	フォントスプライト作成クラス.
*	  GDIでラスタライズした文字をSDF(符号付き距離場)テクスチャに変換する.
**/
class CFontCreate final
{
public:
	CFontCreate( const std::string& FilePath, const std::string& FileName );
	~CFontCreate();

	// フォントSDF画像の作成.
	HRESULT CreateFontTexture2D( const char* c, SFontGlyph* pGlyph );

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
#ifndef _DEBUG
	HANDLE					m_hFontMem;		// メモリフォントのハンドル
	void*					m_pFontMemBuf;	// メモリ復号済みフォントバッファ
#endif

private:
	// デフォルトコンストラクタを禁止する.
	CFontCreate() = delete;
	CFontCreate( const CFontCreate& ) = delete;
	CFontCreate& operator = ( const CFontCreate& rhs ) = default;
};
#endif // ENABLE_FONT
