#include "BloomEffect.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\RenderTexture.h"
#include "..\..\ImGuiManager\ImGuiManager.h"
#include <algorithm>

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[]		= _T( "Data\\Shader\\VisualEffects\\Bloom.hlsl" );
	constexpr TCHAR BLUR_SHADER_NAME[]	= _T( "Data\\Shader\\VisualEffects\\GaussianBlur.hlsl" );
}

const TCHAR* CBloomEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// 初期化.
//----------------------------.
bool CBloomEffect::Init()
{
	if ( CompilePS( SHADER_NAME,		"PS_Bright"  ) == nullptr ) return false;
	if ( CompilePS( SHADER_NAME,		"PS_Combine" ) == nullptr ) return false;
	if ( CompilePS( BLUR_SHADER_NAME,	"PS_BlurH"   ) == nullptr ) return false;
	if ( CompilePS( BLUR_SHADER_NAME,	"PS_BlurV"   ) == nullptr ) return false;
	return true;
}

//----------------------------.
// 適用.
//----------------------------.
void CBloomEffect::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	CRenderTexture* pWork0 = VisualEffectManager::GetWorkRT( 0 );
	CRenderTexture* pWork1 = VisualEffectManager::GetWorkRT( 1 );
	if ( pWork0 == nullptr || pWork1 == nullptr ) return;

	// 高輝度抽出( 元画像 → ワーク0 ).
	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	Params[0].x = m_Param.Threshold;
	VisualEffectManager::DrawPass( GetPS( "PS_Bright" ), pWork0->GetRTV(), pSrcSRV, nullptr, Params, PARAM_VECTOR_NUM );

	// ぼかし( ワーク0 ⇔ ワーク1 を往復 ).
	D3DXVECTOR4 BlurParams[PARAM_VECTOR_NUM] = {};
	BlurParams[0].x = m_Param.BlurRadius;
	const int BlurCount = std::clamp( m_Param.BlurCount, 1, 4 );
	for ( int i = 0; i < BlurCount; ++i ) {
		VisualEffectManager::DrawPass( GetPS( "PS_BlurH" ), pWork1->GetRTV(), pWork0->GetSRV(), nullptr, BlurParams, PARAM_VECTOR_NUM );
		VisualEffectManager::DrawPass( GetPS( "PS_BlurV" ), pWork0->GetRTV(), pWork1->GetSRV(), nullptr, BlurParams, PARAM_VECTOR_NUM );
	}

	// 加算合成( 元画像 + ぼかし結果 → 出力先 ).
	Params[0].y = m_Param.Strength;
	VisualEffectManager::DrawPass( GetPS( "PS_Combine" ), pDstRTV, pSrcSRV, pWork0->GetSRV(), Params, PARAM_VECTOR_NUM );
}

//----------------------------.
// ImGuiでのパラメータ編集.
//----------------------------.
void CBloomEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Threshold",	&m_Param.Threshold,		0.0f, 1.0f );
	ImGui::SliderFloat( "Strength",		&m_Param.Strength,		0.0f, 4.0f );
	ImGui::SliderFloat( "BlurRadius",	&m_Param.BlurRadius,	1.0f, 32.0f );
	ImGui::SliderInt( "BlurCount",		&m_Param.BlurCount,		1, 4 );
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
