#include "StaticMesh.h"
#include "..\..\DirectX\DirectX9.h"
#include "..\..\Fog\Fog.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Light\Light.h"
#include "..\..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

#include <stdlib.h>	// マルチバイト文字→Unicode文字変換で必要.
#include <locale.h>
#include <filesystem>

namespace {
	// シェーダファイル名(ディレクトリも含む).
	const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Mesh.hlsl" );
}

CStaticMesh::CStaticMesh()
	: m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pCBufferPerMesh			( nullptr )
	, m_pCBufferPerMaterial		( nullptr )
	, m_pCBufferPerFrame		( nullptr )
	, m_pVertexBuffer			( nullptr )
	, m_ppIndexBuffer			( nullptr )
	, m_pSampleLinear			( nullptr )
	, m_Model					()
	, m_ModelForRay				()
	, m_pMaterials				( nullptr )
	, m_NumAttr					()
	, m_AttrID					()
	, m_EnableTexture			( false )
	, m_pFog					( nullptr )
	, m_IsFog					( false )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_OrLessDistanceAlphaFlag	( false )
	, m_OrMoreDistanceAlphaFlag	( false )
	, m_OrLessStartDistance		( 0.0f )
	, m_OrMoreStartDistance		( 0.0f )
	, m_AlphaDistance			( 0.0f )
	, m_TexturePathList			()
	, m_MaterialTextureNo		()
{
	m_pFog = std::make_unique<CFog>();
}

CStaticMesh::~CStaticMesh()
{
	// 解放処理.
	Release();
}

//----------------------------.
// 初期化関数.
//----------------------------.
HRESULT CStaticMesh::Init( LPCTSTR lpFileName )
{
	const std::wstring	wFileName	= lpFileName;
	const std::string	FileName	= StringConversion::to_String( wFileName );

	// ファイル読み込み.
	if (FAILED(LoadXMesh( lpFileName ) )			) return E_FAIL;
	// モデル作成.
	if( FAILED( CreateModel() )						) return E_FAIL;
	// シェーダ作成.
	if (FAILED(CreateShader() )						) return E_FAIL;
	// コンスタントバッファ作成.
	if( FAILED( CreateConstantBuffer() )			) return E_FAIL;
	// サンプラ作成.
	if( FAILED( CreateSampler() )					) return E_FAIL;

	// フォグの初期化.
	m_pFog->Init();

	return S_OK;
}

//----------------------------.
// メッシュ読み込み.
//----------------------------.
HRESULT CStaticMesh::LoadXMesh( LPCTSTR lpFileName )
{
	// ファイル名をコピー.
	lstrcpy( m_Model.FileName, lpFileName );
	lstrcpy( m_ModelForRay.FileName, lpFileName );

	// マテリアルバッファ.
	LPD3DXBUFFER pD3DXMtrlBuffer = nullptr;

	// Xファイルのロード.
	if( FAILED( DirectX11::MutexD3DXLoadMeshFromX(
		lpFileName,					// ファイル名.
		D3DXMESH_SYSTEMMEM			// システムメモリに読み込み.
		| D3DXMESH_32BIT,			// 32bit.
		DirectX9::GetDevice(), nullptr,
		&m_Model.pD3DXMtrlBuffer,	// (out)マテリアル情報.
		nullptr,
		&m_Model.NumMaterials,		// (out)マテリアル数.
		&m_Model.pMesh ) ) )		// (out)メッシュオブジェクト.
	{
		ErrorMessage( "Xファイル読込失敗" );
		return E_FAIL;
	}

	// Xファイルのロード(レイとの判定用に別設定で読み込む).
	if( FAILED( DirectX11::MutexD3DXLoadMeshFromX(
		lpFileName,						// ファイル名.
		D3DXMESH_SYSTEMMEM,				// システムメモリに読み込み.
		DirectX9::GetDevice(), nullptr,
		&m_ModelForRay.pD3DXMtrlBuffer,	// (out)マテリアル情報.
		nullptr,
		&m_ModelForRay.NumMaterials,	// (out)マテリアル数.
		&m_ModelForRay.pMesh ) ) )		// (out)メッシュオブジェクト.
	{
		ErrorMessage( "Xファイル読込失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// モデル作成.
//----------------------------.
HRESULT CStaticMesh::CreateModel()
{
	// マテリアル作成.
	if( FAILED( CreateMaterials() )		) return E_FAIL;
	// インデックスバッファ作成.
	if( FAILED( CreateIndexBuffer() )	) return E_FAIL;
	// 頂点バッファ作成.
	if( FAILED( CreateVertexBuffer() )	) return E_FAIL;
	return S_OK;
}

//----------------------------.
// マテリアル作成.
//----------------------------.
HRESULT CStaticMesh::CreateMaterials()
{
	// ファイルのパス(ディレクトリ)を確保.
	const int PATH_MAX = 64;
	TCHAR path[PATH_MAX] = _T( "" );
	int path_count = lstrlen( m_Model.FileName );
	for( int k = path_count; k >= 0; k-- ) {
		if( m_Model.FileName[k] == '\\' ) {
			for( int j = 0; j <= k; j++ ) {
				path[j] = m_Model.FileName[j];
			}
			path[k + 1] = '\0';
			break;
		}
	}

	// 読み込んだ情報から必要な情報を抜き出す.
	D3DXMATERIAL* d3dxMaterials
		= static_cast<D3DXMATERIAL*>( m_Model.pD3DXMtrlBuffer->GetBufferPointer() );
	// マテリアル数分の領域を確保.
	m_pMaterials = new MY_MATERIAL[m_Model.NumMaterials]();
	m_MaterialTextureNo.resize( m_Model.NumMaterials );
	// マテリアル数分繰り返し.
	for( DWORD No = 0; No < m_Model.NumMaterials; No++ )
	{
		// マテリアル情報のコピー.
		//	アンビエント.
		m_pMaterials[No].Ambient.x = d3dxMaterials[No].MatD3D.Ambient.r;
		m_pMaterials[No].Ambient.y = d3dxMaterials[No].MatD3D.Ambient.g;
		m_pMaterials[No].Ambient.z = d3dxMaterials[No].MatD3D.Ambient.b;
		m_pMaterials[No].Ambient.w = d3dxMaterials[No].MatD3D.Ambient.a;
		// ディフューズ.
		m_pMaterials[No].Diffuse.x = d3dxMaterials[No].MatD3D.Diffuse.r;
		m_pMaterials[No].Diffuse.y = d3dxMaterials[No].MatD3D.Diffuse.g;
		m_pMaterials[No].Diffuse.z = d3dxMaterials[No].MatD3D.Diffuse.b;
		m_pMaterials[No].Diffuse.w = d3dxMaterials[No].MatD3D.Diffuse.a;
		// スペキュラ.
		m_pMaterials[No].Specular.x = d3dxMaterials[No].MatD3D.Specular.r;
		m_pMaterials[No].Specular.y = d3dxMaterials[No].MatD3D.Specular.g;
		m_pMaterials[No].Specular.z = d3dxMaterials[No].MatD3D.Specular.b;
		m_pMaterials[No].Specular.w = d3dxMaterials[No].MatD3D.Specular.a;

		// (その面に)テクスチャが貼られているか？.
		if( d3dxMaterials[No].pTextureFilename != nullptr
			&& strlen( d3dxMaterials[No].pTextureFilename ) > 0 )
		{
#ifdef UNICODE
			WCHAR TexFilename_w[32] = L"";
			// テクスチャ名のサイズを取得.
			size_t charSize = strlen( d3dxMaterials[No].pTextureFilename ) + 1;
			size_t ret;	// 変換された文字数.

			// マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
			errno_t err = mbstowcs_s(
				&ret,
				TexFilename_w,
				charSize,
				d3dxMaterials[No].pTextureFilename,
				_TRUNCATE );

			LPTSTR filename = TexFilename_w;
#else// #ifdef UNICODE
			LPTSTR filename = d3dxMaterials[No].pTextureFilename;
#endif// #ifdef UNICODE

			// テクスチャありのフラグを立てる.
			m_EnableTexture = true;

			// パスをコピー.
			lstrcpy( m_pMaterials[No].TextureName, path );
			// テクスチャファイル名を連結.
			lstrcat( m_pMaterials[No].TextureName, filename );

			// テクスチャ作成.
			if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				m_pMaterials[No].TextureName,	// テクスチャファイル名.
				nullptr, nullptr,
				&m_pMaterials[No].pTexture[0],		// (out)テクスチャオブジェクト.
				nullptr ) ) )
			{
				ErrorMessage( "テクスチャ作成失敗" );
				return E_FAIL;
			}

			// 差し替えテクスチャを取得.
			int TexNo = 1;
			while ( 1 ) {
				// 差し替えテクスチャパスの作成.
				std::string TexName	 = StringConversion::to_String( m_pMaterials[No].TextureName );
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
				m_pMaterials[No].pTexture.emplace_back( pTexture );
			}

			// ライトテクスチャパスの作成.
			std::string LightTexName = StringConversion::to_String( m_pMaterials[No].TextureName );
			size_t		LightIndex	 = LightTexName.find( "." );
			LightTexName.insert( LightIndex, "_light" );

			// ライトテクスチャを作成できるか.
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				StringConversion::to_wString( LightTexName ).c_str(),
				nullptr, nullptr, &m_pMaterials[No].pLightTexture, nullptr ) ) )
			{
				m_pMaterials[No].pLightTexture = nullptr;
			}
		}
	}

	return S_OK;
}

//----------------------------.
// インデックスバッファ作成.
//----------------------------.
HRESULT CStaticMesh::CreateIndexBuffer()
{
	D3D11_BUFFER_DESC		bd;			// Dx11バッファ構造体.
	D3D11_SUBRESOURCE_DATA	InitData;	// 初期化データ.

	// マテリアル数分の領域を確保.
	m_ppIndexBuffer = new ID3D11Buffer*[m_Model.NumMaterials]();
	for( DWORD No = 0; No < m_Model.NumMaterials; No++ ){
		m_ppIndexBuffer[No] = nullptr;
	}

	// メッシュの属性情報を得る.
	//	属性情報でインデックスバッファから細かいマテリアルごとのインデックスバッファを分離できる.
	D3DXATTRIBUTERANGE* pAttrTable = nullptr;

	// メッシュの面および頂点の順番変更を制御し、パフォーマンスを最適化する.
	//	D3DXMESHOPT_COMPACT : 面の順番を変更し、使用されていない頂点と面を削除する.
	//	D3DXMESHOPT_ATTRSORT : パフォーマンスを上げる為、面の順番を変更して最適化を行う.
	m_Model.pMesh->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT,
		nullptr, nullptr, nullptr, nullptr );
	// 属性テーブルの取得.
	m_Model.pMesh->GetAttributeTable( nullptr, &m_NumAttr );
	pAttrTable = new D3DXATTRIBUTERANGE[m_NumAttr];
	if( FAILED( m_Model.pMesh->GetAttributeTable( pAttrTable, &m_NumAttr ) ) )
	{
		ErrorMessage( "属性テーブル取得失敗" );
		return E_FAIL;
	}

	// 同じくLockしないと取り出せない.
	int* pIndex = nullptr;
	m_Model.pMesh->LockIndexBuffer(
		D3DLOCK_READONLY, (void**)&pIndex );
	// 属性ごとのインデックスバッファを作成.
	for( DWORD No = 0; No < m_NumAttr; No++ )
	{
		m_AttrID[No] = pAttrTable[No].AttribId;
		// Dx9のインデックスバッファからの情報で、Dx11のインデックスバッファを作成.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth
			= sizeof( int )*pAttrTable[No].FaceCount * 3;//面数×3で頂点数.
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		// 大きいインデックスバッファ内のオフセット(×3する).
		InitData.pSysMem = &pIndex[pAttrTable[No].FaceStart * 3];

		if( FAILED( m_pDevice->CreateBuffer(
			&bd, &InitData, &m_ppIndexBuffer[No] ) ) )
		{
			ErrorMessage( "インデックスバッファ作成失敗" );
			return E_FAIL;
		}
		// 面の数をコピー.
		m_pMaterials[m_AttrID[No]].dwNumFace = pAttrTable[No].FaceCount;
	}
	// 属性テーブルの削除.
	delete[] pAttrTable;
	// 使用済みのインデックスバッファの解放.
	m_Model.pMesh->UnlockIndexBuffer();

	return S_OK;
}

//----------------------------.
// 頂点バッファ作成.
//----------------------------.
HRESULT CStaticMesh::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC		bd;			// Dx11バッファ構造体.
	D3D11_SUBRESOURCE_DATA	InitData;	// 初期化データ.

	// Dx9の場合、mapではなくLockで頂点バッファからデータを取り出す.
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	m_Model.pMesh->GetVertexBuffer( &pVB );
	DWORD	dwStride	= m_Model.pMesh->GetNumBytesPerVertex();
	BYTE*	pVertices	= nullptr;
	VERTEX* pVertex		= nullptr;
	if( SUCCEEDED(
		pVB->Lock( 0, 0, (VOID**)&pVertices, 0 ) ) )
	{
		pVertex				= (VERTEX*)pVertices;
		// Dx9の頂点バッファからの情報で、Dx11頂点バッファを作成.
		bd.Usage			= D3D11_USAGE_DEFAULT;
		// 頂点を格納するのに必要なバイト数.
		bd.ByteWidth		= m_Model.pMesh->GetNumBytesPerVertex()*m_Model.pMesh->GetNumVertices();
		bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= 0;
		bd.MiscFlags		= 0;
		InitData.pSysMem	= pVertex;
		if( FAILED( m_pDevice->CreateBuffer(
			&bd, &InitData, &m_pVertexBuffer ) ) )
		{
			ErrorMessage( "頂点バッファ作成失敗" );
			return E_FAIL;
		}
		pVB->Unlock();
	}
	SAFE_RELEASE( pVB );	// 頂点バッファ解放.
	return S_OK;
}

// サンプラ作成.
HRESULT CStaticMesh::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof( samDesc ) );
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// リニアフィルタ(線形補間).
						// POINT:高速だが粗い.
	samDesc.AddressU
		= D3D11_TEXTURE_ADDRESS_WRAP;// ラッピングモード(WRAP:繰り返し).
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// MIRROR: 反転繰り返し.
	// CLAMP : 端の模様を引き伸ばす.
	// BORDER: 別途境界色を決める.
	// サンプラ作成.
	if( FAILED( m_pDevice->CreateSamplerState(
		&samDesc, &m_pSampleLinear ) ) )//(out)サンプラ.
	{
		ErrorMessage( "サンプラ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// 解放関数.
//----------------------------.
void CStaticMesh::Release()
{
	// インデックスバッファ解放.	←バグるので調査必要.
	if (m_ppIndexBuffer != nullptr) {
		for (int No = m_Model.NumMaterials-1; No >= 0; No--) {
			if (m_ppIndexBuffer[No] != nullptr) {
				SAFE_RELEASE(m_ppIndexBuffer[No]);
			}
		}
		delete[] m_ppIndexBuffer;
		m_ppIndexBuffer = nullptr;
	}
	// マテリアル解放.
	if (m_pMaterials != nullptr) {
		delete[] m_pMaterials;
		m_pMaterials = nullptr;
	}
	// メッシュデータの解放.
	SAFE_RELEASE( m_Model.pMesh );
	SAFE_RELEASE( m_Model.pD3DXMtrlBuffer );

	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pVertexShader );
}

//----------------------------.
//	HLSLファイルを読み込みシェーダを作成する.
//	HLSL: High Level Shading Language の略.
//----------------------------.
HRESULT CStaticMesh::CreateShader()
{
	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrors = nullptr;
	UINT uCompileFlag = 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	if (m_EnableTexture == true){
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "VS_Main", "vs_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl読み込み失敗" );
			return E_FAIL;
		}
	}
	else {
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "VS_NoTex", "vs_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl読み込み失敗" );
			return E_FAIL;
		}
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「バーテックスシェーダ」を作成.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&m_pVertexShader)))	// (out)バーテックスシェーダ.
	{
		ErrorMessage( "バーテックスシェーダ作成失敗" );
		return E_FAIL;
	}

	// 頂点インプットレイアウトを定義.
	D3D11_INPUT_ELEMENT_DESC layout[3];
	// 頂点インプットレイアウトの配列要素数を算出.
	UINT numElements = 0;
	if (m_EnableTexture == true)
	{
		D3D11_INPUT_ELEMENT_DESC tmp[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(tmp) / sizeof(tmp[0]);	// 要素数算出.
		memcpy_s( layout, sizeof(layout),
			tmp, sizeof(D3D11_INPUT_ELEMENT_DESC)*numElements);
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC tmp[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(tmp) / sizeof(tmp[0]);	// 要素数算出.
		memcpy_s(layout, sizeof(layout),
			tmp, sizeof(D3D11_INPUT_ELEMENT_DESC)*numElements);
	}

	// 頂点インプットレイアウトを作成.
	if (FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout,
			numElements,
			pCompiledShader,
			&m_pVertexLayout)))	// (out)頂点インプットレイアウト.
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSLからピクセルシェーダのブロブを作成.
	if (m_EnableTexture == true){
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "PS_Main", "ps_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl読み込み失敗" );
			return E_FAIL;
		}
	}
	else {
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "PS_NoTex", "ps_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl読み込み失敗" );
			return E_FAIL;
		}
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「ピクセルシェーダ」を作成.
	if (FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr,
			&m_pPixelShader)))	// (out)ピクセルシェーダ.
	{
		ErrorMessage( "ピクセルシェーダ作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	return S_OK;
}

//----------------------------.
// コンスタントバッファ作成.
//----------------------------.
HRESULT CStaticMesh::CreateConstantBuffer()
{
	// コンスタントバッファ(メッシュ用).
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;	// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof( CBUFFER_PER_MESH );	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;		// 書き込みでアクセス.
	cb.MiscFlags			= 0;							// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;							// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;			// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerMesh ) ) )
	{
		ErrorMessage( "コンスタントバッファ(メッシュ)作成失敗" );
		return E_FAIL;
	}

	// コンスタントバッファ(マテリアル用).
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof( CBUFFER_PER_MATERIAL );	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// 書き込みでアクセス.
	cb.MiscFlags			= 0;								// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;								// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerMaterial ) ) )
	{
		ErrorMessage( "コンスタントバッファ(マテリアル用)作成失敗" );
		return E_FAIL;
	}

	// コンスタントバッファ(フレーム用).
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof( CBUFFER_PER_FRAME );		// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// 書き込みでアクセス.
	cb.MiscFlags			= 0;								// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;								// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerFrame ) ) )
	{
		ErrorMessage( "コンスタントバッファ(フレーム用)作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// レンダリング用.
//	※DirectX内のレンダリング関数.
//	最終的に画面に出力するのは別クラスのレンダリング関数がやる.
//----------------------------.
void CStaticMesh::Render( STransform* pTransform, const D3DXMATRIX* pRot )
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

	// ワールド行列の取得.
	D3DXMATRIX mWorld = Transform->GetWorldMatrix( pRot );

	// 使用するシェーダのセット.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );	// 頂点シェーダ.
	m_pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );	// ピクセルシェーダ.

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	// バッファ内のデータの書き換え開始時にMap.
	if (SUCCEEDED( m_pContext->Map(
		m_pCBufferPerFrame, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// コンスタントバッファ(フレーム用).
		CBUFFER_PER_FRAME cb;

		// カメラ位置.
		const D3DXVECTOR3 CamPos = CameraManager::GetPosition();
		cb.vCameraPos = D3DXVECTOR4(CamPos.x, CamPos.y, CamPos.z, 0.0f);

		// ライト方向.
		const D3DXVECTOR3 Direction = Light::GetDirection();
		cb.vLightDir			= D3DXVECTOR4( Direction.x, Direction.y, Direction.z, 0.0f );
		cb.vLightIntensity.x	= Light::GetIntensity();

		// ディザ抜きを使用するかを渡す.
		cb.vFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vFlag.y = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		// 距離で透明にしていくかを渡す.
		cb.vFlag.z			= m_OrLessDistanceAlphaFlag == true ? 1.0f : 0.0f;
		cb.vFlag.w			= m_OrMoreDistanceAlphaFlag == true ? 1.0f : 0.0f;
		cb.vAlphaDistance.x = m_OrLessStartDistance;
		cb.vAlphaDistance.y = m_OrMoreStartDistance;
		cb.vAlphaDistance.z = m_AlphaDistance;

		// ライト方向の正規化(ノーマライズ）.
		//	※モデルからライトへ向かう方向. ディレクショナルライトで重要な要素.
		D3DXVec4Normalize(&cb.vLightDir, &cb.vLightDir);

		memcpy_s(
			pData.pData,	// コピー先のバッファ.
			pData.RowPitch,	// コピー先のバッファサイズ.
			(void*)(&cb),	// コピー元のバッファ.
			sizeof(cb));	// コピー元のバッファサイズ.

		// バッファ内のデータの書き換え終了時にUnmap.
		m_pContext->Unmap(m_pCBufferPerFrame, 0);
	}

	// このコンスタントバッファをどのシェーダで使用するか？.
	m_pContext->VSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );// 頂点シェーダ.
	m_pContext->PSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );// ピクセルシェーダ.

	// メッシュのレンダリング.
	RenderMesh(mWorld);
}

//----------------------------.
// レンダリング関数(クラス内でのみ使用する).
//----------------------------.
void CStaticMesh::RenderMesh(
	D3DXMATRIX& mWorld )
{
	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	// バッファ内のデータの書き換え開始時にMap.
	if (SUCCEEDED( m_pContext->Map(
		m_pCBufferPerMesh, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// コンスタントバッファ(メッシュ用).
		CBUFFER_PER_MESH cb;

		// ワールド行列を渡す.
		cb.mW = mWorld;
		D3DXMatrixTranspose(&cb.mW, &cb.mW);

		// ワールド,ビュー,プロジェクション行列を渡す.
		D3DXMATRIX mWVP = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose(&mWVP, &mWVP);// 行列を転置する.
		// ※行列の計算方法がDirectXとGPUで異なるため転置が必要.
		cb.mWVP = mWVP;

		// カラー.
		cb.vColor = m_Color;

		memcpy_s(
			pData.pData,	// コピー先のバッファ.
			pData.RowPitch,	// コピー先のバッファサイズ.
			(void*)(&cb),	// コピー元のバッファ.
			sizeof(cb));	// コピー元のバッファサイズ.

		// バッファ内のデータの書き換え終了時にUnmap.
		m_pContext->Unmap(m_pCBufferPerMesh, 0);
	}

	// このコンスタントバッファをどのシェーダで使用するか？.
	m_pContext->VSSetConstantBuffers(	0, 1, &m_pCBufferPerMesh );	// 頂点シェーダ.
	m_pContext->PSSetConstantBuffers(	0, 1, &m_pCBufferPerMesh );	// ピクセルシェーダ.

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout(m_pVertexLayout);

	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファをセット.
	UINT stride = m_Model.pMesh->GetNumBytesPerVertex();
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0, 1, &m_pVertexBuffer, &stride, &offset);

	// 属性の数だけ、それぞれの属性のインデックスバッファを描画.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		// 使用されていないマテリアル対策.
		if (m_pMaterials[m_AttrID[No]].dwNumFace == 0) {
			continue;
		}
		// インデックスバッファをセット.
		m_pContext->IASetIndexBuffer(
			m_ppIndexBuffer[No], DXGI_FORMAT_R32_UINT, 0);
		// マテリアルの各要素をシェーダに渡す.
		D3D11_MAPPED_SUBRESOURCE pDataMat;
		if (SUCCEEDED(
			m_pContext->Map(m_pCBufferPerMaterial,
				0, D3D11_MAP_WRITE_DISCARD, 0, &pDataMat)))
		{
			// コンスタントバッファ(マテリアル用).
			CBUFFER_PER_MATERIAL cb;
			// アンビエント,ディフューズ,スペキュラをシェーダに渡す.
			cb.vAmbient = m_pMaterials[m_AttrID[No]].Ambient;
			cb.vDiffuse = m_pMaterials[m_AttrID[No]].Diffuse;
			cb.vSpecular = m_pMaterials[m_AttrID[No]].Specular;

			// UV情報を渡す.
			cb.vUV = m_pMaterials[m_AttrID[No]].UV;

			memcpy_s(pDataMat.pData, pDataMat.RowPitch,
				(void*)&cb, sizeof(cb));

			m_pContext->Unmap(m_pCBufferPerMaterial, 0);
		}

		// このコンスタントバッファをどのシェーダで使うか？.
		m_pContext->VSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);
		m_pContext->PSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);

		// テクスチャをシェーダに渡す.
		if (m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]] != nullptr) {
			// テクスチャがあるとき.
			m_pContext->PSSetSamplers(0, 1, &m_pSampleLinear);
			m_pContext->PSSetShaderResources(
				0, 1, &m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]] );
		}
		else {
			// テクスチャがないとき.
			ID3D11ShaderResourceView* pNothing[1] = { 0 };
			m_pContext->PSSetShaderResources(0, 1, pNothing);
		}

		// フォグの描画.
		if ( m_IsFog == true ) m_pFog->Render( m_pMaterials[m_AttrID[No]].dwNumFace * 3, mWorld, m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]], 0.0f, 100.0f );

		// プリミティブ(ポリゴン)をレンダリング.
		m_pContext->DrawIndexed(
			m_pMaterials[m_AttrID[No]].dwNumFace * 3, 0, 0);
	}
}
