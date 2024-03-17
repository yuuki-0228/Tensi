#pragma once
#include "..\..\Global.h"

// ラスタライザステート.
enum class enRS_STATE
{
	None,	// 正背面描画.
	Back,	// 背面を描画しない.
	Front,	// 正面を描画しない.
	Wire,	// ワイヤーフレーム描画.

	Max,
} typedef ERS_STATE;

/************************************************
*	DirectX11 セットアップ.
**/
class DirectX11 final
{
public:
	DirectX11();
	~DirectX11();

	// DirectX11構築.
	static HRESULT Create( std::vector<HWND> hWnd );
	// DirectX11解放.
	static void Release();

	// バックバッファクリア関数.
	static void ClearBackBuffer( int No );
	// 表示.
	static void Present( int No );

	// デバイスを取得.
	static ID3D11Device* GetDevice() { return GetInstance()->m_pDevice11; }
	// デバイスコンテキストを取得.
	static ID3D11DeviceContext* GetContext() { return GetInstance()->m_pContext11; }
	// デプスステンシルビューの取得.
	static ID3D11DepthStencilView* GetDepthSV() { return GetInstance()->m_pBackBuffer_DSTexDSV[0]; }
	// ウィンドウサイズの取得.
	static float GetWndWidth()	{ return static_cast<float>( GetInstance()->m_WndWidth  ); }
	static float GetWndHeight()	{ return static_cast<float>( GetInstance()->m_WndHeight ); }
	// 背景色の取得.
	static D3DXCOLOR4 GetBackColor() { return GetInstance()->m_BackColor; }
	// 深度(Z)テストが有効か取得.
	static bool GetDepth() { return GetInstance()->m_IsDepth; }
	// アルファブレンドが有効か取得.
	static bool GetAlphaBlend() { return GetInstance()->m_IsAlphaBlend; }
	// アルファトゥカバレージが有効か取得.
	static bool GetAlphaToCoverage() { return GetInstance()->m_IsAlphaToCoverage; }

	// 背景の色の初期化.
	static void InitBackColor() { GetInstance()->m_BackColor = GetInstance()->m_InitBackColor; }

	// 背景色の設定.
	static void SetBackColor( const D3DXCOLOR4& Color ) { GetInstance()->m_BackColor = Color; }
	// 深度(Z)テストON/OFF切り替え.
	static void SetDepth( bool flag );
	// アルファブレンドON/OFF切り替え.
	static void SetAlphaBlend( bool flag );
	// アルファトゥカバレージON/OFF切り替え.
	static void SetAlphaToCoverage( bool flag );
	// ラスタライザステート設定.
	static void SetRasterizerState( const ERS_STATE& RsState );
	// マウスカーソルを表示するかの設定.
	static void SetIsDispMouseCursor( const bool Flag ) { GetInstance()->m_IsDispMouseCursor = Flag; }

	// フルスクリーンの設定.
	static void SetFullScreen( const bool Flag );
	// フルスクリーン状態か取得.
	static bool IsFullScreen();
	// ウィンドウサイズが変更された時に呼ぶ.
	static void Resize();

	// アクティブウィンドウか確認.
	static void CheckActiveWindow();
	// ウィンドウがアクティブか取得.
	static bool IsWindowActive() { return GetInstance()->m_IsWindowActive; };

	// D3DX10CompileFromFileマクロを呼び出す.
	static HRESULT	MutexD3DX10CompileFromFile(
		const LPCTSTR filepath, const D3D_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,
		LPCSTR entryPoint, LPCSTR shaderModel, UINT compileFlag, UINT Flags2, ID3DX10ThreadPump  *pPump,
		ID3D10Blob **ppOutShader, ID3D10Blob **ppOutErrorMsgs, HRESULT *pOutHResult );
	// D3DX11CompileFromFileマクロを呼び出す.
	static HRESULT	MutexD3DX11CompileFromFile(
		const LPCTSTR filepath, const D3D_SHADER_MACRO* pDefines, LPD3D10INCLUDE pInclude,
		LPCSTR entryPoint, LPCSTR shaderModel, UINT compileFlag, UINT Flags2, ID3DX11ThreadPump  *pPump,
		ID3D10Blob **ppOutShader, ID3D10Blob **ppOutErrorMsgs, HRESULT *pOutHResult );
	// D3DX11CreateShaderResourceViewFromFileマクロを呼び出す.
	static HRESULT	MutexD3DX11CreateShaderResourceViewFromFile(
		const LPCTSTR pSrcFile, D3DX11_IMAGE_LOAD_INFO* pLoadInfo, ID3DX11ThreadPump* pPump,
		ID3D11ShaderResourceView** ppShaderResourceView, HRESULT* pHResult );
	// D3DXLoadMeshFromXマクロを呼び出す.
	static HRESULT	MutexD3DXLoadMeshFromX(
		const LPCTSTR pFilename, DWORD Options, LPDIRECT3DDEVICE9 pD3DDevice, LPD3DXBUFFER* ppAdjacency,
		LPD3DXBUFFER* ppMaterials, LPD3DXBUFFER* ppEffectInstances, DWORD* pNumMaterials, LPD3DXMESH* ppMesh );
	// D11CreateVertexShaderを呼び出す.
	static HRESULT	MutexDX11CreateVertexShader(ID3DBlob* pCompiledShader,ID3D11ClassLinkage *pClassLinkage, ID3D11VertexShader** pVertexShader );
	// D11CreatePixelShaderを呼び出す.
	static HRESULT	MutexDX11CreatePixelShader(ID3DBlob* pCompiledShader, ID3D11ClassLinkage *pClassLinkage, ID3D11PixelShader** pPixelShader );
	// D11CreateInputLayoutを呼び出す.
	static HRESULT	MutexDX11CreateInputLayout(D3D11_INPUT_ELEMENT_DESC* pLayout,UINT numElements,ID3DBlob* pCompiledShader,ID3D11InputLayout** m_pVertexLayout );
	// D3DXLoadMeshHierarchyFromXを呼び出す.
	static HRESULT	MutexD3DXLoadMeshHierarchyFromX( LPCTSTR Filename, DWORD MeshOptions,
		LPDIRECT3DDEVICE9 pDevice, LPD3DXALLOCATEHIERARCHY pAlloc,
		LPD3DXLOADUSERDATA pUserDataLoader, LPD3DXFRAME* ppFrameHierarchy,
		LPD3DXANIMATIONCONTROLLER* ppAnimController );

private:
	// インスタンスの取得.
	static DirectX11* GetInstance();

	// デバイスとスワップチェイン作成.
	HRESULT CreateDeviceAndSwapChain();

	// バックバッファ作成：カラー用レンダーターゲットビュー作成.
	HRESULT CreateColorBackBufferRTV();

	// バックバッファ作成：デプスステンシル用レンダーターゲットビュー作成.
	HRESULT CreateDepthStencilBackBufferRTV();

	// ラスタライザ作成.
	HRESULT CreateRasterizer();

	// デプスステンシルステート作成.
	HRESULT CreateDepthStencilState();

	// アルファブレンドステート作成.
	HRESULT CreateAlphaBlendState();

	// ビューポート作成.
	HRESULT CreateViewports();

	// エラーを送る
	static void PushError( const std::string& t, const HRESULT& r );

private:
	std::vector<HWND>						m_hWnd;						// ウィンドウハンドル.

	ID3D11Device*							m_pDevice11;				// デバイスオブジェクト.
	ID3D11DeviceContext*					m_pContext11;				// デバイスコンテキスト.
	std::vector<IDXGISwapChain*>			m_pSwapChain;				// スワップチェーン.
	std::vector<ID3D11RenderTargetView*>	m_pBackBuffer_TexRTV;		// レンダ―ターゲットビュー.
	std::vector<ID3D11Texture2D*>			m_pBackBuffer_DSTex;		// デプスステンシル用テクスチャ.
	std::vector<ID3D11DepthStencilView*>	m_pBackBuffer_DSTexDSV;		// デプスステンシルビュー.

	// 深度(Z)テスト設定.
	ID3D11DepthStencilState*				m_pDepthStencilStateOn;		// 有効設定.
	ID3D11DepthStencilState*				m_pDepthStencilStateOff;	// 無効設定.

	// アルファブレンド.
	ID3D11BlendState*						m_pAlphaBlendOn;			// 有効設定.
	ID3D11BlendState*						m_pAlphaBlendOff;			// 無効設定.

	// アルファトゥカバレージ.
	ID3D11BlendState*						m_pAlphaToCoverageOn;		// 有効設定..

	// ラスタライザステート.
	ID3D11RasterizerState*					m_pCullNone;				// 正背面描画.
	ID3D11RasterizerState*					m_pCullBack;				// 背面を描画しない.
	ID3D11RasterizerState*					m_pCullFront;				// 正面を描画しない.
	ID3D11RasterizerState*					m_pWireFrame;				// ワイヤーフレーム描画.

	UINT									m_WndWidth;					// ウィンドウ幅.
	UINT									m_WndHeight;				// ウィンドウ高さ.

	D3DXCOLOR4								m_BackColor;				// 背景色.
	D3DXCOLOR4								m_InitBackColor;			// 初期の背景の色.

	bool									m_IsWindowActive;			// アクティブウィンドウか確認.
	bool									m_IsDepth;					// 深度(Z)テストが有効か.
	bool									m_IsAlphaBlend;				// アルファブレンドが有効か.
	bool									m_IsAlphaToCoverage;		// アルファトゥカバレージが有効か.
	bool									m_IsDispMouseCursor;		// マウスカーソルを表示するか.

	int										m_WindowNum;				// ウィンドウの数.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	DirectX11( const DirectX11 & )				= delete;
	DirectX11& operator = ( const DirectX11 & )	= delete;
	DirectX11( DirectX11 && )						= delete;
	DirectX11& operator = ( DirectX11 && )		= delete;
};