#include "FrostEffect.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\RenderTexture.h"
#include "..\..\ImGuiManager\ImGuiManager.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[]		= _T( "Data\\Shader\\VisualEffects\\Frost.hlsl" );
	constexpr TCHAR BLUR_SHADER_NAME[]	= _T( "Data\\Shader\\VisualEffects\\GaussianBlur.hlsl" );
}

const TCHAR* CFrostEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// 初期化.
//----------------------------.
bool CFrostEffect::Init()
{
	if ( CompilePS( SHADER_NAME,		"PS_Frost" ) == nullptr ) return false;
	if ( CompilePS( BLUR_SHADER_NAME,	"PS_BlurH" ) == nullptr ) return false;
	if ( CompilePS( BLUR_SHADER_NAME,	"PS_BlurV" ) == nullptr ) return false;
	return true;
}

//----------------------------.
// 適用( ぼかし → 歪み+白みの仕上げ ).
//----------------------------.
void CFrostEffect::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	CRenderTexture* pWork0 = VisualEffectManager::GetWorkRT( 0 );
	CRenderTexture* pWork1 = VisualEffectManager::GetWorkRT( 1 );
	if ( pWork0 == nullptr || pWork1 == nullptr ) return;

	// ぼかし( 元画像 → ワーク1 → ワーク0 ).
	D3DXVECTOR4 BlurParams[PARAM_VECTOR_NUM] = {};
	BlurParams[0].x = m_Param.BlurRadius;
	VisualEffectManager::DrawPass( GetPS( "PS_BlurH" ), pWork1->GetRTV(), pSrcSRV, nullptr, BlurParams, PARAM_VECTOR_NUM );
	VisualEffectManager::DrawPass( GetPS( "PS_BlurV" ), pWork0->GetRTV(), pWork1->GetSRV(), nullptr, BlurParams, PARAM_VECTOR_NUM );

	// 仕上げ( 元画像 t0 + ぼかし結果 t1 → 出力先 ).
	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	Params[0].x = m_Param.Frost;
	Params[0].y = m_Param.NoiseStrength;
	Params[0].z = m_Param.NoiseGrain;
	VisualEffectManager::DrawPass( GetPS( "PS_Frost" ), pDstRTV, pSrcSRV, pWork0->GetSRV(), Params, PARAM_VECTOR_NUM );
}

//----------------------------.
// ImGuiでのパラメータ編集.
//----------------------------.
void CFrostEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "BlurRadius",		&m_Param.BlurRadius,	0.0f, 32.0f );
	ImGui::SliderFloat( "Frost",			&m_Param.Frost,			0.0f, 1.0f );
	ImGui::SliderFloat( "NoiseStrength",	&m_Param.NoiseStrength,	0.0f, 64.0f );
	ImGui::SliderFloat( "NoiseGrain",		&m_Param.NoiseGrain,	1.0f, 16.0f );
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
