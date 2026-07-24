#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "VisualEffectBase.h"

/************************************************
*	単パスで完結する視覚効果のまとめ.
*	( 複数パスが必要な物は個別ファイル :
*	  GaussianBlurEffect / BloomEffect / MotionBlurEffect / FrostEffect ).
**/

// 白黒.
class CGrayscaleEffect final : public CVisualEffectBase
{
public:
	virtual EVisualEffect GetType() const override { return EVisualEffect::Grayscale; }
	virtual const char* GetName() const override { return "Grayscale"; }
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Grayscale"; }
};

// セピア.
class CSepiaEffect final : public CVisualEffectWithParam<SSepiaParam>
{
public:
	virtual const char* GetName() const override { return "Sepia"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Sepia"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 色反転.
class CInvertEffect final : public CVisualEffectBase
{
public:
	virtual EVisualEffect GetType() const override { return EVisualEffect::Invert; }
	virtual const char* GetName() const override { return "Invert"; }
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Invert"; }
};

// ポスタライズ(減色).
class CPosterizeEffect final : public CVisualEffectWithParam<SPosterizeParam>
{
public:
	virtual const char* GetName() const override { return "Posterize"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Posterize"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// カラーフィルター.
class CColorFilterEffect final : public CVisualEffectWithParam<SColorFilterParam>
{
public:
	virtual const char* GetName() const override { return "ColorFilter"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_ColorFilter"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 色調補正.
class CColorCorrectionEffect final : public CVisualEffectWithParam<SColorCorrectionParam>
{
public:
	virtual const char* GetName() const override { return "ColorCorrection"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_ColorCorrection"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// ビネット(周辺減光).
class CVignetteEffect final : public CVisualEffectWithParam<SVignetteParam>
{
public:
	virtual const char* GetName() const override { return "Vignette"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Vignette"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// ノイズ.
class CNoiseEffect final : public CVisualEffectWithParam<SNoiseParam>
{
public:
	virtual const char* GetName() const override { return "Noise"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Noise"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// フィルムグレイン.
class CFilmGrainEffect final : public CVisualEffectWithParam<SFilmGrainParam>
{
public:
	virtual const char* GetName() const override { return "FilmGrain"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_FilmGrain"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// スキャンライン.
class CScanlineEffect final : public CVisualEffectWithParam<SScanlineParam>
{
public:
	virtual const char* GetName() const override { return "Scanline"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Scanline"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// CRT(ブラウン管風).
class CCRTEffect final : public CVisualEffectWithParam<SCRTParam>
{
public:
	virtual const char* GetName() const override { return "CRT"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_CRT"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// ピクセル化(モザイク).
class CPixelateEffect final : public CVisualEffectWithParam<SPixelateParam>
{
public:
	virtual const char* GetName() const override { return "Pixelate"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Pixelate"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// RGB分離(色収差).
class CRGBShiftEffect final : public CVisualEffectWithParam<SRGBShiftParam>
{
public:
	virtual const char* GetName() const override { return "RGBShift"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_RGBShift"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// グリッチ.
class CGlitchEffect final : public CVisualEffectWithParam<SGlitchParam>
{
public:
	virtual const char* GetName() const override { return "Glitch"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Glitch"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 波歪み.
class CWaveEffect final : public CVisualEffectWithParam<SWaveParam>
{
public:
	virtual const char* GetName() const override { return "Wave"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Wave"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// シャープ.
class CSharpenEffect final : public CVisualEffectWithParam<SSharpenParam>
{
public:
	virtual const char* GetName() const override { return "Sharpen"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Sharpen"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 放射ブラー.
class CRadialBlurEffect final : public CVisualEffectWithParam<SRadialBlurParam>
{
public:
	virtual const char* GetName() const override { return "RadialBlur"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_RadialBlur"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// トゥーン(セル塗り風).
class CToonEffect final : public CVisualEffectWithParam<SToonParam>
{
public:
	virtual const char* GetName() const override { return "Toon"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Toon"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 水彩.
class CWatercolorEffect final : public CVisualEffectWithParam<SWatercolorParam>
{
public:
	virtual const char* GetName() const override { return "Watercolor"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Watercolor"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 油絵.
class COilPaintingEffect final : public CVisualEffectWithParam<SOilPaintingParam>
{
public:
	virtual const char* GetName() const override { return "OilPainting"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_OilPainting"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 色鉛筆.
class CColoredPencilEffect final : public CVisualEffectWithParam<SColoredPencilParam>
{
public:
	virtual const char* GetName() const override { return "ColoredPencil"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_ColoredPencil"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// アウトライン(輪郭線を重ねる).
class COutlineEffect final : public CVisualEffectWithParam<SOutlineParam>
{
public:
	virtual const char* GetName() const override { return "Outline"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Outline"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// エッジ検出(輪郭のみ表示).
class CEdgeDetectEffect final : public CVisualEffectWithParam<SEdgeDetectParam>
{
public:
	virtual const char* GetName() const override { return "EdgeDetect"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_EdgeDetect"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// エッジ強調.
class CEdgeEnhanceEffect final : public CVisualEffectWithParam<SEdgeEnhanceParam>
{
public:
	virtual const char* GetName() const override { return "EdgeEnhance"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_EdgeEnhance"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// ハーフトーン(漫画のトーン風ドット).
class CHalftoneEffect final : public CVisualEffectWithParam<SHalftoneParam>
{
public:
	virtual const char* GetName() const override { return "Halftone"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Halftone"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// エンボス(凹凸彫刻風).
class CEmbossEffect final : public CVisualEffectWithParam<SEmbossParam>
{
public:
	virtual const char* GetName() const override { return "Emboss"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Emboss"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 2値化.
class CThresholdEffect final : public CVisualEffectWithParam<SThresholdParam>
{
public:
	virtual const char* GetName() const override { return "Threshold"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Threshold"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 魚眼(樽型歪み).
class CFisheyeEffect final : public CVisualEffectWithParam<SFisheyeParam>
{
public:
	virtual const char* GetName() const override { return "Fisheye"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Fisheye"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 色相シフト.
class CHueShiftEffect final : public CVisualEffectWithParam<SHueShiftParam>
{
public:
	virtual const char* GetName() const override { return "HueShift"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_HueShift"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 熱気揺らぎ(陽炎).
class CHeatHazeEffect final : public CVisualEffectWithParam<SHeatHazeParam>
{
public:
	virtual const char* GetName() const override { return "HeatHaze"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_HeatHaze"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// オールドフィルム(古い映像風).
class COldFilmEffect final : public CVisualEffectWithParam<SOldFilmParam>
{
public:
	virtual const char* GetName() const override { return "OldFilm"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_OldFilm"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 雨(雨筋オーバーレイ).
class CRainEffect final : public CVisualEffectWithParam<SRainParam>
{
public:
	virtual const char* GetName() const override { return "Rain"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Rain"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 雪(雪片オーバーレイ).
class CSnowEffect final : public CVisualEffectWithParam<SSnowParam>
{
public:
	virtual const char* GetName() const override { return "Snow"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Snow"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// フォグ(ノイズミスト式).
class CFogEffect final : public CVisualEffectWithParam<SFogParam>
{
public:
	virtual const char* GetName() const override { return "Fog"; }
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual const char* GetMainEntry() const override { return "PS_Fog"; }
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

// 深度フォグ(深度バッファ参照).
class CFogDepthEffect final : public CVisualEffectWithParam<SFogDepthParam>
{
public:
	virtual const char* GetName() const override { return "FogDepth"; }
	// 深度バッファを追加バインドするため Apply を上書きする.
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV ) override;
	virtual void DebugParamEdit() override;
protected:
	virtual const TCHAR* GetShaderPath() const override;
	virtual bool Init() override;
	virtual void PackParam( D3DXVECTOR4* pOut ) const override;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
