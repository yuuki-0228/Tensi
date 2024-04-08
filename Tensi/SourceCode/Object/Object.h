#pragma once
#include "..\Global.h"

/************************************************
*	オブジェクトの基盤クラス.
**/
class CObject
{
public:
	CObject();
	virtual ~CObject();

	// トランスフォームの設定.
	inline void SetTransform( const STransform& t ) { m_Transform = t; }
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
	inline STransform	 GetTransform() {	return m_Transform; }
	// 座標の取得.
	inline D3DXPOSITION3 GetPosition() {	return m_Transform.Position; }
	// 回転の取得.
	inline D3DXROTATION3 GetRotation() {	return m_Transform.Rotation; }
	// 拡縮の取得.
	inline D3DXSCALE3	 GetScale() {		return m_Transform.Scale; }

protected:
	// トランスフォームの更新.
	//	画像のトランスフォームなどで管理している場合.
	//	この関数をUpdate()の最後に呼ぶ.
	virtual void TransformUpdate( const STransform& t ) final { m_Transform = t; }

protected:
	STransform m_Transform;	// トランスフォーム.
};