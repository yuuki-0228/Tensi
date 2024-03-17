#pragma once
#include "..\..\Global.h"
#include <cmath>

/************************************************
*	�I���W�i�������ꗗ.
*		���c�F�P.
**/
namespace Math {
	constexpr float PI			= 3.14159265f;				// ��.
	constexpr float RADIAN_MAX	= 360.0f * ( PI / 180.0f );	// ���W�A���̍ő�l( 360�� ).
	constexpr float RADIAN_MID	= 180.0f * ( PI / 180.0f );	// ���W�A���̒��Ԓl( 180�� ).
	constexpr float RADIAN_MIN	= 0.0f;						// ���W�A���̍ŏ��l(  0��  ).
	constexpr float DEGREE_MAX	= 360.0f;					// �f�O���[�̍ő�l( 360�� ).
	constexpr float DEGREE_MID	= 180.0f;					// �f�O���[�̒��Ԓl( 180�� ).
	constexpr float DEGREE_MIN	= 0.0f;						// �f�O���[�̍ŏ��l(  0��  ).

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

	// �x��(�x) �� ���W�A��(rad) �ɕϊ�.
	D3DXVECTOR4	ToRadian( const D3DXVECTOR4& deg	);
	D3DXVECTOR3	ToRadian( const D3DXVECTOR3& deg	);
	D3DXVECTOR2	ToRadian( const D3DXVECTOR2& deg	);
	D3DXVECTOR4	ToRadian( const float degX, const float degY, const float degZ, const float degW );
	D3DXVECTOR3	ToRadian( const float degX, const float degY, const float degZ );
	D3DXVECTOR2	ToRadian( const float degX, const float degY );
	float		ToRadian( const float deg );

	// ���W�A��(rad) �� �x��(�x) �ɕϊ�.
	D3DXVECTOR4	ToDegree( const D3DXVECTOR4& red );
	D3DXVECTOR3	ToDegree( const D3DXVECTOR3& red );
	D3DXVECTOR2	ToDegree( const D3DXVECTOR2& red );
	D3DXVECTOR4	ToDegree( const float redX, const float redY, const float redZ, const float redW );
	D3DXVECTOR3	ToDegree( const float redX, const float redY, const float redZ );
	D3DXVECTOR2	ToDegree( const float redX, const float redY );
	float		ToDegree( const float red );

	// ���[���h�s�񂩂���W���擾.
	D3DXVECTOR3 GetPosFromWorldMatrix( const D3DXMATRIX& mWorld );

	// ������n�ʂŎl�̌ܓ�����.
	D3DXVECTOR3 Round_n( const D3DXVECTOR3& Num, const int n );
	float		Round_n( const float		Num, const int n );

	// �K��.
	float Factorial( const int n );
	// �񍀌W��.
	float Binomial( const int n, const int k );
	// �x�W�F�Ȑ�.
	D3DXVECTOR3	Evaluate( const std::vector<D3DXVECTOR3>& ControlPoints, const float NowPoint );

	// �x�N�g���ɑ΂��Ē����̃x�N�g�����擾.
	D3DXVECTOR3 GetOrthogonalVector( const D3DXVECTOR3& v );
	// u�̕�����v�̕����Ɠ����ɂ��邽�߂�D3DXQUATERNION���擾.
	D3DXQUATERNION GetRotationQuaternion( const D3DXVECTOR3& _u, const D3DXVECTOR3& _v );
	// �����_������悤�ɉ�]������s����擾.
	D3DXMATRIX GetLookRotationMatrix( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Look, const D3DXVECTOR3& SkyVector = Y_AXIS );
	// �x�N�g���̌�_���擾.
	bool GetIntersection( D3DXVECTOR3* pOut, const D3DXVECTOR3& Start_A, const D3DXVECTOR3& End_A, const D3DXVECTOR3& Start_B, const D3DXVECTOR3& End_B );
	// �x�N�g���ɑ΂��č��E�ǂ���ɂ��邩�擾.
	std::string GetVectorLRDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos = { 0.0f, 0.0f, 0.0f } );
	// �x�N�g���ɑ΂��đO���ǂ���ɂ��邩�擾.
	std::string GetVectorFBDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos = { 0.0f, 0.0f, 0.0f } );
	// �x�N�g���̊p�x���擾.
	float GetVectorAngle( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB );
	// �x�N�g���̉�].
	void VectorRotation( D3DXVECTOR3* pOut, const D3DXVECTOR3& Rotation );
	D3DXVECTOR3 VectorRotation( const D3DXVECTOR3& Vec, const D3DXVECTOR3& Rotation );

	// �ǂ���x�N�g���̎擾.
	D3DXVECTOR3 GetWallScratchVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut = nullptr );
	// ���˃x�N�g���̎擾.
	D3DXVECTOR3 GetReflectVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut = nullptr );
}