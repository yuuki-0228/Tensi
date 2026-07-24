#include "MotionBlurEffect.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\RenderTexture.h"
#include "..\..\ImGuiManager\ImGuiManager.h"
#include "..\..\..\Common\DirectX\DirectX11.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\VisualEffects\\MotionBlur.hlsl" );
}

CMotionBlurEffect::CMotionBlurEffect()
	: m_pHistory	( nullptr )
	, m_pResult		( nullptr )
{
}

CMotionBlurEffect::~CMotionBlurEffect()
{
}

const TCHAR* CMotionBlurEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// パラメータを詰める.
//----------------------------.
void CMotionBlurEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.BlendRate;
}

//----------------------------.
// 適用( 現在の画と履歴を合成し、結果を履歴として保存する ).
//----------------------------.
void CMotionBlurEffect::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	const UINT Width	= static_cast<UINT>( DirectX11::GetWndWidth() );
	const UINT Height	= static_cast<UINT>( DirectX11::GetWndHeight() );

	// 履歴テクスチャの作成( サイズ変更時は作り直してクリアする ).
	if ( m_pHistory == nullptr ) m_pHistory	= std::make_unique<CRenderTexture>();
	if ( m_pResult  == nullptr ) m_pResult	= std::make_unique<CRenderTexture>();
	if ( m_pHistory->IsSameSize( Width, Height ) == false ) {
		if ( FAILED( m_pHistory->Create( Width, Height ) ) ) return;
		if ( FAILED( m_pResult->Create( Width, Height ) ) ) return;
		m_pHistory->Clear();
	}

	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	PackParam( Params );

	// 現在の画( t0 )と履歴( t1 )を合成して結果へ描画.
	VisualEffectManager::DrawPass( GetPS( "PS_Accum" ), m_pResult->GetRTV(), pSrcSRV, m_pHistory->GetSRV(), Params, PARAM_VECTOR_NUM );

	// 結果を履歴へコピー( 次フレームで使用する ).
	DirectX11::GetContext()->CopyResource( m_pHistory->GetTex(), m_pResult->GetTex() );

	// 結果を出力先へコピー.
	VisualEffectManager::DrawCopy( pDstRTV, m_pResult->GetSRV() );
}

//----------------------------.
// ImGuiでのパラメータ編集.
//----------------------------.
void CMotionBlurEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "BlendRate", &m_Param.BlendRate, 0.0f, 0.99f );
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
