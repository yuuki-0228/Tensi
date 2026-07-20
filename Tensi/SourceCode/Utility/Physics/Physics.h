#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_PHYSICS
#include "..\..\Global.h"
#include "..\Math\Math.h"

/************************************************
*	物理演算関数一覧.
*	 2D( D3DXVECTOR2 ) / 3D( D3DXVECTOR3 ) 両対応.
*	 単位はゲーム内スケールに合わせて引数で調整する.
**/
namespace Physics {
	constexpr float GRAVITY_ACCEL = 9.8f;	// 重力加速度の既定値.

	// 既定の重力方向を取得.
	//	2D : スクリーン座標系( Y+ が下方向 ).
	//	3D : ワールド座標系( Y- が下方向 ).
	template<class Vec> Vec GetDefaultGravityDir();
	template<> inline D3DXVECTOR2 GetDefaultGravityDir<D3DXVECTOR2>() { return D3DXVECTOR2( 0.0f, 1.0f ); }
	template<> inline D3DXVECTOR3 GetDefaultGravityDir<D3DXVECTOR3>() { return D3DXVECTOR3( 0.0f, -1.0f, 0.0f ); }

	// ゼロベクトルを取得.
	template<class Vec> Vec GetZeroVector();
	template<> inline D3DXVECTOR2 GetZeroVector<D3DXVECTOR2>() { return D3DXVECTOR2( 0.0f, 0.0f ); }
	template<> inline D3DXVECTOR3 GetZeroVector<D3DXVECTOR3>() { return D3DXVECTOR3( 0.0f, 0.0f, 0.0f ); }

	//----------------------------.
	//	ベクトル汎用.
	//----------------------------.

	// 内積を取得.
	float Dot( const D3DXVECTOR2& vA, const D3DXVECTOR2& vB );
	float Dot( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB );
	// ベクトルの長さを取得.
	float GetLength( const D3DXVECTOR2& v );
	float GetLength( const D3DXVECTOR3& v );
	// 正規化したベクトルを取得( ゼロベクトルはそのまま返す ).
	D3DXVECTOR2 Normalize( const D3DXVECTOR2& v );
	D3DXVECTOR3 Normalize( const D3DXVECTOR3& v );

	//----------------------------.
	//	基本運動.
	//----------------------------.

	// 等速運動 : 速度で座標を進める.
	void Integrate( D3DXVECTOR2* pPos, const D3DXVECTOR2& Velocity, const float DeltaTime );
	void Integrate( D3DXVECTOR3* pPos, const D3DXVECTOR3& Velocity, const float DeltaTime );
	// 等加速度運動 : 加速度で速度を更新してから座標を進める( 半陰的オイラー法 ).
	void Integrate( D3DXVECTOR2* pPos, D3DXVECTOR2* pVelocity, const D3DXVECTOR2& Accel, const float DeltaTime );
	void Integrate( D3DXVECTOR3* pPos, D3DXVECTOR3* pVelocity, const D3DXVECTOR3& Accel, const float DeltaTime );
	// 放物運動 : 開始座標・初速・加速度から経過時間 Time 時点の座標を取得.
	D3DXVECTOR2 GetProjectilePos( const D3DXVECTOR2& StartPos, const D3DXVECTOR2& InitVelocity, const D3DXVECTOR2& Accel, const float Time );
	D3DXVECTOR3 GetProjectilePos( const D3DXVECTOR3& StartPos, const D3DXVECTOR3& InitVelocity, const D3DXVECTOR3& Accel, const float Time );
	// 指定の高さまで跳ぶための初速の大きさを取得( v = √( 2gh ) ).
	float GetJumpSpeed( const float Height, const float GravityAccel = GRAVITY_ACCEL );
	// 指定の高さを自由落下するのに掛かる時間を取得( t = √( 2h / g ) ).
	float GetFallTime( const float Height, const float GravityAccel = GRAVITY_ACCEL );

	//----------------------------.
	//	抵抗・摩擦.
	//----------------------------.

	// 減衰後の速度を取得( 空気抵抗など ).
	//	フレームレートに依存しない指数減衰( Damping が大きいほど早く止まる ).
	D3DXVECTOR2 GetDampedVelocity( const D3DXVECTOR2& Velocity, const float Damping, const float DeltaTime );
	D3DXVECTOR3 GetDampedVelocity( const D3DXVECTOR3& Velocity, const float Damping, const float DeltaTime );
	// 摩擦後の速度を取得 : 一定の減速度で減速し 0 で停止する.
	D3DXVECTOR2 GetFrictionVelocity( const D3DXVECTOR2& Velocity, const float FrictionAccel, const float DeltaTime );
	D3DXVECTOR3 GetFrictionVelocity( const D3DXVECTOR3& Velocity, const float FrictionAccel, const float DeltaTime );
	// 空気抵抗力を取得( F = -k * v * |v| ).
	D3DXVECTOR2 GetDragForce( const D3DXVECTOR2& Velocity, const float DragCoeff );
	D3DXVECTOR3 GetDragForce( const D3DXVECTOR3& Velocity, const float DragCoeff );
	// 終端速度を取得( v = √( mg / k ) ).
	float GetTerminalSpeed( const float Mass, const float DragCoeff, const float GravityAccel = GRAVITY_ACCEL );

	//----------------------------.
	//	反発・バウンド.
	//----------------------------.

	// 反発係数付きの反射速度を取得.
	//	Normal      : 衝突面の法線.
	//	Restitution : 反発係数( 0 : 跳ねない ～ 1 : 完全反射 ).
	//	TangentRate : 接線方向( 壁ずり方向 )の速度維持率( 摩擦で減らす場合は 1 未満にする ).
	D3DXVECTOR2 GetBounceVelocity( const D3DXVECTOR2& Velocity, const D3DXVECTOR2& Normal, const float Restitution, const float TangentRate = 1.0f );
	D3DXVECTOR3 GetBounceVelocity( const D3DXVECTOR3& Velocity, const D3DXVECTOR3& Normal, const float Restitution, const float TangentRate = 1.0f );
	// 2物体の衝突応答( 運動量保存則 ).
	//	法線方向の速度を質量比で再配分する( 離れていく場合は何もしない ).
	//	Normal : B から A へ向かう衝突法線.
	void ResolveCollision( D3DXVECTOR2* pVelocityA, D3DXVECTOR2* pVelocityB, const D3DXVECTOR2& Normal, const float MassA, const float MassB, const float Restitution );
	void ResolveCollision( D3DXVECTOR3* pVelocityA, D3DXVECTOR3* pVelocityB, const D3DXVECTOR3& Normal, const float MassA, const float MassB, const float Restitution );

	//----------------------------.
	//	ばね・振り子.
	//----------------------------.

	// ばねの加速度を取得( フックの法則 + 減衰 ).
	//	Stiffness : ばねの硬さ, Damping : 減衰係数.
	D3DXVECTOR2 GetSpringAccel( const D3DXVECTOR2& Pos, const D3DXVECTOR2& Target, const D3DXVECTOR2& Velocity, const float Stiffness, const float Damping );
	D3DXVECTOR3 GetSpringAccel( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Target, const D3DXVECTOR3& Velocity, const float Stiffness, const float Damping );
	// ばね運動の更新 : 目標座標に向かって座標と速度をまとめて更新する.
	void UpdateSpring( D3DXVECTOR2* pPos, D3DXVECTOR2* pVelocity, const D3DXVECTOR2& Target, const float Stiffness, const float Damping, const float DeltaTime );
	void UpdateSpring( D3DXVECTOR3* pPos, D3DXVECTOR3* pVelocity, const D3DXVECTOR3& Target, const float Stiffness, const float Damping, const float DeltaTime );
	// 振り子の角加速度を取得( α = -( g / L ) * sin(θ) ).
	//	Angle : 真下からの角度(rad), Length : 振り子の長さ.
	float GetPendulumAccel( const float Angle, const float Length, const float GravityAccel = GRAVITY_ACCEL );
	// 単振動 : 経過時間 Time 時点の変位を取得( x = A * sin( ωt + φ ) ).
	float GetSimpleHarmonic( const float Amplitude, const float AngularVelocity, const float Time, const float Phase = 0.0f );
}

#endif