#include "FontCreate.h"
#ifdef ENABLE_FONT
#include "..\..\DirectX\DirectX11.h"
#include "..\..\..\Utility\StringConversion\StringConversion.h"
#include <vector>
#include <cmath>
#ifndef _DEBUG
#include <encrypt/file.h>
#endif

namespace {
	const int	FONT_BMP_SIZE	= 256;		// フォントのラスタライズサイズ.
	const int	SDF_PAD			= 32;		// SDFセル周囲の余白(px).
	const float	SDF_SPREAD		= 32.0f;	// SDFの距離レンジ(px).
	const int	SEDT_INF		= 1 << 14;	// 距離変換の初期値.

	// 8SSEDT用の最近傍点までのベクトル.
	struct SSedtPoint
	{
		int dx, dy;
		int DistSq() const { return dx * dx + dy * dy; }
	};

	// 近傍の画素と比較して近い方を採用する.
	inline void SedtCompare( const std::vector<SSedtPoint>& Grid, SSedtPoint& Point, const int x, const int y, const int ox, const int oy, const int w, const int h )
	{
		const int nx = x + ox;
		const int ny = y + oy;
		SSedtPoint Other = { SEDT_INF, SEDT_INF };
		if ( nx >= 0 && ny >= 0 && nx < w && ny < h ) Other = Grid[ny * w + nx];
		Other.dx += ox;
		Other.dy += oy;
		if ( Other.DistSq() < Point.DistSq() ) Point = Other;
	}

	// 8SSEDT(2パスの距離変換).
	void SedtGenerate( std::vector<SSedtPoint>& Grid, const int w, const int h )
	{
		// 1パス目(左上から右下).
		for ( int y = 0; y < h; ++y ) {
			for ( int x = 0; x < w; ++x ) {
				SSedtPoint Point = Grid[y * w + x];
				SedtCompare( Grid, Point, x, y, -1,  0, w, h );
				SedtCompare( Grid, Point, x, y,  0, -1, w, h );
				SedtCompare( Grid, Point, x, y, -1, -1, w, h );
				SedtCompare( Grid, Point, x, y,  1, -1, w, h );
				Grid[y * w + x] = Point;
			}
			for ( int x = w - 1; x >= 0; --x ) {
				SSedtPoint Point = Grid[y * w + x];
				SedtCompare( Grid, Point, x, y, 1, 0, w, h );
				Grid[y * w + x] = Point;
			}
		}
		// 2パス目(右下から左上).
		for ( int y = h - 1; y >= 0; --y ) {
			for ( int x = w - 1; x >= 0; --x ) {
				SSedtPoint Point = Grid[y * w + x];
				SedtCompare( Grid, Point, x, y,  1,  0, w, h );
				SedtCompare( Grid, Point, x, y,  0,  1, w, h );
				SedtCompare( Grid, Point, x, y, -1,  1, w, h );
				SedtCompare( Grid, Point, x, y,  1,  1, w, h );
				Grid[y * w + x] = Point;
			}
			for ( int x = 0; x < w; ++x ) {
				SSedtPoint Point = Grid[y * w + x];
				SedtCompare( Grid, Point, x, y, -1, 0, w, h );
				Grid[y * w + x] = Point;
			}
		}
	}
}

CFontCreate::CFontCreate( const std::string& FilePath, const std::string& FileName )
	: m_pDevice		( nullptr )
	, m_pContext	( nullptr )
	, m_wFilePath	()
	, m_wFileName	()
#ifndef _DEBUG
	, m_hFontMem	( nullptr )
	, m_pFontMemBuf	( nullptr )
#endif
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
#ifdef _DEBUG
	RemoveFontResourceEx( m_wFilePath.c_str(), FR_PRIVATE, nullptr );
#else
	// メモリに読み込んだフォントを破棄する.
	if ( m_hFontMem != nullptr ) {
		RemoveFontMemResourceEx( m_hFontMem );
		m_hFontMem = nullptr;
	}
	// メモリ復号済みバッファを解放する.
	if ( m_pFontMemBuf != nullptr ) {
		delete[] static_cast<char*>( m_pFontMemBuf );
		m_pFontMemBuf = nullptr;
	}
#endif
}

//-----------------------------------.
// フォントSDF画像の作成.
//-----------------------------------.
HRESULT CFontCreate::CreateFontTexture2D( const char* c, SFontGlyph* pGlyph )
{
	if ( m_pDevice	== nullptr ) return E_FAIL;
	if ( m_pContext	== nullptr ) return E_FAIL;
	if ( pGlyph		== nullptr ) return E_FAIL;

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
	GLYPHMETRICS	GM = {};
	CONST MAT2		Mat		= { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	DWORD			Size	= GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, 0, nullptr, &Mat );
	BYTE*			Ptr		= nullptr;
	if ( Size != GDI_ERROR && Size > 0 ) {
		Ptr = new BYTE[Size];
		GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, Size, Ptr, &Mat );
	}

	// デバイスコンテキストとフォントハンドルの開放.
	SelectObject( Hdc, OldFont );
	DeleteObject( hFont );
	ReleaseDC( nullptr, Hdc );

	//-------------------------------------------------.
	// 文字のカバレッジ(不透明度)をセルに書き込む.
	//	セルの周囲にはアウトラインやグロー用の余白を設ける.
	//-------------------------------------------------.
	const int CellW	= GM.gmCellIncX > 0 ? static_cast<int>( GM.gmCellIncX ) : static_cast<int>( TM.tmAveCharWidth );
	const int CellH	= static_cast<int>( TM.tmHeight );
	const int GridW	= CellW + SDF_PAD * 2;
	const int GridH	= CellH + SDF_PAD * 2;

	std::vector<BYTE> Coverage( GridW * GridH, 0 );
	if ( Ptr != nullptr ) {
		// 書き出し位置( 左上 )とフォントビットマップの幅高.
		//	α値はGGO_GRAY4_BITMAPのため17段階.
		const int iOfs_x	= GM.gmptGlyphOrigin.x;
		const int iOfs_y	= TM.tmAscent - GM.gmptGlyphOrigin.y;
		const int iBmp_w	= GM.gmBlackBoxX + ( 4 - ( GM.gmBlackBoxX % 4 ) ) % 4;
		const int iBmp_h	= GM.gmBlackBoxY;
		const int Level		= 17;
		for ( int y = 0; y < iBmp_h; ++y ) {
			for ( int x = 0; x < iBmp_w; ++x ) {
				const int cx = SDF_PAD + iOfs_x + x;
				const int cy = SDF_PAD + iOfs_y + y;
				if ( cx < 0 || cy < 0 || cx >= GridW || cy >= GridH ) continue;
				const int Alpha = ( 255 * Ptr[x + iBmp_w * y] ) / ( Level - 1 );
				Coverage[cy * GridW + cx] = static_cast<BYTE>( Alpha > 255 ? 255 : Alpha );
			}
		}
		delete[] Ptr;
	}

	//-------------------------------------------------.
	// カバレッジからSDF(符号付き距離場)を生成する.
	//-------------------------------------------------.
	// 内側/外側それぞれの距離変換用グリッドを作成する.
	std::vector<SSedtPoint> GridIn ( GridW * GridH );
	std::vector<SSedtPoint> GridOut( GridW * GridH );
	for ( int i = 0; i < GridW * GridH; ++i ) {
		const bool IsInside = ( Coverage[i] >= 128 );
		GridIn[i]	= IsInside ? SSedtPoint{ 0, 0 } : SSedtPoint{ SEDT_INF, SEDT_INF };
		GridOut[i]	= IsInside ? SSedtPoint{ SEDT_INF, SEDT_INF } : SSedtPoint{ 0, 0 };
	}
	SedtGenerate( GridIn,  GridW, GridH );
	SedtGenerate( GridOut, GridW, GridH );

	// 符号付き距離を0～255にエンコードする(0.5が輪郭).
	std::vector<BYTE> SDF( GridW * GridH, 0 );
	for ( int i = 0; i < GridW * GridH; ++i ) {
		float Dist = 0.0f;
		if ( Coverage[i] > 0 && Coverage[i] < 255 ) {
			// 輪郭付近はカバレッジから距離を近似してサブピクセル精度を出す.
			Dist = Coverage[i] / 255.0f - 0.5f;
		}
		else {
			// 内側で正、外側で負の距離.
			const float DistIn	= std::sqrt( static_cast<float>( GridIn[i].DistSq() ) );
			const float DistOut	= std::sqrt( static_cast<float>( GridOut[i].DistSq() ) );
			Dist = DistOut - DistIn;
		}
		float Encode = 0.5f + Dist / ( SDF_SPREAD * 2.0f );
		if ( Encode < 0.0f ) Encode = 0.0f;
		if ( Encode > 1.0f ) Encode = 1.0f;
		SDF[i] = static_cast<BYTE>( Encode * 255.0f + 0.5f );
	}

	//-------------------------------------------------.
	// 書き込み可能テクスチャ作成.
	//	CPUで書き込みができるテクスチャを作成.
	//-------------------------------------------------.
	D3D11_TEXTURE2D_DESC	Desc		= CreateDesc( GridW, GridH );
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

	// SDFの書き込み(1画素1バイト).
	BYTE* pBits = (BYTE*)hMappedResource.pData;
	for ( int y = 0; y < GridH; ++y ) {
		memcpy( pBits + hMappedResource.RowPitch * y, SDF.data() + GridW * y, GridW );
	}
	m_pContext->Unmap( Texture2D, 0 );

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

	if ( FAILED( m_pDevice->CreateShaderResourceView( Texture2D, &srvDesc, &pGlyph->pTexture ) ) ) {
		SAFE_RELEASE( Texture2D );
		return E_FAIL;
	}
	SAFE_RELEASE( Texture2D );

	// グリフ情報を保存する.
	pGlyph->CellW	= static_cast<float>( CellW );
	pGlyph->CellH	= static_cast<float>( CellH );
	pGlyph->Pad		= static_cast<float>( SDF_PAD );
	pGlyph->Spread	= SDF_SPREAD;
	return S_OK;
}

//-----------------------------------.
// フォントを利用可能にする.
//-----------------------------------.
int CFontCreate::FontAvailable()
{
#ifdef _DEBUG
	return AddFontResourceEx(
		m_wFilePath.c_str(),	// フォントリソース名.
		FR_PRIVATE,				// プロセス終了時にインストールしたフォントを削除.
		nullptr );				// フォント構造体.
#else
	// リリース時は暗号化されたフォントファイルを復号し、メモリから読み込む.
	std::string s = StringConversion::to_String( m_wFilePath );
	std::string es = encrypt::GetEncryptionFilePath( s );
	auto rf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	if ( rf.first == nullptr ) return 0;

	// 破棄までバッファを保持する必要があるためバッファを保持する.
	m_pFontMemBuf = rf.first;

	DWORD numFonts = 0;
	m_hFontMem = AddFontMemResourceEx(
		m_pFontMemBuf,	// メモリ復号済みフォントデータ.
		rf.second,		// フォントデータのサイズ.
		nullptr,		// 予約(NULL指定).
		&numFonts );	// (out)読み込まれたフォント数.
	return static_cast<int>( numFonts );
#endif
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
	desc.Format				= DXGI_FORMAT_R8_UNORM;			// SDF用(1画素1バイト)
	desc.SampleDesc.Count	= 1;
	desc.Usage				= D3D11_USAGE_DYNAMIC;			// 動的（書き込みするための必須条件）
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;	// シェーダリソースとして使う
	desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;		// CPUからアクセスして書き込みOK

	return desc;
}
#endif // ENABLE_FONT
