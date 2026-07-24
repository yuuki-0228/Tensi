#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_VISUAL_EFFECT
#include "..\..\..\Global.h"
#include "..\VisualEffectTypes.h"
#include <string>
#include <unordered_map>

class CRenderTexture;

/************************************************
*	視覚効果の基底クラス.
*	単パスのエフェクトは GetShaderPath / GetMainEntry / PackParam を
*	実装するだけで動作する. 複数パスが必要な場合は Apply を上書きする.
**/
class CVisualEffectBase
{
public:
	// エフェクト毎のパラメータ定数バッファ( b1 )の float4 数.
	static constexpr int PARAM_VECTOR_NUM = 4;

public:
	CVisualEffectBase();
	virtual ~CVisualEffectBase();

	// 種類の取得.
	virtual EVisualEffect GetType() const = 0;
	// 名前の取得( ImGui表示用 ).
	virtual const char* GetName() const = 0;

	// 初期化( 未初期化なら初期化する. 失敗時は false ).
	bool EnsureInit();

	// 適用 : pSrcSRV の内容にエフェクトをかけて pDstRTV へ描画する.
	virtual void Apply( ID3D11ShaderResourceView* pSrcSRV, ID3D11RenderTargetView* pDstRTV );

	// ImGuiでのパラメータ編集( デバッグ用 ).
	virtual void DebugParamEdit() {}

	// パラメータ構造体へのポインタ取得( VisualEffectManager::Param 用 ).
	virtual void* GetParamPtr() { return nullptr; }

	// 適用率( 0:元画像のまま ～ 1:効果100% ).
	void	SetIntensity( const float Value )	{ m_Intensity = Value; }
	float	GetIntensity() const				{ return m_Intensity; }

	// 適用範囲( px, x:左上x, y:左上y, z:幅, w:高さ ). 幅か高さが0以下で画面全体.
	void		SetArea( const D3DXVECTOR4& Area )	{ m_Area = Area; }
	D3DXVECTOR4	GetArea() const						{ return m_Area; }

	// 透明部分の持ち上げアルファ( 0～1 ).
	//	透明ウィンドウで描画物が無い部分にも効果を見せたい時に0より大きくする.
	//	( 大きくした部分はクリック透過判定にも影響する点に注意 ).
	void	SetAlphaLift( const float Value )	{ m_AlphaLift = Value; }
	float	GetAlphaLift() const				{ return m_AlphaLift; }

protected:
	// 派生側 : シェーダファイルパス.
	virtual const TCHAR* GetShaderPath() const = 0;
	// 派生側 : 単パス用のピクセルシェーダエントリ名( 複数パスの場合は使用しない ).
	virtual const char* GetMainEntry() const { return nullptr; }
	// 派生側 : パラメータを定数バッファ用に詰める( pOut は PARAM_VECTOR_NUM 個 ).
	virtual void PackParam( D3DXVECTOR4* pOut ) const { pOut; }
	// 派生側 : シェーダ以外の初期化( 履歴テクスチャ作成等 ).
	virtual bool InitResource() { return true; }

	// 初期化本体( 既定は GetMainEntry のコンパイル ).
	virtual bool Init();

	// ピクセルシェーダのコンパイル( 結果は内部に保持される. 失敗時は nullptr ).
	ID3D11PixelShader* CompilePS( const TCHAR* pShaderPath, const char* pEntry );
	// コンパイル済みピクセルシェーダの取得.
	ID3D11PixelShader* GetPS( const char* pEntry ) const;

protected:
	std::unordered_map<std::string, ID3D11PixelShader*>	m_pPixelShaders;	// コンパイル済みピクセルシェーダ( エントリ名で管理 ).
	float		m_Intensity;	// 適用率.
	float		m_AlphaLift;	// 透明部分の持ち上げアルファ.
	D3DXVECTOR4	m_Area;			// 適用範囲.
	bool		m_IsInit;		// 初期化済みか.
	bool		m_IsInitFailed;	// 初期化に失敗したか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CVisualEffectBase( const CVisualEffectBase& )				= delete;
	CVisualEffectBase& operator = ( const CVisualEffectBase& )	= delete;
	CVisualEffectBase( CVisualEffectBase&& )					= delete;
	CVisualEffectBase& operator = ( CVisualEffectBase&& )		= delete;
};

/************************************************
*	パラメータ構造体付きの基底クラス.
*	TParam : VisualEffectTypes.h のパラメータ構造体.
**/
template<typename TParam>
class CVisualEffectWithParam
	: public CVisualEffectBase
{
public:
	CVisualEffectWithParam() : m_Param() {}
	virtual ~CVisualEffectWithParam() {}

	// 種類の取得.
	virtual EVisualEffect GetType() const override { return TParam::TYPE; }
	// パラメータ構造体へのポインタ取得.
	virtual void* GetParamPtr() override { return &m_Param; }

	// パラメータの取得.
	TParam& Param() { return m_Param; }

protected:
	TParam m_Param;	// パラメータ.
};

#endif	// #ifdef ENABLE_VISUAL_EFFECT
