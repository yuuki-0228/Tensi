#include "RenderTexture.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\..\Common\DirectX\DirectX11.h"

namespace {
	// バックバッファと合わせるカラーフォーマット.
	constexpr DXGI_FORMAT COLOR_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;
	// 深度ステンシルフォーマット.
	constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
}

CRenderTexture::CRenderTexture()
	: m_pTex	( nullptr )
	, m_pRTV	( nullptr )
	, m_pSRV	( nullptr )
	, m_pDSTex	( nullptr )
	, m_pDSV	( nullptr )
	, m_Width	( 0 )
	, m_Height	( 0 )
{
}

CRenderTexture::~CRenderTexture()
{
	Release();
}

//----------------------------.
// 作成.
//----------------------------.
HRESULT CRenderTexture::Create( const UINT Width, const UINT Height, const bool UseDepth )
{
	Release();

	ID3D11Device* pDevice = DirectX11::GetDevice();
	if ( pDevice == nullptr ) return E_FAIL;

	// カラーテクスチャの作成.
	D3D11_TEXTURE2D_DESC Desc = {};
	Desc.Width				= Width;
	Desc.Height				= Height;
	Desc.MipLevels			= 1;
	Desc.ArraySize			= 1;
	Desc.Format				= COLOR_FORMAT;
	Desc.SampleDesc.Count	= 1;
	Desc.Usage				= D3D11_USAGE_DEFAULT;
	Desc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if ( FAILED( pDevice->CreateTexture2D( &Desc, nullptr, &m_pTex ) ) ) {
		Log::PushLogError( "RenderTexture カラーテクスチャ作成失敗" );
		return E_FAIL;
	}
	if ( FAILED( pDevice->CreateRenderTargetView( m_pTex, nullptr, &m_pRTV ) ) ) {
		Log::PushLogError( "RenderTexture RTV作成失敗" );
		Release();
		return E_FAIL;
	}
	if ( FAILED( pDevice->CreateShaderResourceView( m_pTex, nullptr, &m_pSRV ) ) ) {
		Log::PushLogError( "RenderTexture SRV作成失敗" );
		Release();
		return E_FAIL;
	}

	// 深度ステンシルの作成.
	if ( UseDepth ) {
		D3D11_TEXTURE2D_DESC DepthDesc = Desc;
		DepthDesc.Format	= DEPTH_FORMAT;
		DepthDesc.BindFlags	= D3D11_BIND_DEPTH_STENCIL;

		if ( FAILED( pDevice->CreateTexture2D( &DepthDesc, nullptr, &m_pDSTex ) ) ) {
			Log::PushLogError( "RenderTexture 深度テクスチャ作成失敗" );
			Release();
			return E_FAIL;
		}
		if ( FAILED( pDevice->CreateDepthStencilView( m_pDSTex, nullptr, &m_pDSV ) ) ) {
			Log::PushLogError( "RenderTexture DSV作成失敗" );
			Release();
			return E_FAIL;
		}
	}

	m_Width	 = Width;
	m_Height = Height;
	return S_OK;
}

//----------------------------.
// 解放.
//----------------------------.
void CRenderTexture::Release()
{
	SAFE_RELEASE( m_pDSV	);
	SAFE_RELEASE( m_pDSTex	);
	SAFE_RELEASE( m_pSRV	);
	SAFE_RELEASE( m_pRTV	);
	SAFE_RELEASE( m_pTex	);
	m_Width	 = 0;
	m_Height = 0;
}

//----------------------------.
// クリア.
//----------------------------.
void CRenderTexture::Clear( const D3DXCOLOR4& Color )
{
	ID3D11DeviceContext* pContext = DirectX11::GetContext();
	if ( pContext == nullptr || m_pRTV == nullptr ) return;

	const float ClearColor[4] = { Color.x, Color.y, Color.z, Color.w };
	pContext->ClearRenderTargetView( m_pRTV, ClearColor );
	if ( m_pDSV != nullptr ) {
		pContext->ClearDepthStencilView( m_pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	}
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
