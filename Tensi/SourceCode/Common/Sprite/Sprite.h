#pragma once
#include "SpriteStruct.h"
#include "..\..\Utility\FileManager\FileManager.h"

/************************************************
*	スプライトクラス.
**/
class CSprite final
{
public:
	using Sampler = unsigned long long;

public:
	CSprite();
	~CSprite();

	// 初期化.
	HRESULT Init( const std::string& FilePath );
	// 指定したシーンでスプライトの情報を初期化する.
	void InitSceneSpriteState( const std::string& SceneName, const int& No );

	// UIでレンダリング.
	void RenderUI( SSpriteRenderState* pRenderState = nullptr );
	// 3Dでレンダリング.
	void Render3D( SSpriteRenderState* pRenderState = nullptr, const bool IsBillBoard = false );

	// UIエディタのImGuiの描画.
	void UIEdtorImGuiRender( const bool IsAutoSave, SSpriteRenderState* pRenderState = nullptr );
	// マウスのつかみ判定の描画.
	void HitBoxRender( const D3DXVECTOR4& Color = Color4::Red, SSpriteRenderState* pRenderState = nullptr );

	// マウスのドラッグの更新.
	bool DragUpdate( const bool IsDrag, const bool IsAutoSave, SSpriteRenderState* pRenderState = nullptr );
	// マウスに重なっているか取得する.
	bool GetIsOverlappingTheMouse( SSpriteRenderState* pRenderState );

	// スプライト情報を保存する.
	HRESULT SpriteStateDataSave( SSpriteRenderState* pRenderState = nullptr );
	// スプライト情報の取得.
	HRESULT SpriteStateDataLoad();
	// スプライト情報の保存して再読み込み.
	HRESULT SpriteStateSaveReload();

	// 表示しているスプライトから特定のシーンを削除.
	HRESULT DispSpriteDeleteScene( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState = nullptr );
	// 表示しているスプライトの数を減らす.
	HRESULT DispSpriteDeleteNum( const bool IsAutoSave, const std::string& DispScene, const int No, SSpriteRenderState* pRenderState = nullptr );

	// 表示するスプライトの追加.
	HRESULT AddDispSprite( const bool IsAutoSave, const std::string& DispScene, SSpriteRenderState* pRenderState = nullptr );

	// 表示するシーンの取得.
	std::vector<std::string> GetDispSceneList() const { return m_SpriteState.DispSceneList; }
	// 表示する個数の取得.
	int GetSceneDispNum( const std::string& SceneName ) { return m_SpriteState.SceneDispNum[SceneName]; }

	// テクスチャの取得.
	inline ID3D11ShaderResourceView* GetTexture() const { return m_pTexture; }

	// 描画座標の取得.
	inline D3DXVECTOR3 GetRenderPos() const { return m_SpriteRenderState.Transform.Position; }
	// スプライト情報の取得.
	inline SSpriteState GetSpriteState() const { return m_SpriteState; }
	// 画像のアニメーション情報を取得.
	inline SSpriteAnimState GetAnimState() const { return m_SpriteRenderState.AnimState; }
	// 画像の描画情報を取得.
	inline SSpriteRenderState GetRenderState() const { return m_SpriteRenderState; }
	// スプライトの左上の座標を取得する.
	D3DXVECTOR3 GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState );

	// ログファイルを作成するかを設定.
	void SetIsCreaterLog( bool* pFlag ) { m_pIsCreaterLog = pFlag; }
	// ログリストの設定.
	void SetLogList( std::vector<std::string>* pLogList ) { m_pLogList = pLogList; }

	// 描画する範囲の設定.
	inline void SetRenderArea( const D3DXVECTOR4& Area ) { m_SpriteRenderState.RenderArea = Area; }
	inline void SetRenderArea( const float x, const float y, const float w, const float h ) { m_SpriteRenderState.RenderArea = { x, y, w, h }; }

	// テクスチャの設定.
	inline void SetTexture( ID3D11ShaderResourceView* pTexture ) { m_pTexture = pTexture; }

	// アニメーションを行うかの設定.
	inline void SetIsAnimPlay( const bool Flag ) { m_IsAnimPlay = Flag; }
	// この画像を全て表示するかの設定.
	inline void SetIsAllDisp( const bool Flag ) { m_IsAllDisp = Flag; }

private:
	// シェーダ作成.
	HRESULT CreateShader();
	// モデル作成(UI).
	HRESULT CreateModelUI();
	// モデル作成(3D).
	HRESULT CreateModel3D();
	// テクスチャ作成.
	HRESULT CreateTexture();
	// サンプラ作成.
	HRESULT CreateSampler();
	// 頂点情報の作成.
	HRESULT CreateVertex( const float w, const float h, const float u, const float v );

	// スプライト情報の作成.
	HRESULT CreateSpriteState();

	// テクスチャの比率を取得.
	int myGcd( int t, int t2 ) { if ( t2 == 0 ) return t; return myGcd( t2, t % t2 ); }

	// 中心座標に変換するための追加座標の取得.
	D3DXPOSITION3 GetAddCenterPosition();

	// UIアニメーションの更新.
	void UIAnimUpdate( SSpriteRenderState* pRenderState );

	// UIアニメーションの初期化.
	void UIAnimInit( SSpriteRenderState* pRenderState );
	// UIアニメーションを行うか調べる.
	bool UIAnimPlayCheck( SSpriteRenderState* pRenderState );
	// 拡縮UIアニメーション.
	void ScaleUIAnimUpdate( SSpriteRenderState* pRenderState );
	// 透明/不透明UIアニメーション.
	void AlphaUIAnimUpdate( SSpriteRenderState* pRenderState );
	// 回転UIアニメーション.
	void RotateUIAnimUpdate( SSpriteRenderState* pRenderState );
	// 移動UIアニメーション.
	void MoveUIAnimUpdate( SSpriteRenderState* pRenderState );
	// 振り子UIアニメーション.
	void PendulumUIAnimUpdate( SSpriteRenderState* pRenderState );
	// アニメーションUIアニメーション.
	void AnimationUIAnimUpdate( SSpriteRenderState* pRenderState );
	// UVスクロールUIアニメーション.
	void ScrollUIAnimUpdate( SSpriteRenderState* pRenderState );
	// メッセージUIアニメーション.
	void MessageUIAnimUpdate( SSpriteRenderState* pRenderState );

	// UIアニメーションの文字列を変換する.
	std::string ConvertUIAnimationString( const std::string& string );

	// いつ行うかの仮データを作成する.
	void CreateWhenTempData( SSpriteRenderState* pRenderState );
	// いつ行うかの詳細設定.
	void WhenAdvancedSetting( SSpriteRenderState* pRenderState );
	// 特定のキーが押されている時の詳細設定.
	void KeyAdvancedSetting( SSpriteRenderState* pRenderState );
	// クリックされた時の詳細設定.
	void ClickAdvancedSetting( SSpriteRenderState* pRenderState );
	// メッセージを受け取った時の詳細設定.
	void GetMessageAdvancedSetting( SSpriteRenderState* pRenderState );
	// ランダムの詳細設定.
	void RandomAdvancedSetting( SSpriteRenderState* pRenderState );

	// 何を行うかの仮データを作成する.
	void CreateWhatTempData( SSpriteRenderState* pRenderState );
	// 何を行うかの詳細設定.
	void WhatAdvancedSetting( SSpriteRenderState* pRenderState );
	// ループ再生の詳細設定.
	void LoopAdvancedSetting( SSpriteRenderState* pRenderState );
	// 拡縮の詳細設定.
	void ScaleAdvancedSetting( SSpriteRenderState* pRenderState );
	// 透明/不透明の点滅の詳細設定.
	void AlphaAdvancedSetting( SSpriteRenderState* pRenderState );
	// 回転の詳細設定.
	void RotateAdvancedSetting( SSpriteRenderState* pRenderState );
	// 移動の詳細設定.
	void MoveAdvancedSetting( SSpriteRenderState* pRenderState );
	// 振り子の詳細設定.
	void PendulumAdvancedSetting( SSpriteRenderState* pRenderState );
	// アニメーションの詳細設定.
	void AnimationAdvancedSetting( SSpriteRenderState* pRenderState );
	// UVスクロールの詳細設定.
	void ScrollAdvancedSetting( SSpriteRenderState* pRenderState );
	// メッセージの詳細設定.
	void MessageAdvancedSetting( SSpriteRenderState* pRenderState );

	// 新しく追加するスプライトの仮データを作成する.
	void NewDispSpriteTmpData( SSpriteRenderState* pRenderState, const std::string& SceneName, const int No );

private:
	ID3D11Device*					m_pDevice;				// デバイスオブジェクト.
	ID3D11DeviceContext*			m_pContext;				// デバイスコンテキスト.

	ID3D11VertexShader*				m_pVertexShader;		// 頂点シェーダ.
	ID3D11InputLayout*				m_pVertexLayout;		// 頂点レイアウト.
	std::vector<ID3D11PixelShader*>	m_pPixelShaders;		// ピクセルシェーダリスト.

	ID3D11Buffer*					m_pVertexBufferUI;		// 頂点バッファ(UI用).
	ID3D11Buffer*					m_pVertexBuffer3D;		// 頂点バッファ(3D用).
	ID3D11Buffer*					m_pConstantBuffer;		// コンスタントバッファ.

	ID3D11ShaderResourceView*		m_pTexture;				// テクスチャ.
	ID3D11SamplerState*				m_pSampleLinears[static_cast<Sampler>( ESamplerState::Max )]; // サンプラ:テクスチャに各種フィルタをかける.

	SVertex							m_Vertices[4];			// 頂点作成用.
	SSpriteState					m_SpriteState;			// スプライト情報.
	SSpriteRenderState				m_SpriteRenderState;	// スプライトアニメーション情報.

	json							m_SpriteStateData;		// スプライト情報データ.
	std::vector<std::string>*		m_pLogList;				// ログリスト.

	bool							m_IsAnimPlay;			// アニメーションを行うか.
	bool							m_IsAllDisp;			// この画像を全て表示するか.
	bool*							m_pIsCreaterLog;		// ログファイルを作成するか.
};