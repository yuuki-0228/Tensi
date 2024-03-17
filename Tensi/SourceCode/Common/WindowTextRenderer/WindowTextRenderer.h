#pragma once
#include "..\Common.h"
#include "..\..\Global.h"

/************************************************
*	ウィンドウに表示するテキストクラス.
*		﨑田友輝.
**/
class WindowTextRenderer final
	: public CCommon
{
private:
	static constexpr float RECT_W			= 30.0f;
	static constexpr float RECT_H			= 36.0f;
	static constexpr float TEX_DIMENSION	= 384.0f;

public:
	// コンスタントバッファのアプリ側の定義.
	//	※シェーダー内のコンスタントバッファと一致している必要あり.
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16 D3DXMATRIX	mWVP;		// ワールド,ビュー,プロジェクションの合成変換行列.	
		ALIGN16 D3DXCOLOR4	vColor;		// カラー(RGBAの型に合わせる).
		ALIGN16 float fViewPortWidth;	// ビューポート幅.
		ALIGN16 float fViewPortHeight;	// ビューポート高さ.
	};
	// 頂点の構造体.
	struct VERTEX
	{
		D3DXVECTOR3 Pos;	// 頂点座標.
		D3DXVECTOR2	Tex;	// テクスチャ座標.
	};

public:
	WindowTextRenderer();
	~WindowTextRenderer();

	// 初期化.
	static HRESULT Init();

	// 描画.
	static void Render(		 const std::string& text, const D3DXPOSITION3& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void Render(		 const std::string& text, const D3DXPOSITION2& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void Render(		 const std::string& text, const int x, const int y,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const D3DXPOSITION3& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const D3DXPOSITION2& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const int x, const int y,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	
	// 文字の画像サイズの取得.
	static D3DXVECTOR2 GetTextSize() { return { RECT_W * GetInstance()->m_FontSize, RECT_H * GetInstance()->m_FontSize }; }

private:
	// インスタンスの取得.
	static WindowTextRenderer* GetInstance();

	// シェーダー作成.
	static HRESULT CreateShader();
	// モデル作成.
	static HRESULT CreateModel();
	// テクスチャ作成.
	static HRESULT CreateTexture( LPCTSTR lpFileName );
	// サンプラ作成.
	static HRESULT CreateSampler();
	// フォントレンダリング.
	static void RenderFont( int FontIndex, int x, int y );

private:
	ID3D11VertexShader*			m_pVertexShader;		// 頂点シェーダー.
	ID3D11InputLayout*			m_pVertexLayout;		// 頂点レイアウト.
	ID3D11PixelShader*			m_pPixelShader;			// ピクセルシェーダー.
	ID3D11Buffer*				m_pConstantBuffer;		// コンスタントバッファ.

	ID3D11Buffer*				m_pVertexBuffer[100];	// 頂点バッファ(100個分).

	ID3D11ShaderResourceView*	m_pTexture;				// テクスチャ.
	ID3D11SamplerState*			m_pSampleLinear;		// サンプラ:テクスチャに各種フィルタをかける.

	float						m_Kerning[100];			// カーニング(100個分
	float						m_FontSize;				// フォントサイズ.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	WindowTextRenderer( const WindowTextRenderer & )				= delete;
	WindowTextRenderer& operator = ( const WindowTextRenderer & ) = delete;
	WindowTextRenderer( WindowTextRenderer && )					= delete;
	WindowTextRenderer& operator = ( WindowTextRenderer && )		= delete;
};