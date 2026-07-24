#include "GaussianBlurEffect.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\RenderTexture.h"
#include "..\..\ImGuiManager\ImGuiManager.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\VisualEffects\\GaussianBlur.hlsl" );
}

const TCHAR* CGaussianBlurEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// 初期化.
//----------------------------.
bool CGaussianBlurEffect::Init()
{
	if ( CompilePS( SHADER_NAME, "PS_BlurH"      ) == nullptr ) return false;
	if ( CompilePS( SHADER_NAME, "PS_BlurFinish" ) == nullptr ) return false;
	return true;
}

//----------------------------.
// パラメータを詰める.
//----------------------------.
void CGaussianBlurEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Radius;
}

//----------------------------.
// 適用( 横ブラー → 縦ブラー+仕上げ ).
//----------------------------.
void CGaussianBlurEffect::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	CRenderTexture* pWork = VisualEffectManager::GetWorkRT( 0 );
	if ( pWork == nullptr ) return;

	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	PackParam( Params );

	// 横ブラー( 元画像 → ワーク ).
	VisualEffectManager::DrawPass( GetPS( "PS_BlurH" ), pWork->GetRTV(), pSrcSRV, nullptr, Params, PARAM_VECTOR_NUM );
	// 縦ブラー+仕上げ( ワーク → 出力先. 適用率の合成用に元画像を t1 へ渡す ).
	VisualEffectManager::DrawPass( GetPS( "PS_BlurFinish" ), pDstRTV, pWork->GetSRV(), pSrcSRV, Params, PARAM_VECTOR_NUM );
}

//----------------------------.
// ImGuiでのパラメータ編集.
//----------------------------.
void CGaussianBlurEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Radius", &m_Param.Radius, 0.0f, 32.0f );
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
