#include "SinglePassEffects.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\VisualEffectManager.h"
#include "..\..\ImGuiManager\ImGuiManager.h"
#include "..\..\..\Common\DirectX\DirectX11.h"

namespace {
	// 単パス用シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[]			= _T( "Data\\Shader\\VisualEffects\\SinglePass.hlsl" );
	// アート系シェーダファイル名.
	constexpr TCHAR ARTISTIC_SHADER_NAME[]	= _T( "Data\\Shader\\VisualEffects\\Artistic.hlsl" );
	// エッジ系シェーダファイル名.
	constexpr TCHAR EDGE_SHADER_NAME[]		= _T( "Data\\Shader\\VisualEffects\\Edge.hlsl" );
	// レンズ系シェーダファイル名.
	constexpr TCHAR LENS_SHADER_NAME[]		= _T( "Data\\Shader\\VisualEffects\\Lens.hlsl" );
	// 天候・環境系シェーダファイル名.
	constexpr TCHAR WEATHER_SHADER_NAME[]	= _T( "Data\\Shader\\VisualEffects\\Weather.hlsl" );
}

//----------------------------.
// 白黒.
//----------------------------.
const TCHAR* CGrayscaleEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// セピア.
//----------------------------.
const TCHAR* CSepiaEffect::GetShaderPath() const { return SHADER_NAME; }
void CSepiaEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0] = m_Param.Color;
}
void CSepiaEffect::DebugParamEdit()
{
	ImGui::ColorEdit3( "Color", reinterpret_cast<float*>( &m_Param.Color ) );
}

//----------------------------.
// 色反転.
//----------------------------.
const TCHAR* CInvertEffect::GetShaderPath() const { return SHADER_NAME; }

//----------------------------.
// ポスタライズ.
//----------------------------.
const TCHAR* CPosterizeEffect::GetShaderPath() const { return SHADER_NAME; }
void CPosterizeEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Levels;
}
void CPosterizeEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Levels", &m_Param.Levels, 2.0f, 32.0f );
}

//----------------------------.
// カラーフィルター.
//----------------------------.
const TCHAR* CColorFilterEffect::GetShaderPath() const { return SHADER_NAME; }
void CColorFilterEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0] = m_Param.MulColor;
	pOut[1] = m_Param.AddColor;
}
void CColorFilterEffect::DebugParamEdit()
{
	ImGui::DragFloat4( "MulColor", reinterpret_cast<float*>( &m_Param.MulColor ), 0.01f, 0.0f, 3.0f );
	ImGui::DragFloat4( "AddColor", reinterpret_cast<float*>( &m_Param.AddColor ), 0.01f, -1.0f, 1.0f );
	if ( ImGui::Button( "Blue Filter Preset" ) ) {
		m_Param.MulColor = { 0.75f, 0.9f, 1.25f, 1.0f };
		m_Param.AddColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	}
}

//----------------------------.
// 色調補正.
//----------------------------.
const TCHAR* CColorCorrectionEffect::GetShaderPath() const { return SHADER_NAME; }
void CColorCorrectionEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Brightness;
	pOut[0].y = m_Param.Contrast;
	pOut[0].z = m_Param.Saturation;
	pOut[0].w = m_Param.Gamma;
}
void CColorCorrectionEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Brightness",	&m_Param.Brightness,	-1.0f, 1.0f );
	ImGui::SliderFloat( "Contrast",		&m_Param.Contrast,		0.0f, 3.0f );
	ImGui::SliderFloat( "Saturation",	&m_Param.Saturation,	0.0f, 3.0f );
	ImGui::SliderFloat( "Gamma",		&m_Param.Gamma,			0.2f, 3.0f );
}

//----------------------------.
// ビネット.
//----------------------------.
const TCHAR* CVignetteEffect::GetShaderPath() const { return SHADER_NAME; }
void CVignetteEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Radius;
	pOut[0].y	= m_Param.Softness;
	pOut[0].z	= m_Param.EdgeAlpha;
	pOut[1]		= m_Param.Color;
}
void CVignetteEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Radius",		&m_Param.Radius,	0.0f, 1.5f );
	ImGui::SliderFloat( "Softness",		&m_Param.Softness,	0.01f, 1.0f );
	ImGui::SliderFloat( "EdgeAlpha",	&m_Param.EdgeAlpha,	0.0f, 1.0f );
	ImGui::ColorEdit3( "Color", reinterpret_cast<float*>( &m_Param.Color ) );
}

//----------------------------.
// ノイズ.
//----------------------------.
const TCHAR* CNoiseEffect::GetShaderPath() const { return SHADER_NAME; }
void CNoiseEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
	pOut[0].y = m_Param.GrainSize;
	pOut[0].z = m_Param.IsColor ? 1.0f : 0.0f;
	pOut[0].w = m_Param.Speed;
}
void CNoiseEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength",		&m_Param.Strength,	0.0f, 1.0f );
	ImGui::SliderFloat( "GrainSize",	&m_Param.GrainSize,	1.0f, 32.0f );
	ImGui::Checkbox( "ColorNoise",		&m_Param.IsColor );
	ImGui::SliderFloat( "Speed",		&m_Param.Speed,		0.0f, 60.0f );
}

//----------------------------.
// フィルムグレイン.
//----------------------------.
const TCHAR* CFilmGrainEffect::GetShaderPath() const { return SHADER_NAME; }
void CFilmGrainEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
	pOut[0].y = m_Param.GrainSize;
	pOut[0].z = m_Param.Flicker;
	pOut[0].w = m_Param.Response;
}
void CFilmGrainEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength",		&m_Param.Strength,	0.0f, 1.0f );
	ImGui::SliderFloat( "GrainSize",	&m_Param.GrainSize,	1.0f, 8.0f );
	ImGui::SliderFloat( "Flicker",		&m_Param.Flicker,	0.0f, 0.5f );
	ImGui::SliderFloat( "Response",		&m_Param.Response,	0.0f, 1.0f );
}

//----------------------------.
// スキャンライン.
//----------------------------.
const TCHAR* CScanlineEffect::GetShaderPath() const { return SHADER_NAME; }
void CScanlineEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.LineWidth;
	pOut[0].y = m_Param.Strength;
	pOut[0].z = m_Param.ScrollSpeed;
	pOut[0].w = m_Param.IsVertical ? 1.0f : 0.0f;
}
void CScanlineEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "LineWidth",	&m_Param.LineWidth,		1.0f, 32.0f );
	ImGui::SliderFloat( "Strength",		&m_Param.Strength,		0.0f, 1.0f );
	ImGui::SliderFloat( "ScrollSpeed",	&m_Param.ScrollSpeed,	-200.0f, 200.0f );
	ImGui::Checkbox( "Vertical",		&m_Param.IsVertical );
}

//----------------------------.
// CRT.
//----------------------------.
const TCHAR* CCRTEffect::GetShaderPath() const { return SHADER_NAME; }
void CCRTEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Curvature;
	pOut[0].y = m_Param.Scanline;
	pOut[0].z = m_Param.Aperture;
	pOut[0].w = m_Param.ChromaOffset;
	pOut[1].x = m_Param.EdgeFade;
}
void CCRTEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Curvature",	&m_Param.Curvature,		0.0f, 1.0f );
	ImGui::SliderFloat( "Scanline",		&m_Param.Scanline,		0.0f, 1.0f );
	ImGui::SliderFloat( "Aperture",		&m_Param.Aperture,		0.0f, 1.0f );
	ImGui::SliderFloat( "ChromaOffset",	&m_Param.ChromaOffset,	0.0f, 5.0f );
	ImGui::SliderFloat( "EdgeFade",		&m_Param.EdgeFade,		0.0f, 1.0f );
}

//----------------------------.
// ピクセル化.
//----------------------------.
const TCHAR* CPixelateEffect::GetShaderPath() const { return SHADER_NAME; }
void CPixelateEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.PixelSize;
}
void CPixelateEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "PixelSize", &m_Param.PixelSize, 1.0f, 64.0f );
}

//----------------------------.
// RGB分離.
//----------------------------.
const TCHAR* CRGBShiftEffect::GetShaderPath() const { return SHADER_NAME; }
void CRGBShiftEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Offset;
	pOut[0].y = m_Param.AngleDeg;
}
void CRGBShiftEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Offset",	&m_Param.Offset,	0.0f, 20.0f );
	ImGui::SliderFloat( "Angle",	&m_Param.AngleDeg,	-180.0f, 180.0f );
}

//----------------------------.
// グリッチ.
//----------------------------.
const TCHAR* CGlitchEffect::GetShaderPath() const { return SHADER_NAME; }
void CGlitchEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Speed;
	pOut[0].y = m_Param.BlockSize;
	pOut[0].z = m_Param.BlockStrength;
	pOut[0].w = m_Param.ColorShift;
	pOut[1].x = m_Param.LineNoise;
	pOut[1].y = m_Param.Rate;
}
void CGlitchEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Speed",			&m_Param.Speed,			0.0f, 60.0f );
	ImGui::SliderFloat( "BlockSize",		&m_Param.BlockSize,		2.0f, 128.0f );
	ImGui::SliderFloat( "BlockStrength",	&m_Param.BlockStrength,	0.0f, 200.0f );
	ImGui::SliderFloat( "ColorShift",		&m_Param.ColorShift,	0.0f, 30.0f );
	ImGui::SliderFloat( "LineNoise",		&m_Param.LineNoise,		0.0f, 1.0f );
	ImGui::SliderFloat( "Rate",				&m_Param.Rate,			0.0f, 1.0f );
}

//----------------------------.
// 波歪み.
//----------------------------.
const TCHAR* CWaveEffect::GetShaderPath() const { return SHADER_NAME; }
void CWaveEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.AmplitudeX;
	pOut[0].y = m_Param.AmplitudeY;
	pOut[0].z = m_Param.Frequency;
	pOut[0].w = m_Param.Speed;
}
void CWaveEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "AmplitudeX",	&m_Param.AmplitudeX,	0.0f, 64.0f );
	ImGui::SliderFloat( "AmplitudeY",	&m_Param.AmplitudeY,	0.0f, 64.0f );
	ImGui::SliderFloat( "Frequency",	&m_Param.Frequency,		0.0f, 32.0f );
	ImGui::SliderFloat( "Speed",		&m_Param.Speed,			0.0f, 16.0f );
}

//----------------------------.
// シャープ.
//----------------------------.
const TCHAR* CSharpenEffect::GetShaderPath() const { return SHADER_NAME; }
void CSharpenEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
}
void CSharpenEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength", &m_Param.Strength, 0.0f, 5.0f );
}

//----------------------------.
// 放射ブラー.
//----------------------------.
const TCHAR* CRadialBlurEffect::GetShaderPath() const { return SHADER_NAME; }
void CRadialBlurEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.CenterU;
	pOut[0].y = m_Param.CenterV;
	pOut[0].z = m_Param.Strength;
	pOut[0].w = static_cast<float>( m_Param.Samples );
}
void CRadialBlurEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "CenterU",	&m_Param.CenterU,	0.0f, 1.0f );
	ImGui::SliderFloat( "CenterV",	&m_Param.CenterV,	0.0f, 1.0f );
	ImGui::SliderFloat( "Strength",	&m_Param.Strength,	0.0f, 1.0f );
	ImGui::SliderInt( "Samples",	&m_Param.Samples,	2, 24 );
}

//----------------------------.
// トゥーン.
//----------------------------.
const TCHAR* CToonEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CToonEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Levels;
	pOut[0].y	= m_Param.EdgeThreshold;
	pOut[0].z	= m_Param.EdgeThickness;
	pOut[1]		= m_Param.EdgeColor;
}
void CToonEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Levels",			&m_Param.Levels,		2.0f, 16.0f );
	ImGui::SliderFloat( "EdgeThreshold",	&m_Param.EdgeThreshold,	0.0f, 1.0f );
	ImGui::SliderFloat( "EdgeThickness",	&m_Param.EdgeThickness,	0.5f, 4.0f );
	ImGui::ColorEdit3( "EdgeColor", reinterpret_cast<float*>( &m_Param.EdgeColor ) );
}

//----------------------------.
// 水彩.
//----------------------------.
const TCHAR* CWatercolorEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CWatercolorEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Bleed;
	pOut[0].y = m_Param.Paper;
	pOut[0].z = m_Param.EdgeDarken;
	pOut[0].w = m_Param.Wobble;
}
void CWatercolorEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Bleed",		&m_Param.Bleed,			0.0f, 8.0f );
	ImGui::SliderFloat( "Paper",		&m_Param.Paper,			0.0f, 1.0f );
	ImGui::SliderFloat( "EdgeDarken",	&m_Param.EdgeDarken,	0.0f, 1.0f );
	ImGui::SliderFloat( "Wobble",		&m_Param.Wobble,		0.0f, 8.0f );
}

//----------------------------.
// 油絵.
//----------------------------.
const TCHAR* COilPaintingEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void COilPaintingEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = static_cast<float>( m_Param.Radius );
}
void COilPaintingEffect::DebugParamEdit()
{
	ImGui::SliderInt( "Radius", &m_Param.Radius, 2, 6 );
}

//----------------------------.
// 色鉛筆.
//----------------------------.
const TCHAR* CColoredPencilEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CColoredPencilEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.HatchScale;
	pOut[0].y = m_Param.EdgeStrength;
	pOut[0].z = m_Param.Saturation;
	pOut[0].w = m_Param.Paper;
}
void CColoredPencilEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "HatchScale",	&m_Param.HatchScale,	2.0f, 16.0f );
	ImGui::SliderFloat( "EdgeStrength",	&m_Param.EdgeStrength,	0.0f, 1.0f );
	ImGui::SliderFloat( "Saturation",	&m_Param.Saturation,	0.0f, 1.5f );
	ImGui::SliderFloat( "Paper",		&m_Param.Paper,			0.0f, 1.0f );
}

//----------------------------.
// アウトライン.
//----------------------------.
const TCHAR* COutlineEffect::GetShaderPath() const { return EDGE_SHADER_NAME; }
void COutlineEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Threshold;
	pOut[0].y	= m_Param.Thickness;
	pOut[1]		= m_Param.Color;
}
void COutlineEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Threshold",	&m_Param.Threshold,	0.0f, 1.0f );
	ImGui::SliderFloat( "Thickness",	&m_Param.Thickness,	0.5f, 4.0f );
	ImGui::ColorEdit3( "Color", reinterpret_cast<float*>( &m_Param.Color ) );
}

//----------------------------.
// エッジ検出.
//----------------------------.
const TCHAR* CEdgeDetectEffect::GetShaderPath() const { return EDGE_SHADER_NAME; }
void CEdgeDetectEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Threshold;
	pOut[1]		= m_Param.EdgeColor;
	pOut[2]		= m_Param.BackColor;
}
void CEdgeDetectEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Threshold", &m_Param.Threshold, 0.0f, 1.0f );
	ImGui::ColorEdit3( "EdgeColor", reinterpret_cast<float*>( &m_Param.EdgeColor ) );
	ImGui::ColorEdit3( "BackColor", reinterpret_cast<float*>( &m_Param.BackColor ) );
}

//----------------------------.
// エッジ強調.
//----------------------------.
const TCHAR* CEdgeEnhanceEffect::GetShaderPath() const { return EDGE_SHADER_NAME; }
void CEdgeEnhanceEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
	pOut[0].y = m_Param.Thickness;
}
void CEdgeEnhanceEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength",		&m_Param.Strength,	0.0f, 5.0f );
	ImGui::SliderFloat( "Thickness",	&m_Param.Thickness,	0.5f, 4.0f );
}

//----------------------------.
// ハーフトーン.
//----------------------------.
const TCHAR* CHalftoneEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CHalftoneEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.CellSize;
	pOut[0].y	= m_Param.AngleDeg;
	pOut[0].z	= m_Param.IsColor ? 1.0f : 0.0f;
	pOut[1]		= m_Param.InkColor;
	pOut[2]		= m_Param.PaperColor;
}
void CHalftoneEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "CellSize",	&m_Param.CellSize,	2.0f, 32.0f );
	ImGui::SliderFloat( "Angle",	&m_Param.AngleDeg,	0.0f, 90.0f );
	ImGui::Checkbox( "UseSourceColor", &m_Param.IsColor );
	ImGui::ColorEdit3( "InkColor",		reinterpret_cast<float*>( &m_Param.InkColor ) );
	ImGui::ColorEdit3( "PaperColor",	reinterpret_cast<float*>( &m_Param.PaperColor ) );
}

//----------------------------.
// エンボス.
//----------------------------.
const TCHAR* CEmbossEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CEmbossEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
	pOut[0].y = m_Param.AngleDeg;
	pOut[0].z = m_Param.Offset;
}
void CEmbossEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength",	&m_Param.Strength,	0.0f, 8.0f );
	ImGui::SliderFloat( "Angle",	&m_Param.AngleDeg,	-180.0f, 180.0f );
	ImGui::SliderFloat( "Offset",	&m_Param.Offset,	0.5f, 4.0f );
}

//----------------------------.
// 2値化.
//----------------------------.
const TCHAR* CThresholdEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void CThresholdEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Threshold;
	pOut[0].y	= m_Param.Smooth;
	pOut[1]		= m_Param.WhiteColor;
	pOut[2]		= m_Param.BlackColor;
}
void CThresholdEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Threshold",	&m_Param.Threshold,	0.0f, 1.0f );
	ImGui::SliderFloat( "Smooth",		&m_Param.Smooth,	0.0f, 0.5f );
	ImGui::ColorEdit3( "WhiteColor", reinterpret_cast<float*>( &m_Param.WhiteColor ) );
	ImGui::ColorEdit3( "BlackColor", reinterpret_cast<float*>( &m_Param.BlackColor ) );
}

//----------------------------.
// 魚眼.
//----------------------------.
const TCHAR* CFisheyeEffect::GetShaderPath() const { return LENS_SHADER_NAME; }
void CFisheyeEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Strength;
}
void CFisheyeEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Strength", &m_Param.Strength, -1.0f, 1.0f );
}

//----------------------------.
// 色相シフト.
//----------------------------.
const TCHAR* CHueShiftEffect::GetShaderPath() const { return LENS_SHADER_NAME; }
void CHueShiftEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.AngleDeg;
	pOut[0].y = m_Param.Speed;
}
void CHueShiftEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Angle",	&m_Param.AngleDeg,	-360.0f, 360.0f );
	ImGui::SliderFloat( "Speed",	&m_Param.Speed,		0.0f, 360.0f );
}

//----------------------------.
// 熱気揺らぎ.
//----------------------------.
const TCHAR* CHeatHazeEffect::GetShaderPath() const { return LENS_SHADER_NAME; }
void CHeatHazeEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Amplitude;
	pOut[0].y = m_Param.Scale;
	pOut[0].z = m_Param.Speed;
	pOut[0].w = m_Param.RiseSpeed;
}
void CHeatHazeEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Amplitude",	&m_Param.Amplitude,	0.0f, 16.0f );
	ImGui::SliderFloat( "Scale",		&m_Param.Scale,		1.0f, 32.0f );
	ImGui::SliderFloat( "Speed",		&m_Param.Speed,		0.0f, 8.0f );
	ImGui::SliderFloat( "RiseSpeed",	&m_Param.RiseSpeed,	0.0f, 4.0f );
}

//----------------------------.
// オールドフィルム.
//----------------------------.
const TCHAR* COldFilmEffect::GetShaderPath() const { return ARTISTIC_SHADER_NAME; }
void COldFilmEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Sepia;
	pOut[0].y = m_Param.Grain;
	pOut[0].z = m_Param.Scratch;
	pOut[0].w = m_Param.Jitter;
	pOut[1].x = m_Param.Flicker;
	pOut[1].y = m_Param.Vignette;
}
void COldFilmEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Sepia",	&m_Param.Sepia,		0.0f, 1.0f );
	ImGui::SliderFloat( "Grain",	&m_Param.Grain,		0.0f, 1.0f );
	ImGui::SliderFloat( "Scratch",	&m_Param.Scratch,	0.0f, 1.0f );
	ImGui::SliderFloat( "Jitter",	&m_Param.Jitter,	0.0f, 1.0f );
	ImGui::SliderFloat( "Flicker",	&m_Param.Flicker,	0.0f, 1.0f );
	ImGui::SliderFloat( "Vignette",	&m_Param.Vignette,	0.0f, 1.0f );
}

//----------------------------.
// 雨.
//----------------------------.
const TCHAR* CRainEffect::GetShaderPath() const { return WEATHER_SHADER_NAME; }
void CRainEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Amount;
	pOut[0].y = m_Param.Speed;
	pOut[0].z = m_Param.Refract;
	pOut[0].w = m_Param.Blur;
	pOut[1].x = m_Param.Brightness;
}
void CRainEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Amount",		&m_Param.Amount,		0.0f, 1.0f );
	ImGui::SliderFloat( "Speed",		&m_Param.Speed,			0.0f, 1.0f );
	ImGui::SliderFloat( "Refract",		&m_Param.Refract,		0.0f, 3.0f );
	ImGui::SliderFloat( "Blur",			&m_Param.Blur,			0.0f, 12.0f );
	ImGui::SliderFloat( "Brightness",	&m_Param.Brightness,	0.0f, 1.0f );
}

//----------------------------.
// 雪.
//----------------------------.
const TCHAR* CSnowEffect::GetShaderPath() const { return WEATHER_SHADER_NAME; }
void CSnowEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x = m_Param.Density;
	pOut[0].y = m_Param.Speed;
	pOut[0].z = m_Param.Size;
	pOut[0].w = m_Param.Sway;
	pOut[1].x = m_Param.Brightness;
}
void CSnowEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Density",		&m_Param.Density,		0.0f, 1.0f );
	ImGui::SliderFloat( "Speed",		&m_Param.Speed,			0.02f, 1.0f );
	ImGui::SliderFloat( "Size",			&m_Param.Size,			0.1f, 1.0f );
	ImGui::SliderFloat( "Sway",			&m_Param.Sway,			0.0f, 1.0f );
	ImGui::SliderFloat( "Brightness",	&m_Param.Brightness,	0.0f, 1.0f );
}

//----------------------------.
// フォグ(ノイズミスト式).
//----------------------------.
const TCHAR* CFogEffect::GetShaderPath() const { return WEATHER_SHADER_NAME; }
void CFogEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Density;
	pOut[0].y	= m_Param.Scale;
	pOut[0].z	= m_Param.SpeedX;
	pOut[0].w	= m_Param.SpeedY;
	pOut[1].x	= m_Param.BottomBias;
	pOut[2]		= m_Param.Color;
}
void CFogEffect::DebugParamEdit()
{
	ImGui::SliderFloat( "Density",		&m_Param.Density,		0.0f, 1.0f );
	ImGui::SliderFloat( "Scale",		&m_Param.Scale,			0.5f, 16.0f );
	ImGui::SliderFloat( "SpeedX",		&m_Param.SpeedX,		-0.5f, 0.5f );
	ImGui::SliderFloat( "SpeedY",		&m_Param.SpeedY,		-0.5f, 0.5f );
	ImGui::SliderFloat( "BottomBias",	&m_Param.BottomBias,	0.0f, 1.0f );
	ImGui::ColorEdit3( "Color", reinterpret_cast<float*>( &m_Param.Color ) );
}

//----------------------------.
// 深度フォグ.
//----------------------------.
const TCHAR* CFogDepthEffect::GetShaderPath() const { return WEATHER_SHADER_NAME; }

// 初期化( 通常用と MSAA 用の両エントリをコンパイルする ).
bool CFogDepthEffect::Init()
{
	if ( CompilePS( WEATHER_SHADER_NAME, "PS_FogDepth"   ) == nullptr ) return false;
	if ( CompilePS( WEATHER_SHADER_NAME, "PS_FogDepthMS" ) == nullptr ) return false;
	return true;
}

void CFogDepthEffect::PackParam( D3DXVECTOR4* pOut ) const
{
	pOut[0].x	= m_Param.Near;
	pOut[0].y	= m_Param.Far;
	pOut[0].z	= m_Param.Start;
	pOut[0].w	= m_Param.End;
	pOut[1].x	= m_Param.MaxDensity;
	pOut[1].y	= m_Param.ApplySky ? 1.0f : 0.0f;
	pOut[2]		= m_Param.Color;
}

// 適用( 深度バッファを t2 / t3 へ追加バインドして描画する ).
void CFogDepthEffect::Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV )
{
	ID3D11ShaderResourceView* pDepthSRV = DirectX11::GetDepthSRV( 0 );
	if ( pDepthSRV == nullptr ) {
		// 深度が取れない場合は素通しする.
		VisualEffectManager::DrawCopy( pDstRTV, pSrcSRV );
		return;
	}

	// MSAA の有無でエントリを切り替える( 深度バッファのサンプル数が変わるため ).
	const bool IsMsaa = DirectX11::GetMsaaSampleCount() > 1;
	ID3D11PixelShader* pPS = GetPS( IsMsaa ? "PS_FogDepthMS" : "PS_FogDepth" );

	// 深度を t2( 通常用 ) / t3( MSAA用 )へバインドする( DrawPass は t0/t1 のみ扱う ).
	ID3D11DeviceContext* pContext = DirectX11::GetContext();
	ID3D11ShaderResourceView* pDepthSRVs[2] = { pDepthSRV, pDepthSRV };
	pContext->PSSetShaderResources( 2, 2, pDepthSRVs );

	D3DXVECTOR4 Params[PARAM_VECTOR_NUM] = {};
	PackParam( Params );
	VisualEffectManager::DrawPass( pPS, pDstRTV, pSrcSRV, nullptr, Params, PARAM_VECTOR_NUM );

	// 深度のバインドを外す( この後 DSV として使うため ).
	ID3D11ShaderResourceView* pNullSRVs[2] = { nullptr, nullptr };
	pContext->PSSetShaderResources( 2, 2, pNullSRVs );
}

void CFogDepthEffect::DebugParamEdit()
{
	ImGui::DragFloat( "Near",		&m_Param.Near,	0.01f, 0.001f, 10.0f );
	ImGui::DragFloat( "Far",		&m_Param.Far,	1.0f, 1.0f, 100000.0f );
	ImGui::DragFloat( "Start",		&m_Param.Start,	0.1f );
	ImGui::DragFloat( "End",		&m_Param.End,	0.1f );
	ImGui::SliderFloat( "MaxDensity", &m_Param.MaxDensity, 0.0f, 1.0f );
	ImGui::Checkbox( "ApplySky",	&m_Param.ApplySky );
	ImGui::ColorEdit3( "Color", reinterpret_cast<float*>( &m_Param.Color ) );
}

#endif	// #ifdef ENABLE_VISUAL_EFFECT
