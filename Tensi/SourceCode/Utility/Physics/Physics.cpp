#include "Physics.h"
#ifdef ENABLE_PHYSICS
#include <algorithm>

//----------------------------.
// 内積を取得.
//----------------------------.
float Physics::Dot( const D3DXVECTOR2& vA, const D3DXVECTOR2& vB )
{
	return D3DXVec2Dot( &vA, &vB );
}
float Physics::Dot( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB )
{
	return D3DXVec3Dot( &vA, &vB );
}

//----------------------------.
// ベクトルの長さを取得.
//----------------------------.
float Physics::GetLength( const D3DXVECTOR2& v )
{
	return D3DXVec2Length( &v );
}
float Physics::GetLength( const D3DXVECTOR3& v )
{
	return D3DXVec3Length( &v );
}

//----------------------------.
// 正規化したベクトルを取得.
//----------------------------.
D3DXVECTOR2 Physics::Normalize( const D3DXVECTOR2& v )
{
	if ( GetLength( v ) <= 0.0f ) return GetZeroVector<D3DXVECTOR2>();

	D3DXVECTOR2 Out;
	D3DXVec2Normalize( &Out, &v );
	return Out;
}
D3DXVECTOR3 Physics::Normalize( const D3DXVECTOR3& v )
{
	if ( GetLength( v ) <= 0.0f ) return GetZeroVector<D3DXVECTOR3>();

	D3DXVECTOR3 Out;
	D3DXVec3Normalize( &Out, &v );
	return Out;
}

//----------------------------.
// 等速運動.
//----------------------------.
void Physics::Integrate( D3DXVECTOR2* pPos, const D3DXVECTOR2& Velocity, const float DeltaTime )
{
	*pPos += Velocity * DeltaTime;
}
void Physics::Integrate( D3DXVECTOR3* pPos, const D3DXVECTOR3& Velocity, const float DeltaTime )
{
	*pPos += Velocity * DeltaTime;
}

//----------------------------.
// 等加速度運動.
//----------------------------.
void Physics::Integrate( D3DXVECTOR2* pPos, D3DXVECTOR2* pVelocity, const D3DXVECTOR2& Accel, const float DeltaTime )
{
	*pVelocity += Accel * DeltaTime;
	*pPos += *pVelocity * DeltaTime;
}
void Physics::Integrate( D3DXVECTOR3* pPos, D3DXVECTOR3* pVelocity, const D3DXVECTOR3& Accel, const float DeltaTime )
{
	*pVelocity += Accel * DeltaTime;
	*pPos += *pVelocity * DeltaTime;
}

//----------------------------.
// 放物運動.
//----------------------------.
D3DXVECTOR2 Physics::GetProjectilePos( const D3DXVECTOR2& StartPos, const D3DXVECTOR2& InitVelocity, const D3DXVECTOR2& Accel, const float Time )
{
	// p = p0 + v0t + (1/2)at^2.
	return StartPos + InitVelocity * Time + Accel * ( 0.5f * Time * Time );
}
D3DXVECTOR3 Physics::GetProjectilePos( const D3DXVECTOR3& StartPos, const D3DXVECTOR3& InitVelocity, const D3DXVECTOR3& Accel, const float Time )
{
	// p = p0 + v0t + (1/2)at^2.
	return StartPos + InitVelocity * Time + Accel * ( 0.5f * Time * Time );
}

//----------------------------.
// 指定の高さまで跳ぶための初速の大きさを取得.
//----------------------------.
float Physics::GetJumpSpeed( const float Height, const float GravityAccel )
{
	if ( Height <= 0.0f ) return 0.0f;

	return std::sqrt( 2.0f * GravityAccel * Height );
}

//----------------------------.
// 指定の高さを自由落下するのに掛かる時間を取得.
//----------------------------.
float Physics::GetFallTime( const float Height, const float GravityAccel )
{
	if ( Height <= 0.0f || GravityAccel <= 0.0f ) return 0.0f;

	return std::sqrt( 2.0f * Height / GravityAccel );
}

//----------------------------.
// 減衰後の速度を取得.
//----------------------------.
D3DXVECTOR2 Physics::GetDampedVelocity( const D3DXVECTOR2& Velocity, const float Damping, const float DeltaTime )
{
	return Velocity * std::exp( -Damping * DeltaTime );
}
D3DXVECTOR3 Physics::GetDampedVelocity( const D3DXVECTOR3& Velocity, const float Damping, const float DeltaTime )
{
	return Velocity * std::exp( -Damping * DeltaTime );
}

//----------------------------.
// 摩擦後の速度を取得.
//----------------------------.
D3DXVECTOR2 Physics::GetFrictionVelocity( const D3DXVECTOR2& Velocity, const float FrictionAccel, const float DeltaTime )
{
	const float Speed = GetLength( Velocity );
	if ( Speed <= 0.0f ) return Velocity;

	// 減速後の速さが 0 を下回ったら停止する.
	const float NewSpeed = ( std::max )( 0.0f, Speed - FrictionAccel * DeltaTime );
	return Velocity * ( NewSpeed / Speed );
}
D3DXVECTOR3 Physics::GetFrictionVelocity( const D3DXVECTOR3& Velocity, const float FrictionAccel, const float DeltaTime )
{
	const float Speed = GetLength( Velocity );
	if ( Speed <= 0.0f ) return Velocity;

	// 減速後の速さが 0 を下回ったら停止する.
	const float NewSpeed = ( std::max )( 0.0f, Speed - FrictionAccel * DeltaTime );
	return Velocity * ( NewSpeed / Speed );
}

//----------------------------.
// 空気抵抗力を取得.
//----------------------------.
D3DXVECTOR2 Physics::GetDragForce( const D3DXVECTOR2& Velocity, const float DragCoeff )
{
	return -Velocity * ( DragCoeff * GetLength( Velocity ) );
}
D3DXVECTOR3 Physics::GetDragForce( const D3DXVECTOR3& Velocity, const float DragCoeff )
{
	return -Velocity * ( DragCoeff * GetLength( Velocity ) );
}

//----------------------------.
// 終端速度を取得.
//----------------------------.
float Physics::GetTerminalSpeed( const float Mass, const float DragCoeff, const float GravityAccel )
{
	if ( DragCoeff <= 0.0f ) return 0.0f;

	return std::sqrt( Mass * GravityAccel / DragCoeff );
}

//----------------------------.
// 反発係数付きの反射速度を取得.
//----------------------------.
D3DXVECTOR2 Physics::GetBounceVelocity( const D3DXVECTOR2& Velocity, const D3DXVECTOR2& Normal, const float Restitution, const float TangentRate )
{
	// 法線方向と接線方向に分解して個別に係数を掛ける.
	const D3DXVECTOR2 n = Normalize( Normal );
	const D3DXVECTOR2 NormalVel = n * Dot( Velocity, n );
	const D3DXVECTOR2 TangentVel = Velocity - NormalVel;
	return TangentVel * TangentRate - NormalVel * Restitution;
}
D3DXVECTOR3 Physics::GetBounceVelocity( const D3DXVECTOR3& Velocity, const D3DXVECTOR3& Normal, const float Restitution, const float TangentRate )
{
	// 法線方向と接線方向に分解して個別に係数を掛ける.
	const D3DXVECTOR3 n = Normalize( Normal );
	const D3DXVECTOR3 NormalVel = n * Dot( Velocity, n );
	const D3DXVECTOR3 TangentVel = Velocity - NormalVel;
	return TangentVel * TangentRate - NormalVel * Restitution;
}

//----------------------------.
// 2物体の衝突応答.
//----------------------------.
void Physics::ResolveCollision( D3DXVECTOR2* pVelocityA, D3DXVECTOR2* pVelocityB, const D3DXVECTOR2& Normal, const float MassA, const float MassB, const float Restitution )
{
	if ( MassA <= 0.0f || MassB <= 0.0f ) return;

	// 法線方向の相対速度( 離れていく場合は何もしない ).
	const D3DXVECTOR2 n = Normalize( Normal );
	const float RelativeVel = Dot( *pVelocityA - *pVelocityB, n );
	if ( RelativeVel >= 0.0f ) return;

	// 撃力を質量の逆数比で分配する.
	const float InvMassA = 1.0f / MassA;
	const float InvMassB = 1.0f / MassB;
	const float Impulse = -( 1.0f + Restitution ) * RelativeVel / ( InvMassA + InvMassB );
	*pVelocityA += n * ( Impulse * InvMassA );
	*pVelocityB -= n * ( Impulse * InvMassB );
}
void Physics::ResolveCollision( D3DXVECTOR3* pVelocityA, D3DXVECTOR3* pVelocityB, const D3DXVECTOR3& Normal, const float MassA, const float MassB, const float Restitution )
{
	if ( MassA <= 0.0f || MassB <= 0.0f ) return;

	// 法線方向の相対速度( 離れていく場合は何もしない ).
	const D3DXVECTOR3 n = Normalize( Normal );
	const float RelativeVel = Dot( *pVelocityA - *pVelocityB, n );
	if ( RelativeVel >= 0.0f ) return;

	// 撃力を質量の逆数比で分配する.
	const float InvMassA = 1.0f / MassA;
	const float InvMassB = 1.0f / MassB;
	const float Impulse = -( 1.0f + Restitution ) * RelativeVel / ( InvMassA + InvMassB );
	*pVelocityA += n * ( Impulse * InvMassA );
	*pVelocityB -= n * ( Impulse * InvMassB );
}

//----------------------------.
// ばねの加速度を取得.
//----------------------------.
D3DXVECTOR2 Physics::GetSpringAccel( const D3DXVECTOR2& Pos, const D3DXVECTOR2& Target, const D3DXVECTOR2& Velocity, const float Stiffness, const float Damping )
{
	// a = -kx - cv ( 目標へ引き戻す力 + 減衰 ).
	return ( Target - Pos ) * Stiffness - Velocity * Damping;
}
D3DXVECTOR3 Physics::GetSpringAccel( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Target, const D3DXVECTOR3& Velocity, const float Stiffness, const float Damping )
{
	// a = -kx - cv ( 目標へ引き戻す力 + 減衰 ).
	return ( Target - Pos ) * Stiffness - Velocity * Damping;
}

//----------------------------.
// ばね運動の更新.
//----------------------------.
void Physics::UpdateSpring( D3DXVECTOR2* pPos, D3DXVECTOR2* pVelocity, const D3DXVECTOR2& Target, const float Stiffness, const float Damping, const float DeltaTime )
{
	const D3DXVECTOR2 Accel = GetSpringAccel( *pPos, Target, *pVelocity, Stiffness, Damping );
	Integrate( pPos, pVelocity, Accel, DeltaTime );
}
void Physics::UpdateSpring( D3DXVECTOR3* pPos, D3DXVECTOR3* pVelocity, const D3DXVECTOR3& Target, const float Stiffness, const float Damping, const float DeltaTime )
{
	const D3DXVECTOR3 Accel = GetSpringAccel( *pPos, Target, *pVelocity, Stiffness, Damping );
	Integrate( pPos, pVelocity, Accel, DeltaTime );
}

//----------------------------.
// 振り子の角加速度を取得.
//----------------------------.
float Physics::GetPendulumAccel( const float Angle, const float Length, const float GravityAccel )
{
	if ( Length <= 0.0f ) return 0.0f;

	return -( GravityAccel / Length ) * std::sin( Angle );
}

//----------------------------.
// 単振動.
//----------------------------.
float Physics::GetSimpleHarmonic( const float Amplitude, const float AngularVelocity, const float Time, const float Phase )
{
	return Amplitude * std::sin( AngularVelocity * Time + Phase );
}

#endif