#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\..\Global.h"

/************************************************
*	オフスクリーン描画用のレンダーテクスチャ.
*	( 視覚効果のピンポンバッファやキャプチャ先として使用する ).
**/
class CRenderTexture final
{
public:
	CRenderTexture();
	~CRenderTexture();

	// 作成( UseDepth : 深度バッファも作成するか ).
	HRESULT Create( const UINT Width, const UINT Height, const bool UseDepth = false );
	// 解放.
	void Release();

	// 指定色でクリア( 深度バッファがあればそれもクリア ).
	void Clear( const D3DXCOLOR4& Color = { 0.0f, 0.0f, 0.0f, 0.0f } );

	// 同じサイズかどうか.
	bool IsSameSize( const UINT Width, const UINT Height ) const { return m_Width == Width && m_Height == Height; }

	// 各リソースの取得.
	ID3D11Texture2D*			GetTex()	const { return m_pTex;	}
	ID3D11RenderTargetView*		GetRTV()	const { return m_pRTV;	}
	ID3D11ShaderResourceView*	GetSRV()	const { return m_pSRV;	}
	ID3D11DepthStencilView*		GetDSV()	const { return m_pDSV;	}
	UINT						GetWidth()	const { return m_Width;	}
	UINT						GetHeight()	const { return m_Height; }

private:
	ID3D11Texture2D*			m_pTex;		// カラーテクスチャ.
	ID3D11RenderTargetView*		m_pRTV;		// レンダーターゲットビュー.
	ID3D11ShaderResourceView*	m_pSRV;		// シェーダリソースビュー.
	ID3D11Texture2D*			m_pDSTex;	// 深度ステンシルテクスチャ.
	ID3D11DepthStencilView*		m_pDSV;		// 深度ステンシルビュー.
	UINT						m_Width;	// 幅.
	UINT						m_Height;	// 高さ.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CRenderTexture( const CRenderTexture& )					= delete;
	CRenderTexture& operator = ( const CRenderTexture& )	= delete;
	CRenderTexture( CRenderTexture&& )						= delete;
	CRenderTexture& operator = ( CRenderTexture&& )			= delete;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
