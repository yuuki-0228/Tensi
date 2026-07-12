#pragma once
// DirectComposition 合成用ヘルパー.
//	dcomp.h は内部で <d2d1.h> を include するが、このプロジェクトの
//	AdditionalIncludeDirectories は旧DirectX SDK( $(DXSDK_DIR)Include )が
//	Windows SDK より優先されるため、<d2d1.h> は常に旧SDK側の互換性の無い
//	バージョンが解決されてビルドが壊れる.
//	そのため dcomp.h / d2d1.h を Global.h ( 旧DirectX SDKヘッダーを含む ) と
//	同じ翻訳単位に混在させないよう、このファイルだけを独立させている.
#include <Windows.h>
struct IUnknown;
struct IDXGISwapChain;

// サブウィンドウ用スワップチェーンを DirectComposition でウィンドウに合成する.
//	pD3DDevice   : ID3D11Device*( IUnknown* として渡す ).
//	hWnd         : 合成先ウィンドウハンドル.
//	width,height : バックバッファサイズ.
//	ppSwapChain  : (out)作成されたスワップチェーン( IDXGISwapChain として返す ).
//	ppDCompDevice/ppDCompTarget/ppDCompVisual : (out)解放用に保持しておくオブジェクト.
HRESULT CreateDCompositionSwapChainForHwnd(
	IUnknown* pD3DDevice, HWND hWnd, UINT width, UINT height,
	IDXGISwapChain** ppSwapChain,
	IUnknown** ppDCompDevice, IUnknown** ppDCompTarget, IUnknown** ppDCompVisual );
