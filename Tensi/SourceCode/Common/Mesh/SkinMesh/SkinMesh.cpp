/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode対応.
**/
#include "SkinMesh.h"
#include "..\..\DirectX\DirectX9.h"
#include "..\..\Fog\Fog.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Light\Light.h"
#include "..\..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

#include <stdlib.h>	// マルチバイト文字→Unicode文字変換で必要.
#include <locale.h>
#include <crtdbg.h>
#include <filesystem>

// シェーダ名(ディレクトリも含む)
const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\SkinMesh.hlsl" );

CSkinMesh::CSkinMesh()
	: m_pSampleLinear			( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pCBufferPerMesh			( nullptr )
	, m_pCBufferPerMaterial		( nullptr )
	, m_pCBufferPerFrame		( nullptr )
	, m_pCBufferPerBone			( nullptr )
	, m_mWorld					()
	, m_mRotation				()
	, m_mView					()
	, m_mProj					()
	, m_pRotMatrix				( nullptr )
	, m_vLight					()
	, m_vCamPos					()
	, m_dAnimSpeed				( 0.001f )	// 一先ず、この値.
	, m_dBlendSpeed				( 0.003f )	// 一先ず、この値.
	, m_dAnimTime				()
	, m_IsChangeAnim			( false )
	, m_pD3dxMesh				( nullptr )
	, m_pMeshForRay				( nullptr )
	, m_FilePath				()
	, m_iFrame					()
	, m_PointLightIntensity		( 3.0f )
	, m_pFog					( nullptr )
	, m_IsFog					( false )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_OrLessDistanceAlphaFlag	( false )
	, m_OrMoreDistanceAlphaFlag	( false )
	, m_OrLessStartDistance		( 0.0f )
	, m_OrMoreStartDistance		( 0.0f )
	, m_AlphaDistance			( 0.0f )
	, m_NowIndex				( 0 )
	, m_NewIndex				( 0 )
	, m_MaterialTextureNo		()
{
	m_pFog = std::make_unique<CFog>();
}

CSkinMesh::~CSkinMesh()
{
	// 解放処理.
	Release();

	// シェーダやサンプラ関係.
	SAFE_RELEASE( m_pSampleLinear		);
	SAFE_RELEASE( m_pVertexShader		);
	SAFE_RELEASE( m_pPixelShader		);
	SAFE_RELEASE( m_pVertexLayout		);

	// コンスタントバッファ関係.
	SAFE_RELEASE( m_pCBufferPerBone		);
	SAFE_RELEASE( m_pCBufferPerFrame	);
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh		);

	SAFE_RELEASE( m_pD3dxMesh			);
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT CSkinMesh::Init( LPCTSTR fileName )
{
	const std::wstring	wFileName	= fileName;
	const std::string	FileName	= StringConversion::to_String( wFileName );

	// モデル読み込み.
	if( FAILED( LoadXMesh( fileName ) )														) return E_FAIL;
	// シェーダの作成.
	if( FAILED( CreateShader() )															) return E_FAIL;
	// コンスタントバッファ(メッシュごと).
	if( FAILED( CreateCBuffer( &m_pCBufferPerMesh, sizeof( CBUFFER_PER_MESH ) ) )			) return E_FAIL;
	// コンスタントバッファ(メッシュごと).
	if( FAILED( CreateCBuffer( &m_pCBufferPerMaterial, sizeof( CBUFFER_PER_MATERIAL ) ) )	) return E_FAIL;
	// コンスタントバッファ(メッシュごと).
	if( FAILED( CreateCBuffer( &m_pCBufferPerFrame, sizeof( CBUFFER_PER_FRAME ) ) )			) return E_FAIL;
	// コンスタントバッファ(メッシュごと).
	if( FAILED( CreateCBuffer( &m_pCBufferPerBone, sizeof( CBUFFER_PER_BONES ) ) )			) return E_FAIL;
	// テクスチャー用サンプラー作成.
	if( FAILED( CreateSampler( &m_pSampleLinear ) )											) return E_FAIL;

	// ボーン座標を設定する.
	D3DXMATRIX m;
	D3DXMatrixIdentity( &m );
	m_pD3dxMesh->UpdateFrameMatrices( m_pD3dxMesh->m_pFrameRoot, &m );

	// フォグの初期化.
	m_pFog->Init();

	return S_OK;
}

//----------------------------.
// シェーダ初期化.
//----------------------------.
HRESULT	CSkinMesh::CreateShader()
{
	// D3D11関連の初期化
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	if( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME, nullptr, nullptr,
			"VS_Main", "vs_5_0",
			uCompileFlag, 0, nullptr,
			&pCompiledShader, &pErrors, nullptr ) ) )
	{
		int size = static_cast<int>( pErrors->GetBufferSize() );
//		char* ch = (char*)pErrors->GetBufferPointer();
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE( pErrors );

	// 上記で作成したブロブからバーテックスシェーダを作成.
	if( FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr, &m_pVertexShader ) ) )
	{
		SAFE_RELEASE(pCompiledShader);
		ErrorMessage( "バーテックスシェーダ作成失敗" );
		return E_FAIL;
	}
	// 頂点インプットレイアウトを定義	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT,	0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT",0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// 頂点インプットレイアウトを作成
	if( FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout, numElements, pCompiledShader, &m_pVertexLayout ) ) )
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return E_FAIL;
	}
	// 頂点インプットレイアウトをセット
	DirectX11::GetContext()->IASetInputLayout( m_pVertexLayout );

	// HLSLからピクセルシェーダのブロブを作成.
	if(FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME, nullptr, nullptr,
			"PS_Main", "ps_5_0",
			uCompileFlag, 0, nullptr,
			&pCompiledShader, &pErrors, nullptr ) ) )
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE( pErrors );
	// 上記で作成したブロブからピクセルシェーダを作成.
	if( FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr, &m_pPixelShader ) ) )
	{
		SAFE_RELEASE( pCompiledShader );
		ErrorMessage( "ピクセルシェーダ作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE( pCompiledShader );

	return S_OK;
}

//----------------------------.
// Xファイルからスキン関連の情報を読み出す関数.
//----------------------------.
HRESULT CSkinMesh::ReadSkinInfo(
	MYMESHCONTAINER* pContainer, VERTEX* pvVB, SKIN_PARTS_MESH* pParts )
{
	// Xファイルから抽出すべき情報は、
	//	「頂点ごとのボーンインデックス」「頂点ごとのボーンウェイト」.
	//	「バインド行列」「ポーズ行列」の4項目.

	int i, k, m, n;			// 各種ループ変数.
	int iNumVertex	= 0;	// 頂点数.
	int iNumBone	= 0;	// ボーン数.

	// 頂点数.
	iNumVertex	= m_pD3dxMesh->GetNumVertices( pContainer );
	// ボーン数.
	iNumBone	= m_pD3dxMesh->GetNumBones( pContainer );

	// それぞれのボーンに影響を受ける頂点を調べる.
	//	そこから逆に、頂点ベースでボーンインデックス・重みを整頓する.
	for( i=0; i<iNumBone; i++ )
	{
		// このボーンに影響を受ける頂点数.
		int iNumIndex
			= m_pD3dxMesh->GetNumBoneVertices( pContainer, i );

		int*	pIndex = new int[iNumIndex]();
		double*	pWeight = new double[iNumIndex]();

		for( k=0; k<iNumIndex; k++ )
		{
			pIndex[k]
				= m_pD3dxMesh->GetBoneVerticesIndices( pContainer, i, k );
			pWeight[k]
				= m_pD3dxMesh->GetBoneVerticesWeights( pContainer, i, k );
		}

		// 頂点側からインデックスをたどって、頂点サイドで整理する.
		for( k=0; k<iNumIndex; k++ )
		{
			// XファイルやCGソフトがボーン4本以内とは限らない.
			//	5本以上の場合は、重みの大きい順に4本に絞る.

			// ウェイトの大きさ順にソート(バブルソート).
			for( m=4; m>1; m-- )
			{
				for( n=1; n<m; n++ )
				{
					if( pvVB[pIndex[k]].bBoneWeight[n-1] < pvVB[pIndex[k]].bBoneWeight[n] )
					{
						float tmp = pvVB[pIndex[k]].bBoneWeight[n - 1];
						pvVB[pIndex[k]].bBoneWeight[n - 1]	= pvVB[pIndex[k]].bBoneWeight[n];
						pvVB[pIndex[k]].bBoneWeight[n]		= tmp;
						int itmp = pvVB[pIndex[k]].bBoneIndex[n - 1];
						pvVB[pIndex[k]].bBoneIndex[n - 1]	= pvVB[pIndex[k]].bBoneIndex[n];
						pvVB[pIndex[k]].bBoneIndex[n]		= itmp;
					}
				}
			}
			// ソート後は、最後の要素に一番小さいウェイトが入ってるはず.
			bool flag = false;
			for( m=0; m<4; m++ )
			{
				if( pvVB[pIndex[k]].bBoneWeight[m] == 0 )
				{
					flag = true;
					pvVB[pIndex[k]].bBoneIndex[ m] = i;
					pvVB[pIndex[k]].bBoneWeight[m] = (float)pWeight[k];
					break;
				}
			}
			if( flag == false )
			{
				pvVB[pIndex[k]].bBoneIndex[ 3] = i;
				pvVB[pIndex[k]].bBoneWeight[3] = (float)pWeight[k];
				break;
			}
		}
		// 使い終われば削除する.
		delete[] pIndex;
		delete[] pWeight;
	}

	// ボーン生成.
	pParts->iNumBone	= iNumBone;
	pParts->pBoneArray	= new BONE[iNumBone]();
	// ポーズ行列を得る(初期ポーズ).
	for( i=0; i<pParts->iNumBone; i++ )
	{
		pParts->pBoneArray[i].mBindPose
			= m_pD3dxMesh->GetBindPose( pContainer, i );
	}

	return S_OK;
}

//----------------------------.
// UVスクロールする値を設定.
//----------------------------.
void CSkinMesh::SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv )
{
	ChangeUV( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// x方向にUVスクロールする値を設定.
//----------------------------.
void CSkinMesh::SetUVScrollX( const int MaterialNo, const float uv )
{
	ChangeUVX( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// y方向にUVスクロールする値を設定.
//----------------------------.
void CSkinMesh::SetUVScrollY( const int MaterialNo, const float uv )
{
	ChangeUVY( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// Xからスキンメッシュを作成する.
//	注意）素材（X)のほうは、三角ポリゴンにすること.
//----------------------------.
HRESULT CSkinMesh::LoadXMesh( LPCTSTR lpFileName )
{
	// ファイル名をパスごと取得.
	lstrcpy( m_FilePath, lpFileName );

	// Xファイル読み込み.
	m_pD3dxMesh = new D3DXPARSER();
	m_pD3dxMesh->LoadMeshFromX( DirectX9::GetDevice(), lpFileName );


	// 全てのメッシュを作成する.
	BuildAllMesh( m_pD3dxMesh->m_pFrameRoot );

	return S_OK;
}

//----------------------------.
// Direct3Dのインデックスバッファー作成.
//----------------------------.
HRESULT CSkinMesh::CreateIndexBuffer(
	DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer )
{
	D3D11_BUFFER_DESC bd;
	bd.Usage					= D3D11_USAGE_DEFAULT;
	bd.ByteWidth				= dwSize;
	bd.BindFlags				= D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags			= 0;
	bd.MiscFlags				= 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem			= pIndex;
	InitData.SysMemPitch		= 0;
	InitData.SysMemSlicePitch	= 0;
	if( FAILED( m_pDevice->CreateBuffer( &bd, &InitData, ppIndexBuffer ) ) )
	{
		return E_FAIL;
	}
	
	return S_OK;
}

//----------------------------.
// レンダリング.
//----------------------------.
void CSkinMesh::Render( STransform* pTransform, SAnimationController* pAC, D3DXMATRIX* pRot )
{
	STransform* Transform = &m_Transform;
	if ( pTransform != nullptr ) {
		m_Transform = *pTransform;
		Transform	= pTransform;
	}
#ifdef _DEBUG
	PositionRenderer::Render( *Transform );
#endif
	if ( m_Color.w == 0.0f ) return;

	m_pRotMatrix = pRot;

	m_mView		= CameraManager::GetViewMatrix();
	m_mProj		= CameraManager::GetProjMatrix();
	m_vCamPos	= CameraManager::GetPosition();
	m_vLight	= Light::GetDirection();

	// アニメーションの更新.
	AnimUpdate( pAC );

	D3DXMATRIX m;
	D3DXMatrixIdentity( &m );
	m_pD3dxMesh->UpdateFrameMatrices( m_pD3dxMesh->m_pFrameRoot, &m );
	DrawFrame( Transform, m_pD3dxMesh->m_pFrameRoot );

}

//----------------------------.
// 全てのメッシュを作成する.
//----------------------------.
void CSkinMesh::BuildAllMesh( D3DXFRAME* pFrame )
{
	if( pFrame && pFrame->pMeshContainer ) {
		CreateAppMeshFromD3DXMesh( pFrame );
	}

	// 再帰関数.
	if( pFrame->pFrameSibling != nullptr ) {
		BuildAllMesh( pFrame->pFrameSibling );
	}
	if( pFrame->pFrameFirstChild != nullptr ) {
		BuildAllMesh( pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// メッシュ作成.
//----------------------------.
HRESULT CSkinMesh::CreateAppMeshFromD3DXMesh( LPD3DXFRAME p )
{
	MYFRAME* pFrame	= (MYFRAME*)p;

	//LPD3DXMESH pD3DXMesh = pFrame->pMeshContainer->MeshData.pMesh;//D3DXメッシュ(ここから・・・).
	m_pMeshForRay = pFrame->pMeshContainer->MeshData.pMesh; //D3DXﾒｯｼｭ(ここから・・・).
	MYMESHCONTAINER* pContainer = (MYMESHCONTAINER*)pFrame->pMeshContainer;

	// アプリメッシュ(・・・ここにメッシュデータをコピーする).
	SKIN_PARTS_MESH* pAppMesh = new SKIN_PARTS_MESH();
	pAppMesh->bTex = false;

	// 事前に頂点数、ポリゴン数等を調べる.
	pAppMesh->dwNumVert	= m_pD3dxMesh->GetNumVertices( pContainer );
	pAppMesh->dwNumFace	= m_pD3dxMesh->GetNumFaces( pContainer );
	pAppMesh->dwNumUV	= m_pD3dxMesh->GetNumUVs( pContainer );
	// Direct3DではUVの数だけ頂点が必要.
	if( pAppMesh->dwNumVert < pAppMesh->dwNumUV ){
		// 共有頂点等で、頂点が足りないとき.
		ErrorMessage( "Direct3Dは、UVの数だけ頂点が必要です(UVを置く場所が必要です)テクスチャは正しく貼られないと思われます" );
		return E_FAIL;
	}
	// 一時的なメモリ確保(頂点バッファとインデックスバッファ).
	VERTEX* pvVB = new VERTEX[pAppMesh->dwNumVert]();
	int* piFaceBuffer	= new int[pAppMesh->dwNumFace*3]();
	// 3頂点ポリゴンなので、1フェイス=3頂点(3インデックス).

	// 頂点読み込み.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR3 v	= m_pD3dxMesh->GetVertexCoord( pContainer, i );
		pvVB[i].vPos.x	= v.x;
		pvVB[i].vPos.y	= v.y;
		pvVB[i].vPos.z	= v.z;
	}
	// ポリゴン情報(頂点インデックス)読み込み.
	for( DWORD i=0; i<pAppMesh->dwNumFace*3; i++ ){
		piFaceBuffer[i] = m_pD3dxMesh->GetIndex( pContainer, i );
	}
	// 法線読み込み.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR3 v	= m_pD3dxMesh->GetNormal( pContainer, i );
		pvVB[i].vNorm.x	= v.x;
		pvVB[i].vNorm.y	= v.y;
		pvVB[i].vNorm.z	= v.z;
	}
	// テクスチャ座標読み込み.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR2 v	= m_pD3dxMesh->GetUV( pContainer, i );
		pvVB[i].vTex.x	= v.x;
		pvVB[i].vTex.y	= v.y;
	}

	// マテリアル読み込み.
	pAppMesh->dwNumMaterial	= m_pD3dxMesh->GetNumMaterials( pContainer );
	pAppMesh->pMaterial		= new MY_SKINMATERIAL[pAppMesh->dwNumMaterial]();
	m_MaterialTextureNo.resize( pAppMesh->dwNumMaterial );

	// マテリアルの数だけインデックスバッファを作成.
	pAppMesh->ppIndexBuffer = new ID3D11Buffer*[pAppMesh->dwNumMaterial]();
	// 掛け算ではなく「ID3D11Buffer*」の配列という意味.
	for( DWORD i=0; i<pAppMesh->dwNumMaterial; i++ )
	{
		// 環境光(アンビエント).
		pAppMesh->pMaterial[i].Ka.x	= m_pD3dxMesh->GetAmbient( pContainer, i ).y;
		pAppMesh->pMaterial[i].Ka.y	= m_pD3dxMesh->GetAmbient( pContainer, i ).z;
		pAppMesh->pMaterial[i].Ka.z	= m_pD3dxMesh->GetAmbient( pContainer, i ).w;
		pAppMesh->pMaterial[i].Ka.w	= m_pD3dxMesh->GetAmbient( pContainer, i ).x;
		// 拡散反射光(ディフューズ).
		pAppMesh->pMaterial[i].Kd.x	= m_pD3dxMesh->GetDiffuse( pContainer, i ).y;
		pAppMesh->pMaterial[i].Kd.y	= m_pD3dxMesh->GetDiffuse( pContainer, i ).z;
		pAppMesh->pMaterial[i].Kd.z	= m_pD3dxMesh->GetDiffuse( pContainer, i ).w;
		pAppMesh->pMaterial[i].Kd.w	= m_pD3dxMesh->GetDiffuse( pContainer, i ).x;
		// 鏡面反射光(スペキュラ).
		pAppMesh->pMaterial[i].Ks.x	= m_pD3dxMesh->GetSpecular( pContainer, i ).y;
		pAppMesh->pMaterial[i].Ks.y	= m_pD3dxMesh->GetSpecular( pContainer, i ).z;
		pAppMesh->pMaterial[i].Ks.z	= m_pD3dxMesh->GetSpecular( pContainer, i ).w;
		pAppMesh->pMaterial[i].Ks.w	= m_pD3dxMesh->GetSpecular( pContainer, i ).x;

#if 0
		// テクスチャ(ディフューズテクスチャのみ).
#ifdef UNICODE
		WCHAR TexFilename_w[32] = L"";
		// テクスチャ名のサイズを取得.
		size_t charSize = strlen( m_pD3dxMesh->GetTexturePath( pContainer, i ) ) + 1;
		size_t ret;	// 変換された文字数.

		// マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
		errno_t err = mbstowcs_s(
			&ret,
			TexFilename_w,
			charSize,
			m_pD3dxMesh->GetTexturePath( pContainer, i ),
			_TRUNCATE );

		LPTSTR name = TexFilename_w;
#else// #ifdef UNICODE
		LPTSTR name = d3dxMaterials[No].pTextureFilename;
#endif// #ifdef UNICODE
#endif

		// テクスチャ(ディフューズテクスチャのみ).
#ifdef UNICODE
		WCHAR TextureName_w[32] = L"";
		// 文字変換.
		ConvertCharaMultiByteToUnicode( TextureName_w, 32, m_pD3dxMesh->GetTexturePath( pContainer, i ));
		LPTSTR name = TextureName_w;
#else// #ifdef UNICODE
		LPTSTR name = m_pD3dxMesh->GetTexturePath( pContainer, i );
#endif// #ifdef UNICODE

		if( name != nullptr ){
			LPTSTR ret = _tcsrchr( m_FilePath, _T( '\\' ) );
			if( ret != nullptr ){
				int check = static_cast<int>( ret - m_FilePath );
				TCHAR path[512];
				lstrcpy( path, m_FilePath );
				path[check + 1] = '\0';

				// パスのコピー.
				lstrcpy( pAppMesh->pMaterial[i].TextureName, path );
				// テクスチャファイル名を連結.
				lstrcat( pAppMesh->pMaterial[i].TextureName, name );
			}
		}
		// テクスチャを作成.
		if ( pAppMesh->pMaterial[i].TextureName != nullptr ) {
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
					pAppMesh->pMaterial[i].TextureName,
					nullptr, nullptr, &pAppMesh->pMaterial[i].pTexture[0], nullptr ) ) )
			{
				ErrorMessage( "テクスチャ作成失敗" );
				return E_FAIL;
			}

			// 差し替えテクスチャを取得.
			int TexNo = 1;
			while ( 1 ) {
				// 差し替えテクスチャパスの作成.
				std::string TexName	 = StringConversion::to_String( pAppMesh->pMaterial[i].TextureName );
				size_t		TexIndex = TexName.find( "." );
				TexName.insert( TexIndex, "_Tex" + std::to_string( TexNo ) );

				// テクスチャを作成できるか.
				ID3D11ShaderResourceView* pTexture = nullptr;
				if ( FAILED(
					DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
					StringConversion::to_wString( TexName ).c_str(),
					nullptr, nullptr, &pTexture, nullptr ) ) )
				{
					// 作成できなかったため差し替えテクスチャの作成を終了する.
					break;
				}

				// 次のテクスチャがあるか確かめる.
				TexNo++;
				pAppMesh->pMaterial[i].pTexture.emplace_back( pTexture );
			}

			// ライトテクスチャパスの作成.
			std::string LightTexName = StringConversion::to_String( pAppMesh->pMaterial[i].TextureName );
			size_t		LightIndex	 = LightTexName.find( "." );
			LightTexName.insert( LightIndex, "_light" );

			// ライトテクスチャを作成できるか.
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				StringConversion::to_wString( LightTexName ).c_str(),
				nullptr, nullptr, &pAppMesh->pMaterial[i].pLightTexture, nullptr ) ) )
			{
				pAppMesh->pMaterial[i].pLightTexture = nullptr;
			}
		}
		// そのマテリアルであるインデックス配列内の開始インデックスを調べる.
		//	さらにインデックスの個数を調べる.
		int iCount = 0;
		int* pIndex = new int[pAppMesh->dwNumFace*3]();
			// とりあえず、メッシュ内のポリゴン数でメモリ確保.
			//	(ここのぽりごんぐるーぷは必ずこれ以下になる).

		for( DWORD k=0; k<pAppMesh->dwNumFace; k++ )
		{
			// もし i==k 番目のポリゴンのマテリアル番号なら.
			if( i == m_pD3dxMesh->GeFaceMaterialIndex( pContainer, k ) )
			{
				// k番目のポリゴンを作る頂点のインデックス.
				pIndex[iCount]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 0 );	// 1個目.
				pIndex[iCount+1]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 1 );	// 2個目.
				pIndex[iCount+2]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 2 );	// 3個目.
				iCount += 3;
			}
		}
		if( iCount > 0 ){
			// インデックスバッファ作成.
			CreateIndexBuffer( iCount*sizeof(int),
				pIndex, &pAppMesh->ppIndexBuffer[i] );
		}
		else{
			// 解放時の処理に不具合が出たため.
			//	カウント数が0以下の場合は、インデックスバッファを nullptr にしておく.
			pAppMesh->ppIndexBuffer[i] = nullptr;
		}

		// そのマテリアル内のポリゴン数.
		pAppMesh->pMaterial[i].dwNumFace = iCount / 3;
		// 不要になったので削除.
		delete[] pIndex;
	}

	// スキン情報ある？
	if( pContainer->pSkinInfo == nullptr ){
#ifdef _DEBUG
		// 不明なスキンあればここで教える.不要ならコメントアウトしてください.
		TCHAR strDbg[128];
		WCHAR str[64] = L"";
		ConvertCharaMultiByteToUnicode( str, 64, pContainer->Name );
		_stprintf( strDbg, _T( "ContainerName:[%s]" ), str );
		MessageBox( nullptr, strDbg, _T( "Not SkinInfo" ), MB_OK );
#endif// #ifdef _DEBUG
		pAppMesh->bEnableBones = false;
	}
	else{
		// スキン情報(ジョイント、ウェイト)読み込み.
		ReadSkinInfo( pContainer, pvVB, pAppMesh );
	}

	// バーテックスバッファを作成.
	D3D11_BUFFER_DESC bd;
	bd.Usage	= D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( VERTEX ) * ( pAppMesh->dwNumVert );
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags= 0;
	bd.MiscFlags	= 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pvVB;

	HRESULT hRslt = S_OK;
	if( FAILED(
		m_pDevice->CreateBuffer(
			&bd, &InitData, &pAppMesh->pVertexBuffer ) ) )
	{
		hRslt = E_FAIL;
	}

	// パーツメッシュに設定.
	pFrame->pPartsMesh = pAppMesh;
//	m_pReleaseMaterial = pAppMesh;

	// 一時的な入れ物は不要なるので削除.
	if( piFaceBuffer ){
		delete[] piFaceBuffer;
	}
	if( pvVB ){
		delete[] pvVB;
	}

	return hRslt;
}

//----------------------------.
//ボーンを次のポーズ位置にセットする関数.
//----------------------------.
void CSkinMesh::SetNewPoseMatrices(
	SKIN_PARTS_MESH* pParts, int frame, MYMESHCONTAINER* pContainer )
{
	// 望むフレームでUpdateすること.
	//	しないと行列が更新されない.
	//	m_pD3dxMesh->UpdateFrameMatrices(
	//	m_pD3dxMesh->m_pFrameRoot)をレンダリング時に実行すること.

	// また、アニメーション時間に見合った行列を更新するのはD3DXMESHでは
	//	アニメーションコントローラが(裏で)やってくれるものなので、
	//	アニメーションコントローラを使ってアニメを進行させることも必要.
	//	m_pD3dxMesh->m_pAnimController->AdvanceTime(...)を.
	//	レンダリング時に実行すること.

	if( pParts->iNumBone <= 0 ){
		// ボーンが 0　以下の場合.
	}

	for( int i=0; i<pParts->iNumBone; i++ )
	{
		pParts->pBoneArray[i].mNewPose
			= m_pD3dxMesh->GetNewPose( pContainer, i );
	}
}

//----------------------------.
// 次の(現在の)ポーズ行列を返す関数.
//----------------------------.
D3DXMATRIX CSkinMesh::GetCurrentPoseMatrix( SKIN_PARTS_MESH* pParts, int index )
{
	D3DXMATRIX ret =
		pParts->pBoneArray[index].mBindPose * pParts->pBoneArray[index].mNewPose;
	return ret;
}

//----------------------------.
// フレームの描画.
//----------------------------.
VOID CSkinMesh::DrawFrame( STransform* pTransform, LPD3DXFRAME p )
{
	MYFRAME*			pFrame	= (MYFRAME*)p;
	SKIN_PARTS_MESH*	pPartsMesh	= pFrame->pPartsMesh;
	MYMESHCONTAINER*	pContainer	= (MYMESHCONTAINER*)pFrame->pMeshContainer;

	if( pPartsMesh != nullptr )
	{
		DrawPartsMesh(
			pTransform,
			pPartsMesh, 
			pFrame->CombinedTransformationMatrix,
			pContainer );
	}

	// 再帰関数(兄弟).
	if( pFrame->pFrameSibling != nullptr )
	{
		DrawFrame( pTransform, pFrame->pFrameSibling );
	}
	// (親子)
	if( pFrame->pFrameFirstChild != nullptr )
	{
		DrawFrame( pTransform, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// パーツメッシュを描画.
//----------------------------.
void CSkinMesh::DrawPartsMesh(
	STransform* pTransform, SKIN_PARTS_MESH* pMesh, D3DXMATRIX World, MYMESHCONTAINER* pContainer )
{
	D3D11_MAPPED_SUBRESOURCE pData;

	// 使用するシェーダのセット.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader, nullptr, 0 );

	// ワールド行列の取得.
	m_mWorld = pTransform->GetWorldMatrix( m_pRotMatrix );

	// アニメーションフレームを進める スキンを更新.
	m_iFrame++;
	if( m_iFrame >= 3600 ){
		m_iFrame = 0;
	}
	SetNewPoseMatrices( pMesh, m_iFrame, pContainer );

	//------------------------------------------------.
	//	コンスタントバッファに情報を送る(ボーン).
	//------------------------------------------------.
	if( SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerBone, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		CBUFFER_PER_BONES cb;
		for( int i=0; i<pMesh->iNumBone; i++ )
		{
			D3DXMATRIX mat = GetCurrentPoseMatrix( pMesh, i );
			D3DXMatrixTranspose( &mat, &mat );
			cb.mBone[i] = mat;
		}
		memcpy_s( pData.pData, pData.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerBone, 0 );
	}
	m_pContext->VSSetConstantBuffers( 3, 1, &m_pCBufferPerBone );
	m_pContext->PSSetConstantBuffers( 3, 1, &m_pCBufferPerBone );
	
	// バーテックスバッファをセット.
	UINT stride = sizeof( VERTEX );
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0, 1, &pMesh->pVertexBuffer, &stride, & offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout(	m_pVertexLayout );

	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//------------------------------------------------.
	//	コンスタントバッファに情報を設定(フレームごと).
	//------------------------------------------------.
	if( SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerFrame, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		CBUFFER_PER_FRAME cb;
		cb.vCameraPos				= D3DXVECTOR4( m_vCamPos.x, m_vCamPos.y, m_vCamPos.z, 0.0f );
		cb.vLightDir				= D3DXVECTOR4( m_vLight.x, m_vLight.y, m_vLight.z, 0.0f );
		cb.vLightIntensity.x		= Light::GetIntensity();
		cb.vPointLightIntensity.x	= m_PointLightIntensity;
		cb.vFlag.x					= m_DitherFlag				== true ? 1.0f : 0.0f;
		cb.vFlag.y					= m_AlphaBlockFlag			== true ? 1.0f : 0.0f;
		cb.vFlag.z					= m_OrLessDistanceAlphaFlag	== true ? 1.0f : 0.0f;
		cb.vFlag.w					= m_OrMoreDistanceAlphaFlag	== true ? 1.0f : 0.0f;
		cb.vAlphaDistance.x			= m_OrLessStartDistance;
		cb.vAlphaDistance.y			= m_OrMoreStartDistance;
		cb.vAlphaDistance.z			= m_AlphaDistance;
		memcpy_s( pData.pData, pData.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerFrame, 0 );
	}
	m_pContext->VSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );
	m_pContext->PSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );


	//------------------------------------------------.
	//	コンスタントバッファに情報を設定(メッシュごと).
	//------------------------------------------------.
	D3D11_MAPPED_SUBRESOURCE pDat;
	if (SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerMesh, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pDat )))
	{
		CBUFFER_PER_MESH cb;
		cb.mW = m_mWorld;
		D3DXMatrixTranspose( &cb.mW, &cb.mW );
		cb.mWVP = m_mWorld * m_mView * m_mProj;
		D3DXMatrixTranspose( &cb.mWVP, &cb.mWVP );
		cb.vColor = m_Color;
		memcpy_s( pDat.pData, pDat.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerMesh, 0);
	}
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );


	// マテリアルの数だけ、
	//	それぞれのマテリアルのインデックスバッファを描画.
	for( DWORD i=0; i<pMesh->dwNumMaterial; i++ )
	{
		// 使用されていないマテリアル対策.
		if( pMesh->pMaterial[i].dwNumFace == 0 )
		{
			continue;
		}
		// インデックスバッファをセット.
		stride	= sizeof( int );
		offset	= 0;
		m_pContext->IASetIndexBuffer(
			pMesh->ppIndexBuffer[i],
			DXGI_FORMAT_R32_UINT, 0 );

		//------------------------------------------------.
		//	マテリアルの各要素と変換行列をシェーダに渡す.
		//------------------------------------------------.
		D3D11_MAPPED_SUBRESOURCE pDat;
		if( SUCCEEDED(
			m_pContext->Map(
				m_pCBufferPerMaterial, 0,
				D3D11_MAP_WRITE_DISCARD, 0, &pDat )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.vAmbient	= pMesh->pMaterial[i].Ka;
			cb.vDiffuse	= pMesh->pMaterial[i].Kd;
			cb.vSpecular= pMesh->pMaterial[i].Ks;
			cb.vUV		= pMesh->pMaterial[i].UV;
			memcpy_s( pDat.pData, pDat.RowPitch, (void*)&cb, sizeof( cb ) );
			m_pContext->Unmap( m_pCBufferPerMaterial, 0 );
		}
		m_pContext->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );

		// テクスチャをシェーダに渡す.
		if( pMesh->pMaterial[i].TextureName != nullptr )
		{
			m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinear );
			m_pContext->PSSetShaderResources( 0, 1, &pMesh->pMaterial[i].pTexture[m_MaterialTextureNo[i]] );
			m_pContext->PSSetShaderResources( 1, 1, &pMesh->pMaterial[i].pLightTexture );
		}
		else
		{
			ID3D11ShaderResourceView* Nothing[1] = { 0 };
			m_pContext->PSSetShaderResources( 0, 1, Nothing );
		}

		// フォグの描画.
		if ( m_IsFog == true ) m_pFog->Render( pMesh->pMaterial[i].dwNumFace * 3, m_mWorld, pMesh->pMaterial[i].pTexture[m_MaterialTextureNo[i]], 0.0f, 100.0f );

		// Draw.
		m_pContext->DrawIndexed( pMesh->pMaterial[i].dwNumFace * 3, 0, 0 );
	}
}

//----------------------------.
// アニメーションの更新.
//----------------------------.
void CSkinMesh::AnimUpdate( SAnimationController* pAC )
{
	if ( pAC == nullptr ) {
		if ( m_pD3dxMesh->m_pAnimController ) {
			BlendAnimUpdate();
			m_pD3dxMesh->m_pAnimController->AdvanceTime( m_dAnimSpeed, nullptr );
		}
	}
	else {
		pAC->BlendAnimUpdate(  m_dBlendSpeed );
		pAC->pAC->AdvanceTime( m_dAnimSpeed, nullptr );
	}
}

//----------------------------.
// ブレンドアニメーションの更新.
//----------------------------.
void CSkinMesh::BlendAnimUpdate()
{
	// アニメーション切り替えフラグが下りてたら終了.
	if ( m_IsChangeAnim == false ) return;

	m_dAnimTime += m_dAnimSpeed;
	float Weight = static_cast<float>( m_dBlendSpeed ) / 1.0f;			// ウェイトの計算.
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 0, Weight );		// トラック0 にウェイトを設定.
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 1, 1.0f - Weight );	// トラック1 にウェイトを設定.

	if ( m_dAnimTime < 1.0 ) return;
	// アニメーションタイムが一定値に達したら.
	m_IsChangeAnim	= false;	// フラグを下す.
	m_NowIndex		= m_NewIndex;
	m_NewIndex		= -1;
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 0, 1.0f );			// ウェイトを1に固定する.
	m_pD3dxMesh->m_pAnimController->SetTrackEnable( 1, false );			// トラック1を無効にする.
}

//----------------------------.
//解放関数.
//----------------------------.
HRESULT CSkinMesh::Release()
{
	if( m_pD3dxMesh != nullptr ){
		// 全てのメッシュ削除.
		DestroyAllMesh( m_pD3dxMesh->m_pFrameRoot );

		// パーサークラス解放処理.
		m_pD3dxMesh->Release();
		SAFE_DELETE( m_pD3dxMesh );
	}

	return S_OK;
}

//----------------------------.
// 全てのメッシュを削除.
//----------------------------.
void CSkinMesh::DestroyAllMesh( D3DXFRAME* pFrame )
{
	if ((pFrame != nullptr) && (pFrame->pMeshContainer != nullptr))
	{
		DestroyAppMeshFromD3DXMesh( pFrame );
	}

	// 再帰関数.
	if( pFrame->pFrameSibling != nullptr )
	{
		DestroyAllMesh( pFrame->pFrameSibling );
	}
	if( pFrame->pFrameFirstChild != nullptr )
	{
		DestroyAllMesh( pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// メッシュの削除.
//----------------------------.
HRESULT CSkinMesh::DestroyAppMeshFromD3DXMesh( LPD3DXFRAME p )
{
	MYFRAME* pFrame = (MYFRAME*)p;

	MYMESHCONTAINER* pMeshContainerTmp = (MYMESHCONTAINER*)pFrame->pMeshContainer;

	// MYMESHCONTAINERの中身の解放.
	if ( pMeshContainerTmp != nullptr )
	{
		if ( pMeshContainerTmp->pBoneBuffer != nullptr )
		{
			pMeshContainerTmp->pBoneBuffer->Release();
			pMeshContainerTmp->pBoneBuffer = nullptr;
		}

		if ( pMeshContainerTmp->pBoneOffsetMatrices != nullptr )
		{
			delete pMeshContainerTmp->pBoneOffsetMatrices;
			pMeshContainerTmp->pBoneOffsetMatrices = nullptr;
		}
		
		if ( pMeshContainerTmp->ppBoneMatrix != nullptr )
		{
			int iMax = static_cast<int>( pMeshContainerTmp->pSkinInfo->GetNumBones() );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				if ( pMeshContainerTmp->ppBoneMatrix[i] != nullptr )
				{
					pMeshContainerTmp->ppBoneMatrix[i] = nullptr;
				}
			}

			delete[] pMeshContainerTmp->ppBoneMatrix;
			pMeshContainerTmp->ppBoneMatrix = nullptr;
		}

		if ( pMeshContainerTmp->ppTextures != nullptr )
		{
			int iMax = static_cast<int>( pMeshContainerTmp->NumMaterials );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				if ( pMeshContainerTmp->ppTextures[i] != nullptr )
				{
					pMeshContainerTmp->ppTextures[i]->Release();
					pMeshContainerTmp->ppTextures[i] = nullptr;
				}
			}

			delete[] pMeshContainerTmp->ppTextures;
			pMeshContainerTmp->ppTextures = nullptr;
		}
	}

	pMeshContainerTmp = nullptr;

	// MYMESHCONTAINER解放完了.

	// LPD3DXMESHCONTAINERの解放.
	if ( pFrame->pMeshContainer->pAdjacency != nullptr )
	{
		delete[] pFrame->pMeshContainer->pAdjacency;
		pFrame->pMeshContainer->pAdjacency = nullptr;
	}

	if ( pFrame->pMeshContainer->pEffects != nullptr )
	{
		if ( pFrame->pMeshContainer->pEffects->pDefaults != nullptr )
		{
			if ( pFrame->pMeshContainer->pEffects->pDefaults->pParamName != nullptr )
			{
				delete pFrame->pMeshContainer->pEffects->pDefaults->pParamName;
				pFrame->pMeshContainer->pEffects->pDefaults->pParamName = nullptr;
			}

			if ( pFrame->pMeshContainer->pEffects->pDefaults->pValue != nullptr )
			{
				delete pFrame->pMeshContainer->pEffects->pDefaults->pValue;
				pFrame->pMeshContainer->pEffects->pDefaults->pValue = nullptr;
			}

			delete pFrame->pMeshContainer->pEffects->pDefaults;
			pFrame->pMeshContainer->pEffects->pDefaults = nullptr;
		}

		if ( pFrame->pMeshContainer->pEffects->pEffectFilename != nullptr )
		{
			delete pFrame->pMeshContainer->pEffects->pEffectFilename;
			pFrame->pMeshContainer->pEffects->pEffectFilename = nullptr;
		}

		delete pFrame->pMeshContainer->pEffects;
		pFrame->pMeshContainer->pEffects = nullptr;
	}

	if ( pFrame->pMeshContainer->pMaterials != nullptr )
	{
		int iMax = static_cast<int>( pFrame->pMeshContainer->NumMaterials );

		for ( int i = iMax - 1; i >= 0; i-- )
		{
			delete[] pFrame->pMeshContainer->pMaterials[i].pTextureFilename;
			pFrame->pMeshContainer->pMaterials[i].pTextureFilename = nullptr;
		}

		delete[] pFrame->pMeshContainer->pMaterials;
		pFrame->pMeshContainer->pMaterials = nullptr;
	}

	if ( pFrame->pMeshContainer->pNextMeshContainer != nullptr )
	{
		// 次のメッシュコンテナーのポインターを持つのだとしたら.
		//	newで作られることはないはずなので、これで行けると思う.
		pFrame->pMeshContainer->pNextMeshContainer = nullptr;
	}

	if ( pFrame->pMeshContainer->pSkinInfo != nullptr )
	{
		pFrame->pMeshContainer->pSkinInfo->Release();
		pFrame->pMeshContainer->pSkinInfo = nullptr;
	}

	// LPD3DXMESHCONTAINERの解放完了.

	// MYFRAMEの解放.

	if ( pFrame->pPartsMesh != nullptr )
	{
		// ボーン情報の解放.
		if ( pFrame->pPartsMesh->pBoneArray )
		{
			delete[] pFrame->pPartsMesh->pBoneArray;
			pFrame->pPartsMesh->pBoneArray = nullptr;
		}

		if ( pFrame->pPartsMesh->pMaterial != nullptr )
		{
			int iMax = static_cast<int>( pFrame->pPartsMesh->dwNumMaterial );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				const int Size = static_cast<int>( pFrame->pPartsMesh->pMaterial[i].pTexture.size() );
				for ( int n = Size - 1; n >= 0; --n ) {
					if ( pFrame->pPartsMesh->pMaterial[i].pTexture[n] )
					{
						pFrame->pPartsMesh->pMaterial[i].pTexture[n]->Release();
						pFrame->pPartsMesh->pMaterial[i].pTexture[n] = nullptr;
					}
				}
			}

			delete[] pFrame->pPartsMesh->pMaterial;
			pFrame->pPartsMesh->pMaterial = nullptr;
		}
		

		if ( pFrame->pPartsMesh->ppIndexBuffer )
		{
			// インデックスバッファ解放.
			for ( DWORD i = 0; i<pFrame->pPartsMesh->dwNumMaterial; i++ ){
				if ( pFrame->pPartsMesh->ppIndexBuffer[i] != nullptr ){
					pFrame->pPartsMesh->ppIndexBuffer[i]->Release();
					pFrame->pPartsMesh->ppIndexBuffer[i] = nullptr;
				}
			}
			delete[] pFrame->pPartsMesh->ppIndexBuffer;
		}

		// 頂点バッファ開放.
		pFrame->pPartsMesh->pVertexBuffer->Release();
		pFrame->pPartsMesh->pVertexBuffer = nullptr;
	}

	// パーツマテリアル開放.
	delete[] pFrame->pPartsMesh;
	pFrame->pPartsMesh = nullptr;

	// SKIN_PARTS_MESH解放完了.

	// MYFRAMEのSKIN_PARTS_MESH以外のメンバーポインター変数は別の関数で解放されていました.

	return S_OK;
}

//----------------------------.
// アニメーションセットの切り替え.
//----------------------------.
void CSkinMesh::ChangeAnimSet( int index, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr ) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex ) return;

	m_pD3dxMesh->ChangeAnimSet( index, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= false;
		pAC->NowIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= false;
	m_NowIndex		= index;
	m_dAnimTime		= 0.0;
}

//----------------------------.
// アニメーションセットの切り替え(開始フレーム指定可能版).
//----------------------------.
void CSkinMesh::ChangeAnimSet_StartPos( int index, double dStartFramePos, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr		) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex ) return;

	m_pD3dxMesh->ChangeAnimSet_StartPos( index, dStartFramePos, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= false;
		pAC->NowIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= false;
	m_NowIndex		= index;
	m_dAnimTime		= 0.0;
}

//----------------------------.
// アニメーションをブレンドして切り替え.
//----------------------------.
void CSkinMesh::ChangeAnimBlend( int index, int oldIndex, SAnimationController* pAC )
{
	if ( pAC == nullptr ? m_IsChangeAnim : pAC->IsAnimChangeBlend ) return;

	ChangeAnimBlend_Skip( index, oldIndex, pAC );
}
void CSkinMesh::ChangeAnimBlend( int index, SAnimationController* pAC )
{
	if ( pAC != nullptr ) {
		ChangeAnimBlend( index, pAC->NowIndex, pAC );
		return;
	}
	ChangeAnimBlend( index, m_NowIndex, pAC );
}

//----------------------------.
// アニメーションをブレンドして切り替え(他のブレンドをしていても行う).
//----------------------------.
void CSkinMesh::ChangeAnimBlend_Skip( int index, int oldIndex, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr		) return;
	if ( pAC == nullptr ? index == m_NewIndex : index == pAC->NewIndex	) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex	) return;

	m_pD3dxMesh->ChangeAnimBlend( index, oldIndex, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= true;
		pAC->NewIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= true;
	m_NewIndex		= index;
	m_dAnimTime		= 0.0;
}
void CSkinMesh::ChangeAnimBlend_Skip( int index, SAnimationController* pAC )
{
	if ( pAC != nullptr ) {
		ChangeAnimBlend_Skip( index, pAC->NowIndex, pAC );
		return;
	}
	ChangeAnimBlend_Skip( index, m_NowIndex, pAC );
}

//----------------------------.
// アニメーション停止時間を取得.
//----------------------------.
double CSkinMesh::GetAnimPeriod( int index )
{
	if( m_pD3dxMesh == nullptr ){
		return 0.0f;
	}
	return m_pD3dxMesh->GetAnimPeriod( index );
}

//----------------------------.
// アニメーション数を取得.
//----------------------------.
int CSkinMesh::GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC )
{
	if( m_pD3dxMesh != nullptr ){
		return m_pD3dxMesh->GetAnimMax( pAC );
	}
	return -1;
}

//----------------------------.
// 指定したボーン情報(行列)を取得する関数.
//----------------------------.
D3DXMATRIX CSkinMesh::GetMatrixFromBone(
	LPCSTR sBoneName )
{
	D3DXMATRIX OutMatrix = {};
	GetMatrixFromBone( sBoneName, &OutMatrix );
	return OutMatrix;
}
bool CSkinMesh::GetMatrixFromBone(
	LPCSTR sBoneName, D3DXMATRIX* pOutMat )
{
	if( m_pD3dxMesh != nullptr ){
		if( m_pD3dxMesh->GetMatrixFromBone( sBoneName, pOutMat ) ){
			return true;
		}
	}
	return false;
}

//----------------------------.
// 指定したボーン情報(座標)を取得する関数.
//----------------------------.
D3DXVECTOR3 CSkinMesh::GetPosFromBone(
	LPCSTR sBoneName )
{
	D3DXVECTOR3 OutPos = {};
	GetPosFromBone( sBoneName, &OutPos );
	return OutPos;
}
bool CSkinMesh::GetPosFromBone(
	LPCSTR sBoneName, D3DXVECTOR3* pOutPos )
{
	if( m_pD3dxMesh != nullptr ){
		D3DXVECTOR3 tmpPos;
		if( m_pD3dxMesh->GetPosFromBone( sBoneName, &tmpPos ) ){
			D3DXMATRIX mWorld, mTran, mScale, mRot;
			D3DXMatrixScaling(				&mScale,	m_Transform.Scale.x,	m_Transform.Scale.y,	m_Transform.Scale.z );
			D3DXMatrixRotationYawPitchRoll( &mRot,		m_Transform.Rotation.y, m_Transform.Rotation.x, m_Transform.Rotation.z );
			D3DXMatrixTranslation(			&mTran,		tmpPos.x, tmpPos.y, tmpPos.z );
			mWorld	= mTran * mScale * mRot;

			pOutPos->x = mWorld._41 + m_Transform.Position.x;
			pOutPos->y = mWorld._42 + m_Transform.Position.y;
			pOutPos->z = mWorld._43 + m_Transform.Position.z;

			return true;
		}
	}
	return false;
}
D3DXVECTOR3 CSkinMesh::GetDeviaPosFromBone(
	LPCSTR sBoneName, D3DXVECTOR3 vSpecifiedPos )
{
	D3DXVECTOR3 OutPos = {};
	GetDeviaPosFromBone( sBoneName, &OutPos, vSpecifiedPos );
	return OutPos;
}
bool CSkinMesh::GetDeviaPosFromBone( 
	LPCSTR sBoneName, D3DXVECTOR3* pOutPos, D3DXVECTOR3 vSpecifiedPos )
{
	if ( m_pD3dxMesh != nullptr )
	{
		D3DXMATRIX mtmp;
		if ( m_pD3dxMesh->GetMatrixFromBone( sBoneName, &mtmp )){
			D3DXMATRIX mWorld, mScale, mTran, mDevia;
			D3DXMATRIX mRot, mYaw, mPitch, mRoll;
			//おそらくボーンの初期の向きが正位置になっているはずです.
			D3DXMatrixTranslation( &mDevia, vSpecifiedPos.x, vSpecifiedPos.y, vSpecifiedPos.z );//ずらしたい方向の行列を作成.
			D3DXMatrixMultiply( &mtmp, &mDevia, &mtmp );//ボーン位置行列とずらしたい方向行列を掛け合わせる.
			D3DXVECTOR3 tmpPos = D3DXVECTOR3( mtmp._41, mtmp._42, mtmp._43 );//位置のみ取得.

			D3DXMatrixScaling(		&mScale,	m_Transform.Scale.x, m_Transform.Scale.y, m_Transform.Scale.z );
			D3DXMatrixRotationY(	&mYaw,		m_Transform.Rotation.y );
			D3DXMatrixRotationX(	&mPitch,	m_Transform.Rotation.x );
			D3DXMatrixRotationZ(	&mRoll,		m_Transform.Rotation.z );
			D3DXMatrixTranslation(	&mTran,		tmpPos.x, tmpPos.y, tmpPos.z );

			mRot = mYaw * mPitch * mRoll;
			mWorld = mTran * mScale * mRot;

			pOutPos->x = mWorld._41 + m_Transform.Position.x;
			pOutPos->y = mWorld._42 + m_Transform.Position.y;
			pOutPos->z = mWorld._43 + m_Transform.Position.z;

			return true;
		}
	}
	return false;
}

//----------------------------.
// コンスタントバッファ作成関数.
//----------------------------.
HRESULT CSkinMesh::CreateCBuffer(
	ID3D11Buffer** pConstantBuffer,
	UINT size)
{
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth			= size;
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags			= 0;
	cb.StructureByteStride	= 0;
	cb.Usage				= D3D11_USAGE_DYNAMIC;
	if (FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, pConstantBuffer )))
	{
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// サンプラー作成関数.
//----------------------------.
HRESULT CSkinMesh::CreateSampler( ID3D11SamplerState** pSampler )
{
	// テクスチャー用サンプラー作成.
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory( &SamDesc, sizeof( D3D11_SAMPLER_DESC ) );

	SamDesc.Filter	= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(
		m_pDevice->CreateSamplerState( &SamDesc, &m_pSampleLinear )))
	{
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// マルチバイト文字をUnicode文字に変換する.
//----------------------------.
void CSkinMesh::ConvertCharaMultiByteToUnicode(
	WCHAR* Dest,			// (out)変換後の文字列(Unicode文字列).
	size_t DestArraySize,	// 変換後の文字列の配列の要素最大数.
	const CHAR* str )		// (in)変換前の文字列(マルチバイト文字列).
{
	// テクスチャ名のサイズを取得.
	size_t charSize = strlen( str ) + 1;
	size_t ret;	// 変換された文字数.

	// マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
	errno_t err = mbstowcs_s(
		&ret,
		Dest,
		charSize,
		str,
		_TRUNCATE );
}

//----------------------------.
// UVの変更.
//----------------------------.
void CSkinMesh::ChangeUV( const int MaterialNo, const D3DXVECTOR2& uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.x = uv.x;
		pPartsMesh->pMaterial[MaterialNo].UV.y = uv.y;
	}

	// 再帰関数(兄弟).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUV( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (親子)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUV( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// x方向のUVの変更.
//----------------------------.
void CSkinMesh::ChangeUVX( const int MaterialNo, const float uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.x = uv;
	}

	// 再帰関数(兄弟).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUVX( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (親子)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUVX( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// y方向のUVの変更.
//----------------------------.
void CSkinMesh::ChangeUVY( const int MaterialNo, const float uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.y = uv;
	}

	// 再帰関数(兄弟).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUVY( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (親子)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUVY( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}
