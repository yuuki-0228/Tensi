#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_PHYSICS
#include "..\Physics.h"
#include "..\..\FrameRate\FrameRate.h"

/************************************************
*	物理挙動をまとめた物理ボディ構造体.
*	 速度・力・重力・減衰・摩擦を持ち、Update で積分する.
*	 2D / 3D 共通のテンプレート( 下の using を使用する ).
**/
template<class Vec>
struct stPhysicsBodyT
{
	Vec		Position;		// 座標.
	Vec		Velocity;		// 速度.
	Vec		AddedForce;		// このフレームに加える力の合計( 毎フレームリセットされる ).
	Vec		GravityDir;		// 重力方向.
	float	GravityAccel;	// 重力加速度.
	float	GravityScale;	// 重力倍率( 0 で重力無効 ).
	float	Mass;			// 質量( 0 より大きい値 ).
	float	Damping;		// 減衰係数( 空気抵抗 : 0 で無効 ).
	float	FrictionAccel;	// 摩擦の減速度( 0 で無効 ).
	float	Restitution;	// 反発係数( 0 : 跳ねない ～ 1 : 完全反射 ).

	stPhysicsBodyT()
		: Position		( Physics::GetZeroVector<Vec>()			)
		, Velocity		( Physics::GetZeroVector<Vec>()			)
		, AddedForce	( Physics::GetZeroVector<Vec>()			)
		, GravityDir	( Physics::GetDefaultGravityDir<Vec>()	)
		, GravityAccel	( Physics::GRAVITY_ACCEL				)
		, GravityScale	( 1.0f									)
		, Mass			( 1.0f									)
		, Damping		( 0.0f									)
		, FrictionAccel	( 0.0f									)
		, Restitution	( 0.5f									)
	{
	}

	// 力を加える( 継続的な力 : 質量が大きいほど効きにくい ).
	void AddForce( const Vec& Force )
	{
		AddedForce += Force;
	}
	// 撃力を加える( 瞬間的な力 : 速度が直接変わる ).
	void AddImpulse( const Vec& Impulse )
	{
		if ( Mass <= 0.0f ) return;
		Velocity += Impulse / Mass;
	}

	// 更新 : 力と重力から速度・座標を積分する.
	void Update( const float DeltaTime = GetDeltaTime() )
	{
		if ( Mass <= 0.0f ) return;

		// 加速度 = 外力 / 質量 + 重力.
		const Vec Accel = AddedForce / Mass + GravityDir * ( GravityAccel * GravityScale );
		Physics::Integrate( &Position, &Velocity, Accel, DeltaTime );

		// 減衰と摩擦.
		if ( Damping		> 0.0f ) Velocity = Physics::GetDampedVelocity(		Velocity, Damping,			DeltaTime );
		if ( FrictionAccel	> 0.0f ) Velocity = Physics::GetFrictionVelocity(	Velocity, FrictionAccel,	DeltaTime );

		// 加えた力は毎フレームリセットする.
		AddedForce = Physics::GetZeroVector<Vec>();
	}

	// バウンドさせる( Normal : 衝突面の法線 ).
	//	TangentRate : 接線方向( 壁ずり方向 )の速度維持率.
	void Bound( const Vec& Normal, const float TangentRate = 1.0f )
	{
		Velocity = Physics::GetBounceVelocity( Velocity, Normal, Restitution, TangentRate );
	}

	// 相手の物理ボディと衝突させる( 運動量保存則で速度を再配分する ).
	//	Normal : 相手から自分へ向かう衝突法線.
	void Collide( stPhysicsBodyT<Vec>* pOther, const Vec& Normal )
	{
		Physics::ResolveCollision( &Velocity, &pOther->Velocity, Normal, Mass, pOther->Mass, Restitution );
	}

	// 停止 : 速度と力をリセットする.
	void Stop()
	{
		Velocity	= Physics::GetZeroVector<Vec>();
		AddedForce	= Physics::GetZeroVector<Vec>();
	}

	// 速度の大きさを取得.
	float GetSpeed() const
	{
		return Physics::GetLength( Velocity );
	}
};

// 2D用の物理ボディ.
using SPhysicsBody2D = stPhysicsBodyT<D3DXVECTOR2>;
// 3D用の物理ボディ.
using SPhysicsBody3D = stPhysicsBodyT<D3DXVECTOR3>;

#endif