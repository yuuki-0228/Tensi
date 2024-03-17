#include "RayMesh.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Collision\Ray\Ray.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Ray.hlsl" );
}

CRayMesh::CRayMesh()
	: m_pVertexShader	( nullptr )
	, m_pVertexLayout	( nullptr )
	, m_pPixelShader	( nullptr )
	, m_pConstantBuffer	( nullptr )
	, m_pVertexBuffer	( nullptr )
{
}

CRayMesh::~CRayMesh()
{
	SAFE_RELEASE( m_pVertexBuffer	);
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader	);
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//---------------------------.
// 初期化.
//---------------------------.
HRESULT CRayMesh::Init()
{
	if ( FAILED( CreateShader() ) ) return E_FAIL;
	return S_OK;
}

//---------------------------.
// シェーダ作成.
//---------------------------.
HRESULT CRayMesh::CreateShader()
{
	ID3D11Device*	pDevice			= DirectX11::GetDevice();
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG.

	// HLSLからバーテックスシェーダのブロブを作成.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// シェーダファイル名(HLSLファイル).
			nullptr,			// マクロ定義の配列へのポインタ(未使用).
			nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
			"VS_Main",			// シェーダエントリーポイント関数の名前.
			"vs_5_0",			// シェーダのモデルを指定する文字列(プロファイル).
			uCompileFlag,		// シェーダコンパイルフラグ.
			0,					// エフェクトコンパイルフラグ(未使用).
			nullptr,			// スレッド ポンプ インターフェイスへのポインタ(未使用).
			&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
			&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
			nullptr)))			// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
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
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						// 位置.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	// DXGIのフォーマット(32bit float型*3).
			0,
			0,								// データの開始位置.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	// 頂点インプットレイアウトの配列要素数を算出.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

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
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// シェーダファイル名(HLSLファイル).
			nullptr,			// マクロ定義の配列へのポインタ(未使用).
			nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
			"PS_Main",			// シェーダエントリーポイント関数の名前.
			"ps_5_0",			// シェーダのモデルを指定する文字列(プロファイル).
			uCompileFlag,		// シェーダコンパイルフラグ.
			0,					// エフェクトコンパイルフラグ(未使用).
			nullptr,			// スレッド ポンプ インターフェイスへのポインタ(未使用).
			&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
			&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
			nullptr)))			// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
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

	// コンスタント(定数)バッファ作成.
	//	シェーダに特定の数値を送るバッファ.
	//	ここでは変換行列渡し用.
	//	シェーダに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof(SHADER_CONSTANT_BUFFER);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// 書き込みでアクセス.
	cb.MiscFlags			= 0;								// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;								// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		pDevice->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//---------------------------.
// モデル作成.
//---------------------------.
HRESULT CRayMesh::CreateModel( CRay* pRay )
{
	// バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;		// 使用方法(デフォルト).
	bd.ByteWidth			= sizeof(VERTEX) * 2;		// 頂点のサイズ(線分なので始点と終点の２点分).
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;	// 頂点バッファとして扱う.
	bd.CPUAccessFlags		= 0;						// CPUからはアクセスしない.
	bd.MiscFlags			= 0;						// その他のフラグ(未使用).
	bd.StructureByteStride	= 0;						// 構造体のサイズ(未使用).

	// レイ構造体から始点と終点を算出.
	D3DXVECTOR3 vertices[2];
	vertices[0] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);			// レイの始点.
	vertices[1] = pRay->GetVector() * pRay->GetLength();	// レイの終点.

	// サブリソースデータ構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;	// レイの頂点をセット.

	// 頂点バッファの作成.
	if ( FAILED( DirectX11::GetDevice()->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer ) ) )
	{
		ErrorMessage( "頂点バッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//---------------------------.
// レンダリング用.
//---------------------------.
void CRayMesh::Render( CRay* Ray )
{
	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// 外部のレイ構造体を反映.
	m_Transform.Position = Ray->GetPosition();
	m_Transform.Rotation = Ray->GetRotation();

	// ワールド座標の取得.
	D3DXMATRIX mWorld = m_Transform.GetWorldMatrix();

	// 使用するシェーダの登録.
	pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	//コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if (SUCCEEDED(
		pContext->Map(m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		// ワールド,ビュー,プロジェクション行列を渡す.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose(&m, &m);//行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = m_Color;

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		pContext->Unmap(m_pConstantBuffer, 0);
	}

	// このコンスタントバッファをどのシェーダで使うか？.
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// 頂点バッファをセット.
	if ( FAILED( CreateModel( Ray ) ) ) return;
	UINT stride = sizeof(VERTEX);// データの間隔.
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1,
		&m_pVertexBuffer, &stride, &offset);
	SAFE_RELEASE( m_pVertexBuffer );

	// 頂点インプットレイアウトをセット.
	pContext->IASetInputLayout(m_pVertexLayout);
	// プリミティブ・トポロジーをセット.
	pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// プリミティブをレンダリング.
	pContext->Draw( 2, 0 );// レイ(線分なので頂点2つ分).
}
