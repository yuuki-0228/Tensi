#include "FontCreate.h"
#include "..\..\DirectX\DirectX11.h"
#include "..\..\..\Utility\StringConversion\StringConversion.h"

namespace {
	const int FONT_BMP_SIZE	= 256;
}

CFontCreate::CFontCreate( const std::string& FilePath, const std::string& FileName )
	: m_pDevice		( nullptr )
	, m_pContext	( nullptr )
	, m_wFilePath	()
	, m_wFileName	()
{
	m_pDevice	= DirectX11::GetDevice();
	m_pContext	= DirectX11::GetContext();

	// ファイルパス/ファイル名を保存する.
	m_wFilePath = StringConversion::to_wString( FilePath );
	m_wFileName = StringConversion::to_wString( FileName );

	// フォントを利用可能にする.
	FontAvailable();
}

CFontCreate::~CFontCreate()
{
	// 利用可能にしたフォントを破棄する.
	RemoveFontResourceEx( m_wFilePath.c_str(), FR_PRIVATE, nullptr );
}

//-----------------------------------.
// フォント画像の作成.
//-----------------------------------.
HRESULT CFontCreate::CreateFontTexture2D( const char* c, ID3D11ShaderResourceView** resource )
{
	if( m_pDevice	== nullptr ) return E_FAIL;
	if( m_pContext	== nullptr ) return E_FAIL;

	// 文字コード取得.
	std::wstring	wString = StringConversion::to_wString( c );
	UINT			Code	= (UINT)*wString.c_str();

	//-------------------------------------------------.
	// フォントの生成.
	//-------------------------------------------------.
	LOGFONT lf = { 
		FONT_BMP_SIZE,					// 文字セルまたは文字の高さ.
		0,								// 平均文字幅.
		0,								// 文字送りの方向とX軸との角度.
		0,								// ベースラインとX軸との角度.
		0,								// フォントの太さ.
		0,								// イタリック体の指定.
		0,								// 下線付き指定.
		0,								// 打消し線付き指定.
		SHIFTJIS_CHARSET,				// キャラクタセット.
		OUT_TT_ONLY_PRECIS,				// 出力精度.
		CLIP_DEFAULT_PRECIS,			// クリッピングの精度.
		PROOF_QUALITY,					// 出力品質.
		FIXED_PITCH | FF_MODERN,		// ピッチとファミリ.
	};

	// フォント名を設定.
	const int NameSize = static_cast<int>( m_wFileName.size() );
	for ( int i = 0; i < NameSize; ++i ) {
		lf.lfFaceName[i] = m_wFileName[i];
	}

	HFONT hFont = nullptr;
	hFont		= CreateFontIndirect( &lf );
	if(  hFont == nullptr ) return E_FAIL;

	HDC		Hdc		= nullptr;
	HFONT	OldFont	= nullptr;
	// デバイスコンテキスト取得.
	//	デバイスにフォントを持たせないとGetGlyphOutline関数はエラーとなる.
	Hdc		= GetDC( nullptr );
	OldFont = (HFONT)SelectObject( Hdc, hFont );

	// フォントビットマップ取得.
	TEXTMETRIC		TM;
	GetTextMetrics( Hdc, &TM );
	GLYPHMETRICS	GM;
	CONST MAT2		Mat		= { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	DWORD			Size	= GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, 0, nullptr, &Mat );
	BYTE*			Ptr		= new BYTE[Size];
	GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, Size, Ptr, &Mat );

	// デバイスコンテキストとフォントハンドルの開放.
	SelectObject( Hdc, OldFont );
	DeleteObject( hFont );
	ReleaseDC( nullptr, Hdc );


	//-------------------------------------------------.
	// 書き込み可能テクスチャ作成.
	//	CPUで書き込みができるテクスチャを作成.
	//-------------------------------------------------.
	D3D11_TEXTURE2D_DESC	Desc		= CreateDesc( GM.gmCellIncX, TM.tmHeight );
	ID3D11Texture2D*		Texture2D	= nullptr;

	if ( FAILED( m_pDevice->CreateTexture2D( &Desc, 0, &Texture2D ) ) ) {
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE hMappedResource;
	if ( FAILED( m_pContext->Map(
		Texture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &hMappedResource ) ) ) {
		SAFE_RELEASE( Texture2D );
		return E_FAIL;
	}

	// データを取得する.
	BYTE* pBits = (BYTE*)hMappedResource.pData;
	
	// フォント情報の書き込み.
	//	iOfs_x, iOfs_y : 書き出し位置( 左上 ).
	//	iBmp_w, iBmp_h : フォントビットマップの幅高.
	//	Level : α値の段階( GGO_GRAY4_BITMAPなので17段階 ).
	int iOfs_x	= GM.gmptGlyphOrigin.x;
	int iOfs_y	= TM.tmAscent - GM.gmptGlyphOrigin.y;
	int iBmp_w	= GM.gmBlackBoxX + ( 4 - ( GM.gmBlackBoxX % 4 ) ) % 4 ;
	int iBmp_h	= GM.gmBlackBoxY;
	int Level	= 17;
	int x, y;
	DWORD Alpha, Color;
	memset( pBits, 0, hMappedResource.RowPitch * TM.tmHeight );
	for ( y = iOfs_y; y < iOfs_y + iBmp_h; y++ ) {
		for ( x = iOfs_x; x < iOfs_x + iBmp_w; x++ ) {
			Alpha = ( 255 * Ptr[x - iOfs_x + iBmp_w * ( y - iOfs_y )]) / ( Level - 1 );
			Color = 0x00ffffff | ( Alpha << 24 );

			memcpy( (BYTE*) pBits + hMappedResource.RowPitch * y + 4 * x, &Color, sizeof( DWORD ) );
		}
	}
	m_pContext->Unmap( Texture2D, 0 );

	delete[] Ptr;

	//-------------------------------------------------.
	// テクスチャ情報をシェーダーリソースビューにする.
	//-------------------------------------------------.
	D3D11_TEXTURE2D_DESC texDesc;
	Texture2D->GetDesc( &texDesc );

	// ShaderResourceViewの情報を作成する.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof( srvDesc ) );
	srvDesc.Format						= texDesc.Format;
	srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip	= 0;
	srvDesc.Texture2D.MipLevels			= texDesc.MipLevels;

	if ( FAILED( m_pDevice->CreateShaderResourceView( Texture2D, &srvDesc, resource ) ) ) {
		SAFE_RELEASE( Texture2D );
		return E_FAIL;
	}

	SAFE_RELEASE( Texture2D );
	return S_OK;
}

//-----------------------------------.
// フォントを利用可能にする.
//-----------------------------------.
int CFontCreate::FontAvailable()
{
	return AddFontResourceEx( 
		m_wFilePath.c_str(),	// フォントリソース名.
		FR_PRIVATE,				// プロセス終了時にインストールしたフォントを削除.
		nullptr );				// フォント構造体.
}

//-----------------------------------.
// Textur2D_Descを作成.
//-----------------------------------.
D3D11_TEXTURE2D_DESC CFontCreate::CreateDesc( UINT width, UINT height )
{
	D3D11_TEXTURE2D_DESC desc;
	memset( &desc, 0, sizeof( desc ) );
	desc.Width				= width;
	desc.Height				= height;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBA(255,255,255,255)タイプ
	desc.SampleDesc.Count	= 1;
	desc.Usage				= D3D11_USAGE_DYNAMIC;			// 動的（書き込みするための必須条件）
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;	// シェーダリソースとして使う
	desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;		// CPUからアクセスして書き込みOK

	return desc;
}