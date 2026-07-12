// このファイルは Global.h ( 旧DirectX SDKヘッダーを含む ) を一切 include しないこと.
//	dcomp.h が内部で include する <d2d1.h> は、旧DirectX SDKの同名ヘッダーと
//	インクルードガードが衝突するため、混在すると解決に失敗してビルドが壊れる.
#include "DCompHelper.h"
#include <Windows.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dcomp.lib" )

HRESULT CreateDCompositionSwapChainForHwnd(
	IUnknown* pD3DDevice, HWND hWnd, UINT width, UINT height,
	IDXGISwapChain** ppSwapChain,
	IUnknown** ppDCompDevice, IUnknown** ppDCompTarget, IUnknown** ppDCompVisual )
{
	// DXGIデバイス経由でファクトリを取得する.
	IDXGIDevice* pDxgiDevice = nullptr;
	HRESULT hr = pD3DDevice->QueryInterface( __uuidof( IDXGIDevice ), (void**)&pDxgiDevice );
	if ( FAILED( hr ) ) return hr;

	IDXGIAdapter* pAdapter = nullptr;
	hr = pDxgiDevice->GetAdapter( &pAdapter );
	if ( FAILED( hr ) ) { pDxgiDevice->Release(); return hr; }

	IDXGIFactory2* pFactory = nullptr;
	hr = pAdapter->GetParent( __uuidof( IDXGIFactory2 ), (void**)&pFactory );
	pAdapter->Release();
	if ( FAILED( hr ) ) { pDxgiDevice->Release(); return hr; }

	// 合成用スワップチェーンの作成( プリマルチプライドアルファ ).
	DXGI_SWAP_CHAIN_DESC1 sd = {};
	sd.Width			= width;
	sd.Height			= height;
	sd.Format			= DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.SampleDesc.Count	= 1;
	sd.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount		= 2;
	sd.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.AlphaMode		= DXGI_ALPHA_MODE_PREMULTIPLIED;

	IDXGISwapChain1* pSwapChain1 = nullptr;
	hr = pFactory->CreateSwapChainForComposition( pD3DDevice, &sd, nullptr, &pSwapChain1 );
	pFactory->Release();
	if ( FAILED( hr ) ) { pDxgiDevice->Release(); return hr; }

	// DirectComposition でスワップチェーンをウィンドウに関連付ける.
	IDCompositionDevice* pDCompDevice = nullptr;
	hr = DCompositionCreateDevice( pDxgiDevice, __uuidof( IDCompositionDevice ), (void**)&pDCompDevice );
	pDxgiDevice->Release();
	if ( FAILED( hr ) ) { pSwapChain1->Release(); return hr; }

	IDCompositionTarget* pDCompTarget = nullptr;
	hr = pDCompDevice->CreateTargetForHwnd( hWnd, TRUE, &pDCompTarget );

	IDCompositionVisual* pDCompVisual = nullptr;
	if ( SUCCEEDED( hr ) ) hr = pDCompDevice->CreateVisual( &pDCompVisual );
	if ( SUCCEEDED( hr ) ) hr = pDCompVisual->SetContent( pSwapChain1 );
	if ( SUCCEEDED( hr ) ) hr = pDCompTarget->SetRoot( pDCompVisual );
	if ( SUCCEEDED( hr ) ) hr = pDCompDevice->Commit();

	if ( FAILED( hr ) ) {
		if ( pDCompVisual ) pDCompVisual->Release();
		if ( pDCompTarget ) pDCompTarget->Release();
		pDCompDevice->Release();
		pSwapChain1->Release();
		return hr;
	}

	*ppSwapChain	= pSwapChain1;
	*ppDCompDevice	= pDCompDevice;
	*ppDCompTarget	= pDCompTarget;
	*ppDCompVisual	= pDCompVisual;
	return S_OK;
}
