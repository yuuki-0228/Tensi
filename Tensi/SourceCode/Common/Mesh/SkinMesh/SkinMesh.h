/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode対応.
**/
#pragma once

#include "..\..\Common.h"
#include "Parser\SkinMeshParser.h"

class CFog;
class CStaticMesh;

// アニメーションコントローラ構造体.
struct stAnimationController {
	LPD3DXANIMATIONCONTROLLER	pAC;
	double						AnimTime;
	bool						IsAnimChangeBlend;
	int							NowIndex;
	int							NewIndex;

	stAnimationController()
		: pAC				( nullptr )
		, AnimTime			( 0.0 )
		, IsAnimChangeBlend	( false )
		, NowIndex			( 0 )
		, NewIndex			( 0 )
	{}

	// アニメーションブレンドの更新.
	void BlendAnimUpdate( const double& Speed ) {
		if( IsAnimChangeBlend == false ) return;
		AnimTime += Speed;

		float weight = static_cast<float>( AnimTime ) / 1.0f;	// ウェイトの計算.
		pAC->SetTrackWeight( 0, weight );						// トラック0 にウェイトを設定.
		pAC->SetTrackWeight( 1, 1.0f - weight );				// トラック1 にウェイトを設定.

		if( AnimTime < 1.0 ) return;
		// アニメーションタイムが一定値に達したら.
		IsAnimChangeBlend	= false;								// フラグを下す.
		NowIndex			= NewIndex;
		NewIndex			= -1;
		pAC->SetTrackWeight( 0, 1.0f );							// ウェイトを1に固定する.
		pAC->SetTrackEnable( 1, false );						// トラック1を無効にする.
	}

	// アニメーションコントローラの設定.
	HRESULT SetAnimController( LPD3DXANIMATIONCONTROLLER pAc ) {
		if( FAILED( pAc->CloneAnimationController(
			pAc->GetMaxNumAnimationOutputs(),
			pAc->GetMaxNumAnimationSets(),
			pAc->GetMaxNumTracks(),
			pAc->GetMaxNumEvents(),
			&pAC ) ) )
		{
			ErrorMessage( "アニメーションコントローラのクローン作成失敗" );
			return E_FAIL;
		}
		return S_OK;
	}
} typedef SAnimationController;


/************************************************
*	スキンメッシュクラス
**/
class CSkinMesh final
	: public CCommon
{
public:
	// コンスタントバッファのアプリ側の定義.
	//	※シェーダ内のコンスタントバッファと一致している必要あり.
	struct CBUFFER_PER_MESH
	{
		D3DXMATRIX	mW;				// ワールド行列.
		D3DXMATRIX	mWVP;			// ワールドから射影までの変換行列.
		D3DXCOLOR4	vColor;			// カラー.

	};

	// マテリアル単位.
	struct CBUFFER_PER_MATERIAL
	{
		D3DXVECTOR4 vAmbient;		// アンビエント光.
		D3DXVECTOR4 vDiffuse;		// ディフューズ色.
		D3DXVECTOR4 vSpecular;		// 鏡面反射.
		D3DXVECTOR4	vUV;			// UV(x,yのみ使用する).
	};

	// シェーダーに渡す値.
	struct CBUFFER_PER_FRAME
	{
		D3DXPOSITION4	vCameraPos;				// カメラ位置.
		D3DXVECTOR4		vLightDir;				// ライト方向.
		D3DXVECTOR4		vLightIntensity;		// ライトの強弱.
		D3DXVECTOR4		vPointLightIntensity;	// メッシュに対してのポイントライトの強弱.
		D3DXVECTOR4		vFlag;					// x：ﾃﾞｲｻﾞ抜きを使用するか、	y：ｱﾙﾌｧﾌﾞﾛｯｸを使用するか、	z：距離で透明にするか(1以上なら使用する).
		D3DXVECTOR4		vAlphaDistance;			// x：開始する距離(以下)、	y：開始する距離(以上)、	z：透明にしていく間隔.
	};

	// ボーン単位.
	struct CBUFFER_PER_BONES
	{
		D3DXMATRIX mBone[D3DXPARSER::MAX_BONES];
		CBUFFER_PER_BONES()
		{
			for ( int i = 0; i < D3DXPARSER::MAX_BONES; i++ )
			{
				D3DXMatrixIdentity( &mBone[i] );
			}
		}
	};


	// 頂点構造体.
	struct VERTEX
	{
		D3DXPOSITION3	vPos;		// 頂点位置.
		D3DXVECTOR3		vNorm;		// 頂点法線.
		D3DXVECTOR2		vTex;		// UV座標.
		UINT bBoneIndex[4];			// ボーン 番号.
		float bBoneWeight[4];		// ボーン 重み.
		VERTEX()
			: vPos			()
			, vNorm			()
			, vTex			()
			, bBoneIndex	()
			, bBoneWeight	()
		{}
	};

public:
	CSkinMesh();
	~CSkinMesh();

	HRESULT Init( LPCTSTR fileName );

	// 解放関数.
	HRESULT Release();

	// 描画関数.
	void Render( STransform* pTransform = nullptr, SAnimationController* pAC = nullptr, D3DXMATRIX* pRot = nullptr );

	double GetAnimSpeed()				{ return m_dAnimSpeed;		}
	void SetAnimSpeed( double dSpeed )	{ m_dAnimSpeed = dSpeed;	}

	double GetBlendSpeed()				{ return m_dBlendSpeed;		}
	void SetBlendSpeed( double dSpeed )	{ m_dBlendSpeed = dSpeed;	}

	double GetAnimTime()				{ return m_dAnimTime;		}
	void SetAnimTime( double dTime )	{ m_dAnimTime = dTime;		}

	// メッシュに対してのポイントライトの強さの設定.
	void	SetPointLightIntensity( float Intensity ) { m_PointLightIntensity = Intensity; }
	float	GetPointLightIntensity() { return m_PointLightIntensity; }

	// パーサークラスからアニメーションコントローラを取得する.
	LPD3DXANIMATIONCONTROLLER GetAnimationController() { return m_pD3dxMesh->m_pAnimController; }

	// アニメーションセットの切り替え.
	void ChangeAnimSet( int index, SAnimationController* pAC=nullptr );
	// アニメーションセットの切り替え(開始フレーム指定可能版).
	void ChangeAnimSet_StartPos( int index, double dStartFramePos, SAnimationController* pAC=nullptr );
	// アニメーションをブレンドして切り替え.
	void ChangeAnimBlend( int index, int oldIndex, SAnimationController* pAC = nullptr );
	void ChangeAnimBlend( int index, SAnimationController* pAC = nullptr );
	// アニメーションをブレンドして切り替え(他のブレンドをしていても行う).
	void ChangeAnimBlend_Skip( int index, int oldIndex, SAnimationController* pAC = nullptr );
	void ChangeAnimBlend_Skip( int index, SAnimationController* pAC = nullptr );

	// アニメーション停止時間を取得.
	double GetAnimPeriod( int index );
	// アニメーション数を取得.
	int GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC=nullptr );
	// メッシュを取得.
	LPD3DXMESH GetMesh() const { return m_pMeshForRay; }

	// 指定したボーン情報(座標・行列)を取得する関数.
	D3DXMATRIX GetMatrixFromBone( LPCSTR sBoneName );
	bool GetMatrixFromBone( LPCSTR sBoneName, D3DXMATRIX* pOutMat );
	D3DXVECTOR3 GetPosFromBone( LPCSTR sBoneName );
	bool GetPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos);
	D3DXVECTOR3 GetDeviaPosFromBone( LPCSTR sBoneName, D3DXVECTOR3 vSpecifiedPos = { 0.0f, 0.0f, 0.0f });
	bool GetDeviaPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos, D3DXVECTOR3 vSpecifiedPos = { 0.0f, 0.0f, 0.0f });

	// フォグの有効/無効.
	inline void SetIsFog( const bool& Flag ) { m_IsFog = Flag; }
	// ディザ抜きを使用するか.
	inline void SetDither( const bool& Flag ) { m_DitherFlag = Flag; }
	// アルファブロックを使用するか.
	inline void SetAlphaBlock( const bool& Flag ) { m_AlphaBlockFlag = Flag; }
	// アルファブレンドを使用するか.
	inline void SetAlphaBlend( const bool& Flag ) { DirectX11::SetAlphaBlend( Flag ); }
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
	void SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv );
	// x方向にUVスクロールする値を設定.
	void SetUVScrollX( const int MaterialNo, const float uv );
	// y方向にUVスクロールする値を設定.
	void SetUVScrollY( const int MaterialNo, const float uv );

private:
	// Xファイルからスキンメッシュを作成する.
	HRESULT LoadXMesh( LPCTSTR lpFileName );

	// シェーダを作成する.
	HRESULT CreateShader();
	// インデックスバッファを作成する.
	HRESULT CreateIndexBuffer( DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer );
//	void RecursiveSetNewPoseMatrices( BONE* pBone,D3DXMATRIX* pmParent );

	// メッシュを作成する.
	HRESULT CreateAppMeshFromD3DXMesh( LPD3DXFRAME pFrame );

	// コンスタントバッファ作成する.
	HRESULT CreateCBuffer( ID3D11Buffer** pConstantBuffer, UINT size );
	// サンプラを作成する.
	HRESULT CreateSampler( ID3D11SamplerState** pSampler );

	// 全てのメッシュを作成する.
	void BuildAllMesh( D3DXFRAME* pFrame );

	// Xファイルからスキン関連の情報を読み出す関数.
	HRESULT ReadSkinInfo( MYMESHCONTAINER* pContainer, VERTEX* pvVB, SKIN_PARTS_MESH* pParts );

	// ボーンを次のポーズ位置にセットする関数.
	void SetNewPoseMatrices( SKIN_PARTS_MESH* pParts, int frame, MYMESHCONTAINER* pContainer );
	// 次の(現在の)ポーズ行列を返す関数.
	D3DXMATRIX GetCurrentPoseMatrix( SKIN_PARTS_MESH* pParts, int index );

	// フレーム描画.
	void DrawFrame( STransform* pTransform, LPD3DXFRAME pFrame );
	// パーツ描画.
	void DrawPartsMesh( STransform* pTransform, SKIN_PARTS_MESH* p, D3DXMATRIX World, MYMESHCONTAINER* pContainer );

	// アニメーションの更新.
	void AnimUpdate( SAnimationController* pAC );
	// ブレンドアニメーションの更新.
	void BlendAnimUpdate();

	// 全てのメッシュを削除.
	void DestroyAllMesh( D3DXFRAME* pFrame );
	HRESULT DestroyAppMeshFromD3DXMesh( LPD3DXFRAME p );

	// マルチバイト文字をUnicode文字に変換する.
	void ConvertCharaMultiByteToUnicode( WCHAR* Dest, size_t DestArraySize, const CHAR* str );

	// UVの変更.
	void ChangeUV( const int MaterialNo, const D3DXVECTOR2& uv, LPD3DXFRAME p );
	// x方向のUVの変更.
	void ChangeUVX( const int MaterialNo, const float uv, LPD3DXFRAME p );
	// y方向のUVの変更.
	void ChangeUVY( const int MaterialNo, const float uv, LPD3DXFRAME p );

private:
	ID3D11SamplerState*		m_pSampleLinear;
	ID3D11VertexShader*		m_pVertexShader;
	ID3D11PixelShader*		m_pPixelShader;
	ID3D11InputLayout*		m_pVertexLayout;
	ID3D11Buffer*			m_pCBufferPerMesh;		// コンスタントバッファ(メッシュ毎).
	ID3D11Buffer*			m_pCBufferPerMaterial;	// コンスタントバッファ(マテリアル毎).
	ID3D11Buffer*			m_pCBufferPerFrame;		// コンスタントバッファ(フレーム毎).
	ID3D11Buffer*			m_pCBufferPerBone;		// コンスタントバッファ(ボーン毎).

	D3DXMATRIX				m_mWorld;
	D3DXMATRIX				m_mRotation;

	D3DXMATRIX				m_mView;
	D3DXMATRIX				m_mProj;

	D3DXMATRIX*				m_pRotMatrix;

	D3DXVECTOR3				m_vLight;
	D3DXPOSITION3			m_vCamPos;

	double					m_dAnimSpeed;					// アニメーション速度.
	double					m_dBlendSpeed;					// アニメーションブレンド速度.
	double					m_dAnimTime;					// アニメーション速度.

	bool					m_IsChangeAnim;

	D3DXPARSER*				m_pD3dxMesh;					// メッシュ.
	LPD3DXMESH				m_pMeshForRay;					// ﾚｲとﾒｯｼｭ用.

	TCHAR					m_FilePath[256];				// XFileのパス.

	int						m_iFrame;						// アニメーションフレーム.

	float					m_PointLightIntensity;			// メッシュに足してのポイントライトの強さ.

	std::unique_ptr<CFog>	m_pFog;							// フォグ.
	bool					m_IsFog;						// フォグを有効にするか.
	bool					m_DitherFlag;					// ディザ抜きを使用するか.
	bool					m_AlphaBlockFlag;				// アルファブロックを使用するか.
	bool					m_OrLessDistanceAlphaFlag;		// 距離で透明にするか(以下).
	bool					m_OrMoreDistanceAlphaFlag;		// 距離で透明にするか(以上).

	float					m_OrLessStartDistance;			// 開始する距離(以下).
	float					m_OrMoreStartDistance;			// 開始する距離(以上).
	float					m_AlphaDistance;				// 透明にしていく間隔.

	int						m_NowIndex;						// 現在のアニメーションNo.
	int						m_NewIndex;						// 次のアニメーションNo.

	std::vector<int>		m_MaterialTextureNo;			// マテリアルのテクスチャのNo.
};
