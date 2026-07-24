#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\..\Global.h"
#include "VisualEffectTypes.h"
#include "Effects\VisualEffectBase.h"
#include <array>
#include <vector>

class CRenderTexture;

/************************************************
*	視覚効果マネージャー.
*
*	■ 画面全体( カメラ )にかける :
*		VisualEffectManager::AddScreenEffect( EVisualEffect::Bloom );
*		→ 毎フレーム自動で適用される( Main が SceneManager::Render 後に適用 ).
*		UI を対象外にしたい場合は、シーン側で UI 描画前に
*		VisualEffectManager::ApplyScreenEffects() を手動で呼ぶ.
*		( そのフレームの自動適用はスキップされる ).
*
*	■ 一部の描画物( スプライト/メッシュ )だけにかける :
*		VisualEffectManager::BeginCapture();
*		pSprite->RenderUI();	// 対象の描画( 2D/3D どちらでも可 ).
*		VisualEffectManager::EndCapture( { EVisualEffect::Glitch } );
*
*	■ パラメータの設定 :
*		VisualEffectManager::Param<SBloomParam>().Strength = 2.0f;
*		VisualEffectManager::SetIntensity( EVisualEffect::Bloom, 0.5f );
*		VisualEffectManager::SetArea( EVisualEffect::Bloom, { 100.0f, 100.0f, 400.0f, 300.0f } );
*
*	※ 透明ウィンドウ( デスクトップ常駐 )で使用する場合の注意 :
*		エフェクトは透明部分( アルファ0 )には見えない.
*		SetAlphaLift で透明部分を持ち上げると画面全体に見えるようになるが、
*		その部分はクリック透過判定にも影響する.
**/
class VisualEffectManager final
{
public:
	VisualEffectManager();
	~VisualEffectManager();

	// 更新( 毎フレーム呼ぶ. 経過時間の加算とデバッグ表示の登録 ).
	static void Update( const float& DeltaTime );
	// 解放( アプリ終了時に呼ぶ ).
	static void Release();

	//----------------------------.
	//	画面全体エフェクト.
	//----------------------------.
	// 画面全体エフェクトの追加( 追加順に適用される ).
	static void AddScreenEffect( const EVisualEffect Type );
	// 画面全体エフェクトの削除.
	static void RemoveScreenEffect( const EVisualEffect Type );
	// 画面全体エフェクトの全削除.
	static void ClearScreenEffect();
	// 画面全体エフェクトの一括設定( 並び順ごと置き換える ).
	static void SetScreenEffectList( const std::vector<EVisualEffect>& List );
	// 画面全体エフェクトの取得.
	static std::vector<EVisualEffect> GetScreenEffectList();
	// 指定のエフェクトが画面全体に登録されているか.
	static bool HasScreenEffect( const EVisualEffect Type );
	// 画面全体エフェクトの適用.
	//	Main から毎フレーム自動で呼ばれる. シーン側で手動で呼んだ場合、
	//	そのフレームの自動適用はスキップされる( UI を対象外にする用途 ).
	static void ApplyScreenEffects();

	//----------------------------.
	//	部分適用( キャプチャ ).
	//----------------------------.
	// キャプチャ開始( ここから EndCapture までの描画がエフェクト対象になる ).
	//	UseDepth : キャプチャ中に深度バッファを使用するか( 3D メッシュ描画向け ).
	static void BeginCapture( const bool UseDepth = true );
	// キャプチャ終了( エフェクトをかけて元の描画先へ合成する ).
	static void EndCapture( const std::vector<EVisualEffect>& EffectList );

	//----------------------------.
	//	パラメータ.
	//----------------------------.
	// パラメータの取得( 例 : Param<SBloomParam>().Strength = 2.0f; ).
	template<typename TParam>
	static TParam& Param()
	{
		static TParam Dummy;	// エフェクト生成失敗時の受け皿.
		CVisualEffectBase* pEffect = GetEffect( TParam::TYPE );
		if ( pEffect == nullptr ) return Dummy;
		void* pParam = pEffect->GetParamPtr();
		return pParam != nullptr ? *static_cast<TParam*>( pParam ) : Dummy;
	}
	// 適用率の設定( 0:元画像のまま ～ 1:効果100% ).
	static void SetIntensity( const EVisualEffect Type, const float Value );
	// 適用範囲の設定( px, x:左上x, y:左上y, z:幅, w:高さ ). 幅か高さ0以下で画面全体.
	static void SetArea( const EVisualEffect Type, const D3DXVECTOR4& Area );
	// 透明部分の持ち上げアルファの設定( クラスコメントの注意書きを参照 ).
	static void SetAlphaLift( const EVisualEffect Type, const float Value );
	// エフェクトの取得( 未生成なら生成する. 失敗時は nullptr ).
	static CVisualEffectBase* GetEffect( const EVisualEffect Type );

	//----------------------------.
	//	エフェクト実装用ヘルパ.
	//----------------------------.
	// 全画面1パス描画.
	//	pSubSRV : 補助入力( t1. 不要なら nullptr ).
	//	pParams : エフェクトパラメータ( b1. 不要なら nullptr ).
	static void DrawPass(
		ID3D11PixelShader* pPS,
		ID3D11RenderTargetView* pDstRTV,
		ID3D11ShaderResourceView* pSrcSRV,
		ID3D11ShaderResourceView* pSubSRV = nullptr,
		const D3DXVECTOR4* pParams = nullptr,
		const int ParamNum = 0 );
	// 全画面コピー描画( 共通定数バッファを等倍設定で上書きする ).
	static void DrawCopy( ID3D11RenderTargetView* pDstRTV, ID3D11ShaderResourceView* pSrcSRV );
	// ワーク用レンダーテクスチャの取得( No : 0～1. エフェクト内の中間バッファ用 ).
	static CRenderTexture* GetWorkRT( const int No );
	// 経過時間の取得( 秒 ).
	static float GetTime();

private:
	// 共通定数バッファの内容.
	struct SCommonCB
	{
		D3DXVECTOR4	Common0;	// x:画面幅, y:画面高さ, z:経過時間(秒), w:適用率.
		D3DXVECTOR4	Common1;	// x:透明部分の持ち上げアルファ, yzw:未使用.
		D3DXVECTOR4	Area;		// 適用範囲(px).
	};
	// キャプチャ中の退避情報.
	struct SCaptureState
	{
		ID3D11RenderTargetView*	pPrevRTV;	// 退避した描画先.
		ID3D11DepthStencilView*	pPrevDSV;	// 退避した深度ステンシル.
		int						RtIndex;	// 使用中のキャプチャRT番号.
	};

private:
	// インスタンスの取得.
	static VisualEffectManager* GetInstance();

	// 初期化( 未初期化なら初期化する. 失敗時は false ).
	bool EnsureInit();
	// リソースの解放本体.
	void ReleaseResources();
	// 画面サイズに合わせて各レンダーテクスチャを作り直す.
	bool EnsureSize();
	// エフェクトの生成.
	static std::unique_ptr<CVisualEffectBase> CreateEffect( const EVisualEffect Type );
	// エフェクトチェーンの適用( pSrcSRV に List を順にかけて pDstRTV へ描画する ).
	void ApplyChain( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV, const std::vector<EVisualEffect>& List );
	// 共通定数バッファの更新.
	void UpdateCommonCB( const float Intensity, const float AlphaLift, const D3DXVECTOR4& Area );
	// 現在のエフェクトに合わせた共通定数バッファの更新.
	void UpdateCommonCBForEffect( const CVisualEffectBase* pEffect );
	// ステートを視覚効果用に切り替える( 深度OFF/ブレンドOFF/カリングOFF ).
	void PushRenderStates();
	// ステートを元に戻す.
	void PopRenderStates();
	// ImGuiのデバッグ表示.
	void DebugRender();

private:
	ID3D11VertexShader*		m_pVertexShader;	// 全画面三角形用頂点シェーダ.
	ID3D11PixelShader*		m_pCopyPS;			// コピー用ピクセルシェーダ.
	ID3D11Buffer*			m_pCommonCB;		// 共通定数バッファ( b0 ).
	ID3D11Buffer*			m_pParamCB;			// エフェクトパラメータ定数バッファ( b1 ).
	ID3D11SamplerState*		m_pSampler;			// サンプラ( リニア/クランプ ).
	ID3D11RasterizerState*	m_pRasterizerState;	// 全画面描画用ラスタライザ( カリング無し ).

	std::unique_ptr<CRenderTexture>				m_pPingPong[2];		// チェーン適用用のピンポンバッファ.
	std::unique_ptr<CRenderTexture>				m_pWork[2];			// エフェクト内の中間バッファ.
	std::vector<std::unique_ptr<CRenderTexture>>m_pCaptures;		// キャプチャ先( ネスト分だけ増える ).
	std::vector<SCaptureState>					m_CaptureStack;		// キャプチャ中の退避情報.

	std::array<std::unique_ptr<CVisualEffectBase>, static_cast<size_t>( EVisualEffect::Max )> m_pEffects;	// エフェクト本体.
	std::vector<EVisualEffect>	m_ScreenEffects;		// 画面全体エフェクトの適用リスト.

	// ステート退避用.
	ID3D11RasterizerState*	m_pPrevRasterizerState;		// 退避したラスタライザステート.
	bool					m_PrevIsDepth;				// 退避した深度テスト有効か.
	bool					m_PrevIsAlphaBlend;			// 退避したアルファブレンド有効か.
	bool					m_PrevIsAlphaToCoverage;	// 退避したアルファトゥカバレージ有効か.

	float	m_Time;						// 経過時間(秒).
	UINT	m_Width;					// 作成済みレンダーテクスチャの幅.
	UINT	m_Height;					// 作成済みレンダーテクスチャの高さ.
	bool	m_IsInit;					// 初期化済みか.
	bool	m_IsInitFailed;				// 初期化に失敗したか.
	bool	m_IsScreenAppliedThisFrame;	// このフレームで画面全体エフェクトを適用済みか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	VisualEffectManager( const VisualEffectManager& )				= delete;
	VisualEffectManager& operator = ( const VisualEffectManager& )	= delete;
	VisualEffectManager( VisualEffectManager&& )					= delete;
	VisualEffectManager& operator = ( VisualEffectManager&& )		= delete;
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
