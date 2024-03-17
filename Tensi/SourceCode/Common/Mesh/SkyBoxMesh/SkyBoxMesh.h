#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

/************************************************
*	スカイボックスメッシュクラス	
**/
class CSkyBoxMesh final
	: public CCommon
{
public:
	// コンスタントバッファのアプリ側の定義.
	//	※シェーダ内のコンスタントバッファと一致している必要あり.
	//	メッシュ単位で渡す情報.
	struct CBUFFER_PER_MESH
	{
		D3DXMATRIX	mW;			// ワールド行列.
		D3DXMATRIX	mWVP;		// ワールド,ビュー,プロジェクションの合成変換行列.	
		D3DXCOLOR4	vColor;		// カラー.
	};

	// 頂点の構造体.
	struct VERTEX
	{
		D3DXPOSITION3	Pos;	// 頂点座標.
		D3DXVECTOR3		Normal;	// 法線(陰影計算に必須).
		D3DXVECTOR2		UV;		// テクスチャ座標.
	};
	// マテリアル構造体.
	struct MY_MATERIAL
	{
		TCHAR TextureName[64];	// テクスチャファイル名.
		ID3D11ShaderResourceView* pTexture;// テクスチャ.
		DWORD dwNumFace;		// そのマテリアルのポリゴン数.
		// コンストラクタ.
		MY_MATERIAL()
			: TextureName	()
			, pTexture		( nullptr )
			, dwNumFace		( 0 )
		{
		}
		// デストラクタ.
		~MY_MATERIAL() {
			SAFE_RELEASE( pTexture );
		}
	};
	// フレーム単位で渡す情報.
	//	カメラ,ライト情報はここに入れる.
	struct CBUFFER_PER_FRAME
	{
		D3DXVECTOR4 vFlag;		// x：ﾃﾞｲｻﾞ抜きを使用するか、	y：ｱﾙﾌｧﾌﾞﾛｯｸを使用するか.
	};

	// Xファイルのロードで必要受け取るもの.
	struct MY_MODEL
	{
		TCHAR			FileName[64];		// ファイル名.
		LPD3DXMESH		pMesh;				// メッシュオブジェクト.
		LPD3DXBUFFER	pD3DXMtrlBuffer;	// マテリアルバッファ.
		DWORD			NumMaterials;		// マテリアル数.
		MY_MODEL()
			: FileName			()
			, pMesh				( nullptr )
			, pD3DXMtrlBuffer	( nullptr )
			, NumMaterials		()
		{}
		~MY_MODEL(){
			SAFE_RELEASE( pMesh );
			SAFE_RELEASE( pD3DXMtrlBuffer );
		}
	};

public:
	CSkyBoxMesh();
	~CSkyBoxMesh();

	HRESULT Init( LPCTSTR lpFileName );

	// 解放関数.
	void Release();

	// レンダリング用.
	void Render( STransform* pTransform = nullptr, const D3DXMATRIX* pRot = nullptr );

	// メッシュリストを取得.
	inline std::pair<LPD3DXMESH, LPD3DXMESH> GetMeshList() const { return std::make_pair( m_Model.pMesh, m_ModelForRay.pMesh ); }

	// メッシュの取得.
	inline LPD3DXMESH GetMesh()			const { return m_Model.pMesh; }
	// レイとの当たり判定用のメッシュの取得.
	inline LPD3DXMESH GetModelForRay()	const { return m_ModelForRay.pMesh; }

	// ディザ抜きを使用するか.
	inline void SetDither( const bool& Flag ) { m_DitherFlag = Flag; }
	// アルファブロックを使用するか.
	inline void SetAlphaBlock( const bool& Flag ) { m_AlphaBlockFlag = Flag; }
	// アルファブレンドを使用するか.
	inline void SetAlphaBlend( const bool& Flag ) { DirectX11::SetAlphaBlend( Flag ); }

private:
	// メッシュ読み込み.
	HRESULT LoadXMesh( LPCTSTR lpFileName );
	// シェーダ作成.
	HRESULT CreateShader();
	// モデル作成.
	HRESULT CreateModel();
	// マテリアル作成.
	HRESULT CreateMaterials();
	// インデックスバッファ作成.
	HRESULT CreateIndexBuffer();
	// 頂点バッファ作成.
	HRESULT CreateVertexBuffer();
	// コンスタントバッファ作成.
	HRESULT CreateConstantBuffer();
	// サンプラ作成.
	HRESULT CreateSampler();

	// レンダリング関数(クラス内でのみ使用する).
	void RenderMesh( D3DXMATRIX& mWorld );

private:
	ID3D11VertexShader*		m_pVertexShader;		// 頂点シェーダ.
	ID3D11InputLayout*		m_pVertexLayout;		// 頂点レイアウト.
	ID3D11PixelShader*		m_pPixelShader;			// ピクセルシェーダ.

	ID3D11Buffer*			m_pCBufferPerMesh;		// コンスタントバッファ(メッシュ毎).
	ID3D11Buffer*			m_pCBufferPerFrame;		// コンスタントバッファ(フレーム毎).

	ID3D11Buffer*			m_pVertexBuffer;		// 頂点バッファ.
	ID3D11Buffer**			m_ppIndexBuffer;		// インデックスバッファ.
	ID3D11SamplerState*		m_pSampleLinear;		// サンプラ:テクスチャに各種フィルタをかける.

	MY_MODEL				m_Model;
	MY_MODEL				m_ModelForRay;			// レイとの当たり判定で使用する.

	MY_MATERIAL*			m_pMaterials;			// マテリアル構造体.
	DWORD					m_NumAttr;				// 属性数.
	DWORD					m_AttrID[300];			// 属性ID ※300属性まで.

	bool					m_EnableTexture;		// テクスチャあり.
	bool					m_DitherFlag;			// ディザ抜きを使用するか.
	bool					m_AlphaBlockFlag;		// アルファブロックを使用するか.
 };
