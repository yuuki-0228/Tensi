#include "DirectX11.h"
#include "DCompHelper.h"
#include "..\..\Utility\Log\Log.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include <mutex>

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
	std::mutex D10CFFmtx;	// D3DX10CompileFromFile関数用mutex.
	std::mutex D11CFFmtx;	// D3DX11CompileFromFile関数用mutex.
	std::mutex D11CSFmtx;	// D3DX11CreateShaderResourceViewFromFile関数用mutex.
	std::mutex D9LMFmtx;	// D3DXLoadMeshFromX関数用mutex.
	std::mutex D11CVSmtx;	// CreateVertexShader関数用mutex.
	std::mutex D11CPSmtx;	// CreatePixelShader関数用mutex.
	std::mutex D11CIPmtx;	// CreateInputShader関数用mutex.
	std::mutex D9LMHmtx;	// D3DXLoadMeshHierarchyFromX関数用mutex.
}

DirectX11::DirectX11()
	: m_hWnd					()
	, m_pDevice11				( nullptr )	
	, m_pContext11				( nullptr )
	, m_pSwapChain				()
	, m_pBackBuffer_TexRTV		()
	, m_pBackBuffer_DSTex		()
	, m_pBackBuffer_DSTexDSV	()
	, m_pBackBuffer_DSTexSRV	()
	, m_pDCompDevice			( nullptr )
	, m_pDCompTarget			( nullptr )
	, m_pDCompVisual			( nullptr )
	, m_pSceneTex				()
	, m_pSceneRTV				()
	, m_pCursorPixelTex			( nullptr )
	, m_CursorPixelX			( -1 )
	, m_CursorPixelY			( -1 )
	, m_pDepthStencilStateOn	( nullptr )
	, m_pDepthStencilStateOff	( nullptr )
	, m_pAlphaBlendOn			( nullptr )
	, m_pAlphaBlendOff			( nullptr )
	, m_pAlphaToCoverageOn		( nullptr )
	, m_pCullNone				( nullptr )
	, m_pCullBack				( nullptr )
	, m_pCullFront				( nullptr )
	, m_pWireFrame				( nullptr )
	, m_pCurrentBlendState		( nullptr )
	, m_pCurrentDepthState		( nullptr )
	, m_pCurrentRasterState		( nullptr )
	, m_WndWidth				( 0 )
	, m_WndHeight				( 0 )
	, m_MsaaSampleCount			( 1 )
	, m_BackColor				( Color4::White )
	, m_InitBackColor			( Color4::White )
	, m_IsWindowActive			( false )
	, m_IsDepth					( true )
	, m_IsAlphaBlend			( false )
	, m_IsAlphaToCoverage		( false )
	, m_IsDispMouseCursor		( true )
	, m_WindowNum				( 0 )
{
}

DirectX11::~DirectX11()
{
	Release();
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
DirectX11* DirectX11::GetInstance()
{
	static std::unique_ptr<DirectX11> pInstance = std::make_unique<DirectX11>();
	return pInstance.get();
}

//---------------------------.
// DirectX構築関数.
//---------------------------.
HRESULT DirectX11::Create( std::vector<HWND> hWnd )
{
	DirectX11* pI = GetInstance();

	RECT rc;
	GetWindowRect( hWnd[0], &rc );

	pI->m_hWnd		= hWnd;
	pI->m_WndWidth	= static_cast<UINT>( rc.right - rc.left );
	pI->m_WndHeight = static_cast<UINT>( rc.bottom - rc.top );
	pI->m_WindowNum = static_cast<int>( hWnd.size() );
	pI->m_pSwapChain.resize( pI->m_WindowNum );
	pI->m_pBackBuffer_TexRTV.resize( pI->m_WindowNum );
	pI->m_pBackBuffer_DSTex.resize( pI->m_WindowNum );
	pI->m_pBackBuffer_DSTexDSV.resize( pI->m_WindowNum );
	pI->m_pBackBuffer_DSTexSRV.resize( pI->m_WindowNum );
	pI->m_pSceneTex.resize( pI->m_WindowNum );
	pI->m_pSceneRTV.resize( pI->m_WindowNum );

	if ( FAILED( pI->CreateDeviceAndSwapChain()			) ) return E_FAIL;
	if ( FAILED( pI->CreateColorBackBufferRTV()			) ) return E_FAIL;
	if ( FAILED( pI->CreateSceneRenderTargets()			) ) return E_FAIL;
	if ( FAILED( pI->CreateDepthStencilBackBufferRTV()	) ) return E_FAIL;
	if ( FAILED( pI->CreateDepthStencilState()			) ) return E_FAIL;
	if ( FAILED( pI->CreateAlphaBlendState()			) ) return E_FAIL;
	if ( FAILED( pI->CreateViewports()					) ) return E_FAIL;
	if ( FAILED( pI->CreateRasterizer()					) ) return E_FAIL;

	Log::PushLogInfo( "DirectX11 デバイス作成 : 成功" );

	return S_OK;
}

//---------------------------.
// 解放処理.
//	解放するときは、作った順と逆の順で開放する.
//---------------------------.
void DirectX11::Release()
{
	DirectX11* pI = GetInstance();

	SAFE_RELEASE( pI->m_pAlphaToCoverageOn		);
	SAFE_RELEASE( pI->m_pAlphaBlendOff			);
	SAFE_RELEASE( pI->m_pAlphaBlendOn			);

	SAFE_RELEASE( pI->m_pDepthStencilStateOff	);
	SAFE_RELEASE( pI->m_pDepthStencilStateOn	);

	for ( int i = pI->m_WindowNum - 1; i >= 0; --i ) {
		SAFE_RELEASE( pI->m_pSceneRTV[i]			);
		SAFE_RELEASE( pI->m_pSceneTex[i]			);
		SAFE_RELEASE( pI->m_pBackBuffer_DSTexSRV[i] );
		SAFE_RELEASE( pI->m_pBackBuffer_DSTexDSV[i] );
		SAFE_RELEASE( pI->m_pBackBuffer_DSTex[i]	);
		SAFE_RELEASE( pI->m_pBackBuffer_TexRTV[i]	);
		SAFE_RELEASE( pI->m_pSwapChain[i]			);
	}
	SAFE_RELEASE( pI->m_pCursorPixelTex			);
	SAFE_RELEASE( pI->m_pDCompVisual			);
	SAFE_RELEASE( pI->m_pDCompTarget			);
	SAFE_RELEASE( pI->m_pDCompDevice			);	SAFE_RELEASE( pI->m_pContext11				);
	SAFE_RELEASE( pI->m_pDevice11				);
}

//---------------------------.
// バックバッファクリア関数.
//	この関数を呼び出した後にレンダリングをすること.
//---------------------------.
void DirectX11::ClearBackBuffer( int No )
{
	DirectX11* pI = GetInstance();

	// MSAA 有効時はオフスクリーンの MSAA ターゲットへ描画する( Present 時にバックバッファへ解決する ).
	ID3D11RenderTargetView* pRTV =
		pI->m_MsaaSampleCount > 1 ? pI->m_pSceneRTV[No] : pI->m_pBackBuffer_TexRTV[No];

	// カラーバックバッファ.
	if ( No == 0 ) {
		pI->m_BackColor.w = 0.0f;
		pI->m_pContext11->ClearRenderTargetView( pRTV, pI->m_BackColor );
	}
	else {
		// サブウィンドウはプリマルチプライドアルファ合成のため完全透明(0,0,0,0)でクリアする.
		// ( 描画した部分以外は透過して、デスクトップの壁紙がそのまま表示される ).
		constexpr float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		pI->m_pContext11->ClearRenderTargetView( pRTV, ClearColor );
	}

	// レンダーターゲットビューとデプスステンシルビューをパイプラインにセット.
	pI->m_pContext11->OMSetRenderTargets(
		1,
		&pRTV,
		pI->m_pBackBuffer_DSTexDSV[No] );

	// デプスステンシルバックバッファ.
	if ( pI->m_pBackBuffer_DSTexDSV[No] != nullptr ) {
		pI->m_pContext11->ClearDepthStencilView(
			pI->m_pBackBuffer_DSTexDSV[No],
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0 );
	}
}

//---------------------------.
// 表示.
//---------------------------.
void DirectX11::Present( int No )
{
	DirectX11* pI = GetInstance();

	// MSAA 有効時はシーンターゲットをバックバッファへ解決してから表示する.
	//	( サブウィンドウはフリップモデルのため、現在のバックバッファを毎フレーム取得する ).
	if ( pI->m_MsaaSampleCount > 1 || No == 0 ) {
		ID3D11Texture2D* pBackBuffer = nullptr;
		if ( SUCCEEDED( pI->m_pSwapChain[No]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) ) {
			if ( pI->m_MsaaSampleCount > 1 ) {
				pI->m_pContext11->ResolveSubresource(
					pBackBuffer, 0, pI->m_pSceneTex[No], 0, DXGI_FORMAT_B8G8R8A8_UNORM );
			}

			// クリック透過判定用のカーソル位置の色コピー( メインウィンドウのみ ).
			if ( No == 0 ) pI->DoCopyCursorPixel( pBackBuffer );

			SAFE_RELEASE( pBackBuffer );
		}
	}

	pI->m_pSwapChain[No]->Present( 0, 0 );
}

//---------------------------.
// MSAA用オフスクリーンシーンテクスチャの取得( MSAA無効時は nullptr ).
//---------------------------.
ID3D11Texture2D* DirectX11::GetSceneTex( const int No )
{
	DirectX11* pI = GetInstance();
	if ( No < 0 || No >= static_cast<int>( pI->m_pSceneTex.size() ) ) return nullptr;
	return pI->m_pSceneTex[No];
}

//---------------------------.
// MSAA用オフスクリーンシーンRTVの取得( MSAA無効時は nullptr ).
//---------------------------.
ID3D11RenderTargetView* DirectX11::GetSceneRTV( const int No )
{
	DirectX11* pI = GetInstance();
	if ( No < 0 || No >= static_cast<int>( pI->m_pSceneRTV.size() ) ) return nullptr;
	return pI->m_pSceneRTV[No];
}

//---------------------------.
// バックバッファRTVの取得.
//---------------------------.
ID3D11RenderTargetView* DirectX11::GetBackBufferRTV( const int No )
{
	DirectX11* pI = GetInstance();
	if ( No < 0 || No >= static_cast<int>( pI->m_pBackBuffer_TexRTV.size() ) ) return nullptr;
	return pI->m_pBackBuffer_TexRTV[No];
}

//---------------------------.
// バックバッファテクスチャの取得.
//	スワップチェーンが参照を保持しているため、呼び出し側での解放は不要.
//---------------------------.
ID3D11Texture2D* DirectX11::GetBackBufferTex( const int No )
{
	DirectX11* pI = GetInstance();
	if ( No < 0 || No >= static_cast<int>( pI->m_pSwapChain.size() ) ) return nullptr;

	ID3D11Texture2D* pBackBuffer = nullptr;
	if ( FAILED( pI->m_pSwapChain[No]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&pBackBuffer ) ) ) {
		return nullptr;
	}
	// GetBuffer で増えた参照はここで返しておく( スワップチェーンが保持し続ける ).
	pBackBuffer->Release();
	return pBackBuffer;
}

//---------------------------.
// 深度バッファのシェーダリソースビューの取得( 未対応環境では nullptr ).
//---------------------------.
ID3D11ShaderResourceView* DirectX11::GetDepthSRV( const int No )
{
	DirectX11* pI = GetInstance();
	if ( No < 0 || No >= static_cast<int>( pI->m_pBackBuffer_DSTexSRV.size() ) ) return nullptr;
	return pI->m_pBackBuffer_DSTexSRV[No];
}

//---------------------------.
// デバイスとスワップチェーンの作成.
//---------------------------.
HRESULT DirectX11::CreateDeviceAndSwapChain()
{
	DirectX11* pI = GetInstance();

	// ウィンドウの設定の取得.
	Json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	const float FPS = WndSetting["FPS"].Get( 60.0f );

	for ( int i = 0; i < pI->m_WindowNum; ++i ) {
		// スワップチェーン構造体.
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );									// 0で初期化.
		sd.BufferCount			= 1;										// バックバッファの数.
		sd.BufferDesc.Width		= pI->m_WndWidth;							// バックバッファの幅.
		sd.BufferDesc.Height	= pI->m_WndHeight;							// バックバッファの高さ.
		sd.BufferDesc.Format	= DXGI_FORMAT_B8G8R8A8_UNORM;				// フォーマット(32ビットカラー).
		sd.BufferDesc.RefreshRate.Numerator		= static_cast<UINT>( FPS );	// リフレッシュレート(分母) ※FPS:60.
		sd.BufferDesc.RefreshRate.Denominator	= 1;						// リフレッシュレート(分子).
		sd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;			// 使い方(表示先).
		sd.OutputWindow			= pI->m_hWnd[i];							// ウィンドウハンドル.
		sd.SampleDesc.Count		= 1;										// マルチサンプルの数.
		sd.SampleDesc.Quality	= 0;										// マルチサンプルのクオリティ.
		sd.Windowed				= TRUE;										// ウィンドウモード(フルスクリーン時はFALSE).

		// サブウィンドウ( 壁紙レイヤー )は DirectComposition で合成する.
		// Win11 24H2以降、デスクトップ(WorkerW)配下のウィンドウは
		// 通常の HWND スワップチェーンの Present では DWM に合成されず表示されない.
		// プリマルチプライドアルファの合成用スワップチェーンを使うことで表示され、
		// さらに描画した部分以外が透過して壁紙がそのまま見える.
		// ※メインウィンドウのデバイスを使い回すため i == 0 の作成が終わった後に作成すること.
		if ( i != 0 ) {
			if ( FAILED( CreateCompositionSwapChain( i ) ) ) return E_FAIL;
			continue;
		}

		// 作成を試みる機能レベルの優先を指定.
		//	(GPUがサポートする機能セットの定義).
		//	D3D_FEATURE_LEVEL列挙型の配列.
		//	D3D_FEATURE_LEVEL_11_0:Direct3D 11.0 の GPUレベル.
		D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
		D3D_FEATURE_LEVEL* pFeatureLevel = nullptr;//配列の要素数.
	
		// デバイスとスワップチェーンの作成.
		//	ハードウェア(GPU)デバイスでの作成.
		if( FAILED(
			D3D11CreateDeviceAndSwapChain(
				nullptr,					// ビデオアダプタへのポインタ.
				D3D_DRIVER_TYPE_HARDWARE,	// 作成するデバイスの種類.
				nullptr,					// ソフトウェア ラスタライザを実装するDLLのハンドル.
				0,							// 有効にするランタイムレイヤー.
				&pFeatureLevels,			// 作成を試みる機能レベルの順序を指定する配列へのポインタ.
				1,							// ↑の要素数.
				D3D11_SDK_VERSION,			// SDKのバージョン.
				&sd,						// スワップチェーンの初期化パラメータのポインタ.
				&m_pSwapChain[i],			// (out)レンダリングに使用するスワップチェーン.
				&m_pDevice11,				// (out)作成されたデバイス.
				pFeatureLevel,				// 機能レベルの配列にある最初の要素を表すポインタ.
				&m_pContext11 ) ) )			// (out)デバイス コンテキスト.
		{
			// WARPデバイスの作成.
			//	D3D_FEATURE_LEVEL_9_1～D3D_FEATURE_LEVEL_10_1.
			if( FAILED(
				D3D11CreateDeviceAndSwapChain(
					nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
					0, &pFeatureLevels, 1, D3D11_SDK_VERSION,
					&sd, &m_pSwapChain[i], &m_pDevice11,
					pFeatureLevel, &m_pContext11 ) ) )
			{
				// リファレンスデバイスの作成.
				//	DirectX SDKがインストールされていないと使えない.
				auto result = D3D11CreateDeviceAndSwapChain(
					nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr,
					0, &pFeatureLevels, 1, D3D11_SDK_VERSION,
					&sd, &m_pSwapChain[i], &m_pDevice11,
					pFeatureLevel, &m_pContext11 );
				if( FAILED( result ) )
				{
					PushError( "デバイスとスワップチェーン 作成 : 失敗", result );
					return E_FAIL;
				}
			}
		}
	
		// フルスクリーンや一部の機能を無効化する.
		if ( WndSetting["IsFullScreenLock"].Get( false ) ) {
			// ALT + Enterでフルスクリーンを無効化する.
			IDXGIFactory* pFactory = nullptr;
			// 上で作ったIDXGISwapChainを使う.
			m_pSwapChain[i]->GetParent( __uuidof( IDXGIFactory ), (void**) &pFactory );
			// 余計な機能を無効にする設定をする.
			if ( pFactory != nullptr ) pFactory->MakeWindowAssociation( m_hWnd[i], DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER );
			SAFE_RELEASE( pFactory );
		}
	}

	// MSAAのサンプル数を決定する( 非対応の場合は段階的に下げる ).
	UINT MsaaRequest = static_cast<UINT>( WndSetting["MSAA"].Get( 4 ) );
	if ( MsaaRequest > 8 ) MsaaRequest = 8;
	m_MsaaSampleCount = 1;
	for ( UINT Count = MsaaRequest; Count > 1; Count /= 2 ) {
		UINT Quality = 0;
		if ( SUCCEEDED( m_pDevice11->CheckMultisampleQualityLevels(
				DXGI_FORMAT_B8G8R8A8_UNORM, Count, &Quality ) ) && Quality > 0 ) {
			m_MsaaSampleCount = Count;
			break;
		}
	}
	Log::PushLogInfo( "MSAA サンプル数 : " + std::to_string( m_MsaaSampleCount ) );

	// 背景の色の設定.
	m_InitBackColor = Color4::RGBA(
		WndSetting["BackColor"]["R"].Get( 1.0f ),
		WndSetting["BackColor"]["G"].Get( 1.0f ),
		WndSetting["BackColor"]["B"].Get( 1.0f )
	);
	m_BackColor = m_InitBackColor;
	return S_OK;
}

//---------------------------.
// クリック透過判定用: カーソル位置を記録する.
//	メインウィンドウ( swapchain 0 )の Present 前に呼ぶこと.
//	実際のコピーは Present 内で MSAA の解決後に行う.
//---------------------------.
void DirectX11::CopyCursorPixel( const int x, const int y )
{
	DirectX11* pI = GetInstance();

	pI->m_CursorPixelX = x;
	pI->m_CursorPixelY = y;
}

//---------------------------.
// クリック透過判定用: 記録しておいたカーソル位置の 1x1 をコピーする.
//	Present 内で解決済みのバックバッファに対して呼ぶ.
//	( DISCARD モデルは Present 後のバックバッファの内容が不定になるため Present 前に行う ).
//---------------------------.
void DirectX11::DoCopyCursorPixel( ID3D11Texture2D* pBackBuffer )
{
	// 画面外の場合は前回の値を保持する.
	if ( m_CursorPixelX < 0 || m_CursorPixelY < 0 ||
		 m_CursorPixelX >= static_cast<int>( m_WndWidth ) || m_CursorPixelY >= static_cast<int>( m_WndHeight ) ) return;

	// 初回にステージングテクスチャを作成する.
	if ( m_pCursorPixelTex == nullptr ) {
		D3D11_TEXTURE2D_DESC Desc = {};
		Desc.Width				= 1;
		Desc.Height				= 1;
		Desc.MipLevels			= 1;
		Desc.ArraySize			= 1;
		Desc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		Desc.SampleDesc.Count	= 1;
		Desc.Usage				= D3D11_USAGE_STAGING;
		Desc.CPUAccessFlags		= D3D11_CPU_ACCESS_READ;

		// 初期値は黒( 透過扱い ).
		const UINT32 Black = 0xFF000000;
		D3D11_SUBRESOURCE_DATA Init = { &Black, sizeof( UINT32 ), sizeof( UINT32 ) };
		if ( FAILED( m_pDevice11->CreateTexture2D( &Desc, &Init, &m_pCursorPixelTex ) ) ) return;
	}

	// バックバッファのカーソル位置 1x1 をコピーする.
	D3D11_BOX Box = {
		static_cast<UINT>( m_CursorPixelX ),		static_cast<UINT>( m_CursorPixelY ),		0,
		static_cast<UINT>( m_CursorPixelX ) + 1,	static_cast<UINT>( m_CursorPixelY ) + 1,	1 };
	m_pContext11->CopySubresourceRegion( m_pCursorPixelTex, 0, 0, 0, 0, pBackBuffer, 0, &Box );
}

//---------------------------.
// クリック透過判定用: コピーしたカーソル位置の色を取得( COLORREF ).
//	前フレームでコピー済みのため Map で GPU を待つことはほぼ無い.
//---------------------------.
COLORREF DirectX11::GetCursorPixel()
{
	DirectX11* pI = GetInstance();
	if ( pI->m_pCursorPixelTex == nullptr ) return RGB( 0, 0, 0 );

	D3D11_MAPPED_SUBRESOURCE Mapped = {};
	if ( FAILED( pI->m_pContext11->Map( pI->m_pCursorPixelTex, 0, D3D11_MAP_READ, 0, &Mapped ) ) ) return RGB( 0, 0, 0 );
	const UINT32 Bgra = *static_cast<const UINT32*>( Mapped.pData );
	pI->m_pContext11->Unmap( pI->m_pCursorPixelTex, 0 );

	// B8G8R8A8 から COLORREF( 0x00BBGGRR )へ変換.
	return RGB( ( Bgra >> 16 ) & 0xFF, ( Bgra >> 8 ) & 0xFF, Bgra & 0xFF );
}
//---------------------------.
// サブウィンドウ用: DirectComposition 合成スワップチェイン作成.
//	Win11 24H2以降、壁紙レイヤー( WorkerW )配下のウィンドウは通常の
//	HWND スワップチェーンでは DWM に合成されないため、
//	合成用スワップチェーン + DirectComposition でウィンドウに関連付ける.
//	プリマルチプライドアルファのため、完全透明(0,0,0,0)でクリアすると
//	描画した部分以外は壁紙がそのまま透けて見える.
//	( 実際の生成処理は DCompHelper.cpp に隔離している.
//	  dcomp.h が内部で include する d2d1.h が旧DirectX SDKの同名ヘッダーと
//	  衝突するため、Global.h を include するこのファイルに直接持ち込めない ).
//---------------------------.
HRESULT DirectX11::CreateCompositionSwapChain( const int No )
{
	// ウィンドウサイズの取得.
	RECT rc;
	GetClientRect( m_hWnd[No], &rc );

	IDXGISwapChain* pSwapChain = nullptr;
	HRESULT hr = CreateDCompositionSwapChainForHwnd(
		m_pDevice11, m_hWnd[No],
		static_cast<UINT>( rc.right - rc.left ), static_cast<UINT>( rc.bottom - rc.top ),
		&pSwapChain, &m_pDCompDevice, &m_pDCompTarget, &m_pDCompVisual );
	if ( FAILED( hr ) ) {
		PushError( "DirectComposition 合成スワップチェーン 作成 : 失敗", hr );
		return E_FAIL;
	}
	m_pSwapChain[No] = pSwapChain;
	return S_OK;
}
//---------------------------.
// ラスタライザステート設定.
//---------------------------.
void DirectX11::SetRasterizerState( const ERS_STATE& RsState )
{
	DirectX11* pI = GetInstance();

	ID3D11RasterizerState* pTmp = nullptr;
	switch ( RsState ) {
	case ERS_STATE::None:
		// 正背面描画.
		pTmp = pI->m_pCullNone;
		break;
	case ERS_STATE::Back:
		// 背面を描画しない.
		pTmp = pI->m_pCullBack;
		break;
	case ERS_STATE::Front:
		// 正面を描画しない.
		pTmp = pI->m_pCullFront;
		break;
	case ERS_STATE::Wire:
		// ワイヤーフレーム描画.
		pTmp = pI->m_pWireFrame;
		break;
	default:
		return;
	}

	// 既に同じステートがセットされている場合は何もしない( 冗長なステート切り替えの削減 ).
	if ( pTmp == pI->m_pCurrentRasterState ) return;

	pI->m_pContext11->RSSetState( pTmp );
	pI->m_pCurrentRasterState = pTmp;
}

//---------------------------.
// フルスクリーンの設定.
//---------------------------.
void DirectX11::SetFullScreen( const bool Flag )
{
	DirectX11* pI = GetInstance();

	for ( int i = 0; i < pI->m_WindowNum; ++i ) {
		// 現在のスクリーン情報を取得.
		BOOL IsFullScreen = FALSE;
		pI->m_pSwapChain[i]->GetFullscreenState( &IsFullScreen, nullptr );

		// フルスクリーンに変更するか.
		if ( Flag ) {
			// 現在フルスクリーンなら行わない.
			if ( IsFullScreen == TRUE ) return;

			// フルスクリーンに変更.
			pI->m_pSwapChain[i]->SetFullscreenState( TRUE, nullptr );

			// マウスを非表示にする.
			if ( pI->m_IsDispMouseCursor == false ) return;
			ShowCursor( FALSE );
		}
		// 現在ウィンドウ状態に変更するか.
		else {
			// 現在ウィンドウ状態なら行わない.
			if ( IsFullScreen == FALSE ) return;

			// ウィンドウに変更.
			pI->m_pSwapChain[i]->SetFullscreenState( FALSE, nullptr );

			// マウスを表示する.
			if ( pI->m_IsDispMouseCursor == false ) return;
			ShowCursor( TRUE );
		}
	}
}

//----------------------------.
// フルスクリーン状態か取得.
//----------------------------.
bool DirectX11::IsFullScreen()
{
	DirectX11* pI = GetInstance();

	if ( GetInstance()->m_pSwapChain[0] == nullptr ) return false;

	// 現在のスクリーン情報を取得.
	BOOL isState = FALSE;
	GetInstance()->m_pSwapChain[0]->GetFullscreenState( &isState, nullptr );
	return static_cast<bool>( isState );
}

//----------------------------.
// ウィンドウサイズが変更された時に呼ぶ.
//----------------------------.
void DirectX11::Resize()
{
	DirectX11* pI = GetInstance();

	if( pI->m_pContext11 == nullptr ) return;

	for ( int i = 0; i < pI->m_WindowNum; ++i ) {
		// 現在のスクリーン情報を取得.
		BOOL IsFullScreen = FALSE;
		pI->m_pSwapChain[i]->GetFullscreenState( &IsFullScreen, nullptr );

		// セットしてあるレンダーターゲットを外す.
		pI->m_pContext11->OMSetRenderTargets( 0, nullptr, nullptr );

		// 使用していたバックバッファを解放する.
		SAFE_RELEASE( pI->m_pBackBuffer_TexRTV[i]	);
		SAFE_RELEASE( pI->m_pBackBuffer_DSTex[i]	);
		SAFE_RELEASE( pI->m_pBackBuffer_DSTexDSV[i] );
		SAFE_RELEASE( pI->m_pBackBuffer_DSTexSRV[i] );

		// スワップチェーンをリサイズする.
		// width, height を指定しない場合、hWndを参照し、自動で計算してくれる.
		auto result = pI->m_pSwapChain[i]->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 );
		if( FAILED( result ) ) {
			PushError( "デプスステンシルビュー作成失敗", result );
			return;
		}

		// スワップチェーンのバッファの取得.
		ID3D11Texture2D* pBuufer = nullptr;
		result = pI->m_pSwapChain[i]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( void** ) &pBuufer );
		if( FAILED( result ) ) {
			PushError( "デプスステンシルビュー作成失敗", result );
			return;
		}

		// テクスチャ情報の取得.
		D3D11_TEXTURE2D_DESC texDesc = {0};
		pBuufer->GetDesc( &texDesc );
		pI->m_WndWidth	= texDesc.Width;
		pI->m_WndHeight	= texDesc.Height;
		SAFE_RELEASE( pBuufer );

		result = pI->CreateColorBackBufferRTV();
		if ( FAILED( result ) ) {
			PushError( "デプスステンシルビュー作成失敗", result );
			return;
		}
		// MSAA 用オフスクリーンターゲットも新しいサイズで作り直す.
		result = pI->CreateSceneRenderTargets();
		if ( FAILED( result ) ) {
			PushError( "MSAA レンダーターゲット作成失敗", result );
			return;
		}
		result = pI->CreateDepthStencilBackBufferRTV();
		if( FAILED( result ) ) {
			PushError( "デプスステンシルビュー作成失敗", result );
			return;
		}

		// レンダーターゲットの設定( MSAA 有効時はシーンターゲットをセットする ).
		ID3D11RenderTargetView* pRTV =
			pI->m_MsaaSampleCount > 1 ? pI->m_pSceneRTV[i] : pI->m_pBackBuffer_TexRTV[i];
		pI->m_pContext11->OMSetRenderTargets(
			1,
			&pRTV,
			pI->m_pBackBuffer_DSTexDSV[i] );
		// デプスステンシルバッファ.
		pI->m_pContext11->ClearDepthStencilView(
			pI->m_pBackBuffer_DSTexDSV[i],
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0 );

		// ビューポートの設定.
		D3D11_VIEWPORT vp;
		vp.Width	= (FLOAT)pI->m_WndWidth;
		vp.Height	= (FLOAT)pI->m_WndHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;

		pI->m_pContext11->RSSetViewports( 1, &vp );
	}
}

//----------------------------.
// アクティブウィンドウか確認.
//----------------------------.
void DirectX11::CheckActiveWindow()
{
	DirectX11* pI = GetInstance();

	// 自分のウィンドウハンドルとPCの最前面のウィンドウハンドルを比較.
	pI->m_IsWindowActive = pI->m_hWnd[0] == GetForegroundWindow();
}

//----------------------------.
// D3DX10CompileFromFileマクロを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexD3DX10CompileFromFile( const LPCTSTR filepath, const D3D_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,
	LPCSTR entryPoint, LPCSTR shaderModel, UINT compileFlag, UINT Flags2, ID3DX10ThreadPump* pPump,
	ID3D10Blob** ppOutShader, ID3D10Blob** ppOutErrorMsgs, HRESULT* pOutHResult )
{
	std::unique_lock<std::mutex> lock( D10CFFmtx );

	HRESULT hr;
#ifdef _DEBUG
	hr = D3DX10CompileFromFile(
		filepath,		// シェーダーファイルパス.
		pDefines,		// マクロ定義のポインター : オプション.
		pInclude,		// インクルードファイルを処理するためのポインタ : オプション.
		entryPoint,		// シェーダーのエントリーポイント関数名.
		shaderModel,	// シェーダーモデルの名前.
		compileFlag,	// シェーダーコンパイルフラグ.
		Flags2,			// エフェクトコンパイルフラグ (0推奨).
		pPump,			// スレッドポンプインターフェイスへのポインター.
		ppOutShader,	// コンパイルされたシェーダーのデータ (out).
		ppOutErrorMsgs,	// コンパイル中のエラー出力 (out).
		pOutHResult );	// 戻り値のポインタ.
#else
	std::string s	= StringConversion::to_String( filepath );
	std::string es	= encrypt::GetEncryptionFilePath( s );
	auto spos = s.rfind( "\\" ) + 1;
	auto epos = s.rfind( "." );
	std::string name = s.substr( spos, epos - spos );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	hr = D3DX10CompileFromMemory(
		wrf.first,		// メモリ内のシェーダへのポインタ
		wrf.second,		// メモリ内のシェーダのサイズ
		name.c_str(),	// シェーダ コードを含むファイルの名前
		pDefines,		// マクロ定義のポインター : オプション.
		pInclude,		// インクルードファイルを処理するためのポインタ : オプション.
		entryPoint,		// シェーダーのエントリーポイント関数名.
		shaderModel,	// シェーダーモデルの名前.
		compileFlag,	// シェーダーコンパイルフラグ.
		Flags2,			// エフェクトコンパイルフラグ (0推奨).
		pPump,			// スレッドポンプインターフェイスへのポインター.
		ppOutShader,	// コンパイルされたシェーダーのデータ (out).
		ppOutErrorMsgs,	// コンパイル中のエラー出力 (out).
		pOutHResult );	// 戻り値のポインタ.

#endif
	return hr;
}

//----------------------------.
// D3DX11CompileFromFileマクロを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexD3DX11CompileFromFile( const LPCTSTR filepath, const D3D_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,
	LPCSTR entryPoint, LPCSTR shaderModel, UINT compileFlag, UINT Flags2, ID3DX11ThreadPump* pPump,
	ID3D10Blob** ppOutShader, ID3D10Blob** ppOutErrorMsgs, HRESULT* pOutHResult )
{
	std::unique_lock<std::mutex> lock( D11CFFmtx );

	HRESULT hr;
#ifdef _DEBUG
	hr = D3DX11CompileFromFile(
		filepath,		// シェーダーファイルパス.
		pDefines,		// マクロ定義のポインター : オプション.
		pInclude,		// インクルードファイルを処理するためのポインタ : オプション.
		entryPoint,		// シェーダーのエントリーポイント関数名.
		shaderModel,	// シェーダーモデルの名前.
		compileFlag,	// シェーダーコンパイルフラグ.
		Flags2,			// エフェクトコンパイルフラグ (0推奨).
		pPump,			// スレッドポンプインターフェイスへのポインター.
		ppOutShader,	// コンパイルされたシェーダーのデータ (out).
		ppOutErrorMsgs,	// コンパイル中のエラー出力 (out).
		pOutHResult );	// 戻り値のポインタ.
#else
	std::string s	= StringConversion::to_String( filepath );
	std::string es	= encrypt::GetEncryptionFilePath( s );
	auto spos = s.rfind( "\\" ) + 1;
	auto epos = s.rfind( "." );
	std::string name = s.substr( spos, epos - spos );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	hr = D3DX11CompileFromMemory(
		wrf.first,		// メモリ内のシェーダへのポインタ
		wrf.second,		// メモリ内のシェーダのサイズ
		name.c_str(),	// シェーダ コードを含むファイルの名前
		pDefines,		// マクロ定義のポインター : オプション.
		pInclude,		// インクルードファイルを処理するためのポインタ : オプション.
		entryPoint,		// シェーダーのエントリーポイント関数名.
		shaderModel,	// シェーダーモデルの名前.
		compileFlag,	// シェーダーコンパイルフラグ.
		Flags2,			// エフェクトコンパイルフラグ (0推奨).
		pPump,			// スレッドポンプインターフェイスへのポインター.
		ppOutShader,	// コンパイルされたシェーダーのデータ (out).
		ppOutErrorMsgs,	// コンパイル中のエラー出力 (out).
		pOutHResult );	// 戻り値のポインタ.
#endif
	return hr;
}

//----------------------------.
// D3DX11CreateShaderResourceViewFromFileマクロを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
	const LPCTSTR pSrcFile, D3DX11_IMAGE_LOAD_INFO* pLoadInfo, ID3DX11ThreadPump* pPump,
	ID3D11ShaderResourceView** ppShaderResourceView, HRESULT* pHResult )
{
	std::unique_lock<std::mutex> lock( D11CSFmtx );

	HRESULT hr;
#ifdef _DEBUG
	hr = D3DX11CreateShaderResourceViewFromFile(
		GetInstance()->m_pDevice11,		// リソースを使用するデバイスのポインタ.
		pSrcFile,						// ファイル名.
		pLoadInfo,						// シェーダーリソースビューを含むファイルの名前.
		pPump,							// スレッドポンプインターフェイスへのポインター
		ppShaderResourceView,			// (out)テクスチャ.
		pHResult );
#else
	std::string s	= StringConversion::to_String( pSrcFile );
	std::string es	= encrypt::GetEncryptionFilePath( s );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	hr = D3DX11CreateShaderResourceViewFromMemory(
		GetInstance()->m_pDevice11,		// リソースを使用するデバイスのポインタ.
		wrf.first,						// メモリ内のファイルへのポインタ
		wrf.second,						// メモリ内のファイルのサイズ
		pLoadInfo,						// シェーダーリソースビューを含むファイルの名前.
		pPump,							// スレッドポンプインターフェイスへのポインター
		ppShaderResourceView,			// (out)テクスチャ.
		pHResult );
#endif
	return hr;
}

//----------------------------.
// D3DXLoadMeshFromXマクロを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexD3DXLoadMeshFromX(
	const LPCTSTR pFilename, DWORD Options, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXBUFFER* ppAdjacency, 
	LPD3DXBUFFER* ppMaterials, LPD3DXBUFFER* ppEffectInstances, DWORD* pNumMaterials, LPD3DXMESH* ppMesh )
{
	std::unique_lock<std::mutex> lock( D9LMFmtx );

	HRESULT hr;
#ifdef _DEBUG
	hr = D3DXLoadMeshFromX(
		pFilename,			// ファイル名.
		Options,			// メッシュの作成オプション.
		pD3DDevice,			// メッシュに関連付けられているデバイスオブジェクト.
		ppAdjacency,		// 隣接関係データを含むバッファーへのポインター.
		ppMaterials,		// (out)マテリアルデータを含むバッファーへのポインター.
		ppEffectInstances,	// (out)効果インスタンスの配列を含むバッファーへのポインター.
		pNumMaterials,		// (out)マテリアル数.
		ppMesh );			// (out)読み込まれたメッシュ.
#else
	std::string s	= StringConversion::to_String( pFilename );
	std::string es	= encrypt::GetEncryptionFilePath( s );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	hr = D3DXLoadMeshFromXInMemory(
		wrf.first,			// メモリ内のモデルへのポインタ
		wrf.second,			// メモリ内のモデルのサイズ
		Options,			// メッシュの作成オプション.
		pD3DDevice,			// メッシュに関連付けられているデバイスオブジェクト.
		ppAdjacency,		// 隣接関係データを含むバッファーへのポインター.
		ppMaterials,		// (out)マテリアルデータを含むバッファーへのポインター.
		ppEffectInstances,	// (out)効果インスタンスの配列を含むバッファーへのポインター.
		pNumMaterials,		// (out)マテリアル数.
		ppMesh );			// (out)読み込まれたメッシュ.
#endif
	return hr;
}

//----------------------------.
// D11CreateVertexShaderを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexDX11CreateVertexShader( ID3DBlob* pCompiledShader, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** pVertexShader )
{
	std::unique_lock<std::mutex> lock( D11CVSmtx );

	HRESULT hr;

	hr = GetInstance()->m_pDevice11->CreateVertexShader(
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		pClassLinkage,
		pVertexShader );
	return hr;
}

//----------------------------.
// D11CreatePixelShaderを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexDX11CreatePixelShader( ID3DBlob* pCompiledShader, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** pPixelShader )
{
	std::unique_lock<std::mutex> lock( D11CPSmtx );

	HRESULT hr;

	hr = GetInstance()->m_pDevice11->CreatePixelShader(
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		pClassLinkage,
		pPixelShader );
	return hr;
}

//----------------------------.
// D11CreateInputLayoutを呼び出す.
//----------------------------.
HRESULT DirectX11::MutexDX11CreateInputLayout( D3D11_INPUT_ELEMENT_DESC* pLayout, UINT numElements, ID3DBlob* pCompiledShader, ID3D11InputLayout** m_pVertexLayout )
{
	std::unique_lock<std::mutex> lock( D11CIPmtx );

	HRESULT hr;
	hr = GetInstance()->m_pDevice11->CreateInputLayout(
		pLayout,
		numElements,
		pCompiledShader->GetBufferPointer(),
		pCompiledShader->GetBufferSize(),
		m_pVertexLayout );
	return hr;
}

//----------------------------.
// D3DXLoadMeshHierarchyFromXを呼び出す.
//----------------------------.
HRESULT	DirectX11::MutexD3DXLoadMeshHierarchyFromX( LPCTSTR Filename, DWORD MeshOptions,
	LPDIRECT3DDEVICE9 pDevice, LPD3DXALLOCATEHIERARCHY pAlloc,
	LPD3DXLOADUSERDATA pUserDataLoader, LPD3DXFRAME* ppFrameHierarchy,
	LPD3DXANIMATIONCONTROLLER* ppAnimController )
{
	std::unique_lock<std::mutex> lock( D9LMHmtx );

	HRESULT hr;
#ifdef _DEBUG
	hr = D3DXLoadMeshHierarchyFromX(
		Filename,			// ファイル名.
		MeshOptions,		// メッシュの作成オプション.
		pDevice,			// メッシュに関連付けられているデバイスオブジェクト.
		pAlloc,				// インターフェイスへのポインター
		pUserDataLoader,	// アプリケーション提供のインターフェイス
		ppFrameHierarchy,	// (out)読み込まれたフレーム階層へのポインター
		ppAnimController );	// (out)アニメーションコントローラーへのポインター
#else
	std::string s	= StringConversion::to_String( Filename );
	std::string es	= encrypt::GetEncryptionFilePath( s );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	hr = D3DXLoadMeshHierarchyFromXInMemory(
		wrf.first,			// メモリ内のモデルへのポインタ
		wrf.second,			// メモリ内のモデルのサイズ
		MeshOptions,		// メッシュの作成オプション.
		pDevice,			// メッシュに関連付けられているデバイスオブジェクト.
		pAlloc,				// インターフェイスへのポインター
		pUserDataLoader,	// アプリケーション提供のインターフェイス
		ppFrameHierarchy,	// (out)読み込まれたフレーム階層へのポインター
		ppAnimController );	// (out)アニメーションコントローラーへのポインター
#endif
	return hr;
}

//----------------------------.
// ラスタライザ作成.
//----------------------------.
HRESULT DirectX11::CreateRasterizer()
{
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory( &rdc, sizeof( rdc ) );
	rdc.FillMode = D3D11_FILL_SOLID;//塗りつぶし(ソリッド).

	// カリングの設定.
	//	D3D11_CULL_BACK:背面を描画しない.
	//	D3D11_CULL_FRONT:正面を描画しない.
	//	D3D11_CULL_NONE:カリングを切る(正背面を描画する).
	rdc.CullMode = D3D11_CULL_NONE;

	auto result = m_pDevice11->CreateRasterizerState( &rdc, &m_pCullNone );
	if ( FAILED( result ) ) {
		PushError( "ラスタライザー 作成 : 失敗", result );
		return E_FAIL;
	}

	rdc.FillMode = D3D11_FILL_SOLID;// 塗りつぶし(ソリッド).
	rdc.CullMode = D3D11_CULL_BACK;	// BACK:背面を描画しない,
	result = m_pDevice11->CreateRasterizerState( &rdc, &m_pCullBack );
	if ( FAILED( result ) ) {
		PushError( "ラスタライザー 作成 : 失敗", result );
		return E_FAIL;
	}

	rdc.FillMode = D3D11_FILL_SOLID;// 塗りつぶし(ソリッド).
	rdc.CullMode = D3D11_CULL_FRONT;// FRONT:正面を描画しない.
	result = m_pDevice11->CreateRasterizerState( &rdc, &m_pCullFront );
	if ( FAILED( result ) ) {
		PushError( "ラスタライザー 作成 : 失敗", result );
		return E_FAIL;
	}

	rdc.FillMode = D3D11_FILL_WIREFRAME;// ワイヤーフレーム.
	rdc.CullMode = D3D11_CULL_NONE;		// BACK:背面を描画しない, FRONT:正面を描画しない.
	result = m_pDevice11->CreateRasterizerState( &rdc, &m_pWireFrame );
	if ( FAILED( result ) ) {
		PushError( "ラスタライザー 作成 : 失敗", result );
		return E_FAIL;
	}

	// ポリゴンの表裏を決定するフラグ.
	//	TRUE:左回りなら前向き。右回りなら後ろ向き。
	//	FALSE:逆になる.
	rdc.FrontCounterClockwise = FALSE;

	// 距離についてのクリッピング有効.
	rdc.DepthClipEnable = FALSE;

	m_pContext11->RSSetState( m_pCullNone );
	m_pCurrentRasterState = m_pCullNone;

	return S_OK;
}

//----------------------------.
// デプスステンシル設定.
//	この関数１つでON/OFFの2種類を作成する.
//----------------------------.
HRESULT DirectX11::CreateDepthStencilState()
{
	// 深度テスト(Zテスト)の設定.
	//	※on/offの共通部分のみ設定.
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc		= D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable	= FALSE;
	dsDesc.StencilReadMask	= D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask	= D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// 深度テスト(Zテスト)を有効にする.
	dsDesc.DepthEnable = TRUE;//有効.
	auto result = m_pDevice11->CreateDepthStencilState( &dsDesc, &m_pDepthStencilStateOn );
	// 深度設定作成.
	if ( FAILED( result ) )
	{
		PushError( "深度ON設定 作成 : 失敗", result );
		return E_FAIL;
	}

	// 深度テスト(Z)テストを無効にする.
	dsDesc.DepthEnable = FALSE;//無効.
	// 深度設定作成.
	result = m_pDevice11->CreateDepthStencilState( &dsDesc, &m_pDepthStencilStateOff );
	if( FAILED( result ) )
	{
		PushError( "深度OFF設定 作成 : 失敗", result );
		return E_FAIL;
	}

	return S_OK;
}


//----------------------------.
// ブレンドステート作成.
//	アルファブレンドのON/OFFの２種類を作成.
//----------------------------.
HRESULT DirectX11::CreateAlphaBlendState()
{
	// アルファブレンド用ブレンドステート構造体.
	//	pngファイル内にアルファ情報があるので、透過するようにブレンドステートで設定する.
	D3D11_BLEND_DESC BlendDesc;
	ZeroMemory( &BlendDesc, sizeof( BlendDesc ) );	// 初期化.

	BlendDesc.IndependentBlendEnable = false;		// false:RenderTarget[0]のメンバーのみ使用する.
													//	true:RenderTarget[0～7]が使用できる.
													//	(レンダーターゲット毎に独立したブレンド処理).
	// アルファトゥカバレージを使用する.
	BlendDesc.AlphaToCoverageEnable = false;
	// 元素材に対する設定.
	BlendDesc.RenderTarget[0].SrcBlend			= D3D11_BLEND_SRC_ALPHA;		// アルファブレンドを指定.
	// 重ねる素材に対する設定.
	BlendDesc.RenderTarget[0].DestBlend			= D3D11_BLEND_INV_SRC_ALPHA;	// アルファブレンドの反転を指定.
	// ブレンドオプション.
	BlendDesc.RenderTarget[0].BlendOp			= D3D11_BLEND_OP_ADD;			// ADD:加算合成.
	// 元素材のアルファに対する指定.
	BlendDesc.RenderTarget[0].SrcBlendAlpha		= D3D11_BLEND_ONE;				// そのまま使用.
	// 重ねる素材のアルファに対する設定.
	BlendDesc.RenderTarget[0].DestBlendAlpha	= D3D11_BLEND_ZERO;				// 何もしない。
	// アルファのブレンドオプション.
	BlendDesc.RenderTarget[0].BlendOpAlpha		= D3D11_BLEND_OP_ADD;			// ADD:加算合成.
	// ピクセル毎の書き込みマスク.
	BlendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;	// 全ての成分(RGBA)へのデータの格納を許可する.

	// アルファブレンドを使用する.
	BlendDesc.RenderTarget[0].BlendEnable = true;	// 有効.
	// ブレンドステート作成.
	auto result = m_pDevice11->CreateBlendState( &BlendDesc, &m_pAlphaBlendOn );
	if( FAILED( result ) )
	{
		PushError( "アルファブレンドステートON設定 作成 : 失敗", result );
		return E_FAIL;
	}

	// アルファトゥカバレージを使用する.
	BlendDesc.AlphaToCoverageEnable = true;	// 有効.
	// ブレンドステート作成.
	result = m_pDevice11->CreateBlendState( &BlendDesc, &m_pAlphaToCoverageOn );
	if ( FAILED( result ) )
	{
		PushError( "アルファトゥカバレージステートON設定 作成 : 失敗", result );
		return E_FAIL;
	}

	// アルファトゥカバレージを使用しない.
	BlendDesc.AlphaToCoverageEnable			= false;	//無効.
	// アルファブレンドを使用しない.
	BlendDesc.RenderTarget[0].BlendEnable	= false;	//無効.
	// ブレンドステート作成.
	result = m_pDevice11->CreateBlendState( &BlendDesc, &m_pAlphaBlendOff );
	if( FAILED( result ) )
	{
		PushError( "アルファブレンドステートOFF設定 作成 : 失敗", result );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// ビューポート作成.
//----------------------------.
HRESULT DirectX11::CreateViewports()
{
	D3D11_VIEWPORT vp;
	vp.Width	= (FLOAT) m_WndWidth;	// 幅.
	vp.Height	= (FLOAT) m_WndHeight;	// 高さ.
	vp.MinDepth = 0.0f;					// 最小深度(手前).
	vp.MaxDepth = 1.0f;					// 最大深度(奥).
	vp.TopLeftX = 0.0f;					// 左上位置x.
	vp.TopLeftY = 0.0f;					// 左上位置y.

	m_pContext11->RSSetViewports( 1, &vp );

	return S_OK;
}

//----------------------------.
// エラーを送る
//----------------------------.
void DirectX11::PushError( const std::string& t, const HRESULT& r )
{
	// DXGI_ERROR_DEVICE_REMOVEDの場合GetDeviceRemovedReasonの結果も記入
	auto msg = t;
	if( r == DXGI_ERROR_DEVICE_REMOVED ) {
		auto sr = GetInstance()->m_pDevice11->GetDeviceRemovedReason();
		std::ostringstream ss;
		ss << "\nGetDeviceRemovedReason : 0x" << std::hex << static_cast<int>( sr );
		msg += ss.str();
	}

	ErrorMessage( msg, r );
}

//----------------------------.
// 透過設定の切り替え.
//----------------------------.
void DirectX11::SetAlphaBlend( bool flag )
{
	DirectX11* pI = GetInstance();
	if ( pI->m_IsAlphaToCoverage ) return;

	UINT mask = 0xffffffff;	// マスク値.
	ID3D11BlendState* pTmp
		= ( flag == true ) ? pI->m_pAlphaBlendOn : pI->m_pAlphaBlendOff;
	pI->m_IsAlphaBlend = flag;

	// 既に同じステートがセットされている場合は何もしない( 冗長なステート切り替えの削減 ).
	if ( pTmp == pI->m_pCurrentBlendState ) return;

	// アルファブレンド設定をセット.
	pI->m_pContext11->OMSetBlendState( pTmp, nullptr, mask );
	pI->m_pCurrentBlendState = pTmp;
}

//----------------------------.
// アルファトゥカバレージを有効:無効に設定する.
//----------------------------.
void DirectX11::SetAlphaToCoverage( bool flag )
{
	DirectX11* pI = GetInstance();

	// ブレンドステートの設定.
	UINT mask = 0xffffffff;	// マスク値.
	ID3D11BlendState* blend
		= ( flag == true ) ? pI->m_pAlphaToCoverageOn : pI->m_pAlphaBlendOff;
	pI->m_IsAlphaToCoverage = flag;

	// 既に同じステートがセットされている場合は何もしない( 冗長なステート切り替えの削減 ).
	if ( blend == pI->m_pCurrentBlendState ) return;

	pI->m_pContext11->OMSetBlendState( blend, nullptr, mask );
	pI->m_pCurrentBlendState = blend;
}

//----------------------------.
// 深度(Z)テストON/OFF切り替え.
//----------------------------.
void DirectX11::SetDepth(bool flag)
{
	DirectX11* pI = GetInstance();

	ID3D11DepthStencilState* pTmp
		= (flag == true) ? pI->m_pDepthStencilStateOn : pI->m_pDepthStencilStateOff;
	pI->m_IsDepth = flag;

	// 既に同じステートがセットされている場合は何もしない( 冗長なステート切り替えの削減 ).
	if ( pTmp == pI->m_pCurrentDepthState ) return;

	// 深度設定をセット.
	pI->m_pContext11->OMSetDepthStencilState( pTmp, 1 );
	pI->m_pCurrentDepthState = pTmp;
}

//----------------------------.
// バックバッファ作成:カラー用レンダーターゲットビュー作成.
//----------------------------.
HRESULT DirectX11::CreateColorBackBufferRTV()
{
	for ( int i = 0; i < m_WindowNum; ++i ) {
		// 再作成に備えて解放しておく.
		SAFE_RELEASE( m_pBackBuffer_TexRTV[i] );

		// バックバッファテクスチャを取得(既にあるので作成ではない).
		ID3D11Texture2D* pBackBuffer_Tex = nullptr;
		auto result = m_pSwapChain[i]->GetBuffer(
			0,
			__uuidof( ID3D11Texture2D ),	// __uuidof:式に関連付けされたGUIDを取得
											//	Texture2Dの唯一の物として扱う
			(LPVOID*)&pBackBuffer_Tex ) ;	// (out)バックバッファテクスチャ.
		if( FAILED( result ) )
		{
			PushError( "スワップチェインからバックバッファ 取得 : 失敗", result );
			return E_FAIL;
		}

		// そのテクスチャに対してレンダーターゲットビュー(RTV)を作成.
		result = m_pDevice11->CreateRenderTargetView(
			pBackBuffer_Tex,
			nullptr,
			&m_pBackBuffer_TexRTV[i] );	// (out)RTV.
		if( FAILED( result ) )
		{
			// バックバッファテクスチャを解放.
			SAFE_RELEASE( pBackBuffer_Tex );
			PushError( "レンダータイゲットビュー 作成 : 失敗", result );
			return E_FAIL;
		}
		// バックバッファテクスチャを解放.
		SAFE_RELEASE( pBackBuffer_Tex );
	}
	return S_OK;
}

//----------------------------.
// MSAA 用オフスクリーンレンダーターゲット作成.
//	MSAA が有効な場合、描画は一旦こちらへ行い Present 時にバックバッファへ解決する.
//	作成に失敗した場合は MSAA を無効にして続行する.
//----------------------------.
HRESULT DirectX11::CreateSceneRenderTargets()
{
	// MSAA 無効時はバックバッファへ直接描画するため作成しない.
	if ( m_MsaaSampleCount <= 1 ) return S_OK;

	for ( int i = 0; i < m_WindowNum; ++i ) {
		// 再作成に備えて解放しておく.
		SAFE_RELEASE( m_pSceneRTV[i] );
		SAFE_RELEASE( m_pSceneTex[i] );

		// 実際に作成されたバックバッファのサイズに合わせる.
		UINT Width  = ( m_WndWidth  != 0 ) ? m_WndWidth  : 1;
		UINT Height = ( m_WndHeight != 0 ) ? m_WndHeight : 1;
		ID3D11Texture2D* pBackBuffer_Tex = nullptr;
		if ( SUCCEEDED( m_pSwapChain[i]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer_Tex ) ) ) {
			D3D11_TEXTURE2D_DESC BufferDesc = {};
			pBackBuffer_Tex->GetDesc( &BufferDesc );
			SAFE_RELEASE( pBackBuffer_Tex );
			if ( BufferDesc.Width  != 0 ) Width  = BufferDesc.Width;
			if ( BufferDesc.Height != 0 ) Height = BufferDesc.Height;
		}

		// MSAA カラーテクスチャの作成.
		D3D11_TEXTURE2D_DESC Desc = {};
		Desc.Width				= Width;
		Desc.Height				= Height;
		Desc.MipLevels			= 1;
		Desc.ArraySize			= 1;
		Desc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		Desc.SampleDesc.Count	= m_MsaaSampleCount;
		Desc.SampleDesc.Quality	= 0;
		Desc.Usage				= D3D11_USAGE_DEFAULT;
		Desc.BindFlags			= D3D11_BIND_RENDER_TARGET;

		HRESULT result = m_pDevice11->CreateTexture2D( &Desc, nullptr, &m_pSceneTex[i] );
		if ( SUCCEEDED( result ) ) {
			result = m_pDevice11->CreateRenderTargetView( m_pSceneTex[i], nullptr, &m_pSceneRTV[i] );
		}

		// 作成に失敗した場合は MSAA を無効にして従来のバックバッファ直接描画に戻す.
		if ( FAILED( result ) ) {
			std::ostringstream ss;
			ss	<< "MSAA レンダーターゲット作成失敗 ( window " << i << " ), MSAA を無効にします."
				<< " HRESULT : 0x" << std::hex << static_cast<unsigned long>( result );
			Log::PushLogInfo( ss.str() );

			m_MsaaSampleCount = 1;
			for ( int j = 0; j <= i; ++j ) {
				SAFE_RELEASE( m_pSceneRTV[j] );
				SAFE_RELEASE( m_pSceneTex[j] );
			}
			break;
		}
	}
	return S_OK;
}

//----------------------------.
// バックバッファ作成:デプスステンシル用レンダーターゲットビュー作成.
//----------------------------.
HRESULT DirectX11::CreateDepthStencilBackBufferRTV()
{
	DirectX11* pI = GetInstance();

	for ( int i = 0; i < m_WindowNum; ++i ) {
		// 再作成に備えて解放しておく.
		SAFE_RELEASE( m_pBackBuffer_DSTexSRV[i] );
		SAFE_RELEASE( m_pBackBuffer_DSTexDSV[i] );
		SAFE_RELEASE( m_pBackBuffer_DSTex[i] );

		// 各スワップチェーンの実際のバックバッファのサイズを一致させる。
		//	m_WndWidth / m_WndHeight は、実際に作成されたバッファのサイズと
		//	異なる場合がある（コンポジション・スワップチェーン、DPI、ドライバによる調整など）。
		//	サイズが無効または不一致の場合、一部のPCではエラーが発生する。
		UINT Width  = ( m_WndWidth  != 0 ) ? m_WndWidth  : 1;
		UINT Height = ( m_WndHeight != 0 ) ? m_WndHeight : 1;

		ID3D11Texture2D* pBackBuffer_Tex = nullptr;
		if ( SUCCEEDED( m_pSwapChain[i]->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer_Tex ) ) ) {
			D3D11_TEXTURE2D_DESC BufferDesc = {};
			pBackBuffer_Tex->GetDesc( &BufferDesc );
			SAFE_RELEASE( pBackBuffer_Tex );
			if ( BufferDesc.Width  != 0 ) Width  = BufferDesc.Width;
			if ( BufferDesc.Height != 0 ) Height = BufferDesc.Height;
		}

		D3D11_TEXTURE2D_DESC descDepth;
		descDepth.Width					= Width;						// Width.
		descDepth.Height				= Height;						// Height.
		descDepth.MipLevels				= 1;							// Mip levels.
		descDepth.ArraySize				= 1;							// Array size.
		descDepth.Format				= DXGI_FORMAT_D32_FLOAT;		// Set per attempt below.
		descDepth.SampleDesc.Count		= m_MsaaSampleCount;			// Multi sample count( カラーターゲットと一致させる ).
		descDepth.SampleDesc.Quality	= 0;							// Multi sample quality.
		descDepth.Usage					= D3D11_USAGE_DEFAULT;			// Usage.
		descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;		// Use as depth stencil.
		descDepth.CPUAccessFlags		= 0;							// No CPU access.
		descDepth.MiscFlags				= 0;							// No misc flags.

		// 深度をシェーダから参照できるように TYPELESS で作成する( 深度フォグ等のポストエフェクト用 ).
		//	{ テクスチャ, DSV, SRV } のフォーマットの組み合わせ.
		struct SDepthFormat {
			DXGI_FORMAT Tex;
			DXGI_FORMAT Dsv;
			DXGI_FORMAT Srv;
		};
		constexpr SDepthFormat DepthFormats[] = {
			{ DXGI_FORMAT_R32_TYPELESS,		DXGI_FORMAT_D32_FLOAT,			DXGI_FORMAT_R32_FLOAT				},
			{ DXGI_FORMAT_R24G8_TYPELESS,	DXGI_FORMAT_D24_UNORM_S8_UINT,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS	},
			{ DXGI_FORMAT_R16_TYPELESS,		DXGI_FORMAT_D16_UNORM,			DXGI_FORMAT_R16_UNORM				},
			// SRV 付きが全滅した場合の保険( 従来通りの SRV 無し ).
			{ DXGI_FORMAT_D32_FLOAT,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN					},
			{ DXGI_FORMAT_D24_UNORM_S8_UINT,DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN					},
			{ DXGI_FORMAT_D16_UNORM,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN					},
		};
		HRESULT result = E_FAIL;
		for ( const auto& Format : DepthFormats ) {
			const bool UseSrv = ( Format.Srv != DXGI_FORMAT_UNKNOWN );
			descDepth.Format	= Format.Tex;
			descDepth.BindFlags	= D3D11_BIND_DEPTH_STENCIL | ( UseSrv ? D3D11_BIND_SHADER_RESOURCE : 0 );
			result = m_pDevice11->CreateTexture2D( &descDepth, nullptr, &m_pBackBuffer_DSTex[i] );
			if ( FAILED( result ) ) {
				m_pBackBuffer_DSTex[i] = nullptr;
				continue;
			}

			// DSV の作成( TYPELESS の場合はフォーマットを明示する ).
			if ( UseSrv ) {
				D3D11_DEPTH_STENCIL_VIEW_DESC DsvDesc = {};
				DsvDesc.Format			= Format.Dsv;
				DsvDesc.ViewDimension	= m_MsaaSampleCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
				result = m_pDevice11->CreateDepthStencilView( m_pBackBuffer_DSTex[i], &DsvDesc, &m_pBackBuffer_DSTexDSV[i] );
			}
			else {
				result = m_pDevice11->CreateDepthStencilView( m_pBackBuffer_DSTex[i], nullptr, &m_pBackBuffer_DSTexDSV[i] );
			}
			if ( FAILED( result ) ) {
				m_pBackBuffer_DSTexDSV[i] = nullptr;
				SAFE_RELEASE( m_pBackBuffer_DSTex[i] );
				continue;
			}

			// SRV の作成( 失敗しても深度自体は使えるので続行する ).
			if ( UseSrv ) {
				D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
				SrvDesc.Format = Format.Srv;
				if ( m_MsaaSampleCount > 1 ) {
					SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
				}
				else {
					SrvDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
					SrvDesc.Texture2D.MipLevels	= 1;
				}
				if ( FAILED( m_pDevice11->CreateShaderResourceView( m_pBackBuffer_DSTex[i], &SrvDesc, &m_pBackBuffer_DSTexSRV[i] ) ) ) {
					m_pBackBuffer_DSTexSRV[i] = nullptr;
				}
			}
			break;
		}

		if ( m_pBackBuffer_DSTexDSV[i] == nullptr ) {
			std::ostringstream ss;
			ss	<< "DepthStencil create failed ( window " << i << " : "
				<< Width << " x " << Height << " ), continue without depth."
				<< " HRESULT : 0x" << std::hex << static_cast<unsigned long>( result );
			Log::PushLogInfo( ss.str() );
			SAFE_RELEASE( m_pBackBuffer_DSTex[i] );
		}

		// レンダリングターゲットビューと深度・ステンシルビューを設定
		//	( MSAA 有効時はサンプル数を一致させるためシーンターゲットをセットする ).
		ID3D11RenderTargetView* pRTV =
			m_MsaaSampleCount > 1 ? m_pSceneRTV[i] : m_pBackBuffer_TexRTV[i];
		pI->m_pContext11->OMSetRenderTargets(
			1,
			&pRTV,
			m_pBackBuffer_DSTexDSV[i] );
	}
	return S_OK;
}