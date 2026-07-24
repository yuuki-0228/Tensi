#include "VisualEffectManager.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "RenderTexture.h"
#include "Effects\SinglePassEffects.h"
#include "Effects\GaussianBlurEffect.h"
#include "Effects\BloomEffect.h"
#include "Effects\MotionBlurEffect.h"
#include "Effects\FrostEffect.h"
#include "Effects\SoftFocusEffect.h"
#include "..\..\Common\DirectX\DirectX11.h"
#include "..\ImGuiManager\DebugWindow\DebugWindow.h"
#include <algorithm>

namespace {
	// 全画面三角形の頂点シェーダを含むシェーダファイル名(ディレクトリも含む).
	constexpr TCHAR VS_SHADER_NAME[] = _T( "Data\\Shader\\VisualEffects\\SinglePass.hlsl" );

	// 経過時間の折り返し(秒). シェーダへ渡す精度を保つため周期的に折り返す.
	constexpr float TIME_WRAP = 3600.0f;

	// ImGui表示用のエフェクト名( EVisualEffect と同じ並び ).
	constexpr const char* EFFECT_NAMES[static_cast<size_t>( EVisualEffect::Max )] = {
		"Grayscale",
		"Sepia",
		"Invert",
		"Posterize",
		"ColorFilter",
		"ColorCorrection",
		"Vignette",
		"Noise",
		"FilmGrain",
		"Scanline",
		"CRT",
		"Pixelate",
		"RGBShift",
		"Glitch",
		"Wave",
		"Sharpen",
		"RadialBlur",
		"GaussianBlur",
		"Bloom",
		"MotionBlur",
		"Frost",
		"Toon",
		"Watercolor",
		"OilPainting",
		"ColoredPencil",
		"Outline",
		"EdgeDetect",
		"EdgeEnhance",
		"Halftone",
		"Emboss",
		"Threshold",
		"Fisheye",
		"HueShift",
		"SoftFocus",
		"HeatHaze",
		"OldFilm",
		"Rain",
		"Snow",
		"Fog",
		"FogDepth",
	};
}

VisualEffectManager::VisualEffectManager()
	: m_pVertexShader			( nullptr )
	, m_pCopyPS					( nullptr )
	, m_pCommonCB				( nullptr )
	, m_pParamCB				( nullptr )
	, m_pSampler				( nullptr )
	, m_pRasterizerState		( nullptr )
	, m_pPingPong				()
	, m_pWork					()
	, m_pCaptures				()
	, m_CaptureStack			()
	, m_pEffects				()
	, m_ScreenEffects			()
	, m_pPrevRasterizerState	( nullptr )
	, m_PrevIsDepth				( false )
	, m_PrevIsAlphaBlend		( false )
	, m_PrevIsAlphaToCoverage	( false )
	, m_Time					( 0.0f )
	, m_Width					( 0 )
	, m_Height					( 0 )
	, m_IsInit					( false )
	, m_IsInitFailed			( false )
	, m_IsScreenAppliedThisFrame( false )
{
}

VisualEffectManager::~VisualEffectManager()
{
	ReleaseResources();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
VisualEffectManager* VisualEffectManager::GetInstance()
{
	static std::unique_ptr<VisualEffectManager> pInstance = std::make_unique<VisualEffectManager>();
	return pInstance.get();
}

//----------------------------.
// 更新.
//----------------------------.
void VisualEffectManager::Update( const float& DeltaTime )
{
	VisualEffectManager* pI = GetInstance();

	// 経過時間の加算( シェーダのアニメーション用 ).
	pI->m_Time += DeltaTime;
	if ( pI->m_Time >= TIME_WRAP ) pI->m_Time -= TIME_WRAP;

	// 画面全体エフェクトの適用済みフラグを毎フレームリセットする.
	pI->m_IsScreenAppliedThisFrame = false;

#ifdef _DEBUG
	DebugWindow::PushProc( "VisualEffect", [=]() { pI->DebugRender(); } );
#endif	// #ifdef _DEBUG
}

//----------------------------.
// 解放.
//----------------------------.
void VisualEffectManager::Release()
{
	GetInstance()->ReleaseResources();
}

//----------------------------.
// リソースの解放本体.
//----------------------------.
void VisualEffectManager::ReleaseResources()
{
	for ( auto& pEffect : m_pEffects ) pEffect.reset();
	m_pCaptures.clear();
	m_CaptureStack.clear();
	for ( auto& pRT : m_pPingPong )	pRT.reset();
	for ( auto& pRT : m_pWork )		pRT.reset();

	SAFE_RELEASE( m_pPrevRasterizerState );
	SAFE_RELEASE( m_pRasterizerState );
	SAFE_RELEASE( m_pSampler		);
	SAFE_RELEASE( m_pParamCB		);
	SAFE_RELEASE( m_pCommonCB		);
	SAFE_RELEASE( m_pCopyPS			);
	SAFE_RELEASE( m_pVertexShader	);

	m_Width		= 0;
	m_Height	= 0;
	m_IsInit	= false;
}

//----------------------------.
// 画面全体エフェクトの追加.
//----------------------------.
void VisualEffectManager::AddScreenEffect( const EVisualEffect Type )
{
	VisualEffectManager* pI = GetInstance();
	if ( HasScreenEffect( Type ) ) return;
	pI->m_ScreenEffects.push_back( Type );
}

//----------------------------.
// 画面全体エフェクトの削除.
//----------------------------.
void VisualEffectManager::RemoveScreenEffect( const EVisualEffect Type )
{
	VisualEffectManager* pI = GetInstance();
	auto& List = pI->m_ScreenEffects;
	List.erase( std::remove( List.begin(), List.end(), Type ), List.end() );
}

//----------------------------.
// 画面全体エフェクトの全削除.
//----------------------------.
void VisualEffectManager::ClearScreenEffect()
{
	GetInstance()->m_ScreenEffects.clear();
}

//----------------------------.
// 画面全体エフェクトの一括設定.
//----------------------------.
void VisualEffectManager::SetScreenEffectList( const std::vector<EVisualEffect>& List )
{
	GetInstance()->m_ScreenEffects = List;
}

//----------------------------.
// 画面全体エフェクトの取得.
//----------------------------.
std::vector<EVisualEffect> VisualEffectManager::GetScreenEffectList()
{
	return GetInstance()->m_ScreenEffects;
}

//----------------------------.
// 指定のエフェクトが画面全体に登録されているか.
//----------------------------.
bool VisualEffectManager::HasScreenEffect( const EVisualEffect Type )
{
	const auto& List = GetInstance()->m_ScreenEffects;
	return std::find( List.begin(), List.end(), Type ) != List.end();
}

//----------------------------.
// 画面全体エフェクトの適用.
//----------------------------.
void VisualEffectManager::ApplyScreenEffects()
{
	VisualEffectManager* pI = GetInstance();

	// 適用済み( シーン側で手動適用済み )なら何もしない.
	if ( pI->m_IsScreenAppliedThisFrame )	return;
	if ( pI->m_ScreenEffects.empty() )		return;
	if ( pI->EnsureInit() == false )		return;
	if ( pI->EnsureSize() == false )		return;
	pI->m_IsScreenAppliedThisFrame = true;

	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// MSAA 有効時はシーンターゲットから、無効時はバックバッファから取り込む.
	const bool IsMsaa = DirectX11::GetMsaaSampleCount() > 1 && DirectX11::GetSceneTex( 0 ) != nullptr;

	// 描画先を一旦外す( 取り込み元がバインドされたままにならないようにする ).
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pContext->OMSetRenderTargets( 1, &pNullRTV, nullptr );

	// 現在のシーンの画をピンポンバッファへ取り込む.
	if ( IsMsaa ) {
		pContext->ResolveSubresource(
			pI->m_pPingPong[0]->GetTex(), 0, DirectX11::GetSceneTex( 0 ), 0, DXGI_FORMAT_B8G8R8A8_UNORM );
	}
	else {
		ID3D11Texture2D* pBackBuffer = DirectX11::GetBackBufferTex( 0 );
		if ( pBackBuffer == nullptr ) return;
		pContext->CopyResource( pI->m_pPingPong[0]->GetTex(), pBackBuffer );
	}

	// エフェクトチェーンを適用して元の描画先へ書き戻す.
	ID3D11RenderTargetView* pTargetRTV =
		IsMsaa ? DirectX11::GetSceneRTV( 0 ) : DirectX11::GetBackBufferRTV( 0 );
	pI->PushRenderStates();
	pI->ApplyChain( pI->m_pPingPong[0]->GetSRV(), pTargetRTV, pI->m_ScreenEffects );
	pI->PopRenderStates();

	// 描画先を元に戻す( この後の FPS表示等が今まで通り描けるようにする ).
	pContext->OMSetRenderTargets( 1, &pTargetRTV, DirectX11::GetDepthSV() );
}

//----------------------------.
// キャプチャ開始.
//----------------------------.
void VisualEffectManager::BeginCapture( const bool UseDepth )
{
	VisualEffectManager* pI = GetInstance();
	if ( pI->EnsureInit() == false ) return;
	if ( pI->EnsureSize() == false ) return;

	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// キャプチャ先の用意( ネストの深さ分だけ作成する ).
	const int Depth = static_cast<int>( pI->m_CaptureStack.size() );
	while ( static_cast<int>( pI->m_pCaptures.size() ) <= Depth ) {
		pI->m_pCaptures.push_back( std::make_unique<CRenderTexture>() );
	}
	CRenderTexture* pCapture = pI->m_pCaptures[Depth].get();
	if ( pCapture->IsSameSize( pI->m_Width, pI->m_Height ) == false ||
		 ( UseDepth && pCapture->GetDSV() == nullptr ) )
	{
		if ( FAILED( pCapture->Create( pI->m_Width, pI->m_Height, UseDepth ) ) ) return;
	}
	pCapture->Clear();

	// 現在の描画先を退避する.
	SCaptureState State = { nullptr, nullptr, Depth };
	pContext->OMGetRenderTargets( 1, &State.pPrevRTV, &State.pPrevDSV );
	pI->m_CaptureStack.push_back( State );

	// キャプチャ先へ切り替える.
	ID3D11RenderTargetView* pRTV = pCapture->GetRTV();
	pContext->OMSetRenderTargets( 1, &pRTV, UseDepth ? pCapture->GetDSV() : nullptr );
}

//----------------------------.
// キャプチャ終了.
//----------------------------.
void VisualEffectManager::EndCapture( const std::vector<EVisualEffect>& EffectList )
{
	VisualEffectManager* pI = GetInstance();
	if ( pI->m_CaptureStack.empty() ) return;

	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// 退避情報の取り出し.
	SCaptureState State = pI->m_CaptureStack.back();
	pI->m_CaptureStack.pop_back();
	CRenderTexture* pCapture = pI->m_pCaptures[State.RtIndex].get();

	// 描画先を一旦外す( キャプチャ先を入力として使うため ).
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pContext->OMSetRenderTargets( 1, &pNullRTV, nullptr );

	pI->PushRenderStates();

	// エフェクトチェーンの適用( 空ならキャプチャした画をそのまま使う ).
	ID3D11ShaderResourceView* pResultSRV = pCapture->GetSRV();
	int LastPing = -1;
	for ( const EVisualEffect Type : EffectList ) {
		CVisualEffectBase* pEffect = GetEffect( Type );
		if ( pEffect == nullptr || pEffect->EnsureInit() == false ) continue;

		const int Next = ( LastPing == 0 ) ? 1 : 0;
		CRenderTexture* pDst = pI->m_pPingPong[Next].get();
		pI->UpdateCommonCBForEffect( pEffect );
		pEffect->Apply( pResultSRV, pDst->GetRTV() );
		pResultSRV	= pDst->GetSRV();
		LastPing	= Next;
	}

	// 元の描画先へアルファブレンドで合成する.
	DirectX11::SetAlphaBlend( true );
	DrawCopy( State.pPrevRTV, pResultSRV );
	DirectX11::SetAlphaBlend( false );

	pI->PopRenderStates();

	// 描画先を元に戻す.
	pContext->OMSetRenderTargets( 1, &State.pPrevRTV, State.pPrevDSV );
	SAFE_RELEASE( State.pPrevRTV );
	SAFE_RELEASE( State.pPrevDSV );
}

//----------------------------.
// 適用率の設定.
//----------------------------.
void VisualEffectManager::SetIntensity( const EVisualEffect Type, const float Value )
{
	CVisualEffectBase* pEffect = GetEffect( Type );
	if ( pEffect != nullptr ) pEffect->SetIntensity( Value );
}

//----------------------------.
// 適用範囲の設定.
//----------------------------.
void VisualEffectManager::SetArea( const EVisualEffect Type, const D3DXVECTOR4& Area )
{
	CVisualEffectBase* pEffect = GetEffect( Type );
	if ( pEffect != nullptr ) pEffect->SetArea( Area );
}

//----------------------------.
// 透明部分の持ち上げアルファの設定.
//----------------------------.
void VisualEffectManager::SetAlphaLift( const EVisualEffect Type, const float Value )
{
	CVisualEffectBase* pEffect = GetEffect( Type );
	if ( pEffect != nullptr ) pEffect->SetAlphaLift( Value );
}

//----------------------------.
// エフェクトの取得( 未生成なら生成する ).
//----------------------------.
CVisualEffectBase* VisualEffectManager::GetEffect( const EVisualEffect Type )
{
	VisualEffectManager* pI = GetInstance();
	const size_t Index = static_cast<size_t>( Type );
	if ( Index >= static_cast<size_t>( EVisualEffect::Max ) ) return nullptr;

	if ( pI->m_pEffects[Index] == nullptr ) {
		pI->m_pEffects[Index] = CreateEffect( Type );
	}
	return pI->m_pEffects[Index].get();
}

//----------------------------.
// エフェクトの生成.
//----------------------------.
std::unique_ptr<CVisualEffectBase> VisualEffectManager::CreateEffect( const EVisualEffect Type )
{
	switch ( Type ) {
	case EVisualEffect::Grayscale:			return std::make_unique<CGrayscaleEffect>();
	case EVisualEffect::Sepia:				return std::make_unique<CSepiaEffect>();
	case EVisualEffect::Invert:				return std::make_unique<CInvertEffect>();
	case EVisualEffect::Posterize:			return std::make_unique<CPosterizeEffect>();
	case EVisualEffect::ColorFilter:		return std::make_unique<CColorFilterEffect>();
	case EVisualEffect::ColorCorrection:	return std::make_unique<CColorCorrectionEffect>();
	case EVisualEffect::Vignette:			return std::make_unique<CVignetteEffect>();
	case EVisualEffect::Noise:				return std::make_unique<CNoiseEffect>();
	case EVisualEffect::FilmGrain:			return std::make_unique<CFilmGrainEffect>();
	case EVisualEffect::Scanline:			return std::make_unique<CScanlineEffect>();
	case EVisualEffect::CRT:				return std::make_unique<CCRTEffect>();
	case EVisualEffect::Pixelate:			return std::make_unique<CPixelateEffect>();
	case EVisualEffect::RGBShift:			return std::make_unique<CRGBShiftEffect>();
	case EVisualEffect::Glitch:				return std::make_unique<CGlitchEffect>();
	case EVisualEffect::Wave:				return std::make_unique<CWaveEffect>();
	case EVisualEffect::Sharpen:			return std::make_unique<CSharpenEffect>();
	case EVisualEffect::RadialBlur:			return std::make_unique<CRadialBlurEffect>();
	case EVisualEffect::GaussianBlur:		return std::make_unique<CGaussianBlurEffect>();
	case EVisualEffect::Bloom:				return std::make_unique<CBloomEffect>();
	case EVisualEffect::MotionBlur:			return std::make_unique<CMotionBlurEffect>();
	case EVisualEffect::Frost:				return std::make_unique<CFrostEffect>();
	case EVisualEffect::Toon:				return std::make_unique<CToonEffect>();
	case EVisualEffect::Watercolor:			return std::make_unique<CWatercolorEffect>();
	case EVisualEffect::OilPainting:		return std::make_unique<COilPaintingEffect>();
	case EVisualEffect::ColoredPencil:		return std::make_unique<CColoredPencilEffect>();
	case EVisualEffect::Outline:			return std::make_unique<COutlineEffect>();
	case EVisualEffect::EdgeDetect:			return std::make_unique<CEdgeDetectEffect>();
	case EVisualEffect::EdgeEnhance:		return std::make_unique<CEdgeEnhanceEffect>();
	case EVisualEffect::Halftone:			return std::make_unique<CHalftoneEffect>();
	case EVisualEffect::Emboss:				return std::make_unique<CEmbossEffect>();
	case EVisualEffect::Threshold:			return std::make_unique<CThresholdEffect>();
	case EVisualEffect::Fisheye:			return std::make_unique<CFisheyeEffect>();
	case EVisualEffect::HueShift:			return std::make_unique<CHueShiftEffect>();
	case EVisualEffect::SoftFocus:			return std::make_unique<CSoftFocusEffect>();
	case EVisualEffect::HeatHaze:			return std::make_unique<CHeatHazeEffect>();
	case EVisualEffect::OldFilm:			return std::make_unique<COldFilmEffect>();
	case EVisualEffect::Rain:				return std::make_unique<CRainEffect>();
	case EVisualEffect::Snow:				return std::make_unique<CSnowEffect>();
	case EVisualEffect::Fog:				return std::make_unique<CFogEffect>();
	case EVisualEffect::FogDepth:			return std::make_unique<CFogDepthEffect>();
	default:								return nullptr;
	}
}

//----------------------------.
// 全画面1パス描画.
//----------------------------.
void VisualEffectManager::DrawPass(
	ID3D11PixelShader* pPS,
	ID3D11RenderTargetView* pDstRTV,
	ID3D11ShaderResourceView* pSrcSRV,
	ID3D11ShaderResourceView* pSubSRV,
	const D3DXVECTOR4* pParams,
	const int ParamNum )
{
	VisualEffectManager* pI = GetInstance();
	if ( pPS == nullptr || pDstRTV == nullptr || pI->m_IsInit == false ) return;

	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// エフェクトパラメータ定数バッファ( b1 )の更新.
	if ( pParams != nullptr && ParamNum > 0 ) {
		D3DXVECTOR4 Params[CVisualEffectBase::PARAM_VECTOR_NUM] = {};
		const int Num = ( std::min )( ParamNum, CVisualEffectBase::PARAM_VECTOR_NUM );
		for ( int i = 0; i < Num; ++i ) Params[i] = pParams[i];
		pContext->UpdateSubresource( pI->m_pParamCB, 0, nullptr, Params, 0, 0 );
	}

	// 描画先の設定.
	pContext->OMSetRenderTargets( 1, &pDstRTV, nullptr );

	// ビューポートの設定( 全画面 ).
	D3D11_VIEWPORT Viewport = {};
	Viewport.Width		= static_cast<FLOAT>( pI->m_Width );
	Viewport.Height		= static_cast<FLOAT>( pI->m_Height );
	Viewport.MaxDepth	= 1.0f;
	pContext->RSSetViewports( 1, &Viewport );

	// シェーダの設定( 頂点バッファ無しの全画面三角形 ).
	pContext->IASetInputLayout( nullptr );
	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pContext->VSSetShader( pI->m_pVertexShader, nullptr, 0 );
	pContext->PSSetShader( pPS, nullptr, 0 );

	// 定数バッファ・テクスチャ・サンプラの設定.
	ID3D11Buffer* pCBs[2] = { pI->m_pCommonCB, pI->m_pParamCB };
	pContext->PSSetConstantBuffers( 0, 2, pCBs );
	ID3D11ShaderResourceView* pSRVs[2] = { pSrcSRV, pSubSRV };
	pContext->PSSetShaderResources( 0, 2, pSRVs );
	pContext->PSSetSamplers( 0, 1, &pI->m_pSampler );

	pContext->Draw( 3, 0 );

	// 次のパスで入出力が入れ替わっても問題無いように入力を外しておく.
	ID3D11ShaderResourceView* pNullSRVs[2] = { nullptr, nullptr };
	pContext->PSSetShaderResources( 0, 2, pNullSRVs );
	ID3D11RenderTargetView* pNullRTV = nullptr;
	pContext->OMSetRenderTargets( 1, &pNullRTV, nullptr );
}

//----------------------------.
// 全画面コピー描画.
//----------------------------.
void VisualEffectManager::DrawCopy( ID3D11RenderTargetView* pDstRTV, ID3D11ShaderResourceView* pSrcSRV )
{
	VisualEffectManager* pI = GetInstance();

	// 等倍設定( 適用率1, 範囲指定無し )で上書きしてからコピーする.
	pI->UpdateCommonCB( 1.0f, 0.0f, D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	DrawPass( pI->m_pCopyPS, pDstRTV, pSrcSRV );
}

//----------------------------.
// ワーク用レンダーテクスチャの取得.
//----------------------------.
CRenderTexture* VisualEffectManager::GetWorkRT( const int No )
{
	VisualEffectManager* pI = GetInstance();
	if ( No < 0 || No >= 2 ) return nullptr;
	return pI->m_pWork[No].get();
}

//----------------------------.
// 経過時間の取得.
//----------------------------.
float VisualEffectManager::GetTime()
{
	return GetInstance()->m_Time;
}

//----------------------------.
// 初期化.
//----------------------------.
bool VisualEffectManager::EnsureInit()
{
	if ( m_IsInit )			return true;
	if ( m_IsInitFailed )	return false;

	ID3D11Device* pDevice = DirectX11::GetDevice();
	if ( pDevice == nullptr ) return false;

	UINT CompileFlag = 0;
#ifdef _DEBUG
	CompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG

	// 全画面三角形用頂点シェーダの作成.
	ID3DBlob* pCompiledShader	= nullptr;
	ID3DBlob* pErrors			= nullptr;
	if ( FAILED( DirectX11::MutexD3DX11CompileFromFile(
		VS_SHADER_NAME, nullptr, nullptr, "VS_Main", "vs_5_0",
		CompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr ) ) )
	{
		if ( pErrors != nullptr ) {
			Log::PushLogError( std::string( "視覚効果の頂点シェーダのコンパイル失敗 : " )
				+ static_cast<char*>( pErrors->GetBufferPointer() ) );
		}
		SAFE_RELEASE( pErrors );
		m_IsInitFailed = true;
		return false;
	}
	SAFE_RELEASE( pErrors );

	if ( FAILED( DirectX11::MutexDX11CreateVertexShader( pCompiledShader, nullptr, &m_pVertexShader ) ) ) {
		SAFE_RELEASE( pCompiledShader );
		Log::PushLogError( "視覚効果の頂点シェーダの作成失敗" );
		m_IsInitFailed = true;
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// 定数バッファの作成.
	auto CreateCB = [&]( const UINT Size, ID3D11Buffer** ppOut ) -> bool
	{
		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth	= Size;
		Desc.Usage		= D3D11_USAGE_DEFAULT;
		Desc.BindFlags	= D3D11_BIND_CONSTANT_BUFFER;
		return SUCCEEDED( pDevice->CreateBuffer( &Desc, nullptr, ppOut ) );
	};
	if ( CreateCB( sizeof( SCommonCB ), &m_pCommonCB ) == false ||
		 CreateCB( sizeof( D3DXVECTOR4 ) * CVisualEffectBase::PARAM_VECTOR_NUM, &m_pParamCB ) == false )
	{
		Log::PushLogError( "視覚効果の定数バッファ作成失敗" );
		m_IsInitFailed = true;
		return false;
	}

	// サンプラ( リニア/クランプ )の作成.
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU	= D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV	= D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW	= D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.MaxLOD		= D3D11_FLOAT32_MAX;
	if ( FAILED( pDevice->CreateSamplerState( &SamplerDesc, &m_pSampler ) ) ) {
		Log::PushLogError( "視覚効果のサンプラ作成失敗" );
		m_IsInitFailed = true;
		return false;
	}

	// コピー用ピクセルシェーダの作成.
	if ( FAILED( DirectX11::MutexD3DX11CompileFromFile(
		VS_SHADER_NAME, nullptr, nullptr, "PS_Copy", "ps_5_0",
		CompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr ) ) )
	{
		SAFE_RELEASE( pErrors );
		Log::PushLogError( "視覚効果のコピーシェーダのコンパイル失敗" );
		m_IsInitFailed = true;
		return false;
	}
	SAFE_RELEASE( pErrors );
	if ( FAILED( DirectX11::MutexDX11CreatePixelShader( pCompiledShader, nullptr, &m_pCopyPS ) ) ) {
		SAFE_RELEASE( pCompiledShader );
		Log::PushLogError( "視覚効果のコピーシェーダの作成失敗" );
		m_IsInitFailed = true;
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// 全画面描画用ラスタライザ( カリング無し )の作成.
	//	DirectX11 側のステートキャッシュとずれないように自前で持ち、
	//	RSSetState で直接セットして使用後に元のポインタへ戻す.
	D3D11_RASTERIZER_DESC RasterizerDesc = {};
	RasterizerDesc.FillMode			= D3D11_FILL_SOLID;
	RasterizerDesc.CullMode			= D3D11_CULL_NONE;
	RasterizerDesc.DepthClipEnable	= TRUE;
	if ( FAILED( pDevice->CreateRasterizerState( &RasterizerDesc, &m_pRasterizerState ) ) ) {
		Log::PushLogError( "視覚効果のラスタライザ作成失敗" );
		m_IsInitFailed = true;
		return false;
	}

	m_IsInit = true;
	return true;
}

//----------------------------.
// 画面サイズに合わせて各レンダーテクスチャを作り直す.
//----------------------------.
bool VisualEffectManager::EnsureSize()
{
	const UINT Width	= static_cast<UINT>( DirectX11::GetWndWidth() );
	const UINT Height	= static_cast<UINT>( DirectX11::GetWndHeight() );
	if ( Width == 0 || Height == 0 ) return false;
	if ( m_Width == Width && m_Height == Height ) return true;

	for ( int i = 0; i < 2; ++i ) {
		if ( m_pPingPong[i] == nullptr )	m_pPingPong[i]	= std::make_unique<CRenderTexture>();
		if ( m_pWork[i] == nullptr )		m_pWork[i]		= std::make_unique<CRenderTexture>();
		if ( FAILED( m_pPingPong[i]->Create( Width, Height ) ) )	return false;
		if ( FAILED( m_pWork[i]->Create( Width, Height ) ) )		return false;
	}
	// キャプチャ先はサイズが合わなくなったので破棄する( 次回使用時に作り直す ).
	m_pCaptures.clear();

	m_Width		= Width;
	m_Height	= Height;
	return true;
}

//----------------------------.
// エフェクトチェーンの適用.
//----------------------------.
void VisualEffectManager::ApplyChain( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV, const std::vector<EVisualEffect>& List )
{
	ID3D11ShaderResourceView* pCurSRV = pSrcSRV;
	int LastPing = 0;	// pSrcSRV はピンポン0から始まる前提.

	for ( const EVisualEffect Type : List ) {
		CVisualEffectBase* pEffect = GetEffect( Type );
		if ( pEffect == nullptr || pEffect->EnsureInit() == false ) continue;

		const int Next = ( LastPing == 0 ) ? 1 : 0;
		CRenderTexture* pDst = m_pPingPong[Next].get();
		UpdateCommonCBForEffect( pEffect );
		pEffect->Apply( pCurSRV, pDst->GetRTV() );
		pCurSRV		= pDst->GetSRV();
		LastPing	= Next;
	}

	// 最終結果を出力先へコピーする.
	DrawCopy( pDstRTV, pCurSRV );
}

//----------------------------.
// 共通定数バッファの更新.
//----------------------------.
void VisualEffectManager::UpdateCommonCB( const float Intensity, const float AlphaLift, const D3DXVECTOR4& Area )
{
	if ( m_pCommonCB == nullptr ) return;

	SCommonCB CB = {};
	CB.Common0	= D3DXVECTOR4( static_cast<float>( m_Width ), static_cast<float>( m_Height ), m_Time, Intensity );
	CB.Common1	= D3DXVECTOR4( AlphaLift, 0.0f, 0.0f, 0.0f );
	CB.Area		= Area;
	DirectX11::GetContext()->UpdateSubresource( m_pCommonCB, 0, nullptr, &CB, 0, 0 );
}

//----------------------------.
// 現在のエフェクトに合わせた共通定数バッファの更新.
//----------------------------.
void VisualEffectManager::UpdateCommonCBForEffect( const CVisualEffectBase* pEffect )
{
	UpdateCommonCB( pEffect->GetIntensity(), pEffect->GetAlphaLift(), pEffect->GetArea() );
}

//----------------------------.
// ステートを視覚効果用に切り替える.
//----------------------------.
void VisualEffectManager::PushRenderStates()
{
	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// 現在のステートを退避する.
	m_PrevIsDepth			= DirectX11::GetDepth();
	m_PrevIsAlphaBlend		= DirectX11::GetAlphaBlend();
	m_PrevIsAlphaToCoverage	= DirectX11::GetAlphaToCoverage();
	SAFE_RELEASE( m_pPrevRasterizerState );
	pContext->RSGetState( &m_pPrevRasterizerState );

	// 視覚効果用のステートへ切り替える.
	//	( ラスタライザは DirectX11 側のキャッシュとずれないように
	//	  自前のステートを直接セットし、終了時に元のポインタへ戻す ).
	DirectX11::SetAlphaToCoverage( false );
	DirectX11::SetAlphaBlend( false );
	DirectX11::SetDepth( false );
	pContext->RSSetState( m_pRasterizerState );
}

//----------------------------.
// ステートを元に戻す.
//----------------------------.
void VisualEffectManager::PopRenderStates()
{
	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	DirectX11::SetDepth( m_PrevIsDepth );
	if ( m_PrevIsAlphaToCoverage ) {
		DirectX11::SetAlphaToCoverage( true );
	}
	else {
		DirectX11::SetAlphaToCoverage( false );
		DirectX11::SetAlphaBlend( m_PrevIsAlphaBlend );
	}
	pContext->RSSetState( m_pPrevRasterizerState );
	SAFE_RELEASE( m_pPrevRasterizerState );
}

//----------------------------.
// ImGuiのデバッグ表示.
//----------------------------.
void VisualEffectManager::DebugRender()
{
	ImGui::Text( "Screen Effects ( check : apply to whole screen )" );
	ImGui::Separator();

	for ( size_t i = 0; i < static_cast<size_t>( EVisualEffect::Max ); ++i ) {
		const EVisualEffect Type = static_cast<EVisualEffect>( i );

		// 画面全体への適用切り替え.
		bool IsActive = HasScreenEffect( Type );
		ImGui::PushID( static_cast<int>( i ) );
		if ( ImGui::Checkbox( "##Active", &IsActive ) ) {
			if ( IsActive )	AddScreenEffect( Type );
			else			RemoveScreenEffect( Type );
		}
		ImGui::SameLine();

		// パラメータ編集.
		if ( ImGui::TreeNode( EFFECT_NAMES[i] ) ) {
			CVisualEffectBase* pEffect = GetEffect( Type );
			if ( pEffect != nullptr ) {
				float Intensity = pEffect->GetIntensity();
				if ( ImGui::SliderFloat( "Intensity", &Intensity, 0.0f, 1.0f ) ) pEffect->SetIntensity( Intensity );

				float AlphaLift = pEffect->GetAlphaLift();
				if ( ImGui::SliderFloat( "AlphaLift", &AlphaLift, 0.0f, 1.0f ) ) pEffect->SetAlphaLift( AlphaLift );

				D3DXVECTOR4 Area = pEffect->GetArea();
				if ( ImGui::DragFloat4( "Area(x,y,w,h)", reinterpret_cast<float*>( &Area ), 1.0f ) ) pEffect->SetArea( Area );

				pEffect->DebugParamEdit();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
