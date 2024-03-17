#include "Sprite.h"
#include "..\DirectX\DirectX11.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\Utility\Input\Input.h"
#include "..\..\Utility\Input\InputList.h"
#include "..\..\Utility\Random\Random.h"
#include "..\..\Utility\Easing\Easing.h"
#include "..\..\Utility\FileManager\ImagSize\ImageSize.h"
#include "..\..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\..\Utility\ImGuiManager\MessageWindow\MessageWindow.h"
#include "..\..\Utility\Message\Message.h"
#include "..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

namespace {
	// シェーダファイル名(ディレクトリも含む).
	constexpr TCHAR SHADER_NAME[]			= _T( "Data\\Shader\\Sprite.hlsl"	);

	// アンドゥ用のログファイルの移動場所のパス.
	constexpr char	UNDO_LOG_FILE_PATH[]	= "Data\\UIEditor\\UndoLog\\";
	
	// ピクセルシェーダーのエントリポイント名.
	const std::vector<const char*> PS_SHADER_ENTRY_NAMES = {
		"PS_Main",
		"PS_Mask",
		"PS_Transition"
	};
}

CSprite::CSprite()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShaders			()
	, m_pConstantBuffer			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pSampleLinears			()
	, m_pTexture				( nullptr )
	, m_pMaskTexture			( nullptr )
	, m_pRuleTexture			( nullptr )
	, m_Vertices				()
	, m_SpriteState				()
	, m_SpriteRenderState		()
	, m_SpriteStateData			()
	, m_pLogList				( nullptr )
	, m_IsAnimPlay				( true )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_IsAllDisp				( true )
	, m_pIsCreaterLog			( nullptr )
{
}

CSprite::~CSprite()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pMaskTexture	);
	SAFE_RELEASE( m_pTexture		);
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	for ( auto itr = m_pPixelShaders.rbegin(), e = m_pPixelShaders.rend(); itr != e; ++itr ) {
		SAFE_RELEASE( *itr );
	}
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT CSprite::Init( const std::string& FilePath )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_SpriteState.FilePath = FilePath;
	m_pPixelShaders.resize( PS_SHADER_ENTRY_NAMES.size(), nullptr );

	// スプライト情報の取得.
	if( FAILED( SpriteStateDataLoad() ) ) return E_FAIL;

	// 初期化.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateTexture() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;

	Log::PushLog( m_SpriteState.FilePath + " 読み込み : 成功" );
	return S_OK;
}

//----------------------------.
// 指定したシーンでスプライトの情報を初期化する.
//----------------------------.
void CSprite::InitSceneSpriteState( const std::string& SceneName, const int& No )
{
	D3DXPOSITION3*	Pos			= &m_SpriteRenderState.Transform.Position;
	D3DXROTATION3*	Rot			= &m_SpriteRenderState.Transform.Rotation;
	D3DXSCALE3*		Scale		= &m_SpriteRenderState.Transform.Scale;
	D3DXCOLOR4*		Color		= &m_SpriteRenderState.Color;
	D3DXPOSITION3*	StartPos	= &m_SpriteRenderState.UIAnimState.StartTransform.Position;
	D3DXROTATION3*	StartRot	= &m_SpriteRenderState.UIAnimState.StartTransform.Rotation;
	D3DXSCALE3*		StartScale	= &m_SpriteRenderState.UIAnimState.StartTransform.Scale;
	D3DXCOLOR4*		StartColor	= &m_SpriteRenderState.UIAnimState.StartColor;

	// 指定した情報を保存.
	m_SpriteRenderState.SceneName	= SceneName;
	m_SpriteRenderState.No			= std::to_string( No );

	// トランスフォームの設定.
	json TransformData = m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["Transform"];
	Pos->x			= TransformData["Position"]["x"];
	Pos->y			= TransformData["Position"]["y"];
	Pos->z			= TransformData["Position"]["z"];
	Rot->x			= TransformData["Rotation"]["x"];
	Rot->y			= TransformData["Rotation"]["y"];
	Rot->z			= TransformData["Rotation"]["z"];
	Scale->x		= TransformData["Scale"]["x"];
	Scale->y		= TransformData["Scale"]["y"];
	Scale->z		= TransformData["Scale"]["z"];
	StartPos->x		= Pos->x;
	StartPos->y		= Pos->y;
	StartPos->z		= Pos->z;
	StartRot->x		= Rot->x;
	StartRot->y		= Rot->y;
	StartRot->z		= Rot->z;
	StartScale->x	= Scale->x;
	StartScale->y	= Scale->y;
	StartScale->z	= Scale->z;

	// 色の設定.
	json ColorData = m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["Color"];
	Color->x		= ColorData["R"];
	Color->y		= ColorData["G"];
	Color->z		= ColorData["B"];
	Color->w		= ColorData["A"];
	StartColor->x	= ColorData["R"];
	StartColor->y	= ColorData["G"];
	StartColor->z	= ColorData["B"];
	StartColor->w	= ColorData["A"];

	// UIアニメーションの設定.
	json			AnimData	= m_SpriteStateData[m_SpriteRenderState.SceneName][m_SpriteRenderState.No]["UIAnimation"];
	std::string*	UIAnimWhen	= &m_SpriteRenderState.UIAnimState.UIAnimWhen;
	std::string*	UIAnimWhat	= &m_SpriteRenderState.UIAnimState.UIAnimWhat;
	*UIAnimWhen	= AnimData["When"];
	*UIAnimWhat	= AnimData["What"];

	// ループさせるかの設定.
	if ( *UIAnimWhen != "None" && *UIAnimWhat != "None" ) {
		m_SpriteRenderState.UIAnimState.IsLoop		= AnimData["IsLoop"];
		m_SpriteRenderState.UIAnimState.PlayMax		= AnimData["PlayMax"];
		m_SpriteRenderState.UIAnimState.EndMessage	= AnimData["EndMessage"];
	}
	// 拡縮UIアニメーション.
	if ( *UIAnimWhat == "Scale" ) {
		// 拡縮の始める方向の設定.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Scale"]["Start"]["x"] == "Shrink";
		m_SpriteRenderState.UIAnimState.Flag_y = AnimData["Scale"]["Start"]["y"] == "Shrink";
	}
	// 透明/不透明UIアニメーション.
	else if ( *UIAnimWhat == "Alpha" ) {
		// 透明/不透明の始める方向の設定.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Alpha"]["Start"] == "Opacity";

		// 開始時の透明値を保存.
		m_SpriteRenderState.UIAnimState.StartColor.w = Color->w;
	}
	// 振り子UIアニメーション.
	else if ( *UIAnimWhat == "Pendulum" ) {
		// 初めに揺れる方向の設定.
		m_SpriteRenderState.UIAnimState.Flag_x = AnimData["Pendulum"]["Start"] == "Left";
	}
	// アニメーションUIアニメーション.
	else if ( *UIAnimWhat == "Animation" ) {
		// アニメーション速度の設定.
		m_SpriteRenderState.AnimState.FrameTime = AnimData["Anim"]["Time"];
	}
	// UVスクロールUIアニメーション.
	else if ( *UIAnimWhat == "Scroll" ) {
		// UVスクロール速度の設定.
		m_SpriteRenderState.AnimState.ScrollSpeed.x = AnimData["Scroll"]["Speed"]["x"];
		m_SpriteRenderState.AnimState.ScrollSpeed.y = AnimData["Scroll"]["Speed"]["y"];
	}
}

//----------------------------.
// UIでレンダリング.
//----------------------------.
void CSprite::RenderUI( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;

	// 非表示の場合は処理を行わない.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		UIAnimUpdate( RenderState );	// UIアニメーションの更新.
		AnimState->SetAnimNumber();		// アニメーション番号を設定.
		AnimState->SetPatternNo();		// パターン番号を設定.
		AnimState->AnimUpdate();		// アニメーションの更新.
		AnimState->UVScrollUpdate();	// スクロールの更新.
	}

	// 補正値用平行移動行列.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// 使用するシェーダの登録.
	const int PSShaderNo = m_pRuleTexture == nullptr ? m_pMaskTexture == nullptr ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo],  nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド行列を渡す(ビュー,プロジェクションは使わない).
		D3DXMATRIX m = mWorld;
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = RenderState->Color;

		// テクスチャ座標(UV座標).
		// 1マス当たりの割合にパターン番号(マス目)をかけて座標を設定する.
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// 描画するエリア.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea	 = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// ビューポートの幅、高さを渡す.
		cb.fViewPortWidth	= WndW;
		cb.fViewPortHeight	= WndH;

		// ディザ抜きを使用するか.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

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
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBufferUI, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &m_pMaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &m_pRuleTexture );

	// アルファブレンド有効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// 3Dでレンダリング.
//----------------------------.
void CSprite::Render3D( SSpriteRenderState* pRenderState, const bool IsBillBoard )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
#ifdef _DEBUG
	PositionRenderer::Render( RenderState->Transform );
#endif
	// 非表示の場合は処理を行わない.
	if ( m_IsAllDisp			== false	) return;
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	if ( m_IsAnimPlay ) {
		UIAnimUpdate( RenderState );	// UIアニメーションの更新.
		AnimState->SetAnimNumber();		// アニメーション番号を設定.
		AnimState->SetPatternNo();		// パターン番号を設定.
		AnimState->AnimUpdate();		// アニメーションの更新.
		AnimState->UVScrollUpdate();	// スクロールの更新.
	}

	// 補正値用平行移動行列.
	D3DXMATRIX mOffSet;
	D3DXMatrixTranslation( &mOffSet, m_SpriteState.OffSet.x, m_SpriteState.OffSet.y, 0.0f );

	// ワールド行列を取得.
	D3DXMATRIX mWorld = RenderState->Transform.GetWorldMatrix() * mOffSet;

	// ビルボード用.
	if ( IsBillBoard == true ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// ビュー行列.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;			// xyzを0にする.
		// CancelRotationの逆行列を求めます.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// 使用するシェーダの登録.
	const int PSShaderNo = m_pRuleTexture == nullptr ? m_pMaskTexture == nullptr ? 0 : 1 : 2;
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShaders[PSShaderNo], nullptr, 0 );

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE		pData;
	SSpriteShaderConstantBuffer		cb;	// コンスタントバッファ.
	// バッファ内のデータの書き換え開始時にmap.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ワールド,ビュー,プロジェクション行列を渡す.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// 行列を転置する.
		cb.mWVP = m;

		// カラー.
		cb.vColor = RenderState->Color;

		// テクスチャ座標(UV座標).
		cb.vUV.x = AnimState->UV.x + AnimState->Scroll.x / 2;
		cb.vUV.y = AnimState->UV.y + AnimState->Scroll.y / 2;

		// 描画するエリア.
		const float WndW = DirectX11::GetWndWidth();
		const float WndH = DirectX11::GetWndHeight();
		cb.vRenderArea = RenderState->RenderArea;
		if ( cb.vRenderArea.z <= -1.0f ) cb.vRenderArea.z = WndW;
		if ( cb.vRenderArea.w <= -1.0f ) cb.vRenderArea.w = WndH;

		// ビューポートの幅、高さは使用しないため初期化.
		cb.fViewPortWidth	= 0.0f;
		cb.fViewPortHeight	= 0.0f;

		// ディザ抜きを使用するか.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// アルファブロックを使用するかを渡す.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

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
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBuffer3D, &stride, &offset );

	// 頂点インプットレイアウトをセット.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// プリミティブ・トポロジーをセット.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// テクスチャをシェーダに渡す.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( RenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &m_pTexture );
	m_pContext->PSSetShaderResources( 1, 1, &m_pMaskTexture );
	m_pContext->PSSetShaderResources( 2, 1, &m_pRuleTexture );

	// アルファブレンド有効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// プリミティブをレンダリング.
	m_pContext->Draw( 4, 0 );// 板ポリ(頂点4つ分).

	// アルファブレンド無効にする.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//----------------------------.
// UIエディタのImGuiの描画.
//----------------------------.
void CSprite::UIEdtorImGuiRender( const bool IsAutoSave, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState		= &RenderState->UIAnimState;
	std::string*		UIAnimWhen		= &UIAnimState->UIAnimWhen;
	std::string*		UIAnimWhat		= &UIAnimState->UIAnimWhat;
	const json			OldStateData	= m_SpriteStateData;

	// トランスフォームの設定.
	STransform*			pTrance		= &UIAnimState->StartTransform;
	D3DXROTATION3		pDegRot		= Math::ToDegree( pTrance->Rotation );
	const STransform	OldTrance	= *pTrance;
	ImGuiManager::InputVec3( "Position",	&pTrance->Position );
	ImGuiManager::InputVec3( "Rotation",	&pDegRot );
	ImGuiManager::InputVec3( "Scale",		&pTrance->Scale, { 1.0f, 1.0f, 1.0f } );
	pTrance->Rotation = Math::ToRadian( pDegRot );
	if ( *pTrance != OldTrance ) {
		RenderState->Transform = *pTrance;

		// 保存して再読み込み.
		if ( IsAutoSave ) {
			if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
		}
	}

	// 色の設定.
	D3DXCOLOR4*			pColor		= &UIAnimState->StartColor;
	const D3DXCOLOR4	OldColor	= *pColor;
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "Color" ) ) {
		ImGui::ColorEdit4( "##Color", *pColor );
		ImGui::TreePop();
	}
	if ( *pColor != OldColor ) {
		RenderState->Color = *pColor;

		// 保存して再読み込み.
		if ( IsAutoSave ) {
			if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
		}
	}

	// いつ行うかリスト.
	const std::vector<std::string> WhenList = {
		u8"無し",
		u8"常時",
		u8"マウスカーソルが重なった時",
		u8"クリックされた時",
		u8"特定のキーが押されている時",
		u8"メッセージを受け取った時",
		u8"ランダム( 確率 )"
	};
	// 何を行うかリスト.
	const std::vector<std::string> WhatList = {
		u8"無し",
		u8"拡縮",
		u8"透明 / 不透明の点滅",
		u8"回転",
		u8"移動",
		u8"振り子",
		u8"アニメーション",
		u8"UVスクロール",
		u8"メッセージ"
	};

	// 一コマの大きさの設定の表示.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "Stride" ) ) {
		const D3DXVECTOR2 OldStride = { m_SpriteState.Stride.w, m_SpriteState.Stride.h };
		D3DXVECTOR2 NewStride = OldStride;
		ImGui::InputFloat2( "##In2_Stride", NewStride );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"保存後、開きなおすことで反映されます。\n※変更すると、表示サイズは一コマのサイズに変更されます。" );

		// 初期化ボタン.
		if ( ImGuiManager::Button( "Reset" ) ) NewStride = { m_SpriteState.Base.w, m_SpriteState.Base.h };
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"初期状態に戻します" );

		// 一コマの大きさを更新.
		if ( OldStride != NewStride ) {
			m_SpriteState.Stride				= { NewStride.x, NewStride.y };
			m_SpriteState.Disp					= { NewStride.x, NewStride.y };
			m_SpriteStateData["Stride"]["w"]	= NewStride.x;
			m_SpriteStateData["Stride"]["h"]	= NewStride.y;
			m_SpriteStateData["Disp"]["w"]		= NewStride.x;
			m_SpriteStateData["Disp"]["h"]		= NewStride.y;
		}
		ImGui::TreePop();
	}

	// ローカル座標の設定の表示.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "LocalPosition" ) ) {
		// ローカル座標のコンボボックスの作成.
		ELocalPosition SpriteLocalPos		= static_cast<ELocalPosition>( m_SpriteStateData["LocalPosition"] );
		ImGuiManager::Combo( "##NoName", &SpriteLocalPos, { ELocalPosition::Max } );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"保存後、開きなおすことで反映されます。" );

		// ローカル座標の更新.
		m_SpriteStateData["LocalPosition"]	= static_cast<int>( SpriteLocalPos );
		ImGui::TreePop();
	}
	ImGui::Separator();

	// UIアニメーションの設定ツリーを作成.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( "UIAnimation" ) ) {
		// 現在のいつ行うかを日本語に変換してコンボボックスの作成.
		std::string	NowWhen = ConvertUIAnimationString( *UIAnimWhen );
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( u8"いつ行う？" ) ) {
			ImGuiManager::Combo( "##NoName", &NowWhen, WhenList );
			ImGui::TreePop();
		}
		// 前の状態と今の状態を見て変更されているか.
		const std::string OldWhen = *UIAnimWhen;
		*UIAnimWhen = ConvertUIAnimationString( NowWhen );
		m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"]["When"] = *UIAnimWhen;
		// 仮データが必要な場合は作成する.
		if ( OldWhen != *UIAnimWhen ) CreateWhenTempData( RenderState );

		// いつ行うかの詳細設定.
		WhenAdvancedSetting( RenderState );
		ImGui::Separator();
		
		// 現在の何を行うかを日本語に変換してコンボボックスの作成.
		std::string NowWhat = ConvertUIAnimationString( *UIAnimWhat );
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( u8"何を行う？" ) ) {
			ImGuiManager::Combo( "##NoName", &NowWhat, WhatList );
			ImGui::TreePop();
		}

		// 前の状態と今の状態を見て変更されているか.
		const std::string OldWhat = *UIAnimWhat;
		*UIAnimWhat = ConvertUIAnimationString( NowWhat );
		m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"]["What"] = *UIAnimWhat;
		// 仮データが必要な場合は作成する.
		if ( OldWhat != *UIAnimWhat ) CreateWhatTempData( RenderState );

		// 何を行うかの詳細設定.
		WhatAdvancedSetting( RenderState );
		ImGui::Separator();
		ImGui::TreePop();
	}
	if ( ImGuiManager::Button( "Save" ) ) {
		SpriteStateDataSave( RenderState );
		MessageWindow::PushMessage( u8"保存しました。", Color4::Magenta );
	}
	ImGui::Separator();

	// スプライトの情報が変更されている場合、自動で保存をかける.
	if ( IsAutoSave		== false				) return;
	if ( OldStateData	!= m_SpriteStateData	) {
		// 保存して再読み込み.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return;
	}
}

//----------------------------.
// マウスのつかみ判定の描画.
//----------------------------.
void CSprite::HitBoxRender( const D3DXVECTOR4& Color, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	if ( m_IsAllDisp			== false ) return;
	if ( RenderState->IsDisp	== false ) return;
	if ( RenderState->Color.w	== 0.0f	 ) return;

	// 当たり判定の描画.
	SSpriteRenderState HitBoxState;
	HitBoxState.Transform.Position	= GetSpriteUpperLeftPos( RenderState );
	HitBoxState.Transform.Scale.x	= m_SpriteState.Disp.w * RenderState->Transform.Scale.x;
	HitBoxState.Transform.Scale.y	= m_SpriteState.Disp.h * RenderState->Transform.Scale.y;
	HitBoxState.Color				= Color;
	SpriteResource::GetSprite( "HitBox" )->RenderUI( &HitBoxState );
}

//----------------------------.
// マウスのドラッグの更新.
//----------------------------.
bool CSprite::DragUpdate( const bool IsDrag, const bool IsAutoSave, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	if ( IsDrag					== false ) return false;
	if ( m_IsAllDisp			== false ) return false;
	if ( RenderState->IsDisp	== false ) return false;
	if ( RenderState->Color.w	== 0.0f	 ) return false;

	const D3DXPOSITION2& MousePos		= Input::GetMousePosition();
	const D3DXPOSITION2& MouseOldPos	= Input::GetMouseOldPosition();

	// つかまれているか.
	if ( RenderState->IsGrab ) {
		// マウスの移動した分スプライトを動かす.
		const D3DXVECTOR2& MoveValue = MousePos - MouseOldPos;
		RenderState->Transform.Position.x += MoveValue.x;
		RenderState->Transform.Position.y += MoveValue.y;

		// 左クリックが離されているか.
		if ( !KeyInput::IsKeyPress( VK_LBUTTON ) ) {
			// 離した.
			Input::SetIsMousrGrab( false );
			RenderState->IsGrab = false;

			// 座標の更新.
			RenderState->UIAnimState.StartTransform.Position = RenderState->Transform.Position;

			// 保存して再読み込み.
			if ( IsAutoSave ) {
				if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return false;
			}
		}
		return true;
	}
	// つかまれていない.
	else {
		// マウスが別のをつかんでいる.
		if ( Input::IsMousrGrab() ) return false;
		static bool f = false;
		// マウスカーソルに重なっているか.
		if ( GetIsOverlappingTheMouse( pRenderState ) == false ) return false;

		// 左クリックが押された瞬間か.
		if ( !Input::GetIsLeftClickDown() ) return false;

		// つかんだ.
		Input::SetIsMousrGrab( true );
		RenderState->IsGrab = true;
		return true;
	}
}

//----------------------------.
// マウスに重なっているか取得する.
//----------------------------.
bool CSprite::GetIsOverlappingTheMouse( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION2&	MousePos		= Input::GetMousePosition();
	const D3DXPOSITION3&	UpperLeftPos	= GetSpriteUpperLeftPos( RenderState );
	const D3DXSCALE3&		Scale			= RenderState->Transform.Scale;

	return	( ( UpperLeftPos.x <= MousePos.x ) && ( MousePos.x <= UpperLeftPos.x + m_SpriteState.Disp.w * Scale.x ) ) &&
			( ( UpperLeftPos.y <= MousePos.y ) && ( MousePos.y <= UpperLeftPos.y + m_SpriteState.Disp.h * Scale.y ) );
}

//---------------------------.
// スプライト情報を保存する.
//---------------------------.
HRESULT CSprite::SpriteStateDataSave( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;

	// 不要なデータを削除する.
	json AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	if ( UIAnimWhen != "" && UIAnimWhen != "None" ) {
		if ( UIAnimWhen != "Key"		) AnimData.erase( "Key"			);
		if ( UIAnimWhen != "Click"		) AnimData.erase( "Click"		);
		if ( UIAnimWhen != "GetMessage"	) AnimData.erase( "GetMessage"	);
		if ( UIAnimWhen != "Random"		) AnimData.erase( "Random"		);
	}
	if ( UIAnimWhat != "" && UIAnimWhat != "None" ) {
		if ( UIAnimWhat != "Scale"		) AnimData.erase( "Scale"		);
		if ( UIAnimWhat != "Alpha"		) AnimData.erase( "Alpha"		);
		if ( UIAnimWhat != "Rotate"		) AnimData.erase( "Rotate"		);
		if ( UIAnimWhat != "Move"		) AnimData.erase( "Move"		);
		if ( UIAnimWhat != "Pendulum"	) AnimData.erase( "Pendulum"	);
		if ( UIAnimWhat != "Animation"	) AnimData.erase( "Animation"	);
		if ( UIAnimWhat != "Scroll"		) AnimData.erase( "Scroll"		);
		if ( UIAnimWhat != "Message"	) AnimData.erase( "Message"		);
	}
	if ( UIAnimWhen != "" && UIAnimWhen != "None" && UIAnimWhat != "" && UIAnimWhat != "None" ) {
		// ループ再生するかの上書き.
		AnimData["IsLoop"]		= UIAnimState->IsLoop;
		AnimData["PlayMax"]		= UIAnimState->PlayMax;
		AnimData["EndMessage"]	= UIAnimState->EndMessage;
	}
	else if ( AnimData.empty() == false ) {
		// 不要なデータの削除.
		AnimData.erase( "IsLoop" );
		AnimData.erase( "PlayMax" );
		AnimData.erase( "EndMessage" );
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;

	// トランスフォームの上書き
	json TransformData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["Transform"];
	TransformData["Position"]["x"]	= UIAnimState->StartTransform.Position.x;
	TransformData["Position"]["y"]	= UIAnimState->StartTransform.Position.y;
	TransformData["Position"]["z"]	= UIAnimState->StartTransform.Position.z;
	TransformData["Rotation"]["x"]	= UIAnimState->StartTransform.Rotation.x;
	TransformData["Rotation"]["y"]	= UIAnimState->StartTransform.Rotation.y;
	TransformData["Rotation"]["z"]	= UIAnimState->StartTransform.Rotation.z;
	TransformData["Scale"]["x"]		= UIAnimState->StartTransform.Scale.x;
	TransformData["Scale"]["y"]		= UIAnimState->StartTransform.Scale.y;
	TransformData["Scale"]["z"]		= UIAnimState->StartTransform.Scale.z;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["Transform"] = TransformData;
	// 色の上書き.
	json ColorData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["Color"];
	ColorData["R"] = UIAnimState->StartColor.x;
	ColorData["G"] = UIAnimState->StartColor.y;
	ColorData["B"] = UIAnimState->StartColor.z;
	ColorData["A"] = UIAnimState->StartColor.w;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["Color"] = ColorData;

	// つかめなくするかの上書き.
	m_SpriteStateData["IsLock"] = RenderState->IsLock;

	// 保存して再読み込み.
	SpriteStateSaveReload();
	return S_OK;
}

//---------------------------.
// スプライト情報の取得.
//---------------------------.
HRESULT CSprite::SpriteStateDataLoad()
{
	// 補正値テキストの読み込み.
	std::string TextPath	= m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "\\" ), TextPath.size() ) += "\\OffSet.json";
	json OffSetData			= FileManager::JsonLoad( TextPath );
	
	// 同じ名前のテキストの読み込み.
	TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.rfind( "." ), TextPath.size() ) += ".json";
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
	m_SpriteStateData = FileManager::JsonLoad( TextPath );

	// ファイルが無いためファイルを作成する.
	if ( m_SpriteStateData.empty() ) {
#ifdef _DEBUG
		Log::PushLog( TextPath + " が無いため作成します。" );
		if ( FAILED( CreateSpriteState() ) ) return E_FAIL;

		// 作成できたためもう一度読み込み直す.
		Log::PushLog( m_SpriteState.FilePath + " をもう一度読み込み直します。" );
		SpriteStateDataLoad();
		return S_OK;
#else
		return E_FAIL;
#endif
	}

	// スプライト情報の取得.
	m_SpriteState.LocalPosNo	= static_cast<ELocalPosition>( m_SpriteStateData["LocalPosition"] );
	m_SpriteState.Disp.w		= m_SpriteStateData["Disp"]["w"];
	m_SpriteState.Disp.h		= m_SpriteStateData["Disp"]["h"];
	m_SpriteState.Base.w		= m_SpriteStateData["Base"]["w"];
	m_SpriteState.Base.h		= m_SpriteStateData["Base"]["h"];
	m_SpriteState.Stride.w		= m_SpriteStateData["Stride"]["w"];
	m_SpriteState.Stride.h		= m_SpriteStateData["Stride"]["h"];
	m_SpriteState.AddCenterPos	= GetAddCenterPosition();

	// つかめなくするかの取得.
	m_SpriteRenderState.IsLock	= m_SpriteStateData["IsLock"];

	// 表示するシーンの読み込み.
	const int& SceneSize = static_cast<int>( m_SpriteStateData["Scene"].size() );
	m_SpriteState.DispSceneList.resize( SceneSize );
	for ( int i = 0; i < SceneSize; ++i ) {
		const std::string& SceneName	= m_SpriteStateData["Scene"][i];
		m_SpriteState.DispSceneList[i]	= SceneName;

		// 表示数の取得.
		if ( SceneName == "None" )	m_SpriteState.SceneDispNum[SceneName] = 0;
		else						m_SpriteState.SceneDispNum[SceneName] = m_SpriteStateData[SceneName]["Num"];
	}

	// 補正値の設定.
	if ( OffSetData.empty() ) m_SpriteState.OffSet = { 0.0f, 0.0f };
	else {
		m_SpriteState.OffSet.x	= OffSetData["x"];
		m_SpriteState.OffSet.y	= OffSetData["y"];
	}

#if _DEBUG
	// ファイルパスを更新する.
	m_SpriteStateData["FilePath"]	= TextPath;
	if ( FAILED( FileManager::JsonSave( TextPath, m_SpriteStateData ) ) ) return E_FAIL;
#endif	// #if _DEBUG.
	return S_OK;
}

//----------------------------.
// スプライト情報の保存して再読み込み.
//----------------------------.
HRESULT CSprite::SpriteStateSaveReload()
{
	// 現在の時間を取得.
	auto now	= std::chrono::system_clock::now();
	auto now_c	= std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

	// ファイル名を取得.
	std::string FileName = "";
	std::string::size_type Spos = m_SpriteState.FilePath.rfind( "\\" );
	if ( Spos != std::string::npos ) {
		std::string::size_type Epos = m_SpriteState.FilePath.find( ".", Spos + 1 );
		if ( Epos != std::string::npos ) {
			FileName = m_SpriteState.FilePath.substr( Spos + 1, Epos - Spos - 1 );
		}
	}

	// ファイルパスの作成.
	std::string TextPath	= m_SpriteState.FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

	// ログファイルの作成.
	if ( *m_pIsCreaterLog ) {
		// ファイルパスの作成.
		std::string LogTextPath = UNDO_LOG_FILE_PATH + Time.str() + "_" + FileName + ".json";

		// ファイルの作成.
		json j = FileManager::JsonLoad( TextPath );
		if ( j != m_SpriteStateData ) {
			FileManager::CreateFileDirectory( UNDO_LOG_FILE_PATH );
			if ( std::rename( TextPath.c_str(), LogTextPath.c_str() ) ) return E_FAIL;
			m_pLogList->emplace_back( LogTextPath );
			Log::PushLog( TextPath + " ファイルのログの作成 : 成功" );
		}
	}

	// スプライト情報の保存.
	if ( FAILED( FileManager::JsonSave( TextPath, m_SpriteStateData ) ) ) return E_FAIL;
	Log::PushLog( TextPath + " ファイルの作成 : 成功" );

	// 作成できたためもう一度読み込み直す.
	Log::PushLog( m_SpriteState.FilePath + " をもう一度読み込み直します。" );
	if ( FAILED( SpriteStateDataLoad() ) ) return E_FAIL;
	return S_OK;
}

//----------------------------.
// 表示するスプライトの削除.
//----------------------------.
HRESULT CSprite::DispSpriteDeleteScene( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	// 表示するシーンから削除する.
	std::vector<std::string> SceneList = m_SpriteStateData["Scene"];
	m_SpriteStateData["Scene"].clear();
	for ( auto& s : SceneList ) {
		if ( s != DispScene ) m_SpriteStateData["Scene"].emplace_back( s );
	}
	m_SpriteStateData.erase( DispScene );

	// 変更後を保存する.
	if ( IsAutoSave ) {
		// 保存して再読み込み.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// 表示しているスプライトの数を減らす.
//----------------------------.
HRESULT CSprite::DispSpriteDeleteNum( const bool IsAutoSave, const std::string& DispScene, const int No, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	// 現在の表示している数を取得.
	const int NowNum = m_SpriteStateData[DispScene]["Num"];
	if ( NowNum <= No ) return E_FAIL;

	// 表示している数が1個のためシーンごと削除する.
	if ( NowNum == 1 ) return DispSpriteDeleteScene( IsAutoSave, DispScene, RenderState );

	// 最後の要素では無い場合.
	if ( NowNum - 1 != No ) {
		for ( int i = No + 1; i < NowNum; ++i ) {
			// 削除する所から上にずらしていく.
			m_SpriteStateData[DispScene][std::to_string( i - 1 )] = m_SpriteStateData[DispScene][std::to_string( i )];
		}
	}

	// 表示する個数を減らす.
	m_SpriteStateData[DispScene]["Num"] = NowNum - 1;
	m_SpriteStateData[DispScene].erase( std::to_string( NowNum - 1 ) );

	// 変更後を保存する.
	if ( IsAutoSave ) {
		// 保存して再読み込み.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// 表示するスプライトの追加.
//----------------------------.
HRESULT CSprite::AddDispSprite( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;

	bool IsNewScene = true;
	for ( auto& s : m_SpriteState.DispSceneList ) {
		// 追加するシーンにすでに描画しているか.
		if ( s == DispScene ) {
			IsNewScene = false;
			break;
		}
	}
	
	// 新しく表示するシーンを追加する.
	if ( IsNewScene ) {
		// 表示するシーンの追加.
		m_SpriteStateData["Scene"].emplace_back( DispScene );
		const std::vector<std::string> SceneList = m_SpriteStateData["Scene"];
		m_SpriteStateData["Scene"].clear();
		m_SpriteState.DispSceneList.clear();
		for ( auto& s : SceneList ) {
			if ( s != "None" ) {
				m_SpriteStateData["Scene"].emplace_back( s );
				m_SpriteState.DispSceneList.emplace_back( s );
			}
		}
		m_SpriteStateData[DispScene]["Num"] = 1;

		// 仮データを作成.
		NewDispSpriteTmpData( RenderState, DispScene, 0 );
	}
	// 表示する個数を増やす.
	else {
		// 現在の表示している数を取得.
		const int NowNum = m_SpriteStateData[DispScene]["Num"];

		// 表示する個数を増やす.
		m_SpriteStateData[DispScene]["Num"] = NowNum + 1;

		// 仮データを作成.
		NewDispSpriteTmpData( RenderState, DispScene, NowNum );
	}

	// 変更後を保存する.
	if ( IsAutoSave ) {
		// 保存して再読み込み.
		if ( FAILED( SpriteStateDataSave( RenderState ) ) ) return E_FAIL;
	}
	return S_OK;
}

//---------------------------.
// スプライトの左上の座標を取得する.
//---------------------------.
D3DXVECTOR3 CSprite::GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const D3DXPOSITION3&	Pos			= RenderState->Transform.Position;
	const SSize&			Size		= m_SpriteState.Disp;
	const D3DXSCALE3		Scale		= RenderState->Transform.Scale;

	// 左上の座標を計算して返す.
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::Left:		return D3DXPOSITION3( Pos.x,							Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::LeftDown:	return D3DXPOSITION3( Pos.x,							Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Down:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::RightDown:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - Size.h * Scale.y,			Pos.z );
	case ELocalPosition::Right:		return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	case ELocalPosition::RightUp:	return D3DXPOSITION3( Pos.x - Size.w * Scale.x,			Pos.y,								Pos.z );
	case ELocalPosition::Up:		return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y,								Pos.z );
	case ELocalPosition::Center:	return D3DXPOSITION3( Pos.x - ( Size.w / 2 ) * Scale.x,	Pos.y - ( Size.h / 2 ) * Scale.y,	Pos.z );
	default:						return Pos;
	}
}

//----------------------------.
// シェーダ作成.
//----------------------------.
HRESULT CSprite::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSLからバーテックスシェーダのブロブを作成.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
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
			nullptr)))			// 戻り値へのポインタ(未使用).
	{
		ErrorMessage( "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// 上記で作成したブロブから「バーテックスシェーダ」を作成.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&m_pVertexShader)))	// (out)バーテックスシェーダ.
	{
		ErrorMessage( "バーテックスシェーダ作成失敗" );
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
	if (FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout,
			numElements,
			pCompiledShader,
			&m_pVertexLayout)))	//(out)頂点インプットレイアウト.
	{
		ErrorMessage( "頂点インプットレイアウト作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	const int Size = static_cast<int>( m_pPixelShaders.size() );
	for( int i = 0; i < Size; ++i ){
		// HLSLからピクセルシェーダのブロブを作成.
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME,				// シェーダファイル名(HLSLファイル).
				nullptr,					// マクロ定義の配列へのポインタ(未使用).
				nullptr,					// インクルードファイルを扱うインターフェイスへのポインタ(未使用).
				PS_SHADER_ENTRY_NAMES[i],	// シェーダエントリーポイント関数の名前.
				"ps_5_0",					// シェーダのモデルを指定する文字列(プロファイル).
				uCompileFlag,				// シェーダコンパイルフラグ.
				0,							// エフェクトコンパイルフラグ(未使用).
				nullptr,					// スレッド ポンプ インターフェイスへのポインタ(未使用).
				&pCompiledShader,			// ブロブを格納するメモリへのポインタ.
				&pErrors,					// エラーと警告一覧を格納するメモリへのポインタ.
				nullptr)))					// 戻り値へのポインタ(未使用).
		{
			ErrorMessage( "hlsl読み込み失敗" );
			return E_FAIL;
		}
		SAFE_RELEASE(pErrors);

		// 上記で作成したブロブから「ピクセルシェーダ」を作成.
		if (FAILED(
			DirectX11::MutexDX11CreatePixelShader(
				pCompiledShader,
				nullptr,
				&m_pPixelShaders[i] )))	// (out)ピクセルシェーダ.
		{
			ErrorMessage( "ピクセルシェーダ「" + std::string( PS_SHADER_ENTRY_NAMES[i] ) + "」作成失敗" );
			return E_FAIL;
		}
		SAFE_RELEASE(pCompiledShader);
	}

	// コンスタント(定数)バッファ作成.
	//	シェーダに特定の数値を送るバッファ.
	//	ここでは変換行列渡し用.
	//	シェーダに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;				// コンスタントバッファを指定.
	cb.ByteWidth			= sizeof( SSpriteShaderConstantBuffer );	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;					// 書き込みでアクセス.
	cb.MiscFlags			= 0;										// その他のフラグ(未使用).
	cb.StructureByteStride	= 0;										// 構造体のサイズ(未使用).
	cb.Usage				= D3D11_USAGE_DYNAMIC;						// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		m_pDevice->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		ErrorMessage( "コンスタントバッファ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// モデル作成(UI).
//----------------------------.
HRESULT CSprite::CreateModelUI()
{
	SSpriteAnimState* AnimState = &m_SpriteRenderState.AnimState;

	float w = m_SpriteState.Disp.w;
	float h = m_SpriteState.Disp.h;
	float u = m_SpriteState.Stride.w / m_SpriteState.Base.w;// 1マス当たりの幅.
	float v = m_SpriteState.Stride.h / m_SpriteState.Base.h;// 1マス当たりの高さ.

	// x,yそれぞれの最大マス数.
	AnimState->PatternMax.x		= static_cast<SHORT>( m_SpriteState.Base.w / m_SpriteState.Stride.w );
	AnimState->PatternMax.y		= static_cast<SHORT>( m_SpriteState.Base.h / m_SpriteState.Stride.h );
	AnimState->AnimNumberMax	= AnimState->PatternMax.x * AnimState->PatternMax.y;

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, h, u, v );
	// 最大要素数を算出する.
	UINT uVerMax = sizeof(m_Vertices) / sizeof(m_Vertices[0]);

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
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBufferUI)))
	{
		ErrorMessage( "頂点バッファUI作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// モデル作成(3D).
//----------------------------.
HRESULT CSprite::CreateModel3D()
{
	// 画像の比率を求める.
	int		as = myGcd( static_cast<int>( m_SpriteState.Disp.w ), static_cast<int>( m_SpriteState.Disp.h ) );
	float	w = ( m_SpriteState.Disp.w / as ) * m_SpriteState.Disp.w * 0.1f;
	float	h = ( m_SpriteState.Disp.h / as ) * m_SpriteState.Disp.h * 0.1f;
	float	u = m_SpriteState.Stride.w / m_SpriteState.Base.w;	// 1コマ当たりの幅,
	float	v = m_SpriteState.Stride.h / m_SpriteState.Base.h;	// 1コマ当たりの高さ.

	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	CreateVertex( w, -h, u, v );
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
	if (FAILED( m_pDevice->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer3D)))
	{
		ErrorMessage( "頂点バッファ3D作成失敗" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// テクスチャ作成.
//----------------------------.
HRESULT CSprite::CreateTexture()
{
	// ファイルパスを w_char に変換する.
	const std::wstring wFilePath = StringConversion::to_wString( m_SpriteState.FilePath );

	//テクスチャ作成.
	if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
		wFilePath.c_str(),	// ファイル名.
		nullptr, nullptr,
		&m_pTexture,		// (out)テクスチャ.
		nullptr ) ) )
	{
		ErrorMessage( "テクスチャ作成失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// サンプラ作成.
//----------------------------.
HRESULT CSprite::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;

	// サンプラ作成.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		if( FAILED( m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] ))){
			ErrorMessage( "サンプラ作成 : 失敗" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// 頂点情報の作成.
//---------------------------.
HRESULT CSprite::CreateVertex( const float w, const float h, const float u, const float v )
{
	// 板ポリ(四角形)の頂点を作成.
	//	頂点座標(x,y,z), UV座標(u,v).
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Left:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::LeftDown:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(    w,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Down:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightDown:
		m_Vertices[0] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Right:
		m_Vertices[0] = { D3DXPOSITION3(   -w,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::RightUp:
		m_Vertices[0] = { D3DXPOSITION3(   -w,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Up:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	case ELocalPosition::Center:
	default: 
		m_Vertices[0] = { D3DXVECTOR3( -w / 2,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// 頂点１(左下).
		m_Vertices[1] = { D3DXVECTOR3( -w / 2, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// 頂点１(左下).
		m_Vertices[2] = { D3DXVECTOR3(  w / 2,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// 頂点１(左下).
		m_Vertices[3] = { D3DXVECTOR3(  w / 2, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// 頂点１(左下).
		break;
	}
	return S_OK;
}

//---------------------------.
// スプライト情報の作成.
//---------------------------.
HRESULT CSprite::CreateSpriteState()
{
	// 作成するファイルパス.
	std::string TextPath = m_SpriteState.FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

	// 画像の幅、高さの取得.
	const D3DXVECTOR2& BaseSize = ImageSize::GetImageSize( m_SpriteState.FilePath );

	// 情報を追加していく.
	json SpriteState;
	SpriteState["LocalPosition"]		= 0;
	SpriteState["Disp"]["w"]			= BaseSize.x;
	SpriteState["Disp"]["h"]			= BaseSize.y;
	SpriteState["Base"]["w"]			= BaseSize.x;
	SpriteState["Base"]["h"]			= BaseSize.y;
	SpriteState["Stride"]["w"]			= BaseSize.x;
	SpriteState["Stride"]["h"]			= BaseSize.y;
	SpriteState["Scene"]				= { "None" };
	SpriteState["IsLock"]				= false;

	// スプライト情報の作成.
	if( FAILED( FileManager::JsonSave( TextPath, SpriteState ) ) ) return E_FAIL;
	Log::PushLog( TextPath + " ファイルの作成 : 成功" );

	// 作成成功.
	return S_OK;
}

//---------------------------.
// 中心座標に変換するための追加座標の取得.
//---------------------------.
D3DXPOSITION3 CSprite::GetAddCenterPosition()
{
	switch ( m_SpriteState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Left:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::LeftDown:
		return D3DXPOSITION3(  m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Down:
		return D3DXPOSITION3(  0.0f,						-m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::RightDown:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f, -m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Right:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  0.0f,							0.0f );
	case ELocalPosition::RightUp:
		return D3DXPOSITION3( -m_SpriteState.Disp.w / 2.0f,  m_SpriteState.Disp.h / 2.0f,	0.0f );
	case ELocalPosition::Up:
		return D3DXPOSITION3(  0.0f,						 m_SpriteState.Disp.h / 2.0f,	0.0f );
	default:
		return INIT_FLOAT3;
	}
}

//---------------------------.
// UIアニメーションの更新.
//	UIアニメーションを行わない場合は処理は行わない.
//---------------------------.
void CSprite::UIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;

	// UIアニメーションを行うか調べる.
	if ( UIAnimWhen							== "None"	) return;
	if ( UIAnimPlayCheck( pRenderState )	== false	) return;

	// UIアニメーションの初期化.
	UIAnimInit( pRenderState );

	// UIのアニメーションを行う.
	if (	  UIAnimWhat == "Scale"		) ScaleUIAnimUpdate(	 pRenderState );	// 拡縮.
	else if ( UIAnimWhat == "Alpha"		) AlphaUIAnimUpdate(	 pRenderState );	// 透明/不透明の点滅.
	else if ( UIAnimWhat == "Rotate"	) RotateUIAnimUpdate(	 pRenderState );	// 回転.
	else if ( UIAnimWhat == "Move"		) MoveUIAnimUpdate(		 pRenderState );	// 移動.
	else if ( UIAnimWhat == "Pendulum"	) PendulumUIAnimUpdate(	 pRenderState );	// 振り子.
	else if ( UIAnimWhat == "Animation"	) AnimationUIAnimUpdate( pRenderState );	// アニメーション.
	else if ( UIAnimWhat == "Scroll"	) ScrollUIAnimUpdate(	 pRenderState );	// UVスクロール.
	else if ( UIAnimWhat == "Message"	) MessageUIAnimUpdate(	 pRenderState );	// メッセージ.
}

//----------------------------.
// UIアニメーションの初期化.
//----------------------------.
void CSprite::UIAnimInit( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
	std::string*		UIAnimWhat	= &RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// アニメーション.
	if ( *UIAnimWhat == "Animation" ) {
		AnimState->IsAnimation = false;
	}
	// UVスクロール.
	else if ( *UIAnimWhat == "Scroll" ) {
		if ( AnimData["Scroll"]["IsPlay"]["x"] ) {
			AnimState->IsUVScrollX = false;
		}
		else if ( AnimData["Scroll"]["IsPlay"]["y"] ) {
			AnimState->IsUVScrollY = false;
		}
	}
}

//----------------------------.
// UIアニメーションを行うか調べる.
//----------------------------.
bool CSprite::UIAnimPlayCheck( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;

	// UIとして表示するシーンが設定されていないため行わない.
	if ( RenderState->SceneName == "" ) return false;

	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// アニメーションを再生する状態.
	if ( UIAnimState->IsAnimPlay ) {
		return true;
	}
	// 常時.
	else if ( UIAnimWhen == "Always" ) {
		return true;
	}
	// マウスカーソルが重なった時.
	else if ( UIAnimWhen == "CursorOverlap" ) {
		return GetIsOverlappingTheMouse( pRenderState );
	}
	// クリックされた時.
	else if ( UIAnimWhen == "Click" ) {
		bool IsClick = false;
		if ( AnimData["Click"]["Left"]		) IsClick = KeyInput::IsKeyDown( VK_LBUTTON );
		if ( AnimData["Click"]["Center"]	) IsClick = KeyInput::IsKeyDown( VK_MBUTTON );
		if ( AnimData["Click"]["Right"]		) IsClick = KeyInput::IsKeyDown( VK_RBUTTON );
		return GetIsOverlappingTheMouse( pRenderState ) && IsClick;
	}
	// 特定のキーが押されている時.
	else if ( UIAnimWhen == "Key" ) {
		if ( AnimData["Key"]["Press"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyPress(	InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyPress(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyPress(		AnimData["Key"]["InputName"] );
		}
		if ( AnimData["Key"]["Down"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyDown(	InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyDown(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyDown(		AnimData["Key"]["InputName"] );
		}
		if ( AnimData["Key"]["Up"] ) {
			if ( AnimData["Key"]["KeyName"]		!= "None" ) return KeyInput::IsKeyUp(		InputList::StringToKeyData( AnimData["Key"]["KeyName"] ) );
			if ( AnimData["Key"]["ButName"]		!= "None" ) return XInput::IsKeyUp(		InputList::StringToButData( AnimData["Key"]["ButName"] ) );
			if ( AnimData["Key"]["InputName"]	!= "None" ) return Input::IsKeyUp(			AnimData["Key"]["InputName"] );
		}
	}
	// メッセージを受け取った時.
	else if ( UIAnimWhen == "GetMessage" ) {
		return Message::Check( AnimData["GetMessage"]["Name"] );
	}
	// ランダム( 確率 ).
	else if ( UIAnimWhen == "Random" ) {
		return Random::GetRand( 1, 100 ) <= AnimData["Random"]["Probability"];
	}
	return false;
}

//----------------------------.
// 拡縮UIアニメーション.
//----------------------------.
void CSprite::ScaleUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXSCALE3*			Scale		= &RenderState->Transform.Scale;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	const D3DXVECTOR2&	ShrinkSpeed	= {
		AnimData["Scale"]["ShrinkSpeed"]["x"],
		AnimData["Scale"]["ShrinkSpeed"]["y"]
	};
	const D3DXVECTOR2& ExpansionSpeed = {
		AnimData["Scale"]["ExpansionSpeed"]["x"],
		AnimData["Scale"]["ExpansionSpeed"]["y"]
	};
	const D3DXVECTOR2& MaxSize = {
		AnimData["Scale"]["MaxSize"]["x"],
		AnimData["Scale"]["MaxSize"]["y"]
	};
	const D3DXVECTOR2& MinSize = {
		AnimData["Scale"]["MinSize"]["x"],
		AnimData["Scale"]["MinSize"]["y"]
	};

	// 特定の回数分アニメーションを再生させる.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// 拡縮させる.
	bool IsCnt = false;
	if ( UIAnimState->Flag_x )	Scale->x -= ShrinkSpeed.x;
	else						Scale->x += ExpansionSpeed.x;
	if ( UIAnimState->Flag_y )	Scale->y -= ShrinkSpeed.y;
	else						Scale->y += ExpansionSpeed.y;

	// 拡縮を反転させる.
	if ( Scale->x >= MaxSize.x ) {
		UIAnimState->Flag_x = true;
		IsCnt = true;
	}
	else if ( Scale->x <= MinSize.x ) {
		UIAnimState->Flag_x = false;
		IsCnt = true;
	}
	if ( Scale->y >= MaxSize.y ) {
		UIAnimState->Flag_y = true;
		IsCnt				= true;
	}
	else if ( Scale->y <= MinSize.y ) {
		UIAnimState->Flag_y = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// 透明/不透明UIアニメーション.
//----------------------------.
void CSprite::AlphaUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState		= &RenderState->UIAnimState;
	json				AnimData		= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const float&		OpacitySpeed	= AnimData["Alpha"]["Speed"]["Opacity"];
	const float&		ClearSpeed		= AnimData["Alpha"]["Speed"]["Clear"];
	const float&		MaxAlpha		= AnimData["Alpha"]["MaxAlpha"];
	const float&		MinAlpha		= AnimData["Alpha"]["MinAlpha"];

	// 特定の回数分アニメーションを再生させる.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// 透明/不透明にしていく.
	if ( UIAnimState->Flag_x )	RenderState->Color.w -= OpacitySpeed;
	else						RenderState->Color.w += ClearSpeed;

	// 反転させる.
	bool IsCnt = false;
	if ( RenderState->Color.w >= MaxAlpha ) {
		UIAnimState->Flag_x = true;
		IsCnt				= true;
	}
	else if ( RenderState->Color.w <= MinAlpha ) {
		UIAnimState->Flag_x = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// 回転UIアニメーション.
//----------------------------.
void CSprite::RotateUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXROTATION3*		Rot			= &RenderState->Transform.Rotation;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&	Direction	= AnimData["Rotate"]["Direction"];
	const float&		Speed		= AnimData["Rotate"]["Speed"];

	// 特定の回数分アニメーションを再生させる.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// 回転方向に応じて回転させる.
	if ( Direction == "Right" ) Rot->z += Math::ToRadian( Speed );
	else						Rot->z -= Math::ToRadian( Speed );

	// 角度を0 ~ 360以内に収める.
	bool IsCnt = false;
	if ( Rot->z >= Math::RADIAN_MAX ) {
		Rot->z -= Math::RADIAN_MAX;
		IsCnt = true;
	}
	else if ( Rot->z <= Math::RADIAN_MIN ) {
		Rot->z += Math::RADIAN_MIN;
		IsCnt = true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//---------------------------.
// 移動UIアニメーション.
//---------------------------.
void CSprite::MoveUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState*		RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*			UIAnimState = &RenderState->UIAnimState;
	D3DXPOSITION3*			Pos			= &RenderState->Transform.Position;
	json					AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&		XEasingName	= AnimData["Move"]["EasingName"]["x"];
	const std::string&		YEasingName	= AnimData["Move"]["EasingName"]["y"];
	const D3DXPOSITION3&	StartPos	= RenderState->UIAnimState.StartTransform.Position;
	const D3DXPOSITION2&	EndPos		= { AnimData["Move"]["End"]["x"],	AnimData["Move"]["End"]["y"] };
	const D3DXVECTOR2&		Speed		= { AnimData["Move"]["Speed"]["x"], AnimData["Move"]["Speed"]["y"] };
	const D3DXVECTOR2&		Back		= { AnimData["Move"]["Back"]["x"],	AnimData["Move"]["Back"]["y"] };

	// 特定の回数分アニメーションを再生させる.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// 動かす.
	UIAnimState->Cnt_x += Speed.x;
	UIAnimState->Cnt_y += Speed.y;
	if ( UIAnimState->Flag_x  )	Pos->x = Easing::Easing( XEasingName, UIAnimState->Cnt_x, 100.0f, Back.x, StartPos.x,				StartPos.x + EndPos.x );
	else						Pos->x = Easing::Easing( XEasingName, UIAnimState->Cnt_x, 100.0f, Back.x, StartPos.x + EndPos.x,	StartPos.x );
	if ( UIAnimState->Flag_y  )	Pos->y = Easing::Easing( YEasingName, UIAnimState->Cnt_y, 100.0f, Back.y, StartPos.y,				StartPos.y + EndPos.y );
	else						Pos->y = Easing::Easing( YEasingName, UIAnimState->Cnt_y, 100.0f, Back.y, StartPos.y + EndPos.y,	StartPos.y );
	
	// 移動方向を反転させる.
	bool IsCnt = false;
	if ( UIAnimState->Cnt_x >= 100.0f ) {
		UIAnimState->Cnt_x	= 0.0f;
		UIAnimState->Flag_x = !UIAnimState->Flag_x;
		IsCnt				= true;
	}
	if ( UIAnimState->Cnt_y >= 100.0f ) {
		UIAnimState->Cnt_y	= 0.0f;
		UIAnimState->Flag_y = !UIAnimState->Flag_y;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// 振り子UIアニメーション.
//----------------------------.
void CSprite::PendulumUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	D3DXROTATION3*		Rot			= &RenderState->Transform.Rotation;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const float&		Speed		= AnimData["Pendulum"]["Speed"];
	const float&		EndAng		= AnimData["Pendulum"]["EndAng"];

	// 特定の回数分アニメーションを再生させる.
	if ( UIAnimState->IsLoop == false ) {
		if ( UIAnimState->IsAnimPlay == false ) UIAnimState->IsAnimPlay = true;
	}

	// 回転させる.
	if ( UIAnimState->Flag_x )	Rot->z -= Math::ToRadian( Speed );
	else						Rot->z += Math::ToRadian( Speed );

	// 回転方向を反転させる.
	bool IsCnt = false;
	if ( Rot->z >= Math::ToRadian( EndAng ) ) {
		UIAnimState->Flag_x = true;
		IsCnt				= true;
	}
	else if ( Rot->z <= -Math::ToRadian( EndAng ) ) {
		UIAnimState->Flag_x = false;
		IsCnt				= true;
	}

	if ( UIAnimState->IsLoop == false && IsCnt ) {
		UIAnimState->PlayCnt++;
		if ( UIAnimState->PlayCnt >= UIAnimState->PlayMax ) {
			UIAnimState->PlayCnt = 0;
			UIAnimState->IsAnimPlay = false;
			if ( !UIAnimState->EndMessage.empty() ) Message::Send( UIAnimState->EndMessage, Msg::EMsgType::CheckMsg );
		}
	}
}

//----------------------------.
// アニメーションUIアニメーション.
//----------------------------.
void CSprite::AnimationUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;

	// アニメーションを使用する.
	AnimState->IsAnimation = true;
}

//----------------------------.
// UVスクロールUIアニメーション.
//----------------------------.
void CSprite::ScrollUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SSpriteAnimState*	AnimState	= &RenderState->AnimState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const bool&			IsXScroll	= AnimData["Scroll"]["IsPlay"]["x"];
	const bool&			IsYScroll	= AnimData["Scroll"]["IsPlay"]["y"];

	// スクロールを使用する.
	if ( IsXScroll ) AnimState->IsUVScrollX = true;
	if ( IsYScroll ) AnimState->IsUVScrollY = true;
}

//---------------------------.
// メッセージUIアニメーション.
//---------------------------.
void CSprite::MessageUIAnimUpdate( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];
	const std::string&	MsgName		= AnimData["Message"];

	// メッセージを送る.
	if ( MsgName.empty() ) return;
	Message::Send( AnimData["Message"], Msg::EMsgType::CheckMsg );
}

//---------------------------.
// UIアニメーションの文字列を変換する.
//---------------------------.
std::string CSprite::ConvertUIAnimationString( const std::string& string )
{
	if(		 string == "None"								) return u8"無し";
	else if( string == "Always"								) return u8"常時";
	else if( string == "CursorOverlap"						) return u8"マウスカーソルが重なった時";
	else if( string == "Click"								) return u8"クリックされた時";
	else if( string == "Key"								) return u8"特定のキーが押されている時";
	else if( string == "GetMessage"							) return u8"メッセージを受け取った時";
	else if( string == "Random"								) return u8"ランダム( 確率 )";
	else if( string == "Scale"								) return u8"拡縮";
	else if( string == "Alpha"								) return u8"透明 / 不透明の点滅";
	else if( string == "Rotate"								) return u8"回転";
	else if( string == "Move"								) return u8"移動";
	else if( string == "Pendulum"							) return u8"振り子";
	else if( string == "Animation"							) return u8"アニメーション"	;
	else if( string == "Scroll"								) return u8"UVスクロール";
	else if( string == "Message"							) return u8"メッセージ";
	else if( string == u8"無し"								) return "None";
	else if( string == u8"常時"								) return "Always";
	else if( string == u8"マウスカーソルが重なった時"			) return "CursorOverlap";
	else if( string == u8"クリックされた時"					) return "Click";
	else if( string == u8"特定のキーが押されている時"			) return "Key";
	else if( string == u8"メッセージを受け取った時"			) return "GetMessage";
	else if( string == u8"ランダム( 確率 )"					) return "Random";
	else if( string == u8"拡縮"								) return "Scale";
	else if( string == u8"透明 / 不透明の点滅"				) return "Alpha";
	else if( string == u8"回転"								) return "Rotate";
	else if( string == u8"移動"								) return "Move";
	else if( string == u8"振り子"							) return "Pendulum";
	else if( string == u8"アニメーション"						) return "Animation";
	else if( string == u8"UVスクロール"						) return "Scroll";
	else if( string == u8"メッセージ"						) return "Message";
	return "None";
}

//---------------------------.
// いつ行うかの仮データを作成する.
//---------------------------.
void CSprite::CreateWhenTempData( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	if ( UIAnimWhen == "Key" && AnimData["Key"].empty() ) {
		AnimData["Key"]["Press"]		= true;
		AnimData["Key"]["Down"]			= false;
		AnimData["Key"]["Up"]			= false;
		AnimData["Key"]["KeyName"]		= "None";
		AnimData["Key"]["ButName"]		= "None";
		AnimData["Key"]["InputName"]	= "None";
	}
	else if ( UIAnimWhen == "Click" && AnimData["Click"].empty() ) {
		AnimData["Click"]["Left"]		= true;
		AnimData["Click"]["Center"]		= false;
		AnimData["Click"]["Right"]		= false;
	}
	else if ( UIAnimWhen == "GetMessage" && AnimData["GetMessage"].empty() ) {
		AnimData["GetMessage"]["Name"]	= "";
	}
	else if ( UIAnimWhen == "Random" && AnimData["Random"].empty() ) {
		AnimData["Random"]["Probability"] = 20;
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// いつ行うかの詳細設定.
//---------------------------.
void CSprite::WhenAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhen	= RenderState->UIAnimState.UIAnimWhen;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;

	if (		UIAnimWhen == "Key"			) KeyAdvancedSetting(			RenderState );	// 特定のキーが押されている時の詳細設定.
	if (		UIAnimWhen == "Click"		) ClickAdvancedSetting(			RenderState );	// クリックが押されている時の詳細設定.
	else if (	UIAnimWhen == "GetMessage"	) GetMessageAdvancedSetting(	RenderState );	// メッセージを受け取った時の詳細設定.
	else if (	UIAnimWhen == "Random"		) RandomAdvancedSetting(		RenderState );	// ランダムの詳細設定.
}

//---------------------------.
// 特定のキーが押されている時の詳細設定.
//---------------------------.
void CSprite::KeyAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// ボタンリスト.
	const std::vector<std::string> ButtonList = {
		u8"押している時",
		u8"押した瞬間",
		u8"離した瞬間"
	};

	// 現在の状態を取得.
	std::string NowButton;
	if (		AnimData["Key"]["Press"]	) NowButton = ButtonList[0];
	else if (	AnimData["Key"]["Down"]		) NowButton = ButtonList[1];
	else if (	AnimData["Key"]["Up"]		) NowButton = ButtonList[2];
	
	// どのタイミングで行うかのラジオボタンを作成.
	std::string OldButton = NowButton;
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"どのタイミングで行いますか？" ) ) {
		ImGuiManager::RadioButton( "KeyTypeRadio##NoName", &NowButton, ButtonList );
		ImGui::TreePop();
	}
	// 前の状態と今の状態を見て変更されているか.
	if ( OldButton != NowButton ) {
		// フラグの更新.
		if ( NowButton == ButtonList[0] ) {
			AnimData["Key"]["Press"]	= true;
			AnimData["Key"]["Down"]		= false;
			AnimData["Key"]["Up"]		= false;
		}
		if ( NowButton == ButtonList[1] ) {
			AnimData["Key"]["Press"]	= false;
			AnimData["Key"]["Down"]		= true;
			AnimData["Key"]["Up"]		= false;
		}
		if ( NowButton == ButtonList[2] ) {
			AnimData["Key"]["Press"]	= false;
			AnimData["Key"]["Down"]		= false;
			AnimData["Key"]["Up"]		= true;
		}
	}

	// 押されたことを調べるキーの名前の設定.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"押されたことを調べるキーの名前" ) ) {
		std::string KeyName = AnimData["Key"]["KeyName"];
		AnimData["Key"]["KeyName"] = ImGuiManager::Combo("KeyCombo##NoName", KeyName, InputList::GetKeyList());
		ImGui::TreePop();
	}
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"押されたことを調べるボタンの名前" ) ) {
		std::string ButName = AnimData["Key"]["ButName"];
		AnimData["Key"]["ButName"] = ImGuiManager::Combo( "ButCombo##NoName", ButName, InputList::GetButList() );
		ImGui::TreePop();
	}
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"押されたことを調べる操作の名前" ) ) {
		std::string InputName = AnimData["Key"]["InputName"];
		ImGui::InputText( "##KeyName", &InputName );
		if ( InputName == "" ) InputName = "None";
		AnimData["Key"]["InputName"] = InputName;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// クリックされた時の詳細設定.
//---------------------------.
void CSprite::ClickAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// クリックリスト.
	const std::vector<std::string> ClickList = {
		u8"左クリック",
		u8"真ん中クリック",
		u8"右クリック"
	};

	// 現在の状態を取得.
	std::string NowButton;
	if (		AnimData["Click"]["Left"]	) NowButton = ClickList[0];
	else if (	AnimData["Click"]["Center"]	) NowButton = ClickList[1];
	else if (	AnimData["Click"]["Right"]	) NowButton = ClickList[2];
	
	// どのタイミングで行うかのラジオボタンを作成.
	std::string OldButton = NowButton;
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"どのクリックで行いますか？" ) ) {
		ImGuiManager::RadioButton( "ClickTypeRadio##NoName", &NowButton, ClickList );
		ImGui::TreePop();
	}
	// 前の状態と今の状態を見て変更されているか.
	if ( OldButton != NowButton ) {
		// フラグの更新.
		if ( NowButton == ClickList[0] ) {
			AnimData["Click"]["Left"]	= true;
			AnimData["Click"]["Center"] = false;
			AnimData["Click"]["Right"]	= false;
		}
		if ( NowButton == ClickList[1] ) {
			AnimData["Click"]["Left"]	= false;
			AnimData["Click"]["Center"] = true;
			AnimData["Click"]["Right"]	= false;
		}
		if ( NowButton == ClickList[2] ) {
			AnimData["Click"]["Left"]	= false;
			AnimData["Click"]["Center"] = false;
			AnimData["Click"]["Right"]	= true;
		}
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// メッセージを受け取った時.
//---------------------------.
void CSprite::GetMessageAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"受け取るメッセージ名" ) ) {
		std::string MsgName = AnimData["GetMessage"]["Name"];
		ImGui::InputText( "##MsgName", &MsgName );
		AnimData["GetMessage"]["Name"] = MsgName;

		// テスト用のメッセージを送るボタン.
		if( ImGuiManager::Button( u8"メッセージを送る", !MsgName.empty() ) ) {
			Message::Send( MsgName, Msg::EMsgType::CheckMsg );
		}
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// ランダムの詳細設定.
//---------------------------.
void CSprite::RandomAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 確率の設定.
	int Prob = AnimData["Random"]["Probability"];
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"確率の設定" ) ) {
		ImGui::SliderInt( "##SliderInt", &Prob, 0, 100 );
		ImGui::InputInt( "##InputInt", &Prob );
		ImGui::TreePop();
	}
	// 確率を範囲内に収める.
	Prob = std::clamp( Prob, 0, 100 );
	AnimData["Random"]["Probability"] = Prob;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// 何を行うかの仮データを作成する.
//---------------------------.
void CSprite::CreateWhatTempData( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	const std::string&	UIAnimWhat	= RenderState->UIAnimState.UIAnimWhat;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	if ( UIAnimWhat == "Scale" && AnimData["Scale"].empty() ) {
		AnimData["Scale"]["ShrinkSpeed"]["x"]		= 0.01f;
		AnimData["Scale"]["ShrinkSpeed"]["y"]		= 0.01f;
		AnimData["Scale"]["ExpansionSpeed"]["x"]	= 0.01f;
		AnimData["Scale"]["ExpansionSpeed"]["y"]	= 0.01f;
		AnimData["Scale"]["MaxSize"]["x"]			= 1.0f;
		AnimData["Scale"]["MaxSize"]["y"]			= 1.0f;
		AnimData["Scale"]["MinSize"]["x"]			= 0.0f;
		AnimData["Scale"]["MinSize"]["y"]			= 0.0f;
		AnimData["Scale"]["Start"]["x"]				= "Shrink";
		AnimData["Scale"]["Start"]["y"]				= "Shrink";
	}
	else if ( UIAnimWhat == "Alpha" && AnimData["Alpha"].empty() ) {
		AnimData["Alpha"]["Speed"]["Opacity"]		= 0.01f;
		AnimData["Alpha"]["Speed"]["Clear"]			= 0.01f;
		AnimData["Alpha"]["MaxAlpha"]				= 1.0f;
		AnimData["Alpha"]["MinAlpha"]				= 0.0f;
		AnimData["Alpha"]["Start"]					= "Opacity";
	}
	else if ( UIAnimWhat == "Rotate" && AnimData["Rotate"].empty() ) {
		AnimData["Rotate"]["Direction"]				= "Right";
		AnimData["Rotate"]["Speed"]					= 0.1f;
	}
	else if ( UIAnimWhat == "Move" && AnimData["Move"].empty() ) {
		AnimData["Move"]["EasingName"]["x"]			= "Linear";
		AnimData["Move"]["EasingName"]["y"]			= "Linear";
		AnimData["Move"]["End"]["x"]				= 50.0f;
		AnimData["Move"]["End"]["y"]				= 0.0f;
		AnimData["Move"]["Speed"]["x"]				= 1.0f;
		AnimData["Move"]["Speed"]["y"]				= 0.0f;
		AnimData["Move"]["Back"]["x"]				= 0.0f;
		AnimData["Move"]["Back"]["y"]				= 0.0f;
	}
	else if ( UIAnimWhat == "Pendulum" && AnimData["Pendulum"].empty() ) {
		AnimData["Pendulum"]["Speed"]				= 0.1f;
		AnimData["Pendulum"]["EndAng"]				= 30.0f;
		AnimData["Pendulum"]["Start"]				= "Left";
	}
	else if ( UIAnimWhat == "Animation" && AnimData["Animation"].empty() ) {
		AnimData["Anim"]["Time"]					= 0.01f;
	}
	else if ( UIAnimWhat == "Scroll" && AnimData["Scroll"].empty() ) {
		AnimData["Scroll"]["IsPlay"]["x"]			= true;
		AnimData["Scroll"]["IsPlay"]["y"]			= false;
		AnimData["Scroll"]["Speed"]["x"]			= 0.01f;
		AnimData["Scroll"]["Speed"]["y"]			= 0.00f;
	}
	else if ( UIAnimWhat == "Message" && AnimData["Message"].empty() ) {
		AnimData["Message"]							= "";
	}
	AnimData["IsLoop"]		= true;
	AnimData["PlayMax"]		= 0;
	AnimData["EndMessage"]	= "";

	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// 何を行うかの詳細設定.
//----------------------------.
void CSprite::WhatAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;
	const std::string&	UIAnimWhat	= UIAnimState->UIAnimWhat;
	const std::string&	UIAnimWhen	= UIAnimState->UIAnimWhen;

	if ( UIAnimWhat != "None" ) {
		ImGui::Separator();
		if (		UIAnimWhat == "Scale"		) ScaleAdvancedSetting(		RenderState );	// 拡縮の詳細設定.
		else if (	UIAnimWhat == "Alpha"		) AlphaAdvancedSetting(		RenderState );	// 透明/不透明の点滅の詳細設定.
		else if (	UIAnimWhat == "Rotate"		) RotateAdvancedSetting(	RenderState );	// 回転の詳細設定.
		else if (	UIAnimWhat == "Move"		) MoveAdvancedSetting(		RenderState );	// 移動の詳細設定.
		else if (	UIAnimWhat == "Pendulum"	) PendulumAdvancedSetting(	RenderState );	// 振り子の詳細設定.
		else if (	UIAnimWhat == "Animation"	) AnimationAdvancedSetting( RenderState );	// アニメーションの詳細設定.
		else if (	UIAnimWhat == "Scroll"		) ScrollAdvancedSetting(	RenderState );	// UVスクロールの詳細設定.
		else if (	UIAnimWhat == "Message"		) MessageAdvancedSetting(	RenderState );	// メッセージの詳細設定.

		// ループ再生するかの詳細設定.
		if ( UIAnimWhat == "Animation" || UIAnimWhat == "Scroll" || UIAnimWhen == "Always" ) return;
		LoopAdvancedSetting( RenderState );
	}
}

//----------------------------.
// ループ再生の詳細設定.
//----------------------------.
void CSprite::LoopAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	SUIAnimState*		UIAnimState = &RenderState->UIAnimState;

	// ループ再生を行うかの設定.
	ImGui::Separator();
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"ループ再生するか" ) ) {
		ImGuiManager::CheckBox( "##IsLoopCheckBox", &UIAnimState->IsLoop );
		ImGui::TreePop();
	}
	if ( UIAnimState->IsLoop ) return;

	// ループ再生を行う場合、何回再生するかの設定.
	ImGui::Separator();
	ImGuiManager::InputInt( u8"再生回数", &UIAnimState->PlayMax );

	// 再生終了時に送るメッセージ名の設定.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"再生終了時に送るメッセージ名" ) ) {
		ImGui::InputText( "##EndMessage", &UIAnimState->EndMessage );
		ImGui::TreePop();
	}
}

//----------------------------.
// 拡縮の詳細設定.
//----------------------------.
void CSprite::ScaleAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 最大サイズの設定.
	D3DXVECTOR2 MaxSize = {
		AnimData["Scale"]["MaxSize"]["x"],
		AnimData["Scale"]["MaxSize"]["y"]
	};
	ImGuiManager::InputVec2( u8"最大サイズの設定", &MaxSize, { 1.0f, 1.0f } );
	AnimData["Scale"]["MaxSize"]["x"] = MaxSize.x;
	AnimData["Scale"]["MaxSize"]["y"] = MaxSize.y;
	// 最小サイズの設定.
	D3DXVECTOR2 MinSize = {
		AnimData["Scale"]["MinSize"]["x"],
		AnimData["Scale"]["MinSize"]["y"]
	};
	ImGuiManager::InputVec2( u8"最小サイズの設定", &MinSize );
	AnimData["Scale"]["MinSize"]["x"] = MinSize.x;
	AnimData["Scale"]["MinSize"]["y"] = MinSize.y;

	// 拡大速度の設定.
	D3DXVECTOR2 ExpansionSpeed = {
		AnimData["Scale"]["ExpansionSpeed"]["x"],
		AnimData["Scale"]["ExpansionSpeed"]["y"]
	};
	ImGuiManager::InputVec2( u8"拡大速度の設定", &ExpansionSpeed, { 0.01f, 0.01f } );
	AnimData["Scale"]["ExpansionSpeed"]["x"] = ExpansionSpeed.x;
	AnimData["Scale"]["ExpansionSpeed"]["y"] = ExpansionSpeed.y;
	// 縮小速度の設定.
	D3DXVECTOR2 ShrinkSpeed = {
		AnimData["Scale"]["ShrinkSpeed"]["x"],
		AnimData["Scale"]["ShrinkSpeed"]["y"]
	};
	ImGuiManager::InputVec2( u8"縮小速度の設定", &ShrinkSpeed, { 0.01f, 0.01f } );
	AnimData["Scale"]["ShrinkSpeed"]["x"] = ShrinkSpeed.x;
	AnimData["Scale"]["ShrinkSpeed"]["y"] = ShrinkSpeed.y;

	// 開始時の状態リスト.
	const std::vector<std::string> StartList = {
		u8"拡大していく",
		u8"縮小していく"
	};
	// x方向の開始時の状態の設定.
	std::string NowStart = AnimData["Scale"]["Start"]["x"] == "Expansion" ?
		u8"拡大していく" : u8"縮小していく";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"x方向の開始時の状態の設定" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Scale"]["Start"]["x"] = NowStart == u8"拡大していく" ?
		"Expansion" : "Shrink";

	// y方向の開始時の状態の設定.
	NowStart = AnimData["Scale"]["Start"]["y"] == "Expansion" ?
		u8"拡大していく" : u8"縮小していく";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"y方向の開始時の状態の設定" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Scale"]["Start"]["y"] = NowStart == u8"拡大していく" ?
		"Expansion" : "Shrink";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// 透明/不透明の点滅の詳細設定.
//----------------------------.
void CSprite::AlphaAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 開始時の透明度の設定.
	ImGuiManager::SliderInputFloat( u8"開始時の透明度", &RenderState->UIAnimState.StartColor.w, 0.0f, 1.0f );

	// 不透明になる速度の設定.
	float OpacitySpeed = AnimData["Alpha"]["Speed"]["Opacity"];
	ImGuiManager::InputFloat( u8"不透明になる速度の設定", &OpacitySpeed, 0.01f );
	AnimData["Alpha"]["Speed"]["Opacity"] = OpacitySpeed;

	// 透明になる速度の設定.
	float ClearSpeed = AnimData["Alpha"]["Speed"]["Clear"];
	ImGuiManager::InputFloat( u8"透明になる速度の設定", &ClearSpeed, 0.01f );
	AnimData["Alpha"]["Speed"]["Clear"] = ClearSpeed;

	// 最大の透明度の設定.
	float MaxAlpha = AnimData["Alpha"]["MaxAlpha"];
	ImGuiManager::SliderInputFloat( u8"最大の透明度の設定", &MaxAlpha, 0.0f, 1.0f, 1.0f );
	AnimData["Alpha"]["MaxAlpha"] = MaxAlpha;

	// 最小の透明度の設定.
	float MinAlpha = AnimData["Alpha"]["MinAlpha"];
	ImGuiManager::SliderInputFloat( u8"最小の透明度の設定", &MinAlpha, 0.0f, 1.0f, 0.0f );
	AnimData["Alpha"]["MinAlpha"] = MinAlpha;

	// 開始時の状態リスト.
	const std::vector<std::string> StartList = {
		u8"不透明になっていく",
		u8"透明になっていく"
	};
	// 開始時の状態の設定.
	std::string NowStart = AnimData["Alpha"]["Start"] == "Opacity" ?
		u8"不透明になっていく" : u8"透明になっていく";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"開始時の状態の設定" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowStart, StartList );
		ImGui::TreePop();
	}
	AnimData["Alpha"]["Start"] = NowStart == u8"不透明になっていく" ?
		"Opacity" : "Clear";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// 回転の詳細設定.
//----------------------------.
void CSprite::RotateAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 回転方向リスト.
	const std::vector<std::string> DireList = {
		u8"時計回り( 右回り )",
		u8"反時計回り( 左回り )"
	};
	// 回転方向の設定.
	std::string NowDire = AnimData["Rotate"]["Direction"] == "Right" ?
		u8"時計回り( 右回り )" : u8"反時計回り( 左回り )";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"回転方向の設定" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowDire, DireList );
		ImGui::TreePop();
	}
	AnimData["Rotate"]["Direction"] = NowDire == u8"時計回り( 右回り )" ?
		"Right" : "Left";

	// 回転速度の設定.
	float Speed = AnimData["Rotate"]["Speed"];
	ImGuiManager::InputFloat( u8"回転速度の設定", &Speed, 0.1f );
	AnimData["Rotate"]["Speed"] = Speed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// 移動の詳細設定.
//----------------------------.
void CSprite::MoveAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 移動終了座標の設定.
	D3DXPOSITION2 MoveEndPos;
	MoveEndPos.x = AnimData["Move"]["End"]["x"];
	MoveEndPos.y = AnimData["Move"]["End"]["y"];
	ImGuiManager::InputVec2( u8"移動距離", &MoveEndPos );
	AnimData["Move"]["End"]["x"] = MoveEndPos.x;
	AnimData["Move"]["End"]["y"] = MoveEndPos.y;

	// 移動速度の設定.
	D3DXVECTOR2 Speed;
	Speed.x = AnimData["Move"]["Speed"]["x"];
	Speed.y = AnimData["Move"]["Speed"]["y"];
	ImGuiManager::InputVec2( u8"移動速度", &Speed );
	AnimData["Move"]["Speed"]["x"] = Speed.x;
	AnimData["Move"]["Speed"]["y"] = Speed.y;

	// 戻る距離の設定.
	D3DXVECTOR2 Back;
	Back.x = AnimData["Move"]["Back"]["x"];
	Back.y = AnimData["Move"]["Back"]["y"];
	ImGuiManager::InputVec2( u8"戻る距離", &Back );
	AnimData["Move"]["Back"]["x"] = Back.x;
	AnimData["Move"]["Back"]["y"] = Back.y;

	// 使用するイージングの設定.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"使用するイージング" ) ) {
		std::string XEasing = AnimData["Move"]["EasingName"]["x"];
		std::string YEasing = AnimData["Move"]["EasingName"]["y"];
		ImGuiManager::Combo( u8"X方向", &XEasing, Easing::GetEasingList() );
		ImGuiManager::Combo( u8"Y方向", &YEasing, Easing::GetEasingList() );
		AnimData["Move"]["EasingName"]["x"] = XEasing;
		AnimData["Move"]["EasingName"]["y"] = YEasing;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// 振り子の詳細設定.
//----------------------------.
void CSprite::PendulumAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 速度の設定.
	float Speed = AnimData["Pendulum"]["Speed"];
	ImGuiManager::InputFloat( u8"速度の設定", &Speed, 0.1f );
	AnimData["Pendulum"]["Speed"] = Speed;

	// 最終角度の設定.
	float EndAng = AnimData["Pendulum"]["EndAng"];
	ImGuiManager::InputFloat( u8"最終角度の設定", &EndAng, 30.0f );
	AnimData["Pendulum"]["EndAng"] = EndAng;

	// 開始時の方向リスト.
	const std::vector<std::string> DireList = {
		u8"時計回り( 右回り )",
		u8"反時計回り( 左回り )"
	};
	// 開始時の方向の設定.
	std::string NowDire = AnimData["Pendulum"]["Start"] == "Right" ?
		u8"時計回り( 右回り )" : u8"反時計回り( 左回り )";
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"開始時の方向の設定" ) ) {
		ImGuiManager::RadioButton( "##NoName", &NowDire, DireList );
		ImGui::TreePop();
	}
	AnimData["Pendulum"]["Start"] = NowDire == u8"時計回り( 右回り )" ?
		"Right" : "Left";
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// アニメーションの詳細設定.
//----------------------------.
void CSprite::AnimationAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 速度の設定.
	float Speed = AnimData["Anim"]["Time"];
	ImGuiManager::InputFloat( u8"速度の設定", &Speed, 0.01f );
	AnimData["Anim"]["Time"] = Speed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//----------------------------.
// UVスクロールの詳細設定.
//----------------------------.
void CSprite::ScrollAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData	= m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"UVスクロールを行うか" ) ) {
		// x方向のUVスクロールを行うか.
		RenderState->AnimState.IsUVScrollX	= AnimData["Scroll"]["IsPlay"]["x"];
		ImGuiManager::CheckBox( u8"x方向のUVスクロールを行うか", &RenderState->AnimState.IsUVScrollX );
		AnimData["Scroll"]["IsPlay"]["x"]	= RenderState->AnimState.IsUVScrollX;

		// y方向のUVスクロールを行うか.
		RenderState->AnimState.IsUVScrollY	= AnimData["Scroll"]["IsPlay"]["y"];
		ImGuiManager::CheckBox( u8"y方向のUVスクロールを行うか", &RenderState->AnimState.IsUVScrollY );
		AnimData["Scroll"]["IsPlay"]["y"]	= RenderState->AnimState.IsUVScrollY;
		ImGui::TreePop();
	}
	// スクロール速度の設定.
	D3DXVECTOR2 ScrollSpeed = {
		AnimData["Scroll"]["Speed"]["x"],
		AnimData["Scroll"]["Speed"]["y"]
	};
	ImGuiManager::InputVec2( u8"スクロール速度の設定", &ScrollSpeed, { 0.01f, 0.01f } );
	AnimData["Scroll"]["Speed"]["x"]		= ScrollSpeed.x;
	AnimData["Scroll"]["Speed"]["y"]		= ScrollSpeed.y;
	RenderState->AnimState.ScrollSpeed		= ScrollSpeed;
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// メッセージの詳細設定.
//---------------------------.
void CSprite::MessageAdvancedSetting( SSpriteRenderState* pRenderState )
{
	SSpriteRenderState* RenderState = pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData = m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"];

	// 送るメッセージ名の設定.
	ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
	if ( ImGui::TreeNode( u8"送るメッセージ名" ) ) {
		std::string MsgName = AnimData["Message"];
		ImGui::InputText( "##Message", &MsgName );
		AnimData["Message"] = MsgName;
		ImGui::TreePop();
	}
	m_SpriteStateData[RenderState->SceneName][RenderState->No]["UIAnimation"] = AnimData;
}

//---------------------------.
// 新しく追加するスプライトの仮データを作成する.
//---------------------------.
void CSprite::NewDispSpriteTmpData( SSpriteRenderState* pRenderState, const std::string& SceneName, const int No )
{
	SSpriteRenderState* RenderState		= pRenderState == nullptr ? &m_SpriteRenderState : pRenderState;
	json				AnimData		= m_SpriteStateData[SceneName][std::to_string( No )];

	// スプライトの状態を初期化.
	const SSpriteAnimState AnimState	= RenderState->AnimState;
	*RenderState						= SSpriteRenderState();
	RenderState->AnimState				= AnimState;
	RenderState->SceneName				= SceneName;
	RenderState->No						= std::to_string( No );

	// 必要な仮データを作成する.
	AnimData["Transform"]["Position"]["x"]	= 0.0f;
	AnimData["Transform"]["Position"]["y"]	= 0.0f;
	AnimData["Transform"]["Position"]["z"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["x"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["y"]	= 0.0f;
	AnimData["Transform"]["Rotation"]["z"]	= 0.0f;
	AnimData["Transform"]["Scale"]["x"]		= 1.0f;
	AnimData["Transform"]["Scale"]["y"]		= 1.0f;
	AnimData["Transform"]["Scale"]["z"]		= 1.0f;
	AnimData["Color"]["R"]					= 1.0f;
	AnimData["Color"]["G"]					= 1.0f;
	AnimData["Color"]["B"]					= 1.0f;
	AnimData["Color"]["A"]					= 1.0f;
	AnimData["UIAnimation"]["When"]			= "None";
	AnimData["UIAnimation"]["What"]			= "None";
	m_SpriteStateData[SceneName][std::to_string( No )] = AnimData;
}
