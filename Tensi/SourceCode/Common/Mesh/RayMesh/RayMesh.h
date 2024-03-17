#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

class CRay;

/************************************************
*	レイメッシュクラス.
**/
class CRayMesh final
	: public CCommon
{
public:
	// コンスタントバッファのアプリ側の定義.
	//	※シェーダ内のコンスタントバッファと一致している必要あり.
	struct SHADER_CONSTANT_BUFFER {
		D3DXMATRIX	mWVP;		// ワールド,ビュー,プロジェクションの合成変換行列.	
		D3DXCOLOR4	vColor;		// カラー.
	};
	// 頂点の構造体.
	struct VERTEX {
		D3DXPOSITION3 Pos;		// 頂点座標.
	};

public:
	CRayMesh();
	~CRayMesh();

	// 初期化.
	HRESULT Init();

	// レンダリング用.
	void Render( CRay* Ray );

private:
	// シェーダ作成.
	HRESULT CreateShader();
	// モデル作成.
	HRESULT CreateModel( CRay* pRay );

private:
	ID3D11VertexShader*		m_pVertexShader;	// 頂点シェーダ.
	ID3D11InputLayout*		m_pVertexLayout;	// 頂点レイアウト.
	ID3D11PixelShader*		m_pPixelShader;		// ピクセルシェーダ.
	ID3D11Buffer*			m_pConstantBuffer;	// コンスタントバッファ.

	ID3D11Buffer*			m_pVertexBuffer;	// 頂点バッファ.
};