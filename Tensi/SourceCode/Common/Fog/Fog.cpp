#include "Fog.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"

namespace{
	constexpr TCHAR SHADER_NAME[]	= _T( "Data\\Shader\\Fog.hlsl" );	// シェーダファイル名( ディレクトリも含む ).
	constexpr float FOG_E			= 2.71828f;							// 指数対数の底.
}

CFog::CFog()
	: m_vertexShader	( 0 )
	, m_pixelShader		( 0 )
	, m_layout			( 0 )
	, m_constantBuffer	( 0 )
	, m_sampleState		( 0 )
	, m_fogBuffer		( 0 )
	, m_Density			( 0.111f )
{
}

CFog::~CFog()
{
	Release();
}

//---------------------------.
// 初期化.
//---------------------------.
bool CFog::Init()
{
	bool result;

	// 頂点シェーダーとピクセルシェーダーを初期化.
	result = InitializeShader();
	if ( !result ) return false;

	return true;
}

//---------------------------.
// 描画.
//---------------------------.
bool CFog::Render( int indexCount, D3DXMATRIX worldMatrix,
	ID3D11ShaderResourceView* texture, float fogStart, float fogEnd )
{
	bool result;

	// レンダリングに使用するシェーダーパラメータを設定.
	result = SetShaderParameters( worldMatrix, texture, fogStart, fogEnd );
	if ( !result ) return false;

	// バッファをシェーダーでレンダリング.
	RenderShader( indexCount );

	return true;
}

//---------------------------.
// シェーダの初期化.
//---------------------------.
bool CFog::InitializeShader()
{
	HRESULT				result;
	D3D11_BUFFER_DESC	constantBufferDesc	= {};
	D3D11_SAMPLER_DESC	samplerDesc			= {};
	D3D11_BUFFER_DESC	fogBufferDesc		= {};

	ID3DBlob*			pCompiledShader		= nullptr;
	ID3DBlob*			pErrors				= nullptr;
	UINT				uCompileFlag		= 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	if ( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// シェーダーファイル名.
		nullptr,			// マクロ定義の配列へのポインタ(未使用).
		nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
		"VS_Main",			// シェーダーエントリーポイント関数の名前.
		"vs_5_0",			// シェーダーのモデルを指定する文字列(プロファイル).
		uCompileFlag,		// シェーダーコンパイルフラグ.
		0,					// エフェクトコンパイルフラグ(未使用).
		nullptr,			// スレッドポンプインターフェイスへのポインタ(未使用).
		&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
		&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
		nullptr ) ) )		// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return false;
	}
	SAFE_RELEASE( pErrors );

	// 上記で作成したブロブから「バーテックスシェーダ」を作成.
	if ( FAILED(
		DirectX11::MutexDX11CreateVertexShader(
		pCompiledShader,
		nullptr,
		&m_vertexShader ) ) )	//(out)バーテックスシェーダ.
	{
		ErrorMessage( "バーテックスシェーダ作成失敗" );
		return false;
	}

	// 頂点インプットレイアウトを定義.
	D3D11_INPUT_ELEMENT_DESC layout[2];
	// 頂点インプットレイアウトの配列要素数を算出.
	UINT numElements = 0;
	D3D11_INPUT_ELEMENT_DESC tmp[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = sizeof( tmp ) / sizeof( tmp[0] );	// 要素数算出.
	memcpy_s( layout, sizeof( layout ),
		tmp, sizeof( D3D11_INPUT_ELEMENT_DESC )*numElements );
	// 頂点インプットレイアウトを作成.
	if ( FAILED(
		DirectX11::MutexDX11CreateInputLayout(
		layout,
		numElements,
		pCompiledShader,
		&m_layout ) ) )	// (out)頂点インプットレイアウト.
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// HLSLからピクセルシェーダのブロブを作成.
	if ( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// シェーダーファイル名.
		nullptr,			// マクロ定義の配列へのポインタ(未使用).
		nullptr,			// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
		"PS_Main",			// シェーダーエントリーポイント関数の名前.
		"ps_5_0",			// シェーダーのモデルを指定する文字列(プロファイル).
		uCompileFlag,		// シェーダーコンパイルフラグ.
		0,					// エフェクトコンパイルフラグ(未使用).
		nullptr,			// スレッドポンプインターフェイスへのポインタ(未使用).
		&pCompiledShader,	// ブロブを格納するメモリへのポインタ.
		&pErrors,			// エラーと警告一覧を格納するメモリへのポインタ.
		nullptr ) ) )		// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return false;
	}
	SAFE_RELEASE( pErrors );

	// 上記で作成したﾌﾞﾛﾌﾞから「ピクセルシェーダ」を作成.
	if ( FAILED(
		DirectX11::MutexDX11CreatePixelShader(
		pCompiledShader,
		nullptr,
		&m_pixelShader ) ) )	// (out)ピクセルシェーダ.
	{
		ErrorMessage( "ピクセルシェーダ作成失敗" );
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// 頂点シェーダーの設定.
	constantBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth			= sizeof( ConstantBufferType );
	constantBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags			= 0;
	constantBufferDesc.StructureByteStride	= 0;

	// 定数バッファーポインターを作成.
	result = m_pDevice->CreateBuffer( &constantBufferDesc, NULL, &m_constantBuffer );
	if ( FAILED( result ) ) return false;

	// テクスチャサンプラーの設定.
	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias		= 0.0f;
	samplerDesc.MaxAnisotropy	= 1;
	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0]	= 0;
	samplerDesc.BorderColor[1]	= 0;
	samplerDesc.BorderColor[2]	= 0;
	samplerDesc.BorderColor[3]	= 0;
	samplerDesc.MinLOD			= 0;
	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	// テクスチャサンプラーの作成.
	result = m_pDevice->CreateSamplerState( &samplerDesc, &m_sampleState );
	if ( FAILED( result ) ) return false;

	// 頂点シェーダーにあるダイナミックフォグ定数バッファーの設定.
	fogBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
	fogBufferDesc.ByteWidth				= sizeof( FogBufferType );
	fogBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	fogBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	fogBufferDesc.MiscFlags				= 0;
	fogBufferDesc.StructureByteStride	= 0;

	// フォグバッファーポインターを作成.
	result = m_pDevice->CreateBuffer( &fogBufferDesc, NULL, &m_fogBuffer );
	if ( FAILED( result ) ) return false;

	return true;
}

//---------------------------.
// 初期化.
//---------------------------.
void CFog::Release()
{
	// フォグ定数バッファを解放.
	if ( m_fogBuffer ) {
		m_fogBuffer->Release();
		m_fogBuffer = 0;
	}

	// サンプラーの状態を開放.
	if ( m_sampleState ) {
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// 定数バッファを解放.
	if ( m_constantBuffer ) {
		m_constantBuffer->Release();
		m_constantBuffer = 0;
	}

	// レイアウトを解放.
	if ( m_layout ) {
		m_layout->Release();
		m_layout = 0;
	}

	// ピクセルシェーダーを解放.
	if ( m_pixelShader ) {
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// 頂点シェーダーを解放.
	if ( m_vertexShader ) {
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}

//---------------------------.
// エラーメッセージを出力.
//---------------------------.
void CFog::OutputShaderErrorMessage( ID3D10Blob* errorMessage, WCHAR* shaderFilename )
{
	char*			compileErrors;
	unsigned long	bufferSize, i;
	std::ofstream	fout;

	// エラーメッセージテキストバッファへのポインタを取得.
	compileErrors = (char*) ( errorMessage->GetBufferPointer() );

	// メッセージの長さを取得.
	bufferSize = static_cast<unsigned long>( errorMessage->GetBufferSize() );

	// ファイルを開く.
	fout.open( "shader-error.txt" );

	// エラーメッセージを書き出す.
	for ( i = 0; i < bufferSize; i++ ) {
		fout << compileErrors[i];
	}

	// ファイルを閉じる.
	fout.close();

	// エラーメッセージを開放.
	errorMessage->Release();
	errorMessage = 0;

	// エラーメッセージを表示.
	ErrorMessage( 
		"シェーダーのコンパイル中にエラーが発生しました。\n"
		"メッセージについては「shader-error.txt」を確認してください。" );
}

//---------------------------.
// シェーダーパラメータを設定.
//---------------------------.
bool CFog::SetShaderParameters( D3DXMATRIX worldMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd )
{
	HRESULT						result;
	D3D11_MAPPED_SUBRESOURCE	mappedResource;
	ConstantBufferType*			dataPtr;
	unsigned int				bufferNumber;
	FogBufferType*				dataPtr2;

	// 定数バッファをロックして、書き込み可能にする.
	result = m_pContext->Map( m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( FAILED( result ) ) return false;

	// 定数バッファ内のデータへのポインタを取得.
	dataPtr = (ConstantBufferType*) mappedResource.pData;

	// マトリックスを転置して、シェーダー用に準備.
	D3DXMATRIX viewMatrix		= CameraManager::GetViewMatrix();
	D3DXMATRIX projectionMatrix = CameraManager::GetProjMatrix();
	D3DXMatrixTranspose( &worldMatrix, &worldMatrix				);
	D3DXMatrixTranspose( &viewMatrix, &viewMatrix				);
	D3DXMatrixTranspose( &projectionMatrix, &projectionMatrix	);

	// 行列を定数バッファーにコピー.
	dataPtr->world		= worldMatrix;
	dataPtr->view		= viewMatrix;
	dataPtr->projection = projectionMatrix;

	// コンスタントバッファのロックを解除.
	m_pContext->Unmap( m_constantBuffer, 0 );

	// 頂点シェーダーで定数バッファーの位置を設定.
	bufferNumber = 0;

	// 更新された値を使用して、頂点シェーダーに定数バッファーを設定.
	m_pContext->VSSetConstantBuffers( bufferNumber, 1, &m_constantBuffer );

	// ピクセルシェーダーでシェーダーテクスチャリソースを設定.
	m_pContext->PSSetShaderResources( 0, 1, &texture );

	// フォグ定数バッファをロックして、次のように書き込めるようにする.
	result = m_pContext->Map( m_fogBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( FAILED( result ) ) return false;

	// 定数バッファ内のデータへのポインタを取得.
	dataPtr2 = (FogBufferType*) mappedResource.pData;

	// 線形フォグの設定.
	dataPtr2->fogStart	= fogStart;
	dataPtr2->fogEnd	= fogEnd;

	// 指数フォグの設定.
	dataPtr2->e			= FOG_E;
	dataPtr2->density	= m_Density;

	// コンスタントバッファのロックを解除.
	m_pContext->Unmap( m_fogBuffer, 0 );

	// 頂点シェーダーでフォグコンスタントバッファの位置を設定.
	bufferNumber = 1;

	// 更新された値を使用して、頂点シェーダーにフォグバッファを設定.
	m_pContext->VSSetConstantBuffers( bufferNumber, 1, &m_fogBuffer );

	return true;
}

//---------------------------.
// シェーダの描画.
//---------------------------.
void CFog::RenderShader( int indexCount )
{
	// 頂点入力レイアウトを設定.
	m_pContext->IASetInputLayout( m_layout );

	// 三角形のレンダリングに使用される頂点シェーダーとピクセルシェーダーを設定.
	m_pContext->VSSetShader( m_vertexShader, NULL, 0 );
	m_pContext->PSSetShader( m_pixelShader,	NULL, 0 );

	// ピクセルシェーダーでサンプラーの状態を設定.
	m_pContext->PSSetSamplers( 0, 1, &m_sampleState );

	// 三角形をレンダリング.
	m_pContext->DrawIndexed( indexCount, 0, 0 );
}