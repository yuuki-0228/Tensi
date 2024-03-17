#pragma once
#include "..\Global.h"
#include "DirectX\DirectX11.h"

/************************************************
*	デバイスを使用する系の基盤クラス.
**/
class CCommon
{
public:
	CCommon();
	virtual ~CCommon();

	// 色の設定.
	inline void SetColor( const D3DXCOLOR3& Color ) { m_Color = Color4::RGBA( Color, m_Color.w ); }
	inline void SetColor( const D3DXCOLOR4& Color ) { m_Color = Color; }
	// アルファ値の設定.
	void SetAlpha( const float Alpha );

	// トランスフォームの設定.
	inline void SetTransform(	const STransform& t ) { m_Transform = t; }
	// 座標設定.
	inline void SetPosition( const D3DXPOSITION3& p							) { m_Transform.Position = p; }
	inline void SetPosition( const float x, const float y, const float z	) { m_Transform.Position = { x, y, z }; }
	// 回転設定.
	inline void SetRotation( const D3DXROTATION3& r							) { m_Transform.Rotation = r; }
	inline void SetRotation( const float x, const float y, const float z	) { m_Transform.Rotation = { x, y, z }; }
	// 拡縮設定.
	inline void SetScale(	 const D3DXSCALE3& s							) { m_Transform .Scale = s; }
	inline void SetScale(	 const float x, const float y, const float z	) { m_Transform .Scale = { x, y, z }; }


	// トランスフォームの取得.
	inline STransform		GetTransform() {	return m_Transform; }
	// 座標の取得.
	inline D3DXPOSITION3	GetPosition() {		return m_Transform.Position; }
	// 回転の取得.
	inline D3DXROTATION3	GetRotation() {		return m_Transform.Rotation; }
	// 拡縮の取得.
	inline D3DXSCALE3		GetScale() {		return m_Transform.Scale; }

protected:
	ID3D11Device*			m_pDevice;		// デバイスオブジェクト.
	ID3D11DeviceContext*	m_pContext;		// デバイスコンテキスト.

	STransform				m_Transform;	// トランスフォーム.
	D3DXCOLOR4				m_Color;		// 色.
};
