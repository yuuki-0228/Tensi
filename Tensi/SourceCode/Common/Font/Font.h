#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_FONT
#include "FontStruct.h"
#include "FontTagParser\FontTagParser.h"

/********************************************
*	フォントクラス.
*	  SDF(符号付き距離場)を使用してアウトラインや太文字などを1ドローで描画する.
*	  SFontRenderState.IsRichText を有効にするとTextMeshPro相当のタグが使用できる.
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
	// フォントの描画情報取得.
	inline SFontRenderState GetRenderState() const { return m_FontRenderState; }

	// 描画する範囲の設定.
	inline void SetRenderArea( const D3DXVECTOR4& Area ) { m_FontRenderState.RenderArea = Area; }
	inline void SetRenderArea( const float x, const float y, const float w, const float h ) { m_FontRenderState.RenderArea = { x, y, w, h }; }

	// ディザ抜きを使用するか.
	inline void SetDither( const bool Flag ) { m_DitherFlag = Flag; }
	// アルファブロックを使用するか.
	inline void SetAlphaBlock( const bool Flag ) { m_AlphaBlockFlag = Flag; }

private:
	// 1回のドローに使用する情報.
	struct SFontDrawParam
	{
		D3DXMATRIX					mWorld;			// ワールド行列(ローカル補正込み).
		ID3D11ShaderResourceView*	pTexture;		// SDFテクスチャ(nullptrで矩形描画).
		D3DXCOLOR4					Color;			// 文字色.
		D3DXCOLOR4					OutLineColor;	// アウトラインの色.
		D3DXCOLOR4					GlowColor;		// グローの色.
		D3DXVECTOR4					SDFParam;		// SDF情報(x:アウトライン幅)(y:太字の膨張量)(z:ぼかし量)(w:アウトライン使用).
		D3DXVECTOR4					GlowParam;		// グロー情報(x:グロー幅)(y:グローの減衰)(z:グロー使用).
		ESamplerState				SmaplerNo;		// サンプラ番号.
		bool						Is3D;			// 3D描画か.
	};

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

	// テキストの描画(UI/3D共通).
	void RenderText( const std::string& Text, SFontRenderState* pRenderState, const bool Is3D, const bool IsBillBoard );
	// 1文字の描画.
	void RenderGlyph( const std::string& Char, const SFontCharStyle& Style, const STransform& Transform, SFontRenderState* pState, const bool Is3D, const bool IsBillBoard );
	// 矩形の描画(下線・取り消し線・マーカー用).
	//	WidthScale: クアッド幅に対する矩形の幅の倍率.
	//	PosRateY  : セルの高さに対する矩形上端の位置の割合.
	//	SizeRateY : セルの高さに対する矩形の太さの割合.
	void RenderLineRect( const D3DXCOLOR4& Color, const float WidthScale, const float PosRateY, const float SizeRateY, const STransform& Transform, SFontRenderState* pState, const bool Is3D, const bool IsBillBoard );
	// クアッドの描画.
	void DrawQuad( const SFontDrawParam& Param, SFontRenderState* pState );

	// 使用するローカル座標の番号を取得(Defaultの場合はフォント情報の値を使用).
	ELocalPosition GetLocalPosition( const SFontRenderState* pState ) const;
	// クアッドのローカル左上座標などの取得.
	void GetQuadTopLeft( const ELocalPosition LocalPos, const bool Is3D, float* pX, float* pY, float* pW, float* pH, float* pYSign );

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
	SSize							m_Size3D;				// 3D用モデルの幅高さ.

	ID3D11SamplerState* m_pSampleLinears[static_cast<Sampler>(ESamplerState::Max)]; // サンプラ:テクスチャに各種フィルタをかける.

	std::string						m_FileName;				// フォントのファイル名.

	bool							m_DitherFlag;			// ディザ抜きを使用するか.
	bool							m_AlphaBlockFlag;		// アルファブロックを使用するか.
};
#endif // ENABLE_FONT
