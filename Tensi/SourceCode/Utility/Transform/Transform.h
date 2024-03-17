#pragma once
#include "..\..\Global.h"
#include "..\Math\Math.h"
#include <algorithm>

using D3DXPOSITION2 = D3DXVECTOR2;
using D3DXPOSITION3 = D3DXVECTOR3;
using D3DXPOSITION4 = D3DXVECTOR4;
using D3DXROTATION2 = D3DXVECTOR2;
using D3DXROTATION3 = D3DXVECTOR3;
using D3DXROTATION4 = D3DXVECTOR4;
using D3DXSCALE2	= D3DXVECTOR2;
using D3DXSCALE3	= D3DXVECTOR3;
using D3DXSCALE4	= D3DXVECTOR4;

/************************************************
*	トランスフォーム構造体.
*		﨑田友輝.
**/
struct stTransform
{
	D3DXPOSITION3	Position;			// ローカル座標( 親がいない場合ワールド座標 ).
	D3DXROTATION3	Rotation;			// ローカル回転( 親がいない場合ワールド回転 ).
	D3DXSCALE3		Scale;				// ローカル拡縮( 親がいない場合ワールド拡縮 ).
	D3DXPOSITION3	OldPosition;		// 前のフレームのローカル座標( 親がいない場合ワールド座標 ).
	D3DXROTATION3	OldRotation;		// 前のフレームのローカル回転( 親がいない場合ワールド回転 ).
	D3DXSCALE3		OldScale;			// 前のフレームのローカル拡縮( 親がいない場合ワールド拡縮 ).
	stTransform*	pParent;			// 親(「AttachParent() / AddParent()」を使用して設定する ).
	bool			IsPostionRender;	// 座標を描画するか.

	stTransform();
	stTransform(
		const D3DXPOSITION3&	NewPos,
		const D3DXROTATION3&	NewRot,
		const D3DXSCALE3&		NewScale,
		stTransform*			pNewParent = nullptr );

	// ワールド座標を設定.
	void SetWorldPosition( const D3DXPOSITION3& WorldPos );
	// ワールド回転を設定.
	void SetWorldRotation( const D3DXROTATION3& WorldRot );
	// ワールド拡縮を設定.
	void SetWorldScale( const D3DXSCALE3& WorldScale );

	// 度数で回転を取得.
	D3DXROTATION3 GetDegRotation();
	// ラジアンで回転を取得する.
	D3DXROTATION3 GetRadRotation();
	// 回転方向のベクトルを取得する.
	D3DXVECTOR3 GetRotationVector();

	// 特定のトランスフォームからローカルトランスフォームを取得.
	stTransform GetLocalTransform( const stTransform& Parent ) const;
	// 特定の座標からローカル座標を取得.
	D3DXPOSITION3 GetLocalPosition( const D3DXPOSITION3& ParPos ) const;
	// 特定の回転値からローカル回転を取得.
	D3DXROTATION3 GetLocalRotation( const D3DXROTATION3& ParRot ) const;
	// 特定の拡縮からローカル拡縮を取得.
	D3DXSCALE3 GetLocalScale( const D3DXSCALE3& ParScale ) const;

	// ワールド空間のトランスフォームを取得.
	stTransform GetWorldTransform( const D3DXMATRIX* pRot = nullptr ) const;
	// ワールド空間の座標を取得.
	D3DXPOSITION3 GetWorldPosition( const D3DXMATRIX* pRot = nullptr ) const;
	// ワールド空間の回転を取得.
	D3DXROTATION3 GetWorldRotation() const;
	// ワールド空間の拡縮を取得.
	D3DXSCALE3 GetWorldScale() const;
	
	// ワールド行列を取得.
	D3DXMATRIX GetWorldMatrix( const D3DXMATRIX* pRot = nullptr );

	// 親を設定する.
	void AttachParent( stTransform* NewParent );
	// 親を追加する.
	void AddParent( stTransform* NewParent );
	// 親オブジェクトから切り離す.
	void DetachParent();

	// 座標を範囲内に収める.
	void PostionClamp( const float Max );
	void PostionClamp( const float Max,		const float Min );
	void PostionClamp( const float Max_x,	const float Max_y, const float Max_z );
	void PostionClamp( const float Max_x,	const float Min_x, const float Max_y, const float Min_y, const float Max_z, const float Min_z );
	// 回転を範囲内に収める.
	void RotationClamp();

	// オペレーター.
	bool operator==( const stTransform& Trans );
	bool operator!=( const stTransform& Trans );
	void operator+=( const stTransform& Trans );
	void operator-=( const stTransform& Trans );
	void operator*=( const float& f );
	void operator/=( const float& f );
	stTransform operator+( const stTransform& Trans ) const;
	stTransform operator-( const stTransform& Trans ) const;
	stTransform operator*( const float& f ) const;
	stTransform operator/( const float& f ) const;

	// 定数.
	static constexpr float SCALE_MIN			= 0.0f;											// 拡縮の最小値.
	static constexpr float SCALE_MIN_VEC3[3]	= { SCALE_MIN, SCALE_MIN, SCALE_MIN };			// 拡縮の最小値.
	static constexpr float NORMAL_SCALE			= 1.0f;											// 通常の拡縮値.
	static constexpr float NORMAL_SCALE_VEC3[3]	= { NORMAL_SCALE, NORMAL_SCALE, NORMAL_SCALE };	// 通常の拡縮値.
} typedef STransform;