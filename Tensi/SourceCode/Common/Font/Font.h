#pragma once
#include "FontStruct.h"

/********************************************
*	フォントクラス.
*		﨑田友輝.
**/
class CFont final
{
public:
	using Sampler = unsigned long long;

public:
	CFont();
	virtual ~CFont();

	// 初期化.
	HRESULT Init( const std::string& FilePath, const std::string& FileName );

	// UIで描画.
	void RenderUI( SFontRenderState* pRenderState = nullptr );
	void RenderUI( const std::string& Text, SFontRenderState* pRenderState = nullptr );
	// 3Dで描画.
	void Render3D( SFontRenderState* pRenderState = nullptr, const bool& IsBillBoard = false );
	void Render3D( const std::string& Text, SFontRenderState* pRenderState = nullptr, const bool& IsBillBoard = false );

	// 描画座標の取得.
	inline D3DXVECTOR3 GetRenderPos() const { return m_FontRenderState.Transform.Position; }
	// フォント情報の取得.
	inline SFontState GetFontState() const { return m_FontState; }
	// フォントの描画情報を取得.
	inline SFontRenderState GetRenderState() const { return m_FontRenderState; }

	// 描画する範囲の設定.
	inline void SetRenderArea( const D3DXVECTOR4& Area ) { m_FontRenderState.RenderArea = Area; }
	inline void SetRenderArea( const float x, const float y, const float w, const float h ) { m_FontRenderState.RenderArea = { x, y, w, h }; }

	// ディザ抜きを使用するか.
	inline void SetDither( const bool Flag ) { m_DitherFlag = Flag; }
	// アルファブロックを使用するか.
	inline void SetAlphaBlock( const bool Flag ) { m_AlphaBlockFlag = Flag; }

private:
	// フォント情報の読み込み.
	HRESULT FontStateDataLoad( const std::string& FilePath );

	// シェーダ作成.
	HRESULT CreateShader();
	// モデル作成(UI).
	HRESULT CreateModelUI();
	// モデル作成(3D).
	HRESULT CreateModel3D();
	// サンプラ作成.
	HRESULT CreateSampler();
	// 頂点情報の作成.
	HRESULT CreateVertex( const float w, const float h, const float u, const float v );

	// テクスチャの比率を取得.
	int myGcd( int t, int t2 ) { if ( t2 == 0 ) return t; return myGcd( t2, t % t2 ); }

	// テクスチャの描画.
	void RenderFontUI( const char* c, SFontRenderState* pRenderState );
	// テクスチャの描画.
	void RenderFont3D( const char* c, SFontRenderState* pRenderState, const bool& IsBillBoard );

private:
	ID3D11Device*					m_pDevice;				// デバイスオブジェクト.
	ID3D11DeviceContext*			m_pContext;				// デバイスコンテキスト.

	ID3D11VertexShader*				m_pVertexShader;		// 頂点シェーダ.
	ID3D11InputLayout*				m_pVertexLayout;		// 頂点レイアウト.
	ID3D11PixelShader*				m_pPixelShader;			// ピクセルシェーダ.

	ID3D11Buffer*					m_pVertexBufferUI;		// 頂点バッファ(UI用).
	ID3D11Buffer*					m_pVertexBuffer3D;		// 頂点バッファ(3D用).
	ID3D11Buffer*					m_pConstantBuffer;		// コンスタントバッファ.

	SVertex							m_Vertices[4];			// 頂点作成用.
	SFontState						m_FontState;			// フォント情報.
	SFontRenderState				m_FontRenderState;		// フォントアニメーション情報.

	ID3D11SamplerState* m_pSampleLinears[static_cast<Sampler>(ESamplerState::Max)]; // サンプラ:テクスチャに各種フィルタをかける.

	std::string						m_FileName;				// フォントのファイル名.

	bool							m_DitherFlag;			// ディザ抜きを使用するか.
	bool							m_AlphaBlockFlag;		// アルファブロックを使用するか.
};