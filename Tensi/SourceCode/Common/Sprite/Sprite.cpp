#include "Sprite.h"
#include "..\DirectX\DirectX11.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\FileManager\ImagSize\ImageSize.h"
#include "..\..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"
#include "..\..\Utility\Message\Message.h"
#include "..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[]			= _T( "Data\\Shader\\Sprite.hlsl"	);

	// アンドゥ用のログファイルの移動場所のパス.
	constexpr char	UNDO_LOG_FILE_PATH[]	= "Data\\UIEditor\\UndoLog\\";
	
	// ピクセルシェーダーのエントリポイント名.
	const std::vector<const char*> PS_SHADER_ENTRY_NAMES = {
		"PS_Main",
		"PS_Mask",
		"PS_Transition"
	};
}

CSprite::CSprite()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShaders			()
	, m_pConstantBuffer			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pSampleLinears			()
	, m_pTexture				( nullptr )
	, m_Vertices				()
	, m_SpriteState				()
	, m_SpriteRenderState		()
	, m_SpriteStateData			()
	, m_pLogList				( nullptr )
	, m_IsAnimPlay				( true )
	, m_IsAllDisp				( true )
	, m_pIsCreaterLog			( nullptr )
{
}

CSprite::~CSprite()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pTexture		);
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	for ( auto itr = m_pPixelShaders.rbegin(), e = m_pPixelShaders.rend(); itr != e; ++itr ) {
		SAFE_RELEASE( *itr );
	}
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT CSprite::Init( const std::string& FilePath )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_SpriteState.FilePath = FilePath;
	m_pPixelShaders.resize( PS_SHADER_ENTRY_NAMES.size(), nullptr );

	// スプライト情報の取得.
	if( FAILED( SpriteStateDataLoad() ) ) return E_FAIL;

	// 初期化.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateTexture() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;

	Log::PushLog( m_SpriteState.FilePath + " 読み込み : 成功" );
	return S_OK;
}

//----------------------------.
// UIでレンダリング.
//----------------------------.
void CSprite::RenderUI( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;

	// 非表示の場合は処理を行わない.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		AnimState->SetAnimNumber();		// アニメーション番号を設定.
		AnimState->SetPatternNo();		// パターン番号を設定.
		AnimState->AnimUpdate();		// アニメーションの更新.
		AnimState->UVScrollUpdate();	// スクロールの更新.
	}

	// 補正値用平行移動行列.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// 使用するシェーダの登録.
	const int PSShaderNo = RenderState->RuleTexture == nullptr ? RenderState->MaskTexture == nullptr || RenderState->IsColorMask ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo],  nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド行列を渡す(ビュー,プロジェクションは使わない).
		D3DXMATRIX m = mWorld;
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = RenderState->Color;

		// テクスチャ座標(UV座標).
		// 1マス当たりの割合にパターン番号(マス目)をかけて座標を設定する.
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// 描画するエリア.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea	 = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// ビューポートの幅、高さを渡す.
		cb.fViewPortWidth	= WndW;
		cb.fViewPortHeight	= WndH;

		// ディザ抜きを使用するか.
		cb.vFlag.x = RenderState->IsDither == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vFlag.y = RenderState->IsAlphaBlock == true ? 1.0f : 0.0f;

		// カラーマスクを使用するかを渡す.
		cb.vFlag.z = RenderState->IsColorMask == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// このコンスタントバッファをどのシェーダで使うか？.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// 頂点バッファをセット.
	UINT stride = sizeof( SVertex );// データの間隔.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBufferUI, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &RenderState->MaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &RenderState->RuleTexture );

	// アルファブレンド有効にする.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// 3Dでレンダリング.
//----------------------------.
void CSprite::Render3D( SSpriteRenderState* pRenderState, const bool IsBillBoard )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
#ifdef _DEBUG
	PositionRenderer::Render( RenderState->Transform );
#endif
	// 非表示の場合は処理を行わない.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		AnimState->SetAnimNumber();		// アニメーション番号を設定.
		AnimState->SetPatternNo();		// パターン番号を設定.
		AnimState->AnimUpdate();		// アニメーションの更新.
		AnimState->UVScrollUpdate();	// スクロールの更新.
	}

	// 補正値用平行移動行列.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// ビルボード用.
	if ( IsBillBoard == true ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// ビュー行列.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;			// xyzを0にする.
		// CancelRotationの逆行列を求めます.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// 使用するシェーダの登録.
	const int PSShaderNo = RenderState->RuleTexture == nullptr ? RenderState->MaskTexture == nullptr || RenderState->IsColorMask ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo], nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド,ビュー,プロジェクション行列を渡す.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = RenderState->Color;

		// テクスチャ座標(UV座標).
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// 描画するエリア.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// ビューポートの幅、高さは使用しないため初期化.
		cb.fViewPortWidth	= 0.0f;
		cb.fViewPortHeight	= 0.0f;

		// ディザ抜きを使用するか.
		cb.vFlag.x = RenderState->IsDither == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vFlag.y = RenderState->IsAlphaBlock == true ? 1.0f : 0.0f;

		// カラーマスクを使用するかを渡す.
		cb.vFlag.z = RenderState->IsColorMask == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// このコンスタントバッファをどのシェーダで使うか？.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// 頂点バッファをセット.
	UINT stride = sizeof( SVertex );// データの間隔.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBuffer3D, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &RenderState->MaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &RenderState->RuleTexture );

	// アルファブレンド有効にする.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !RenderState->IsDither ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// マウスに重なっているか取得する.
//----------------------------.
bool CSprite::GetIsOverlappingTheMouse( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION2&	MousePos		= Input::GetMousePosition();
	const D3DXPOSITION3&	UpperLeftPos	= GetSpriteUpperLeftPos( RenderState );
	const D3DXSCALE3&		Scale			= RenderState->Transform.Scale;

	return	( ( UpperLeftPos.x <= MousePos.x ) && ( MousePos.x <= UpperLeftPos.x + m_SpriteState.Disp.w * Scale.x ) ) &&
			( ( UpperLeftPos.y <= MousePos.y ) && ( MousePos.y <= UpperLeftPos.y + m_SpriteState.Disp.h * Scale.y ) );
}

//---------------------------.
// スプライト情報の取得.
//---------------------------.
HRESULT CSprite::SpriteStateDataLoad()
{
	// 補正値テキストの読み込み.
	std::string TextPath	= m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "\\" ), TextPath.size() ) += "\\OffSet.json";
	json OffSetData			= FileManager::JsonLoad( TextPath );
	
	// 同じ名前のテキストの読み込み.
	TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "." ), TextPath.size() ) += ".json";
#ifndef _DEBUG
	// 暗号化されているため元のパスに戻す
	auto dp = TextPath.rfind( "." );
	auto fp = TextPath.rfind( "\\" ) + 1;
	if ( dp != std::string::npos && fp != std::string::npos ) {
		std::string n = TextPath.substr( fp, dp - fp );
		n.erase( 0, 1 );
		n.erase( n.length() - 2 );
		TextPath.erase( fp, dp - fp );
		TextPath.insert( fp, n );
	}
#endif
	m_SpriteStateData = FileManager::JsonLoad( TextPath );

	// ファイルが無いためファイルを作成する.
	if ( m_SpriteStateData.empty() ) {
#ifdef _DEBUG
		Log::PushLog( TextPath + " が無いため作成します。" );
		if ( FAILED( CreateSpriteState() ) ) return E_FAIL;

		// 作成できたためもう一度読み込み直す.
		Log::PushLog( m_SpriteState.FilePath + " をもう一度読み込み直します。" );
		SpriteStateDataLoad();
		return S_OK;
#else
		return E_FAIL;
#endif
	}

	// スプライト情報の取得.
	m_SpriteState.LocalPosNo	= static_cast<ELocalPosition>( m_SpriteStateData["LocalPosition"] );
	m_SpriteState.Disp.w		= m_SpriteStateData["Disp"]["w"];
	m_SpriteState.Disp.h		= m_SpriteStateData["Disp"]["h"];
	m_SpriteState.Base.w		= m_SpriteStateData["Base"]["w"];
	m_SpriteState.Base.h		= m_SpriteStateData["Base"]["h"];
	m_SpriteState.Stride.w		= m_SpriteStateData["Stride"]["w"];
	m_SpriteState.Stride.h		= m_SpriteStateData["Stride"]["h"];
	m_SpriteState.AddCenterPos	= GetAddCenterPosition();

	// 補正値の設定.
	if ( OffSetData.empty() ) m_SpriteState.OffSet = { 0.0f, 0.0f };
	else {
		m_SpriteState.OffSet.x	= OffSetData["x"];
		m_SpriteState.OffSet.y	= OffSetData["y"];
	}
	return S_OK;
}

//---------------------------.
// スプライトの左上の座標を取得する.
//---------------------------.
D3DXVECTOR3 CSprite::GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION3&	Pos			= RenderState->Transform.Position;
	const SSize&			Size		= m_SpriteState.Disp;
	const D3DXSCALE3		Scale		= RenderState->Transform.Scale;

	// 左上の座標を計算して返す.
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::Left:		return D3DXPOSITION3( Pos.x,							Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::LeftDown:	return D3DXPOSITION3( Pos.x,							Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Down:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::RightDown:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Right:		return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::RightUp:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y,								Pos.z );
	case ELocalPosition::Up:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y,								Pos.z );
	case ELocalPosition::Center:	return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	default:						return Pos;
	}
}

//----------------------------.
// シェーダ作成.
//----------------------------.
HRESULT CSprite::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

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
		},
		{
			"TEXCOORD",						// テクスチャ位置.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		// DXGIのフォーマット(32bit float型*2).
			0,
			12,								// データの開始位置.
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
			&m_pVertexLayout)))	//(out)頂点インプットレイアウト.
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	const int Size = static_cast<int>( m_pPixelShaders.size() );
	for( int i = 0; i < Size; ++i ){
		// HLSLからピクセルシェーダのブロブを作成.
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME,				// シェーダファイル名(HLSLファイル).
				nullptr,					// マクロ定義の配列へのポインタ(未使用).
				nullptr,					// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
				PS_SHADER_ENTRY_NAMES[i],	// シェーダエントリーポイント関数の名前.
				"ps_5_0",					// シェーダのモデルを指定する文字列(プロファイル).
				uCompileFlag,				// シェーダコンパイルフラグ.
				0,							// エフェクトコンパイルフラグ(未使用).
				nullptr,					// スレッド ポンプ インターフェイスへのポインタ(未使用).
				&pCompiledShader,			// ブロブを格納するメモリへのポインタ.
				&pErrors,					// エラーと警告一覧を格納するメモリへのポインタ.
				nullptr)))					// 戻り値へのポインタ(未使用).
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
				&m_pPixelShaders[i] )))	// (out)ピクセルシェーダ.
		{
			ErrorMessage( "ピクセルシェーダ「" + std::string( PS_SHADER_ENTRY_NAMES[i] ) + "」作成失敗" );
			return E_FAIL;
		}
		SAFE_RELEASE(pCompiledShader);
	}

	// コンスタント(定数)バッファ作成.
	//	シェーダに特定の数値を送るバッファ.
	//	ここでは変換行列渡し用.
	//	シェーダに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;				// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof( SSpriteShaderConstantBuffer );	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;					// 書き込みでアクセス.
	cb.MiscFlags			= 0;										// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;										// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;						// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		m_pDevice->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// モデル作成(UI).
//----------------------------.
HRESULT CSprite::CreateModelUI()
{
	SSpriteAnimState* AnimState = &m_SpriteRenderState.AnimState;

	float w = m_SpriteState.Disp.w;
	float h = m_SpriteState.Disp.h;
	float u = m_SpriteState.Stride.w / m_SpriteState.Base.w;// 1マス当たりの幅.
	float v = m_SpriteState.Stride.h / m_SpriteState.Base.h;// 1マス当たりの高さ.

	// x,yそれぞれの最大マス数.
	AnimState->PatternMax.x		= static_cast<SHORT>( m_SpriteState.Base.w / m_SpriteState.Stride.w );
	AnimState->PatternMax.y		= static_cast<SHORT>( m_SpriteState.Base.h / m_SpriteState.Stride.h );
	AnimState->AnimNumberMax	= AnimState->PatternMax.x * AnimState->PatternMax.y;

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, h, u, v );
	// 最大要素数を算出する.
	UINT uVerMax = sizeof(m_Vertices) / sizeof(m_Vertices[0]);

	// バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// 使用方法(デフォルト).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// 頂点のサイズ.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// 頂点バッファとして扱う.
	bd.CPUAccessFlags		= 0;							// CPUからはアクセスしない.
	bd.MiscFlags			= 0;							// その他のフラグ(未使用).
	bd.StructureByteStride	= 0;							// 構造体のサイズ(未使用).

	// サブリソースデータ構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;	// 板ポリの頂点をセット.

	// 頂点バッファの作成.
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBufferUI)))
	{
		ErrorMessage( "頂点バッファUI作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// モデル作成(3D).
//----------------------------.
HRESULT CSprite::CreateModel3D()
{
	// 画像の比率を求める.
	int		as = myGcd( static_cast<int>( m_SpriteState.Disp.w ), static_cast<int>( m_SpriteState.Disp.h ) );
	float	w = ( m_SpriteState.Disp.w / as ) * m_SpriteState.Disp.w * 0.1f;
	float	h = ( m_SpriteState.Disp.h / as ) * m_SpriteState.Disp.h * 0.1f;
	float	u = m_SpriteState.Stride.w / m_SpriteState.Base.w;	// 1コマ当たりの幅,
	float	v = m_SpriteState.Stride.h / m_SpriteState.Base.h;	// 1コマ当たりの高さ.

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, -h, u, v );
	// 最大要素数を算出する.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// 使用方法(デフォルト).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// 頂点のサイズ.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// 頂点バッファとして扱う.
	bd.CPUAccessFlags		= 0;							// CPUからはアクセスしない.
	bd.MiscFlags			= 0;							// その他のフラグ(未使用).
	bd.StructureByteStride	= 0;							// 構造体のサイズ(未使用).

	// サブリソース構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;

	// 頂点バッファの作成.
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer3D)))
	{
		ErrorMessage( "頂点バッファ3D作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// テクスチャ作成.
//----------------------------.
HRESULT CSprite::CreateTexture()
{
	// ファイルパスを w_char に変換する.
	const std::wstring wFilePath = StringConversion::to_wString( m_SpriteState.FilePath );

	//テクスチャ作成.
	if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
		wFilePath.c_str(),	// ファイル名.
		nullptr, nullptr,
		&m_pTexture,		// (out)テクスチャ.
		nullptr ) ) )
	{
		ErrorMessage( "テクスチャ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// サンプラ作成.
//----------------------------.
HRESULT CSprite::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;

	// サンプラ作成.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		if( FAILED( m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] ))){
			ErrorMessage( "サンプラ作成 : 失敗" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// 頂点情報の作成.
//---------------------------.
HRESULT CSprite::CreateVertex( const float w, const float h, const float u, const float v )
{
	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Left:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::LeftDown:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Down:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightDown:
		m_Vertices[0] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Right:
		m_Vertices[0] = { D3DXPOSITION3(   -w,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightUp:
		m_Vertices[0] = { D3DXPOSITION3(   -w,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Up:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Center:
	default: 
		m_Vertices[0] = { D3DXVECTOR3( -w / 2,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXVECTOR3( -w / 2, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXVECTOR3(  w / 2,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXVECTOR3(  w / 2, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	}
	return S_OK;
}

//---------------------------.
// スプライト情報の作成.
//---------------------------.
HRESULT CSprite::CreateSpriteState()
{
	// 作成するファイルパス.
	std::string TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

	// 画像の幅、高さの取得.
	const D3DXVECTOR2& BaseSize = ImageSize::GetImageSize( m_SpriteState.FilePath );

	// 情報を追加していく.
	json SpriteState;
	SpriteState["LocalPosition"]		= 0;
	SpriteState["Disp"]["w"]			= BaseSize.x;
	SpriteState["Disp"]["h"]			= BaseSize.y;
	SpriteState["Base"]["w"]			= BaseSize.x;
	SpriteState["Base"]["h"]			= BaseSize.y;
	SpriteState["Stride"]["w"]			= BaseSize.x;
	SpriteState["Stride"]["h"]			= BaseSize.y;

	// スプライト情報の作成.
	if( FAILED( FileManager::JsonSave( TextPath, SpriteState ) ) ) return E_FAIL;
	Log::PushLog( TextPath + " ファイルの作成 : 成功" );

	// 作成成功.
	return S_OK;
}

//---------------------------.
// 中心座標に変換するための追加座標の取得.
//---------------------------.
D3DXPOSITION3 CSprite::GetAddCenterPosition()
{
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Left:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::LeftDown:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Down:
		return D3DXPOSITION3(  0.0f,						-m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::RightDown:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Right:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::RightUp:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Up:
		return D3DXPOSITION3(  0.0f,						 m_SpriteState.Disp.h / 2.0f,	0.0f );
	default:
		return INIT_FLOAT3;
	}
}