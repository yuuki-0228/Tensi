#include "Font.h"
#include "..\DirectX\DirectX11.h"
#include "..\Sprite\Sprite.h"
#include "..\..\Resource\FontResource\FontResource.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Font.hlsl" );

	// フォントデータファイルパス.
	constexpr char FONT_DATA_FILE_PATH[] = "Data\\Sprite\\Font\\FontData.json";
}

CFont::CFont()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pConstantBuffer			( nullptr )
	, m_Vertices				()
	, m_FontState				()
	, m_FontRenderState			()
	, m_pSampleLinears			()
	, m_FileName				( "" )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
{
}

CFont::~CFont()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader	);
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//---------------------------------.
// 初期化.
//---------------------------------.
HRESULT CFont::Init( const std::string& FilePath, const std::string& FileName )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_FileName	= FileName;

	// スプライト情報の取得.
	if ( FAILED( FontStateDataLoad( FilePath ) ) ) return E_FAIL;

	// 初期化.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;
	return S_OK;
}


//---------------------------------.
// UIで描画.
//---------------------------------.
void CFont::RenderUI( SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderUI( RenderState->Text, RenderState );
}
void CFont::RenderUI( const std::string& Text, SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	// 非表示の場合は処理を行わない.
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	// 文字の大きさ取得.
	const float			TextSizeW = m_FontState.Disp.w * RenderState->Transform.GetWorldScale().x;
	const float			TextSizeH = m_FontState.Disp.h * RenderState->Transform.GetWorldScale().x;

	// 文字数の取得.
	const std::wstring	wText	= StringConversion::to_wString( Text.substr( 0, Text.find( "\n" ) ) );
	const int			TextNum = static_cast<int>( wText.size() );

	// テキストの表示形式に対応した開始位置の変更.
	const D3DXPOSITION3 Pos		= RenderState->Transform.Position;
	bool				IsMove	= false;
	if ( RenderState->TextAlign == ETextAlign::Right ) {
		RenderState->Transform.Position.x -= TextSizeW * TextNum;
		IsMove = true;
	}
	else if ( RenderState->TextAlign == ETextAlign::Center ) {
		RenderState->Transform.Position.x -= TextSizeW * ( TextNum / 2.0f );
		IsMove = true;
	}

	// 中心座標によて位置を調整する.
	if ( IsMove ){
		if ( m_FontState.LocalPosNo == ELocalPosition::Up ||
			 m_FontState.LocalPosNo == ELocalPosition::Center ||
			 m_FontState.LocalPosNo == ELocalPosition::Up )
		{
			RenderState->Transform.Position.x += TextSizeW / 2.0f;
		}
		if ( m_FontState.LocalPosNo == ELocalPosition::RightUp ||
			 m_FontState.LocalPosNo == ELocalPosition::Right ||
			 m_FontState.LocalPosNo == ELocalPosition::RightDown )
		{
			RenderState->Transform.Position.x += TextSizeW;
		}
	}

	// 文字数分ループ.
	for ( int i = 0; i < static_cast<int>( Text.length() ); ++i ){
		// 表示する文字の取得.
		std::string f = Text.substr( i, 1 );
		if( IsDBCSLeadByte( Text[i] ) == TRUE ){
			f = Text.substr( i++, 2 );	// 全角文字.
		} else {
			f = Text.substr( i, 1 );	// 半角文字.
		}

		// 改行.
		if ( f == "\n" ) {
			// 文字数の取得.
			const int			NextNo	= i + 1;
			const std::wstring	wText	= StringConversion::to_wString( Text.substr( NextNo, Text.find( "\n", NextNo ) - NextNo ) );
			const int			TextNum = static_cast<int>( wText.size() );

			// テキストの表示形式に対応した開始位置の変更.
			RenderState->Transform.Position.x = Pos.x;
			if ( RenderState->TextAlign == ETextAlign::Right ) {
				RenderState->Transform.Position.x -= TextSizeW * TextNum;
			}
			else if ( RenderState->TextAlign == ETextAlign::Center ) {
				RenderState->Transform.Position.x -= TextSizeW * static_cast< float >( TextNum ) / 2.0f;
			}
			RenderState->Transform.Position.y += TextSizeH + RenderState->TextInterval.h;
			continue;
		}

		// アウトラインの描画( 4方向にずらしたフォントを描画する ).
		if ( RenderState->IsOutLine || RenderState->IsBold ) {
			for ( int i = 0; i < 4; ++i ) {
				SFontRenderState OutLineState = *RenderState;

				// アウトラインの場合アウトラインの色を反映させる.
				if ( OutLineState.IsOutLine ) OutLineState.Color = OutLineState.OutLineColor;

				// アウトラインの描画.
				OutLineState.Transform.Position.x += -( OutLineState.OutLineSize / 2 ) + OutLineState.OutLineSize * ( i / 2 );
				OutLineState.Transform.Position.y += -( OutLineState.OutLineSize / 2 ) + OutLineState.OutLineSize * ( i % 2 );
				RenderFontUI( f.c_str(), &OutLineState );
			}
		}

		// 表示する文字の描画.
		RenderFontUI( f.c_str(), RenderState );
		RenderState->Transform.Position.x += TextSizeW + +RenderState->TextInterval.w;
	}
	RenderState->Transform.Position = Pos;
}

//---------------------------------.
// 3Dで描画.
//---------------------------------.
void CFont::Render3D( SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	Render3D( RenderState->Text, RenderState );
}
void CFont::Render3D( const std::string& Text, SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	// 非表示の場合は処理を行わない.
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	// 文字数分ループ.
	const D3DXPOSITION3 Pos = RenderState->Transform.Position;
	for( int i = 0; i < static_cast<int>( Text.length() ); ++i ){
		// 表示する文字の取得.
		std::string f = Text.substr( i, 1 );
		if( IsDBCSLeadByte( Text[i] ) == TRUE ){
			f = Text.substr( i++, 2 );	// 全角文字.
		} else {
			f = Text.substr( i, 1 );	// 半角文字.
		}

		// 改行.
		if ( f == "\n" ) {
			RenderState->Transform.Position.x = Pos.x;
			RenderState->Transform.Position.y += m_FontState.Disp.h * RenderState->Transform.Scale.y + 12.0f;
			continue;
		}

		// アウトラインの描画.
		if ( RenderState->IsOutLine || RenderState->IsBold ) {
			const D3DXPOSITION3& FontPos = RenderState->Transform.Position;

			// 4方向にずらしたフォントを描画する.
			for ( int i = 0; i < 4; ++i ) {
				RenderState->Transform.Position.x += -(RenderState->OutLineSize / 2) + RenderState->OutLineSize * (i / 2);
				RenderState->Transform.Position.y += -(RenderState->OutLineSize / 2) + RenderState->OutLineSize * (i % 2);
				RenderFontUI( f.c_str(), RenderState );
			}

			// フォントの位置を元に戻す.
			RenderState->Transform.Position = FontPos;
		}

		// 表示する文字の描画.
		RenderFont3D( f.c_str(), RenderState, IsBillBoard );
		RenderState->Transform.Position.x += m_FontState.Disp.w * RenderState->Transform.Scale.x;
	}
	RenderState->Transform.Position = Pos;
}

//----------------------------.
// フォント情報の読み込み.
//----------------------------.
HRESULT CFont::FontStateDataLoad( const std::string& FilePath )
{
	// 同じ名前のテキストの読み込み.
	std::string TextPath = FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

#ifndef _DEBUG
	// 暗号化されているため元のパスに戻す
	auto dp = TextPath.rfind( "." );
	auto fp = TextPath.rfind( "\\" ) + 1;
	if ( dp != std::string::npos && fp != std::string::npos ) {
		std::string n = TextPath.substr( fp, dp - fp );
		n.erase( 0, 1 );
		n.erase( n.length() - 2 );
		TextPath.erase( fp, dp - fp );
		TextPath.insert( fp, n );
	}
#endif
	json j = FileManager::JsonLoad( TextPath );

	// 同じ名前のテキストが無かったため共有のファイルを読み込む.
	if ( j.empty() ) {
		// フォントデータファイルの読み込み.
		j = FileManager::JsonLoad( FONT_DATA_FILE_PATH );

		// ファイルが無いためファイルを作成する.
		if ( j.empty() ) return E_FAIL;
	}

	// スプライト情報の取得.
	m_FontState.LocalPosNo	= static_cast<ELocalPosition>( j["LocalPosition"] );
	m_FontState.Disp.w		= j["Disp"]["w"];
	m_FontState.Disp.h		= j["Disp"]["h"];
	return S_OK;
}

//----------------------------.
// シェーダ作成.
//----------------------------.
HRESULT CFont::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	auto result = DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// シェーダファイル名(HLSLファイル).
		nullptr,			// マクロ定義の配列へのポインタ(未使用).
		nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
		"VS_Main",			// シェーダエントリーポイント関数の名前.
		"vs_5_0",			// シェーダのモデルを指定する文字列(プロファイル).
		uCompileFlag,		// シェーダコンパイルフラグ.
		0,					// エフェクトコンパイルフラグ(未使用).
		nullptr,			// スレッド ポンプ インターフェイスへのポインタ(未使用).
		&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
		&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
		nullptr );			// 戻り値へのポインタ(未使用).
	if ( FAILED( result ) )
	{
		ErrorMessage( "hlsl読み込み失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「バーテックスシェーダ」を作成.
	result = DirectX11::MutexDX11CreateVertexShader(
		pCompiledShader,
		nullptr,
		&m_pVertexShader );	// (out)バーテックスシェーダ.
	if ( FAILED( result ) )
	{
		ErrorMessage( "バーテックスシェーダ作成失敗",result );
		return E_FAIL;
	}

	// 頂点インプットレイアウトを定義.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						// 位置.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	// DXGIのフォーマット(32bit float型*3).
			0,
			0,								// データの開始位置.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD",						// テクスチャ位置.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		// DXGIのフォーマット(32bit float型*2).
			0,
			12,								// データの開始位置.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	// 頂点インプットレイアウトの配列要素数を算出.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// 頂点インプットレイアウトを作成.
	result = DirectX11::MutexDX11CreateInputLayout(
		layout,
		numElements,
		pCompiledShader,
		&m_pVertexLayout );	//(out)頂点インプットレイアウト.
	if (FAILED( result ) )
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSLからピクセルシェーダのブロブを作成.
	 DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,				// シェーダファイル名(HLSLファイル).
		nullptr,					// マクロ定義の配列へのポインタ(未使用).
		nullptr,					// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
		"PS_Main",					// シェーダエントリーポイント関数の名前.
		"ps_5_0",					// シェーダのモデルを指定する文字列(プロファイル).
		uCompileFlag,				// シェーダコンパイルフラグ.
		0,							// エフェクトコンパイルフラグ(未使用).
		nullptr,					// スレッド ポンプ インターフェイスへのポインタ(未使用).
		&pCompiledShader,			// ブロブを格納するメモリへのポインタ.
		&pErrors,					// エラーと警告一覧を格納するメモリへのポインタ.
		nullptr );					// 戻り値へのポインタ(未使用).
	if (FAILED( result ) )
	{
		ErrorMessage( "hlsl読み込み失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「ピクセルシェーダ」を作成.
	result = DirectX11::MutexDX11CreatePixelShader(
		pCompiledShader,
		nullptr,
		&m_pPixelShader );	// (out)ピクセルシェーダ.
	if (FAILED( result ) )
	{
		ErrorMessage( "ピクセルシェーダ作成失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// コンスタント(定数)バッファ作成.
	//	シェーダに特定の数値を送るバッファ.
	//	ここでは変換行列渡し用.
	//	シェーダに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;			// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof(SFontShaderConstantBuffer);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;				// 書き込みでアクセス.
	cb.MiscFlags			= 0;									// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;									// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;					// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	result = m_pDevice->CreateBuffer(
		&cb,
		nullptr,
		&m_pConstantBuffer );
	if (FAILED( result ) )
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// モデル作成(UI).
//----------------------------.
HRESULT CFont::CreateModelUI()
{
	float w = m_FontState.Disp.w;
	float h = m_FontState.Disp.h;

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, h, 1.0f, 1.0f );
	// 最大要素数を算出する.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// 使用方法(デフォルト).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// 頂点のサイズ.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// 頂点バッファとして扱う.
	bd.CPUAccessFlags		= 0;							// CPUからはアクセスしない.
	bd.MiscFlags			= 0;							// その他のフラグ(未使用).
	bd.StructureByteStride	= 0;							// 構造体のサイズ(未使用).

	// サブリソースデータ構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;	// 板ポリの頂点をセット.

	// 頂点バッファの作成.
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBufferUI );
	if ( FAILED( result ) )
	{
		ErrorMessage( "頂点バッファUI作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// モデル作成(3D).
//----------------------------.
HRESULT CFont::CreateModel3D()
{
	// 画像の比率を求める.
	int		as = myGcd( static_cast<int>( m_FontState.Disp.w ), static_cast<int>( m_FontState.Disp.h ) );
	float	w = ( m_FontState.Disp.w / as ) * m_FontState.Disp.w * 0.1f;
	float	h = ( m_FontState.Disp.h / as ) * m_FontState.Disp.h * 0.1f;

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, -h, 1.0f, 1.0f );
	// 最大要素数を算出する.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// 使用方法(デフォルト).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// 頂点のサイズ.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// 頂点バッファとして扱う.
	bd.CPUAccessFlags		= 0;							// CPUからはアクセスしない.
	bd.MiscFlags			= 0;							// その他のフラグ(未使用).
	bd.StructureByteStride	= 0;							// 構造体のサイズ(未使用).

	// サブリソース構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;

	// 頂点バッファの作成.
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer3D );
	if (FAILED( result ) )
	{
		ErrorMessage( "頂点バッファ3D作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// サンプラ作成.
//----------------------------.
HRESULT CFont::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;

	// サンプラ作成.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		auto result = m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] );
		if( FAILED( result ) ){
			ErrorMessage( "サンプラ作成 : 失敗" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// 頂点情報の作成.
//---------------------------.
HRESULT CFont::CreateVertex( const float w, const float h, const float u, const float v )
{
	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	switch ( m_FontState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Left:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::LeftDown:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Down:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightDown:
		m_Vertices[0] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Right:
		m_Vertices[0] = { D3DXPOSITION3(   -w,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightUp:
		m_Vertices[0] = { D3DXPOSITION3(   -w,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Up:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Center:
	default: 
		m_Vertices[0] = { D3DXVECTOR3( -w / 2,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXVECTOR3( -w / 2, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXVECTOR3(  w / 2,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXVECTOR3(  w / 2, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	}
	return S_OK;
}


//---------------------------------.
// テクスチャの描画.
//---------------------------------.
void CFont::RenderFontUI( const char* c, SFontRenderState* pRenderState )
{
	// テクスチャの取得.
	ID3D11ShaderResourceView* pTexture = FontResource::GetFontTexture( m_FileName, c );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = pRenderState->Transform.GetWorldMatrix();

	// 使用するシェーダの登録.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE	pData;
	SFontShaderConstantBuffer	cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド行列を渡す(ビュー,プロジェクションは使わない).
		D3DXMATRIX m = mWorld;
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = pRenderState->Color;

		// テクスチャ座標(UV座標).
		// 1マス当たりの割合にパターン番号(マス目)をかけて座標を設定する.
		cb.vUV.x = 0.0f;
		cb.vUV.y = 0.0f;

		// 描画するエリア.
		D3DXVECTOR4 RenderArea = pRenderState->RenderArea;
		const float WndScalseW = DirectX11::GetWndWidth()  / FWND_W;
		const float WndScalseH = DirectX11::GetWndHeight() / FWND_H;
		RenderArea.x	*= WndScalseW;
		RenderArea.z	*= WndScalseW;
		RenderArea.y	*= WndScalseH;
		RenderArea.w	*= WndScalseH;
		cb.vRenderArea	 = RenderArea;

		// ビューポートの幅、高さを渡す.
		cb.fViewPortWidth	= FWND_W;
		cb.fViewPortHeight	= FWND_H;

		// ディザ抜きを使用するか.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// このコンスタントバッファをどのシェーダで使うか？.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// 頂点バッファをセット.
	UINT stride = sizeof( SVertex );// データの間隔.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBufferUI, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( pRenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &pTexture );

	// アルファブレンド有効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//---------------------------------.
// テクスチャの描画.
//---------------------------------.
void CFont::RenderFont3D( const char* c, SFontRenderState* pRenderState, const bool& IsBillBoard  )
{
	// テクスチャの取得.
	ID3D11ShaderResourceView* pTexture = FontResource::GetFontTexture( m_FileName, c );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = pRenderState->Transform.GetWorldMatrix();

	// ビルボード用.
	if ( IsBillBoard == true ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// ビュー行列.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;			// xyzを0にする.
		// CancelRotationの逆行列を求めます.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// 使用するシェーダの登録.
	m_pContext->VSSetShader( m_pVertexShader,	nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader,	nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE	pData;
	SFontShaderConstantBuffer	cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド,ビュー,プロジェクション行列を渡す.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = pRenderState->Color;

		// テクスチャ座標(UV座標).
		cb.vUV.x = 0.0f;
		cb.vUV.y = 0.0f;

		// 描画するエリア.
		D3DXVECTOR4 RenderArea = pRenderState->RenderArea;
		const float WndScalseW = DirectX11::GetWndWidth()  / FWND_W;
		const float WndScalseH = DirectX11::GetWndHeight() / FWND_H;
		RenderArea.x	*= WndScalseW;
		RenderArea.z	*= WndScalseW;
		RenderArea.y	*= WndScalseH;
		RenderArea.w	*= WndScalseH;
		cb.vRenderArea	 = RenderArea;

		// ビューポートの幅、高さは使用しないため初期化.
		cb.fViewPortWidth	= 0.0f;
		cb.fViewPortHeight	= 0.0f;

		// ディザ抜きを使用するか.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// このコンスタントバッファをどのシェーダで使うか？.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// 頂点バッファをセット.
	UINT stride = sizeof( SVertex );// データの間隔.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBuffer3D, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( pRenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &pTexture );

	// アルファブレンド有効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}