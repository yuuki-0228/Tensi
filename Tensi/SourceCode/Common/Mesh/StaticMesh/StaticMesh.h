#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

class CFog;

/************************************************
*	スタティックメッシュクラス	
**/
class CStaticMesh final
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
	// マテリアル単位で渡す情報.
	struct CBUFFER_PER_MATERIAL
	{
		D3DXVECTOR4 vAmbient;	// アンビエント色(環境色).
		D3DXVECTOR4	vDiffuse;	// ディフューズ色(拡散反射色).
		D3DXVECTOR4	vSpecular;	// スペキュラ色(鏡面反射色).
		D3DXVECTOR4	vUV;		// スペキュラ色(鏡面反射色).
	};
	// フレーム単位で渡す情報.
	//	カメラ,ライト情報はここに入れる.
	struct CBUFFER_PER_FRAME
	{
		D3DXPOSITION4	vCameraPos;			// カメラ位置(視点位置).
		D3DXVECTOR4		vLightDir;			// ライト方向.
		D3DXVECTOR4		vLightIntensity;	// ライトの強弱.
		D3DXVECTOR4		vFlag;				// x：ﾃﾞｲｻﾞ抜きを使用するか、	y：ｱﾙﾌｧﾌﾞﾛｯｸを使用するか、	z：距離で透明にするか(1以上なら使用する).
		D3DXVECTOR4		vAlphaDistance;		// x：開始する距離(以下)、	y：開始する距離(以上)、	z：透明にしていく間隔.
	};

	// 頂点の構造体.
	struct VERTEX
	{
		D3DXPOSITION3	Pos;	// 頂点座標.
		D3DXVECTOR3		Normal;	// 法線(陰影計算に必須).
		D3DXVECTOR2		UV;		// UV(x,yのみ使用する).
	};
	// マテリアル構造体.
	struct MY_MATERIAL
	{
		D3DXVECTOR4								Ambient;			// アンビエント.
		D3DXVECTOR4								Diffuse;			// ディフューズ.
		D3DXVECTOR4								Specular;			// スペキュラ.
		TCHAR									TextureName[64];	// テクスチャファイル名.
		std::vector<ID3D11ShaderResourceView*>	pTexture;			// テクスチャ.
		ID3D11ShaderResourceView*				pLightTexture;		// ライトテクスチャ.
		DWORD									dwNumFace;			// そのマテリアルのポリゴン数.
		D3DXVECTOR4								UV;					// UV(x,yのみ使用する).

		// コンストラクタ.
		MY_MATERIAL()
			: Ambient		()
			, Diffuse		()
			, Specular		()
			, TextureName	()
			, pTexture		( 1 )
			, pLightTexture	( nullptr )
			, dwNumFace		( 0 )
			, UV			( INIT_FLOAT4 )
		{
		}
		// デストラクタ.
		~MY_MATERIAL() {
			const int Size = static_cast<int>( pTexture.size() );
			for ( int i = Size - 1; i >= 0; --i ) {
				SAFE_RELEASE( pTexture[i] );
			}
			if ( pLightTexture != nullptr )
			{
				pLightTexture->Release();
				pLightTexture = nullptr;
			}
		}
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
	CStaticMesh();
	~CStaticMesh();

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

	// フォグの有効/無効.
	inline void SetIsFog( const bool Flag ) { m_IsFog = Flag; }
	// ディザ抜きを使用するか.
	inline void SetDither( const bool Flag ) { m_DitherFlag = Flag; }
	// アルファブロックを使用するか.
	inline void SetAlphaBlock( const bool Flag ) { m_AlphaBlockFlag = Flag; }
	// アルファブレンドを使用するか.
	inline void SetAlphaBlend( const bool Flag ) { DirectX11::SetAlphaBlend( Flag ); }
	// 距離で透明にするか(以下).
	inline void SetDistanceAlphaOrLess( const bool Flag, const float StartDistance = 0.0f, const float AlphaDistance = 0.0f ) {
		m_OrLessDistanceAlphaFlag	= Flag;
		m_OrLessStartDistance		= StartDistance;
		m_AlphaDistance				= AlphaDistance;
	}
	// 距離で透明にするか(以上).
	inline void SetDistanceAlphaOrMore( const bool Flag, const float StartDistance = 0.0f, const float AlphaDistance = 0.0f ) {
		m_OrMoreDistanceAlphaFlag	= Flag;
		m_OrMoreStartDistance		= StartDistance;
		m_AlphaDistance				= AlphaDistance;
	}

	// テクスチャの変更.
	inline void ChangeTexture( const int MaterialNo, const int TextureNo ) { m_MaterialTextureNo[MaterialNo] = TextureNo; }

	// UVスクロールする値を設定.
	inline void SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv ) {
		m_pMaterials[MaterialNo].UV.x = uv.x;
		m_pMaterials[MaterialNo].UV.y = uv.y;
	}
	// x方向にUVスクロールする値を設定.
	inline void SetUVScrollX( const int MaterialNo, const float uv ) {
		m_pMaterials[MaterialNo].UV.x = uv;
	}
	// y方向にUVスクロールする値を設定.
	inline void SetUVScrollY( const int MaterialNo, const float uv ) {
		m_pMaterials[MaterialNo].UV.y = uv;
	}

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
	ID3D11VertexShader*		m_pVertexShader;				// 頂点シェーダ.
	ID3D11InputLayout*		m_pVertexLayout;				// 頂点レイアウト.
	ID3D11PixelShader*		m_pPixelShader;					// ピクセルシェーダ.

	ID3D11Buffer*			m_pCBufferPerMesh;				// コンスタントバッファ(メッシュ毎).
	ID3D11Buffer*			m_pCBufferPerMaterial;			// コンスタントバッファ(マテリアル毎).
	ID3D11Buffer*			m_pCBufferPerFrame;				// コンスタントバッファ(フレーム毎).

	ID3D11Buffer*			m_pVertexBuffer;				// 頂点バッファ.
	ID3D11Buffer**			m_ppIndexBuffer;				// インデックスバッファ.
	ID3D11SamplerState*		m_pSampleLinear;				// サンプラ:テクスチャに各種フィルタをかける.

	MY_MODEL				m_Model;
	MY_MODEL				m_ModelForRay;					// レイとの当たり判定で使用する.

	MY_MATERIAL*			m_pMaterials;					// マテリアル構造体.
	DWORD					m_NumAttr;						// 属性数.
	DWORD					m_AttrID[300];					// 属性ID ※300属性まで.

	bool					m_EnableTexture;				// テクスチャあり.

	std::unique_ptr<CFog>	m_pFog;							// フォグ.
	bool					m_IsFog;						// フォグを有効にするか.
	bool					m_DitherFlag;					// ディザ抜きを使用するか.
	bool					m_AlphaBlockFlag;				// アルファブロックを使用するか.
	bool					m_OrLessDistanceAlphaFlag;		// 距離で透明にするか(以下).
	bool					m_OrMoreDistanceAlphaFlag;		// 距離で透明にするか(以上).

	float					m_OrLessStartDistance;			// 開始する距離(以下).
	float					m_OrMoreStartDistance;			// 開始する距離(以上).
	float					m_AlphaDistance;				// 透明にしていく間隔.

	std::vector<std::string> m_TexturePathList;				// 差し替えテクスチャパスリスト.
	std::vector<int>		m_MaterialTextureNo;			// マテリアルのテクスチャのNo.
 };
