#include "Font.h"
#include "Obfuscate/Obfuscate.h"
#ifdef ENABLE_FONT
#include "..\DirectX\DirectX11.h"
#include "..\..\Resource\FontResource\FontResource.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include <cmath>

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Font.hlsl" );

	// フォントデータファイルパス.
	const std::string FONT_DATA_FILE_PATH = OBF( "Data\\Sprite\\Font\\FontData.json" );

	// 3D描画時の行間.
	constexpr float LINE_SPACING_3D	= 12.0f;
	// 下線の位置(セルの高さに対する割合).
	constexpr float UNDERLINE_POS	= 0.88f;
	// 取り消し線の位置(セルの高さに対する割合).
	constexpr float STRIKEOUT_POS	= 0.52f;
	// 下線・取り消し線の太さ(セルの高さに対する割合).
	constexpr float LINE_SIZE		= 0.05f;

	// アウトライン幅の最大値(SDF距離値).
	constexpr float OUTLINE_DIST_MAX	= 0.4f;
	// 太字の膨張量の最大値(SDF距離値).
	constexpr float BOLD_DIST_MAX		= 0.2f;
	// グロー幅の最大値(SDF距離値).
	constexpr float GLOW_DIST_MAX		= 0.45f;
}

CFont::CFont()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pConstantBuffer			( nullptr )
	, m_Vertices				()
	, m_FontState				()
	, m_FontRenderState			()
	, m_Size3D					()
	, m_pSampleLinears			()
	, m_FileName				( "" )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
{
}

CFont::~CFont()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader	);
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//---------------------------------.
// 初期化.
//---------------------------------.
HRESULT CFont::Init( const std::string& FilePath, const std::string& FileName )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_FileName	= FileName;

	// スプライト情報の取得.
	if ( FAILED( FontStateDataLoad( FilePath ) ) ) return E_FAIL;

	// 初期化.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;
	return S_OK;
}


//---------------------------------.
// UIで描画.
//---------------------------------.
void CFont::RenderUI( SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderText( RenderState->Text, RenderState, false, false );
}
void CFont::RenderUI( const std::string& Text, SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderText( Text, RenderState, false, false );
}

//---------------------------------.
// 3Dで描画.
//---------------------------------.
void CFont::Render3D( SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderText( RenderState->Text, RenderState, true, IsBillBoard );
}
void CFont::Render3D( const std::string& Text, SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderText( Text, RenderState, true, IsBillBoard );
}

//---------------------------------.
// テキストの描画(UI/3D共通).
//---------------------------------.
void CFont::RenderText( const std::string& Text, SFontRenderState* pRenderState, const bool Is3D, const bool IsBillBoard )
{
	SFontRenderState* State = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	// 非表示の場合は処理を行わない.
	if ( State->IsDisp	== false	) return;
	if ( State->Color.w	== 0.0f		) return;
	if ( Text.empty()				) return;

	// 文字送り・行送りの基準値.
	const D3DXSCALE3	WScale		= State->Transform.GetWorldScale();
	const float			ScaleX		= Is3D ? State->Transform.Scale.x : WScale.x;
	const float			ScaleY		= Is3D ? State->Transform.Scale.y : WScale.y;
	const float			BaseAdvW	= m_FontState.Disp.w * ScaleX;	// 1文字の送り幅.
	const float			BaseCharH	= m_FontState.Disp.h * ScaleY;	// 1文字の高さ.
	const float			IntervalW	= Is3D ? 0.0f : State->TextInterval.w;
	const float			LineSpace	= Is3D ? LINE_SPACING_3D : State->TextInterval.h;
	const float			YSign		= Is3D ? -1.0f : 1.0f;			// 下方向の符号(UI:正, 3D:負).

	// タグの解析(無効時は文字のみのリストが作成される).
	const SFontDrawItemList Items	= CFontTagParser::Parse( Text, *State, m_FontState.Disp.h );
	const int				ItemNum	= static_cast<int>( Items.size() );

	// 行ごとの情報を計測する.
	struct SLineInfo
	{
		int			Begin;		// 開始アイテム番号.
		int			End;		// 終了アイテム番号(この番号は含まない).
		float		Width;		// 行の幅.
		float		Height;		// 行の高さ.
		ETextAlign	Align;		// 行揃え.
		bool		HasChar;	// 文字があるか.
	};
	std::vector<SLineInfo> Lines;
	{
		SLineInfo	Line		= { 0, 0, 0.0f, BaseCharH, State->TextAlign, false };
		float		LastTrail	= 0.0f;	// 行末の余分な間隔(字間など).
		for ( int i = 0; i < ItemNum; ++i ) {
			const SFontDrawItem& Item = Items[i];
			if ( Item.IsNewLine ) {
				Line.End	= i;
				Line.Width	-= LastTrail;	// 行末の字間は幅に含めない.
				Lines.emplace_back( Line );
				Line		= { i + 1, i + 1, 0.0f, BaseCharH, State->TextAlign, false };
				LastTrail	= 0.0f;
				continue;
			}
			if ( Item.Char.empty() ) {
				// 送りのみのアイテム.
				Line.Width += Item.Space * ScaleX;
				LastTrail	= 0.0f;
				continue;
			}
			// 行揃えは行の最初の文字のスタイルを使用する.
			if ( Line.HasChar == false ) Line.Align = Item.Style.Align;
			Line.HasChar = true;

			const float CharH = BaseCharH * Item.Style.SizeScale;
			if ( CharH > Line.Height ) Line.Height = CharH;

			const float Trail = IntervalW + Item.Style.CharSpace * ScaleX;
			Line.Width	+= BaseAdvW * Item.Style.SizeScale + Trail;
			LastTrail	 = Trail;
		}
		Line.End	= ItemNum;
		Line.Width	-= LastTrail;
		Lines.emplace_back( Line );
	}

	// 中心座標による行揃えの補正量.
	const ELocalPosition LocalPos = GetLocalPosition( State );
	float AlignAnchorOffset = 0.0f;
	if ( LocalPos == ELocalPosition::Up ||
		 LocalPos == ELocalPosition::Center ||
		 LocalPos == ELocalPosition::Down )
	{
		AlignAnchorOffset = BaseAdvW / 2.0f;
	}
	if ( LocalPos == ELocalPosition::RightUp ||
		 LocalPos == ELocalPosition::Right ||
		 LocalPos == ELocalPosition::RightDown )
	{
		AlignAnchorOffset = BaseAdvW;
	}

	// 1行ずつ描画する.
	const D3DXPOSITION3 BasePos	= State->Transform.Position;
	float				PenY	= BasePos.y;
	for ( const auto& Line : Lines ) {
		// 行揃えに対応した開始位置.
		float PenX = BasePos.x;
		if ( Line.Align == ETextAlign::Right ) {
			PenX -= Line.Width - AlignAnchorOffset;
		}
		else if ( Line.Align == ETextAlign::Center ) {
			PenX -= Line.Width / 2.0f - AlignAnchorOffset;
		}

		for ( int i = Line.Begin; i < Line.End; ++i ) {
			const SFontDrawItem& Item = Items[i];
			if ( Item.IsNewLine ) continue;
			if ( Item.Char.empty() ) {
				// 送りのみのアイテム.
				PenX += Item.Space * ScaleX;
				continue;
			}

			const SFontCharStyle&	Style	= Item.Style;
			const float				CharH	= BaseCharH * Style.SizeScale;
			const float				AdvW	= BaseAdvW * Style.SizeScale;
			const float				Trail	= IntervalW + Style.CharSpace * ScaleX;

			// 透明の場合は送りのみ進める.
			if ( Style.Color.w <= 0.0f && Style.IsMark == false ) {
				PenX += AdvW + Trail;
				continue;
			}

			// 行の下端に揃え、縦オフセットを反映する.
			const float OffsetY = ( Line.Height - CharH ) - Style.VOffset * ScaleY;

			// 1文字分のトランスフォームを作成する.
			STransform Trans	= State->Transform;
			Trans.Position		= D3DXPOSITION3( PenX, PenY + YSign * OffsetY, BasePos.z );
			Trans.Scale.x	   *= Style.SizeScale;
			Trans.Scale.y	   *= Style.SizeScale;

			// 矩形の幅の倍率(送り幅と字間を覆うようにする).
			const float QuadW		= Is3D ? m_Size3D.w : m_FontState.Disp.w;
			const float RectBase	= QuadW * ScaleX * Style.SizeScale;
			const float WidthScale	= RectBase > 0.0f ? ( AdvW + Trail ) / RectBase : 1.0f;

			// マーカー(背景).
			if ( Style.IsMark ) {
				RenderLineRect( Style.MarkColor, WidthScale, 0.0f, 1.0f, Trans, State, Is3D, IsBillBoard );
			}
			// 文字本体.
			RenderGlyph( Item.Char, Style, Trans, State, Is3D, IsBillBoard );
			// 下線.
			if ( Style.IsUnderline ) {
				RenderLineRect( Style.Color, WidthScale, UNDERLINE_POS, LINE_SIZE, Trans, State, Is3D, IsBillBoard );
			}
			// 取り消し線.
			if ( Style.IsStrikeout ) {
				RenderLineRect( Style.Color, WidthScale, STRIKEOUT_POS, LINE_SIZE, Trans, State, Is3D, IsBillBoard );
			}

			PenX += AdvW + Trail;
		}

		// 次の行へ.
		PenY += YSign * ( Line.Height + LineSpace );
	}
}

//---------------------------------.
// 1文字の描画.
//---------------------------------.
void CFont::RenderGlyph( const std::string& Char, const SFontCharStyle& Style, const STransform& Transform, SFontRenderState* pState, const bool Is3D, const bool IsBillBoard )
{
	// グリフ(SDFテクスチャ)の取得.
	const SFontGlyph Glyph = FontResource::GetFontGlyph( m_FileName, Char );
	if ( Glyph.pTexture	== nullptr ) return;
	if ( Glyph.CellW	<= 0.0f	   ) return;
	if ( Glyph.CellH	<= 0.0f	   ) return;

	// ワールド行列を取得.
	STransform Trans	= Transform;
	D3DXMATRIX mWorld	= Trans.GetWorldMatrix();

	// ビルボード用.
	if ( Is3D && IsBillBoard ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// ビュー行列.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;		// xyzを0にする.
		// CancelRotationの逆行列を求めます.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// クアッドのローカル情報.
	float x0, y0, w, h, YSign;
	GetQuadTopLeft( GetLocalPosition( pState ), Is3D, &x0, &y0, &w, &h, &YSign );

	// 中心座標に合わせてクアッドを移動する行列.
	D3DXMATRIX mLocal;
	D3DXMatrixTranslation( &mLocal, x0, y0, 0.0f );

	// SDFの余白分クアッドを拡張する行列(セル中心基準).
	const float sx = ( Glyph.CellW + Glyph.Pad * 2.0f ) / Glyph.CellW;
	const float sy = ( Glyph.CellH + Glyph.Pad * 2.0f ) / Glyph.CellH;
	const float cx = x0 + w / 2.0f;
	const float cy = y0 + YSign * h / 2.0f;
	D3DXMATRIX mPad;
	D3DXMatrixIdentity( &mPad );
	mPad._11 = sx;
	mPad._22 = sy;
	mPad._41 = cx * ( 1.0f - sx );
	mPad._42 = cy * ( 1.0f - sy );
	mLocal *= mPad;

	// イタリック(せん断)行列(文字の下端を基準に上側を傾ける).
	if ( Style.IsItalic ) {
		const float Slant	= tanf( Math::ToRadian( pState->ItalicSlant ) );
		const float Bottom	= y0 + YSign * h;
		D3DXMATRIX mItalic;
		D3DXMatrixIdentity( &mItalic );
		mItalic._21 = -Slant * YSign;
		mItalic._41 = Slant * YSign * Bottom;
		mLocal *= mItalic;
	}
	mWorld = mLocal * mWorld;

	// 画面上の文字の高さから「画面px→SDF距離値」の変換係数を求める.
	const float QuadH		= Is3D ? m_Size3D.h : m_FontState.Disp.h;
	const float ScreenH		= QuadH * ( Is3D ? Trans.Scale.y : Trans.GetWorldScale().y );
	const float PxPerGlyph	= Glyph.CellH > 0.0f ? ScreenH / Glyph.CellH : 0.0f;
	const float ToDist		= PxPerGlyph > 0.0f ? 1.0f / ( PxPerGlyph * Glyph.Spread * 2.0f ) : 0.0f;

	// アウトライン幅(従来の見た目に合わせて半分にする).
	float OutLineW = 0.0f;
	if ( Style.IsOutLine ) {
		OutLineW = Style.OutLineSize * 0.5f * ToDist;
		if ( OutLineW > OUTLINE_DIST_MAX ) OutLineW = OUTLINE_DIST_MAX;
	}
	// 太字の膨張量.
	float Bold = 0.0f;
	if ( Style.IsBold ) {
		Bold = pState->BoldWeight * ToDist;
		if ( Bold > BOLD_DIST_MAX ) Bold = BOLD_DIST_MAX;
	}
	// 縁のぼかし量(0の場合はシェーダ側で自動計算).
	const float Soft = pState->Softness * ToDist;
	// グロー幅.
	float GlowW = 0.0f;
	if ( Style.IsGlow ) {
		GlowW = Style.GlowSize * ToDist;
		if ( GlowW > GLOW_DIST_MAX ) GlowW = GLOW_DIST_MAX;
	}

	// 描画.
	SFontDrawParam Param;
	Param.mWorld		= mWorld;
	Param.pTexture		= Glyph.pTexture;
	Param.Color			= Style.Color;
	Param.OutLineColor	= Style.OutLineColor;
	Param.GlowColor		= Style.GlowColor;
	Param.SDFParam		= D3DXVECTOR4( OutLineW, Bold, Soft, Style.IsOutLine ? 1.0f : 0.0f );
	Param.GlowParam		= D3DXVECTOR4( GlowW, Style.GlowPower, Style.IsGlow ? 1.0f : 0.0f, 0.0f );
	Param.SmaplerNo		= pState->SmaplerNo;
	Param.Is3D			= Is3D;
	DrawQuad( Param, pState );
}

//---------------------------------.
// 矩形の描画(下線・取り消し線・マーカー用).
//---------------------------------.
void CFont::RenderLineRect( const D3DXCOLOR4& Color, const float WidthScale, const float PosRateY, const float SizeRateY, const STransform& Transform, SFontRenderState* pState, const bool Is3D, const bool IsBillBoard )
{
	if ( Color.w <= 0.0f ) return;

	// ワールド行列を取得.
	STransform Trans	= Transform;
	D3DXMATRIX mWorld	= Trans.GetWorldMatrix();

	// ビルボード用.
	if ( Is3D && IsBillBoard ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// ビュー行列.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;		// xyzを0にする.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// クアッドのローカル情報.
	float x0, y0, w, h, YSign;
	GetQuadTopLeft( GetLocalPosition( pState ), Is3D, &x0, &y0, &w, &h, &YSign );

	// 中心座標に合わせてクアッドを移動する行列.
	D3DXMATRIX mLocal;
	D3DXMatrixTranslation( &mLocal, x0, y0, 0.0f );

	// クアッドを指定位置の帯(矩形)に変形する行列.
	D3DXMATRIX mBand;
	D3DXMatrixIdentity( &mBand );
	mBand._11 = WidthScale;
	mBand._41 = x0 * ( 1.0f - WidthScale );
	mBand._22 = SizeRateY;
	mBand._42 = y0 * ( 1.0f - SizeRateY ) + YSign * h * PosRateY;
	mLocal *= mBand;
	mWorld = mLocal * mWorld;

	// 描画.
	SFontDrawParam Param;
	Param.mWorld		= mWorld;
	Param.pTexture		= nullptr;
	Param.Color			= Color;
	Param.OutLineColor	= Color;
	Param.GlowColor		= Color;
	Param.SDFParam		= D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );
	Param.GlowParam		= D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );
	Param.SmaplerNo		= pState->SmaplerNo;
	Param.Is3D			= Is3D;
	DrawQuad( Param, pState );
}

//---------------------------------.
// クアッドの描画.
//---------------------------------.
void CFont::DrawQuad( const SFontDrawParam& Param, SFontRenderState* pState )
{
	// 使用するシェーダの登録.
	m_pContext->VSSetShader( m_pVertexShader,	nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader,	nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE	pData;
	SFontShaderConstantBuffer	cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド行列を渡す(3Dの場合はビュー,プロジェクションも合成する).
		D3DXMATRIX m = Param.mWorld;
		if ( Param.Is3D ) m = Param.mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor			= Param.Color;
		cb.vOutLineColor	= Param.OutLineColor;
		cb.vGlowColor		= Param.GlowColor;

		// テクスチャ座標(UV座標).
		cb.vUV = D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f );

		// 描画するエリア.
		D3DXVECTOR4 RenderArea = pState->RenderArea;
		const float WndScalseW = DirectX11::GetWndWidth()  / Const::GameWindow().SIZE.x;
		const float WndScalseH = DirectX11::GetWndHeight() / Const::GameWindow().SIZE.y;
		RenderArea.x	*= WndScalseW;
		RenderArea.z	*= WndScalseW;
		RenderArea.y	*= WndScalseH;
		RenderArea.w	*= WndScalseH;
		cb.vRenderArea	 = RenderArea;

		// ビューポートの幅、高さを渡す(3Dでは使用しない).
		cb.vViewPort = Param.Is3D
			? D3DXVECTOR4( 0.0f, 0.0f, 0.0f, 0.0f )
			: D3DXVECTOR4( DirectX11::GetWndWidth(), DirectX11::GetWndHeight(), 0.0f, 0.0f );

		// 各種フラグ.
		cb.vFlag = D3DXVECTOR4(
			m_DitherFlag				? 1.0f : 0.0f,		// ディザ抜きを使用するか.
			m_AlphaBlockFlag			? 1.0f : 0.0f,		// アルファブロックを使用するか.
			Param.pTexture == nullptr	? 1.0f : 0.0f,		// 矩形描画か.
			0.0f );

		// SDF・グローのパラメータ.
		cb.vSDFParam	= Param.SDFParam;
		cb.vGlowParam	= Param.GlowParam;

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
	ID3D11Buffer* pVertexBuffer = Param.Is3D ? m_pVertexBuffer3D : m_pVertexBufferUI;
	m_pContext->IASetVertexBuffers( 0, 1,
		&pVertexBuffer, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( Param.SmaplerNo )] );
	ID3D11ShaderResourceView* pTexture = Param.pTexture;
	m_pContext->PSSetShaderResources( 0, 1, &pTexture );

	// アルファブレンド有効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//---------------------------------.
// 使用するローカル座標の番号を取得.
//---------------------------------.
ELocalPosition CFont::GetLocalPosition( const SFontRenderState* pState ) const
{
	// 未設定の場合はフォント情報の値を使用する.
	if ( pState == nullptr							) return m_FontState.LocalPosNo;
	if ( pState->LocalPosNo == ELocalPosition::Default	) return m_FontState.LocalPosNo;
	return pState->LocalPosNo;
}

//---------------------------------.
// クアッドのローカル左上座標などの取得.
//---------------------------------.
void CFont::GetQuadTopLeft( const ELocalPosition LocalPos, const bool Is3D, float* pX, float* pY, float* pW, float* pH, float* pYSign )
{
	const float w		= Is3D ? m_Size3D.w : m_FontState.Disp.w;
	const float h		= Is3D ? m_Size3D.h : m_FontState.Disp.h;
	const float YSign	= Is3D ? -1.0f : 1.0f;

	// UI(y正方向が下)基準の左上座標.
	float x0 = 0.0f;
	float y0 = 0.0f;
	switch ( LocalPos ) {
	case ELocalPosition::LeftUp:	x0 = 0.0f;		y0 = 0.0f;		break;
	case ELocalPosition::Left:		x0 = 0.0f;		y0 = -h / 2.0f;	break;
	case ELocalPosition::LeftDown:	x0 = 0.0f;		y0 = -h;		break;
	case ELocalPosition::Down:		x0 = -w / 2.0f;	y0 = -h;		break;
	case ELocalPosition::RightDown:	x0 = -w;		y0 = -h;		break;
	case ELocalPosition::Right:		x0 = -w;		y0 = -h / 2.0f;	break;
	case ELocalPosition::RightUp:	x0 = -w;		y0 = 0.0f;		break;
	case ELocalPosition::Up:		x0 = -w / 2.0f;	y0 = 0.0f;		break;
	case ELocalPosition::Center:
	default:						x0 = -w / 2.0f;	y0 = -h / 2.0f;	break;
	}

	if ( pX		!= nullptr ) *pX		= x0;
	if ( pY		!= nullptr ) *pY		= YSign * y0;	// 3Dはy正方向が上のため反転する.
	if ( pW		!= nullptr ) *pW		= w;
	if ( pH		!= nullptr ) *pH		= h;
	if ( pYSign	!= nullptr ) *pYSign	= YSign;
}

//----------------------------.
// フォント情報の読み込み.
//----------------------------.
HRESULT CFont::FontStateDataLoad( const std::string& FilePath )
{
	// 同じ名前のテキストの読み込み.
	std::string TextPath = FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

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
	Json j = FileManager::JsonLoad( TextPath );

	// 同じ名前のテキストが無かったため共有のファイルを読み込む.
	if ( j.empty() ) {
		// フォントデータファイルの読み込み.
		j = FileManager::JsonLoad( FONT_DATA_FILE_PATH );

		// ファイルが無いためファイルを作成する.
		if ( j.empty() ) return E_FAIL;
	}

	// スプライト情報の取得.
	m_FontState.LocalPosNo	= static_cast<ELocalPosition>( j["LocalPosition"].Get<int>() );
	m_FontState.Disp.w		= j["Disp"]["w"].Get<float>();
	m_FontState.Disp.h		= j["Disp"]["h"].Get<float>();
	return S_OK;
}

//----------------------------.
// シェーダ作成.
//----------------------------.
HRESULT CFont::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	auto result = DirectX11::MutexD3DX11CompileFromFile(
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
		nullptr );			// 戻り値へのポインタ(未使用).
	if ( FAILED( result ) )
	{
		ErrorMessage( "hlsl読み込み失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「バーテックスシェーダ」を作成.
	result = DirectX11::MutexDX11CreateVertexShader(
		pCompiledShader,
		nullptr,
		&m_pVertexShader );	// (out)バーテックスシェーダ.
	if ( FAILED( result ) )
	{
		ErrorMessage( "バーテックスシェーダ作成失敗",result );
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
	result = DirectX11::MutexDX11CreateInputLayout(
		layout,
		numElements,
		pCompiledShader,
		&m_pVertexLayout );	//(out)頂点インプットレイアウト.
	if (FAILED( result ) )
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSLからピクセルシェーダのブロブを作成.
	 DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,				// シェーダファイル名(HLSLファイル).
		nullptr,					// マクロ定義の配列へのポインタ(未使用).
		nullptr,					// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
		"PS_Main",					// シェーダエントリーポイント関数の名前.
		"ps_5_0",					// シェーダのモデルを指定する文字列(プロファイル).
		uCompileFlag,				// シェーダコンパイルフラグ.
		0,							// エフェクトコンパイルフラグ(未使用).
		nullptr,					// スレッド ポンプ インターフェイスへのポインタ(未使用).
		&pCompiledShader,			// ブロブを格納するメモリへのポインタ.
		&pErrors,					// エラーと警告一覧を格納するメモリへのポインタ.
		nullptr );					// 戻り値へのポインタ(未使用).
	if (FAILED( result ) )
	{
		ErrorMessage( "hlsl読み込み失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「ピクセルシェーダ」を作成.
	result = DirectX11::MutexDX11CreatePixelShader(
		pCompiledShader,
		nullptr,
		&m_pPixelShader );	// (out)ピクセルシェーダ.
	if (FAILED( result ) )
	{
		ErrorMessage( "ピクセルシェーダ作成失敗", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// コンスタント(定数)バッファ作成.
	//	シェーダに特定の数値を送るバッファ.
	//	ここでは変換行列渡し用.
	//	シェーダに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;			// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof(SFontShaderConstantBuffer);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;				// 書き込みでアクセス.
	cb.MiscFlags			= 0;									// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;									// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;					// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	result = m_pDevice->CreateBuffer(
		&cb,
		nullptr,
		&m_pConstantBuffer );
	if (FAILED( result ) )
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// モデル作成(UI).
//----------------------------.
HRESULT CFont::CreateModelUI()
{
	float w = m_FontState.Disp.w;
	float h = m_FontState.Disp.h;

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, h, 1.0f, 1.0f );
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

	// サブリソースデータ構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;	// 板ポリの頂点をセット.

	// 頂点バッファの作成.
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBufferUI );
	if ( FAILED( result ) )
	{
		ErrorMessage( "頂点バッファUI作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// モデル作成(3D).
//----------------------------.
HRESULT CFont::CreateModel3D()
{
	// 画像の比率を求める.
	int		as = myGcd( static_cast<int>( m_FontState.Disp.w ), static_cast<int>( m_FontState.Disp.h ) );
	float	w = ( m_FontState.Disp.w / as ) * m_FontState.Disp.w * 0.1f;
	float	h = ( m_FontState.Disp.h / as ) * m_FontState.Disp.h * 0.1f;

	// 3D用モデルの幅高さを保存する.
	m_Size3D = { w, h };

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, -h, 1.0f, 1.0f );
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
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer3D );
	if (FAILED( result ) )
	{
		ErrorMessage( "頂点バッファ3D作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// サンプラ作成.
//----------------------------.
HRESULT CFont::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;
	// 縮小時にミップマップを使用できるようにする( 0 のままだと常に最大解像度が使われてちらつく ).
	samDesc.MaxLOD		= D3D11_FLOAT32_MAX;

	// サンプラ作成.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		auto result = m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] );
		if( FAILED( result ) ){
			ErrorMessage( "サンプラ作成 : 失敗" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// 頂点情報の作成.
//---------------------------.
HRESULT CFont::CreateVertex( const float w, const float h, const float u, const float v )
{
	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	//	常に左上基準で作成し、中心座標は描画時の行列で反映する.
	m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
	m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点２(左上).
	m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点３(右下).
	m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点４(右上).
	return S_OK;
}
#endif // ENABLE_FONT
