#pragma once
#include "..\..\Global.h"
#include <cmath>

/************************************************
*	オリジナル数式一覧.
*		﨑田友輝.
**/
namespace Math {
	constexpr float PI			= 3.14159265f;				// π.
	constexpr float RADIAN_MAX	= 360.0f * ( PI / 180.0f );	// ラジアンの最大値( 360° ).
	constexpr float RADIAN_MID	= 180.0f * ( PI / 180.0f );	// ラジアンの中間値( 180° ).
	constexpr float RADIAN_MIN	= 0.0f;						// ラジアンの最小値(  0°  ).
	constexpr float DEGREE_MAX	= 360.0f;					// デグリーの最大値( 360° ).
	constexpr float DEGREE_MID	= 180.0f;					// デグリーの中間値( 180° ).
	constexpr float DEGREE_MIN	= 0.0f;						// デグリーの最小値(  0°  ).

	constexpr float  X_AXIS[]	= {  1.0f,  0.0f,  0.0f };
	constexpr float _X_AXIS[]	= { -1.0f,  0.0f,  0.0f };
	constexpr float  Y_AXIS[]	= {  0.0f,  1.0f,  0.0f };
	constexpr float _Y_AXIS[]	= {  0.0f, -1.0f,  0.0f };
	constexpr float  Z_AXIS[]	= {  0.0f,  0.0f,  1.0f };
	constexpr float _Z_AXIS[]	= {  0.0f,  0.0f, -1.0f };
	constexpr float  X_VECTOR[] = {  X_AXIS[0],  X_AXIS[1],  X_AXIS[2] };
	constexpr float _X_VECTOR[]	= { _X_AXIS[0], _X_AXIS[1], _X_AXIS[2] };
	constexpr float  Y_VECTOR[]	= {  Y_AXIS[0],  Y_AXIS[1],  Y_AXIS[2] };
	constexpr float _Y_VECTOR[]	= { _Y_AXIS[0], _Y_AXIS[1], _Y_AXIS[2] };
	constexpr float  Z_VECTOR[]	= {  Z_AXIS[0],  Z_AXIS[1],  Z_AXIS[2] };
	constexpr float _Z_VECTOR[]	= { _Z_AXIS[0], _Z_AXIS[1], _Z_AXIS[2] };

	// 度数(度) を ラジアン(rad) に変換.
	D3DXVECTOR4	ToRadian( const D3DXVECTOR4& deg	);
	D3DXVECTOR3	ToRadian( const D3DXVECTOR3& deg	);
	D3DXVECTOR2	ToRadian( const D3DXVECTOR2& deg	);
	D3DXVECTOR4	ToRadian( const float degX, const float degY, const float degZ, const float degW );
	D3DXVECTOR3	ToRadian( const float degX, const float degY, const float degZ );
	D3DXVECTOR2	ToRadian( const float degX, const float degY );
	float		ToRadian( const float deg );

	// ラジアン(rad) を 度数(度) に変換.
	D3DXVECTOR4	ToDegree( const D3DXVECTOR4& red );
	D3DXVECTOR3	ToDegree( const D3DXVECTOR3& red );
	D3DXVECTOR2	ToDegree( const D3DXVECTOR2& red );
	D3DXVECTOR4	ToDegree( const float redX, const float redY, const float redZ, const float redW );
	D3DXVECTOR3	ToDegree( const float redX, const float redY, const float redZ );
	D3DXVECTOR2	ToDegree( const float redX, const float redY );
	float		ToDegree( const float red );

	// ワールド行列から座標を取得.
	D3DXVECTOR3 GetPosFromWorldMatrix( const D3DXMATRIX& mWorld );

	// 少数第n位で四捨五入する.
	D3DXVECTOR3 Round_n( const D3DXVECTOR3& Num, const int n );
	float		Round_n( const float		Num, const int n );

	// 階乗.
	float Factorial( const int n );
	// 二項係数.
	float Binomial( const int n, const int k );
	// ベジェ曲線.
	D3DXVECTOR3	Evaluate( const std::vector<D3DXVECTOR3>& ControlPoints, const float NowPoint );

	// ベクトルに対して直交のベクトルを取得.
	D3DXVECTOR3 GetOrthogonalVector( const D3DXVECTOR3& v );
	// uの方向をvの方向と同じにするためのD3DXQUATERNIONを取得.
	D3DXQUATERNION GetRotationQuaternion( const D3DXVECTOR3& _u, const D3DXVECTOR3& _v );
	// 注視点を見るように回転させる行列を取得.
	D3DXMATRIX GetLookRotationMatrix( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Look, const D3DXVECTOR3& SkyVector = Y_AXIS );
	// ベクトルの交点を取得.
	bool GetIntersection( D3DXVECTOR3* pOut, const D3DXVECTOR3& Start_A, const D3DXVECTOR3& End_A, const D3DXVECTOR3& Start_B, const D3DXVECTOR3& End_B );
	// ベクトルに対して左右どちらにいるか取得.
	std::string GetVectorLRDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos = { 0.0f, 0.0f, 0.0f } );
	// ベクトルに対して前後ろどちらにいるか取得.
	std::string GetVectorFBDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos = { 0.0f, 0.0f, 0.0f } );
	// ベクトルの角度を取得.
	float GetVectorAngle( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB );
	// ベクトルの回転.
	void VectorRotation( D3DXVECTOR3* pOut, const D3DXVECTOR3& Rotation );
	D3DXVECTOR3 VectorRotation( const D3DXVECTOR3& Vec, const D3DXVECTOR3& Rotation );

	// 壁ずりベクトルの取得.
	D3DXVECTOR3 GetWallScratchVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut = nullptr );
	// 反射ベクトルの取得.
	D3DXVECTOR3 GetReflectVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut = nullptr );
}