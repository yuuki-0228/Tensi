#include "WindowTextRenderer.h"

namespace {
	constexpr float RECT_W			= 30.0f;
	constexpr float RECT_H			= 36.0f;
	constexpr float TEX_DIMENSION	= 384.0f;

	// シェーダーファイルパス.
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\WindowText.hlsl" );
}

WindowTextRenderer::WindowTextRenderer()
	: m_pVertexShader	( nullptr )
	, m_pVertexLayout	( nullptr )
	, m_pPixelShader	( nullptr )
	, m_pConstantBuffer	( nullptr )
	, m_pVertexBuffer	()
	, m_pSampleLinear	( nullptr )
	, m_pTexture		( nullptr )
	, m_Kerning			()
	, m_FontSize		( 1.0f )
{
}

WindowTextRenderer::~WindowTextRenderer()
{
	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pTexture );
	for ( int i = 100 - 1; i >= 0; i-- ) {
		SAFE_RELEASE( m_pVertexBuffer[i] );
	}
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pVertexShader );
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
WindowTextRenderer* WindowTextRenderer::GetInstance()
{
	static std::unique_ptr<WindowTextRenderer> pInstance = std::make_unique<WindowTextRenderer>();
	return pInstance.get();
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT WindowTextRenderer::Init()
{
	WindowTextRenderer* pI = GetInstance();

	// 100文字分繰り返し.
	for ( int i = 0; i < 100; i++ ) {
		pI->m_Kerning[i] = 30.0f;
	}

	if( FAILED( CreateShader()	) ) return E_FAIL;
	if( FAILED( CreateModel()	) ) return E_FAIL;
	if( FAILED( CreateSampler() ) ) return E_FAIL;
	if( FAILED( CreateTexture( _T( "Data\\Sprite\\WindowText\\ascii.png" ) ) ) ) return E_FAIL;

	Log::PushLog( "ウィンドウテキストの初期化 : 成功" );
	return S_OK;
}

//----------------------------.
// HLSLﾌｧｲﾙを読み込みｼｪｰﾀﾞを作成する.
//----------------------------.
HRESULT WindowTextRenderer::CreateShader()
{
	WindowTextRenderer*	pI				= GetInstance();
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダーのブロブを作成.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// シェーダーファイル名.
			nullptr,			// マクロ定義の配列へのポインタ(未使用).
			nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
			"VS_Main",			// シェーダーエントリーポイント関数の名前.
			"vs_5_0",			// シェーダーのモデルを指定する文字列(プロファイル).
			uCompileFlag,		// シェーダーコンパイルフラグ.
			0,					// エフェクトコンパイルフラグ(未使用).
			nullptr,			// スレッドポンプインターフェイスへのポインタ(未使用).
			&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
			&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
			nullptr)))			// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「バーテックスシェーダー」を作成.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&pI->m_pVertexShader)))	// (out)バーテックスシェーダー.
	{
		ErrorMessage( "バーテックスシェーダー作成失敗" );
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
	if (FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout,
			numElements,
			pCompiledShader,
			&pI->m_pVertexLayout)))	// (out)頂点インプットレイアウト.
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSLからピクセルシェーダーのﾌﾞﾛﾌﾞを作成.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// シェーダーファイル名.
			nullptr,			// マクロ定義の配列へのポインタ(未使用).
			nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
			"PS_Main",			// シェーダーエントリーポイント関数の名前.
			"ps_5_0",			// シェーダーのモデルを指定する文字列(プロファイル).
			uCompileFlag,		// シェーダーコンパイルフラグ.
			0,					// エフェクトコンパイルフラグ(未使用).
			nullptr,			// スレッドポンプインターフェイスへのポインタ(未使用).
			&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
			&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
			nullptr)))			// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「ピクセルシェーダー」を作成.
	if (FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr,
			&pI->m_pPixelShader)))	// (out)ピクセルシェーダー.
	{
		ErrorMessage( "ピクセルシェーダー作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// コンスタント(定数)バッファ作成.
	// シェーダーに特定の数値を送るバッファ.
	// ここでは変換行列渡し用.
	// シェーダーに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof(SHADER_CONSTANT_BUFFER);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// 書き込みでアクセス.
	cb.MiscFlags			= 0;								// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;								// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		pI->m_pDevice->CreateBuffer(
			&cb,
			nullptr,
			&pI->m_pConstantBuffer)))
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// モデル作成.
//----------------------------.
HRESULT WindowTextRenderer::CreateModel()
{
	WindowTextRenderer* pI = GetInstance();

	// フォント毎に矩形作成.
	float	left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
	int		count = 0;

	// 2重ループで1文字ずつ指定する.
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 10; y++) {

			// 一つ当たりの大きさ.
			left	= x * RECT_W;
			top		= y * RECT_H;
			right	= left + pI->m_Kerning[count];
			bottom	= top + RECT_H;

			// テクスチャの大きさから比率計算.
			left	/= TEX_DIMENSION;
			top		/= TEX_DIMENSION;
			right	/= TEX_DIMENSION;
			bottom	/= TEX_DIMENSION;

			// 板ポリ(四角形)の頂点を作成.
			VERTEX vertices[] =
			{
				// 頂点座標(x,y,z)					 UV座標(u,v)
				D3DXVECTOR3(   0.0f, RECT_H, 0.0f ), D3DXVECTOR2(  left, bottom ),	// 頂点１(左下).
				D3DXVECTOR3(   0.0f,   0.0f, 0.0f ), D3DXVECTOR2(  left,    top ),	// 頂点２(左上).
				D3DXVECTOR3( RECT_W, RECT_H, 0.0f ), D3DXVECTOR2( right, bottom ),	// 頂点３(右下).
				D3DXVECTOR3( RECT_W,   0.0f, 0.0f ), D3DXVECTOR2( right,    top )	// 頂点４(右上).
			};
			// 最大要素数を算出する.
			UINT uVerMax = sizeof(vertices) / sizeof(vertices[0]);

			// バッファ構造体.
			D3D11_BUFFER_DESC bd;
			bd.Usage				= D3D11_USAGE_DEFAULT;		// 使用方法(デフォルト).
			bd.ByteWidth			= sizeof(VERTEX) * uVerMax;	// 頂点のサイズ.
			bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;	// 頂点バッファとして扱う.
			bd.CPUAccessFlags		= 0;						// CPUからはアクセスしない.
			bd.MiscFlags			= 0;						// その他のフラグ(未使用).
			bd.StructureByteStride	= 0;						// 構造体のサイズ(未使用).

			// サブリソースデータ構造体.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = vertices;	// 板ポリの頂点をセット.

			// 頂点バッファの作成.
			if ( FAILED( pI->m_pDevice->CreateBuffer(
				&bd, &InitData, &pI->m_pVertexBuffer[count])))
			{
				ErrorMessage( "頂点バッファ作成失敗" );
				return E_FAIL;
			}

			count++;// 1文字作成したので次へ.
		}
	}

	return S_OK;
}

//----------------------------.
// テクスチャ作成.
//----------------------------.
HRESULT WindowTextRenderer::CreateTexture( LPCTSTR lpFileName )
{
	WindowTextRenderer* pI = GetInstance();

	// テクスチャ作成.
	if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
		lpFileName,			// ファイル名.
		nullptr, nullptr,
		&pI->m_pTexture,	// (out)テクスチャ.
		nullptr ) ) )
	{
		ErrorMessage( "テクスチャ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// サンプラ作成.
//----------------------------.
HRESULT WindowTextRenderer::CreateSampler()
{
	WindowTextRenderer* pI = GetInstance();

	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof( samDesc ) );
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// リニアフィルタ(線形補間).
						// POINT:高速だが粗い.
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		// ラッピング方法(WRAP:繰り返し).
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// MIRROR: 反転繰り返し.
	// CLAMP : 端の模様を引き伸ばす.
	// BORDER: 別途境界色を決める.
	// サンプラ作成.
	if( FAILED( pI->m_pDevice->CreateSamplerState(
		&samDesc, &pI->m_pSampleLinear ) ) ) //(out)サンプラ.
	{
		ErrorMessage( "サンプラ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// 描画.
//----------------------------.
void WindowTextRenderer::Render( const std::string& text, const D3DXPOSITION3& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer::Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
}
void WindowTextRenderer::Render( const std::string& text, const D3DXPOSITION2& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer::Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
}
void WindowTextRenderer::Render( const std::string& text, const int x, const int y, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer* pI = GetInstance();

	// フォントサイズの設定.
	pI->m_FontSize	= FontSize;

	// 色の設定.
	pI->m_Color = Color;

	// 使用するシェーダーの登録.
	pI->m_pContext->VSSetShader( pI->m_pVertexShader, nullptr, 0 );
	pI->m_pContext->PSSetShader( pI->m_pPixelShader, nullptr, 0 );

	// このコンスタントバッファをどのシェーダーで使うか？.
	pI->m_pContext->VSSetConstantBuffers( 0, 1, &pI->m_pConstantBuffer );
	pI->m_pContext->PSSetConstantBuffers( 0, 1, &pI->m_pConstantBuffer );

	// 頂点インプットレイアウトをセット.
	pI->m_pContext->IASetInputLayout( pI->m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	pI->m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャをシェーダーに渡す.
	pI->m_pContext->PSSetSamplers( 0, 1, &pI->m_pSampleLinear );
	pI->m_pContext->PSSetShaderResources( 0, 1, &pI->m_pTexture );

	// 文字数分ループ.
	int				RenderX = x;
	int				RenderY = y;
	std::wstring	wString	= StringConversion::to_wString( text );
	LPCTSTR			wText	= wString.c_str();
	for ( int i = 0; i < lstrlen( wText ); i++ ) {
		TCHAR	font	= text[i];
		int		index	= font - 32;	// フォントインデックス作成.

		// 改行.
		if ( font == '\n' ) {
			RenderX  = x;
			RenderY += static_cast<int>( RECT_H * pI->m_FontSize );
			continue;
		}

		// フォントレンダリング.
		RenderFont( index, RenderX, RenderY );

		// 文字を描画する位置を動かす.
		RenderX += static_cast<int>( pI->m_Kerning[index] * pI->m_FontSize );
	}
}
void WindowTextRenderer::DebugRender( const std::string& text, const D3DXPOSITION3& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
#endif	// #ifdef _DEBUG.
}
void WindowTextRenderer::DebugRender( const std::string& text, const D3DXPOSITION2& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
#endif	// #ifdef _DEBUG.
}
void WindowTextRenderer::DebugRender( const std::string& text, const int x, const int y, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, x, y, FontSize, Color );
#endif	// #ifdef _DEBUG.
}

//----------------------------.
// フォントレンダリング
//----------------------------.
void WindowTextRenderer::RenderFont(
	int FontIndex, int x, int y )
{
	if ( FontIndex == 0 ) return;

	WindowTextRenderer* pI = GetInstance();

	// ワールド行列.
	D3DXMATRIX	mWorld;
	D3DXMATRIX	mTrans, mScale;

	// 拡大縮小行列.
	D3DXMatrixScaling( &mScale, pI->m_FontSize, pI->m_FontSize, pI->m_FontSize );

	// 平行移動行列.
	D3DXMatrixTranslation( &mTrans,
		static_cast<float>(x), static_cast<float>(y), 0.0f );

	// ワールド座標変換.
	//	重要: 拡縮行列 * 平行行列.
	mWorld = mScale * mTrans;

	// シェーダーのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if (SUCCEEDED(
		pI->m_pContext->Map( pI->m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		// ワールド行列を渡す(ビュー,プロジェクションは使わない).
		D3DXMATRIX m	= mWorld;
		D3DXMatrixTranspose(&m, &m);//行列を転置する.
		cb.mWVP			= m;

		// カラー.
		cb.vColor = pI->m_Color;

		// ビューポートの幅、高さを渡す.
		cb.fViewPortWidth	= FWND_W;
		cb.fViewPortHeight	= FWND_H;

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		pI->m_pContext->Unmap( pI->m_pConstantBuffer, 0 );
	}

	// 頂点バファをセット.
	UINT stride = sizeof(VERTEX);// データの間隔.
	UINT offset = 0;
	pI->m_pContext->IASetVertexBuffers( 0, 1,
		&pI->m_pVertexBuffer[FontIndex], &stride, &offset );

	// アルファブレンド有効にする.
	DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	pI->m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	DirectX11::SetAlphaBlend( false );
}
