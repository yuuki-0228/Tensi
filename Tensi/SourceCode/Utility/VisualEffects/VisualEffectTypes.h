#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\..\Global.h"

// 視覚効果の種類.
enum class enVisualEffect : unsigned char
{
	Grayscale,			// 白黒.
	Sepia,				// セピア.
	Invert,				// 色反転.
	Posterize,			// ポスタライズ(減色).
	ColorFilter,		// カラーフィルター(青色フィルター等).
	ColorCorrection,	// 色調補正(明度/コントラスト/彩度/ガンマ).
	Vignette,			// ビネット(周辺減光).
	Noise,				// ノイズ.
	FilmGrain,			// フィルムグレイン.
	Scanline,			// スキャンライン.
	CRT,				// CRT(ブラウン管風).
	Pixelate,			// ピクセル化(モザイク).
	RGBShift,			// RGB分離(色収差).
	Glitch,				// グリッチ.
	Wave,				// 波歪み.
	Sharpen,			// シャープ.
	RadialBlur,			// 放射ブラー.
	GaussianBlur,		// ガウスブラー.
	Bloom,				// ブルーム.
	MotionBlur,			// モーションブラー(残像).
	Frost,				// 曇り(すりガラス).
	Toon,				// トゥーン(セル塗り風).
	Watercolor,			// 水彩.
	OilPainting,		// 油絵.
	ColoredPencil,		// 色鉛筆.
	Outline,			// アウトライン(輪郭線を重ねる).
	EdgeDetect,			// エッジ検出(輪郭のみ表示).
	EdgeEnhance,		// エッジ強調.
	Halftone,			// ハーフトーン(漫画のトーン風ドット).
	Emboss,				// エンボス(凹凸彫刻風).
	Threshold,			// 2値化.
	Fisheye,			// 魚眼(樽型歪み).
	HueShift,			// 色相シフト.
	SoftFocus,			// ソフトフォーカス(ふんわりグロー).
	HeatHaze,			// 熱気揺らぎ(陽炎).
	OldFilm,			// オールドフィルム(古い映像風).
	Rain,				// 雨(雨筋オーバーレイ).
	Snow,				// 雪(雪片オーバーレイ).
	Fog,				// フォグ(ノイズミスト式. 深度不要で2D/3D両用).
	FogDepth,			// 深度フォグ(深度バッファ参照. 3Dメッシュ向け).

	Max
} typedef EVisualEffect;

//----------------------------.
//	各エフェクトのパラメータ.
//	VisualEffectManager::Param<SXXXParam>() で取得して書き換える.
//	( Grayscale / Invert はパラメータ無し. 適用率は SetIntensity で設定する ).
//----------------------------.

// ノイズ.
struct SNoiseParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Noise;
	float	Strength	= 0.2f;		// ノイズの強さ(0～1).
	float	GrainSize	= 2.0f;		// 粒の大きさ(px).
	bool	IsColor		= false;	// カラーノイズにするか.
	float	Speed		= 15.0f;	// 更新回数(回/秒).
};

// フィルムグレイン.
struct SFilmGrainParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::FilmGrain;
	float	Strength	= 0.15f;	// 粒子の強さ(0～1).
	float	GrainSize	= 1.5f;		// 粒の大きさ(px).
	float	Flicker		= 0.06f;	// 明滅の強さ(0～1).
	float	Response	= 0.7f;		// 明部で粒子を弱める度合い(0～1).
};

// モーションブラー(残像).
struct SMotionBlurParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::MotionBlur;
	float	BlendRate	= 0.75f;	// 前フレームの残す割合(0～1). 大きいほど残像が長い.
};

// ブルーム.
struct SBloomParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Bloom;
	float	Threshold	= 0.6f;		// 光らせる輝度のしきい値(0～1).
	float	Strength	= 1.2f;		// 加算する光の強さ.
	float	BlurRadius	= 12.0f;	// 光のぼかし半径(px).
	int		BlurCount	= 2;		// ぼかし回数(1～4). 多いほど柔らかい.
};

// CRT(ブラウン管風).
struct SCRTParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::CRT;
	float	Curvature		= 0.15f;	// 画面の湾曲量(0～1).
	float	Scanline		= 0.35f;	// 走査線の濃さ(0～1).
	float	Aperture		= 0.25f;	// RGBマスク(アパーチャグリル)の濃さ(0～1).
	float	ChromaOffset	= 1.2f;		// 色ずれ量(px).
	float	EdgeFade		= 0.15f;	// 四隅の減光量(0～1).
};

// スキャンライン.
struct SScanlineParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Scanline;
	float	LineWidth	= 3.0f;		// 線の間隔(px).
	float	Strength	= 0.3f;		// 線の濃さ(0～1).
	float	ScrollSpeed	= 30.0f;	// スクロール速度(px/秒).
	bool	IsVertical	= false;	// 縦線にするか(通常は横線).
};

// ピクセル化(モザイク).
struct SPixelateParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Pixelate;
	float	PixelSize	= 8.0f;		// 1ドットの大きさ(px).
};

// RGB分離(色収差).
struct SRGBShiftParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::RGBShift;
	float	Offset		= 2.0f;		// ずらす量(px).
	float	AngleDeg	= 0.0f;		// ずらす方向(度).
};

// グリッチ.
struct SGlitchParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Glitch;
	float	Speed			= 8.0f;		// 乱れの更新速度(回/秒).
	float	BlockSize		= 32.0f;	// ずれるブロックの高さ(px).
	float	BlockStrength	= 24.0f;	// ブロックの最大ずれ量(px).
	float	ColorShift		= 6.0f;		// 乱れ発生時の色ずれ量(px).
	float	LineNoise		= 0.3f;		// ノイズ線の発生量(0～1).
	float	Rate			= 0.3f;		// 乱れの発生率(0～1).
};

// カラーフィルター.
//	例) 青色フィルター : MulColor = { 0.75f, 0.9f, 1.25f, 1.0f }.
struct SColorFilterParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::ColorFilter;
	D3DXCOLOR4	MulColor	= { 1.0f, 1.0f, 1.0f, 1.0f };	// 乗算する色.
	D3DXCOLOR4	AddColor	= { 0.0f, 0.0f, 0.0f, 0.0f };	// 加算する色.
};

// 曇り(すりガラス).
struct SFrostParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Frost;
	float	BlurRadius		= 10.0f;	// ぼかし半径(px).
	float	Frost			= 0.5f;		// 白みの強さ(0～1).
	float	NoiseStrength	= 14.0f;	// ざらつきによる歪み量(px).
	float	NoiseGrain		= 3.0f;		// ざらつきの粒の大きさ(px).
};

// セピア.
struct SSepiaParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Sepia;
	D3DXCOLOR4	Color	= { 1.07f, 0.74f, 0.43f, 1.0f };	// セピアの色味.
};

// ビネット(周辺減光).
struct SVignetteParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Vignette;
	float		Radius		= 0.55f;	// 減光が始まる半径(0～1, 画面中心基準).
	float		Softness	= 0.35f;	// 減光の滑らかさ(0～1).
	float		EdgeAlpha	= 0.0f;		// 縁の不透明度(透明ウィンドウで縁を見せたい時用).
	D3DXCOLOR4	Color		= { 0.0f, 0.0f, 0.0f, 1.0f };	// 縁の色.
};

// ガウスブラー.
struct SGaussianBlurParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::GaussianBlur;
	float	Radius	= 8.0f;		// ぼかし半径(px, 最大32).
};

// 放射ブラー.
struct SRadialBlurParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::RadialBlur;
	float	CenterU		= 0.5f;		// 中心位置U(0～1).
	float	CenterV		= 0.5f;		// 中心位置V(0～1).
	float	Strength	= 0.15f;	// ブラーの強さ(0～1).
	int		Samples		= 12;		// サンプル数(2～24).
};

// ポスタライズ(減色).
struct SPosterizeParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Posterize;
	float	Levels	= 6.0f;		// 色の段階数(2～32).
};

// 波歪み.
struct SWaveParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Wave;
	float	AmplitudeX	= 8.0f;		// 横方向の振れ幅(px).
	float	AmplitudeY	= 0.0f;		// 縦方向の振れ幅(px).
	float	Frequency	= 6.0f;		// 波の数(画面あたり).
	float	Speed		= 2.0f;		// 波の速さ.
};

// 色調補正.
struct SColorCorrectionParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::ColorCorrection;
	float	Brightness	= 0.0f;		// 明度(-1～1, 加算).
	float	Contrast	= 1.0f;		// コントラスト(1で等倍).
	float	Saturation	= 1.0f;		// 彩度(0で白黒, 1で等倍).
	float	Gamma		= 1.0f;		// ガンマ(1で等倍).
};

// シャープ.
struct SSharpenParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Sharpen;
	float	Strength	= 1.0f;		// 強調の強さ.
};

// トゥーン(セル塗り風).
struct SToonParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Toon;
	float		Levels			= 5.0f;		// 色の段階数(2～16).
	float		EdgeThreshold	= 0.25f;	// 輪郭線を出すエッジの強さのしきい値(0～1).
	float		EdgeThickness	= 1.0f;		// 輪郭線の太さ(px).
	D3DXCOLOR4	EdgeColor		= { 0.1f, 0.05f, 0.05f, 1.0f };	// 輪郭線の色.
};

// 水彩.
struct SWatercolorParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Watercolor;
	float	Bleed		= 3.0f;		// にじみ(ぼかし)の強さ(px).
	float	Paper		= 0.35f;	// 紙の質感の強さ(0～1).
	float	EdgeDarken	= 0.5f;		// 輪郭に絵の具が溜まる濃さ(0～1).
	float	Wobble		= 2.0f;		// 手描き風の揺らぎ(px).
};

// 油絵.
struct SOilPaintingParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::OilPainting;
	int		Radius	= 4;	// 筆のタッチの大きさ(2～6). 大きいほど重い.
};

// 色鉛筆.
struct SColoredPencilParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::ColoredPencil;
	float	HatchScale		= 6.0f;		// ハッチング(斜線)の間隔(px).
	float	EdgeStrength	= 0.8f;		// 輪郭の線の濃さ(0～1).
	float	Saturation		= 0.75f;	// 彩度(鉛筆らしさのため少し下げる).
	float	Paper			= 0.15f;	// 紙の白さの持ち上げ(0～1).
};

// アウトライン(輪郭線を重ねる).
struct SOutlineParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Outline;
	float		Threshold	= 0.2f;	// 輪郭と判定するエッジの強さ(0～1).
	float		Thickness	= 1.0f;	// 輪郭線の太さ(px).
	D3DXCOLOR4	Color		= { 0.0f, 0.0f, 0.0f, 1.0f };	// 輪郭線の色.
};

// エッジ検出(輪郭のみ表示).
struct SEdgeDetectParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::EdgeDetect;
	float		Threshold	= 0.1f;	// 輪郭と判定するエッジの強さ(0～1).
	D3DXCOLOR4	EdgeColor	= { 1.0f, 1.0f, 1.0f, 1.0f };	// 輪郭の色.
	D3DXCOLOR4	BackColor	= { 0.0f, 0.0f, 0.0f, 1.0f };	// 背景の色.
};

// エッジ強調.
struct SEdgeEnhanceParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::EdgeEnhance;
	float	Strength	= 1.0f;	// 強調の強さ.
	float	Thickness	= 1.0f;	// 検出幅(px).
};

// ハーフトーン(漫画のトーン風ドット).
struct SHalftoneParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Halftone;
	float		CellSize	= 6.0f;		// ドットの間隔(px).
	float		AngleDeg	= 45.0f;	// ドット網の角度(度).
	bool		IsColor		= false;	// 元の色でドットを描くか.
	D3DXCOLOR4	InkColor	= { 0.05f, 0.05f, 0.05f, 1.0f };	// インクの色.
	D3DXCOLOR4	PaperColor	= { 0.97f, 0.96f, 0.9f, 1.0f };		// 紙の色.
};

// エンボス(凹凸彫刻風).
struct SEmbossParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Emboss;
	float	Strength	= 2.0f;		// 凹凸の強さ.
	float	AngleDeg	= 45.0f;	// 光の方向(度).
	float	Offset		= 1.0f;		// 検出幅(px).
};

// 2値化.
struct SThresholdParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Threshold;
	float		Threshold	= 0.5f;		// しきい値(0～1).
	float		Smooth		= 0.02f;	// 境界の滑らかさ(0～0.5).
	D3DXCOLOR4	WhiteColor	= { 1.0f, 1.0f, 1.0f, 1.0f };	// 明部の色.
	D3DXCOLOR4	BlackColor	= { 0.0f, 0.0f, 0.0f, 1.0f };	// 暗部の色.
};

// 魚眼(樽型歪み).
struct SFisheyeParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Fisheye;
	float	Strength	= 0.5f;	// 歪みの強さ(-1～1. 負で糸巻き型).
};

// 色相シフト.
struct SHueShiftParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::HueShift;
	float	AngleDeg	= 180.0f;	// 色相の回転量(度).
	float	Speed		= 0.0f;		// 回転アニメーション速度(度/秒. 0で停止).
};

// ソフトフォーカス(ふんわりグロー).
struct SSoftFocusParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::SoftFocus;
	float	BlurRadius	= 10.0f;	// ぼかし半径(px).
	float	Strength	= 0.6f;		// グローの強さ(0～1).
};

// 熱気揺らぎ(陽炎).
struct SHeatHazeParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::HeatHaze;
	float	Amplitude	= 3.0f;	// 揺らぎの振れ幅(px).
	float	Scale		= 8.0f;	// 揺らぎの細かさ.
	float	Speed		= 1.5f;	// 揺らぎの速さ.
	float	RiseSpeed	= 0.8f;	// 立ち上る速さ.
};

// オールドフィルム(古い映像風).
struct SOldFilmParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::OldFilm;
	float	Sepia		= 0.7f;	// セピア調の強さ(0～1).
	float	Grain		= 0.2f;	// 粒子ノイズの強さ(0～1).
	float	Scratch		= 0.4f;	// 縦傷の発生量(0～1).
	float	Jitter		= 0.3f;	// 画面の上下ジッター(0～1).
	float	Flicker		= 0.1f;	// 明滅の強さ(0～1).
	float	Vignette	= 0.5f;	// 周辺減光の強さ(0～1).
};

// 雨(窓ガラスを流れ落ちる雨滴).
//	列を流れ落ちる水滴＋背後に残る雫のトレイルを高さ場として生成し、その勾配を法線として
//	ぼかしたガラス面を屈折させる. 水滴はすべて流れ落ちる(静止した水玉は無い).
//	水滴部分はアルファも持ち上がる(透明ウィンドウでも見える).
struct SRainParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Rain;
	float	Amount		= 0.4f;		// 雨の量(0～1. 水滴の多さ).
	float	Speed		= 0.25f;	// 流れ落ちる速さ.
	float	Refract		= 1.0f;		// 屈折の強さ(水滴に映る背景のゆがみ. 0～3).
	float	Blur		= 3.0f;		// ガラス面のぼかし(px. すりガラスの曇り感).
	float	Brightness	= 0.6f;		// ハイライトの強さ(0～1).
};

// 雪(雪片オーバーレイ).
//	雪片が乗った部分はアルファも持ち上がる(透明ウィンドウでも見える).
struct SSnowParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Snow;
	float	Density		= 0.5f;	// 雪の量(0～1).
	float	Speed		= 0.08f;	// 落ちる速さ(雪はゆっくり舞い落ちる).
	float	Size		= 0.6f;	// 雪片の大きさ(0～1).
	float	Sway		= 0.5f;	// 横揺れの強さ(0～1).
	float	Brightness	= 0.9f;	// 雪片の明るさ(0～1).
};

// フォグ(ノイズミスト式).
//	深度を使わず、流れる霧をオーバーレイ描画する. 2D/3Dどちらでも使える.
//	霧が乗った部分はアルファも持ち上がる(透明ウィンドウでも見える).
struct SFogParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::Fog;
	float		Density		= 0.5f;		// 霧の濃さ(0～1).
	float		Scale		= 3.0f;		// 霧の粗さ(小さいほど大きな塊).
	float		SpeedX		= 0.03f;	// 横方向の流れる速さ.
	float		SpeedY		= 0.0f;		// 縦方向の流れる速さ.
	float		BottomBias	= 0.5f;		// 画面下部に溜まる度合い(0～1).
	D3DXCOLOR4	Color		= { 0.85f, 0.87f, 0.9f, 1.0f };	// 霧の色.
};

// 深度フォグ(深度バッファ参照).
//	カメラからの距離でフォグをかける. 3Dメッシュ描画向け.
//	Near / Far は使用しているカメラのニア/ファークリップと合わせること.
struct SFogDepthParam
{
	static constexpr EVisualEffect TYPE = EVisualEffect::FogDepth;
	float		Near		= 0.1f;		// カメラのニアクリップ.
	float		Far			= 1000.0f;	// カメラのファークリップ.
	float		Start		= 10.0f;	// フォグが始まる距離.
	float		End			= 100.0f;	// フォグが最大になる距離.
	float		MaxDensity	= 1.0f;		// フォグの最大濃度(0～1).
	bool		ApplySky	= false;	// 何も描画されていない部分(深度最大)にもかけるか.
	D3DXCOLOR4	Color		= { 0.7f, 0.75f, 0.8f, 1.0f };	// フォグの色.
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
