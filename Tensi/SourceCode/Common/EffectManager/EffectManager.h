#pragma once
#include "..\..\Global.h"
#include "Effect\Effect.h"

/************************************************
*	エフェクトマネージャー.
**/
class CEffectManager
{
public:
	CEffectManager();
	~CEffectManager();

	// エフェクトの設定.
	bool SetEffect( const std::string& Name );

	// 描画.
	void Render();

	// 再生.
	bool Play( const STransform& Trans );
	bool Play( const D3DXPOSITION3& Pos, const D3DXROTATION3& Rot = { 0.0f, 0.0f, 0.0f }, const D3DXSCALE3& Scale = { 1.0f, 1.0f, 1.0f } );

	// 停止.
	void Stop();
	// 徐々に停止させる.
	void StopRoot();
	// 全て停止.
	void StopAll();

	// 一時停止もしくは再開する.
	void Pause( const bool& Flag );
	// すべてを一時停止もしくは再開する.
	void AllPause( const bool& Flag );

	// エフェクトに使用されているインスタンス数を取得.
	int GetTotalInstanceCount();

	// トランスフォームの取得.
	inline STransform  GetTransform() { return m_Transform; }
	// 座標の取得.
	inline D3DXPOSITION3 GetPosition() { return m_Transform.Position; }
	// 回転の取得.
	inline D3DXROTATION3 GetRotation() { return m_Transform.Rotation; }
	// 拡縮の取得.
	inline D3DXSCALE3 GetScale() { return m_Transform.Scale; }

	// 存在しているか.
	bool IsShow();

	// 位置を指定する.
	void SetPosition( const D3DXPOSITION3& Pos );
	// 回転を指定する.
	void SetRotation( const D3DXROTATION3& Rot );
	// サイズを指定する.
	void SetScale( const D3DXSCALE3& Scale );
	void SetScale( const float& Scale );
	// 再生速度を設定する.
	void SetSpeed( const float& Speed );
	// 色の設定.
	void SetColor( const D3DXCOLOR3& Color );
	void SetColor( const D3DXCOLOR4& Color );
	// アルファ値の設定.
	void SetAlpha( const float& Alpha );

private:
	CEffect*			m_pEffect;		// エフェクトクラス.
	Effekseer::Handle	m_Handle;		// エフェクトハンドル.
	STransform			m_Transform;	// トランスフォーム.
};
