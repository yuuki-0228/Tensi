#pragma once
#include "..\..\Global.h"

// ローカル座標の番号.
enum class enLocalPosition : unsigned char
{
	LeftUp,		// 0:左上.
	Left,		// 1:左.
	LeftDown,	// 2:左下.
	Down,		// 3:下.
	RightDown,	// 4:右下.
	Right,		// 5:右.
	RightUp,	// 6:右上.
	Up,			// 7:上.
	Center,		// 8:中央.

	Max,
	None = 0
} typedef ELocalPosition;

// サンプラステート.
enum class enSamplerState : unsigned char
{
	Wrap,		// タイル状に繰り返される.
	Mirror,		// 反転しながら繰り返される.
	Clamp,		// 0~1に設定される.
	Border,		// 0~1の範囲外はhlslで指定した色になる.
	MirrorOne,	// 0を中心に反転される.

	Max,
	None = 0
} typedef ESamplerState;

// コンスタントバッファのアプリ側の定義.
//	※シェーダ内のコンスタントバッファと一致している必要あり.
struct stSpriteShaderConstantBuffer
{
	ALIGN16 D3DXMATRIX	mWVP;				// ワールド,ビュー,プロジェクションの合成変換行列.	
	ALIGN16 D3DXCOLOR4	vColor;				// カラー(RGBAの型に合わせる).
	ALIGN16 D3DXVECTOR4	vUV;				// UV座標(x,yのみ使用).
	ALIGN16 D3DXVECTOR4	vRenderArea;		// 描画するエリア.
	ALIGN16 float		fViewPortWidth;		// ビューポート幅.
	ALIGN16 float		fViewPortHeight;	// ビューポート高さ.
	ALIGN16 D3DXVECTOR4 vFlag;				// フラグ(1以上なら使用する)(x:ディザ抜きを使用するか)(y:アルファブロックを使用するか)(z:カラーにマスクを使用するか).
} typedef SSpriteShaderConstantBuffer;

// 幅高さ構造体.
struct stSize
{
	float w;
	float h;

	stSize()
		: stSize ( 0.0f, 0.0f )
	{
	}
	stSize( const float size[2] )
		: w	( size[0] )
		, h	( size[1] )
	{
	}
	stSize( const float w, const float h )
		: w	( w )
		, h	( h )
	{
	}
} typedef SSize;

// 頂点の構造体.
struct stVertex
{
	D3DXPOSITION3	Pos;	// 頂点座標.
	D3DXVECTOR2		Tex;	// テクスチャ座標.
} typedef SVertex;

//スプライト構造体.
struct stSpriteState
{
	SSize									Disp;				// 表示幅高さ.
	SSize									Base;				// 元画像幅高さ.
	SSize									Stride;				// 1コマ当たりの幅高さ.
	D3DXVECTOR2								OffSet;				// 補正値.
	D3DXPOSITION3							AddCenterPos;		// 中心座標に変換用の追加座標.
	ELocalPosition							LocalPosNo;			// ローカル座標の番号.
	std::string								FilePath;			// ファイルパス.
} typedef SSpriteState;

// スプライトアニメーション情報構造体.
struct stSpriteAnimState
{
	POINT			PatternNo;				// パターン番号.
	POINT			PatternMax;				// パターン最大数.
	D3DXVECTOR2		UV;						// テクスチャUV座標.
	D3DXVECTOR2		Scroll;					// UVスクロール.
	D3DXVECTOR2		ScrollSpeed;			// テクスチャUV座標.
	int				AnimNumber;				// アニメーション番号.
	int				AnimNumberMax;			// アニメーション番号の最大数.
	int				FrameCount;				// フレームカウント.
	int				FrameCountSpeed;		// フレームカウント速度.
	int				FrameTime;				// フレームタイム.
	int				AnimPlayMax;			// アニメーションの行う最大数(0:無制限).
	int				AnimPlayCnt;			// アニメーションの行った回数.
	bool			IsSetAnimNumber;		// アニメーション番号を設定するか.
	bool			IsSetPatternNo;			// パターン番号を設定するか.
	bool			IsAnimation;			// アニメーションするか.
	bool			IsAnimationX;			// X方向にアニメーションするか.
	bool			IsAnimationY;			// Y方向にアニメーションするか.
	bool			IsUVScrollX;			// X方向にUVスクロールをするか.
	bool			IsUVScrollY;			// Y方向にUVスクロールをするか.

	stSpriteAnimState()
		: PatternNo			( { 1, 1 } )
		, PatternMax		( { 1, 1 } )
		, UV				( 0.0f, 0.0f )
		, ScrollSpeed		( 0.0f, 0.0f )
		, AnimNumber		( 0 )
		, AnimNumberMax		( 1 )
		, FrameCount		( 1 )
		, FrameCountSpeed	( 1 )
		, FrameTime			( 5 )
		, AnimPlayMax		( 0 )
		, AnimPlayCnt		( 0 )
		, IsSetAnimNumber	( false )
		, IsSetPatternNo	( false )
		, IsAnimation		( false )
		, IsAnimationX		( false )
		, IsAnimationY		( false )
		, IsUVScrollX		( false )
		, IsUVScrollY		( false )
	{}
	
	//--------------------------.
	// UV座標に変換.
	//--------------------------.
	D3DXVECTOR2 ConvertUV() {
		return {
			// x座標.
			static_cast<float>( PatternNo.x ) / static_cast<float>( PatternMax.x ),
			// y座標.
			static_cast<float>( PatternNo.y ) / static_cast<float>( PatternMax.y )
		};
	}

	//--------------------------.
	// アニメーション番号の設定.
	//	アニメーション番号を設定をしない場合は処理は行わない.
	//--------------------------.
	void SetAnimNumber() {
		if ( IsSetAnimNumber == false ) return;

		// アニメーションをしない.
		IsAnimation = false;

		// アニメーション番号を算出.
		PatternNo.x = AnimNumber % PatternMax.x;
		PatternNo.y = AnimNumber / PatternMax.x;

		// UV座標に変換.
		UV = ConvertUV();
	}

	//--------------------------.
	// アニメーション番号の設定.
	//	アニメーション番号を設定をしない場合は処理は行わない.
	//--------------------------.
	void SetPatternNo() {
		if ( IsSetPatternNo == false ) return;

		// アニメーションをしない.
		IsAnimation = false;

		// UV座標に変換.
		UV = ConvertUV();
	}

	//--------------------------.
	// アニメーションの更新.
	//	アニメーションしない場合は処理は行わない.
	//--------------------------.
	void AnimUpdate() {
		if( IsAnimation == false && IsAnimationX == false && IsAnimationY == false ) return;

		if( FrameCount % FrameTime == 0 ){
			if ( IsAnimation ) {
				// アニメーション番号を算出.
				PatternNo.x = AnimNumber % PatternMax.x;
				PatternNo.y = AnimNumber / PatternMax.x;
				AnimNumber++;	// アニメーション番号を加算.
				if ( AnimNumber >= AnimNumberMax ) {
					// アニメーション番号が最大アニメーション数より多ければ.
					// 初期化する.
					FrameCount = 0;
					AnimNumber = 0;

					// アニメーションの使用回数制限
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt = 0;
							IsAnimation = false;
						}
					}
				}
			}
			else if ( IsAnimationX ) {
				// アニメーション番号を算出.
				PatternNo.x++;
				if ( PatternNo.x >= PatternMax.x ) {
					// アニメーション番号が最大アニメーション数より多ければ.
					// 初期化する.
					PatternNo.x = 0;
					FrameCount  = 0;
					AnimNumber  = 0;

					// アニメーションの使用回数制限
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt  = 0;
							IsAnimationX = false;
						}
					}
				}
			}
			else if ( IsAnimationY ) {
				// アニメーション番号を算出.
				PatternNo.y++;
				if ( PatternNo.y >= PatternMax.y ) {
					// アニメーション番号が最大アニメーション数より多ければ.
					// 初期化する.
					PatternNo.y = 0;
					FrameCount  = 0;
					AnimNumber  = 0;

					// アニメーションの使用回数制限
					if ( AnimPlayMax > 0 ) {
						AnimPlayCnt++;
						if ( AnimPlayCnt >= AnimPlayMax ) {
							AnimPlayCnt  = 0;
							IsAnimationY = false;
						}
					}
				}
			}
		}
		FrameCount += FrameCountSpeed;
		// UV座標に変換.
		UV = ConvertUV();
	}

	//--------------------------.
	// UVスクロールの更新.
	//--------------------------.
	void UVScrollUpdate()
	{
		if ( IsUVScrollX ) {
			UV.x += ScrollSpeed.x;
			if ( UV.x > 1.0f ) UV.x -= 1.0f;
			if ( UV.x < 0.0f ) UV.x += 1.0f;
		}

		if ( IsUVScrollY ) {
			UV.y += ScrollSpeed.y;
			if ( UV.y > 1.0f ) UV.y -= 1.0f;
			if ( UV.y < 0.0f ) UV.y += 1.0f;
		}
	}

} typedef SSpriteAnimState;

// 描画用画像情報構造体.
struct stSpriteRenderState
{
	SSpriteAnimState			AnimState;		// アニメーション情報.
	STransform					Transform;		// トランスフォーム.
	ID3D11ShaderResourceView*	MaskTexture;	// マスクテクスチャ.
	ID3D11ShaderResourceView*	RuleTexture;	// ルールテクスチャ.
	ESamplerState				SmaplerNo;		// サンプラ番号.
	D3DXCOLOR4					Color;			// 色.
	D3DXVECTOR4					RenderArea;		// 描画するエリア( 左上x, 左上y, 幅, 高さ ).
	bool						IsDisp;			// 表示するか.
	bool						IsDither;		// ディザ抜きを使用するか.
	bool						IsAlphaBlock;	// アルファブロックを使用するか.
	bool						IsColorMask;	// 色の変更を指定するためにマスクを使用するか.


	stSpriteRenderState()
		: AnimState		()
		, Transform		()
		, MaskTexture	( nullptr )
		, RuleTexture	( nullptr )
		, SmaplerNo		( ESamplerState::Wrap )
		, Color			( Color4::White )
		, RenderArea	( 0.0f, 0.0f, -1.0f, -1.0f )
		, IsDisp		( true )
		, IsDither		( false )
		, IsAlphaBlock	( true )
		, IsColorMask	( false )
	{}

	~stSpriteRenderState()
	{
		SAFE_RELEASE( RuleTexture	);
		SAFE_RELEASE( MaskTexture	);
	}

} typedef SSpriteRenderState;
using SSpriteRenderStateList = std::vector<SSpriteRenderState>;
