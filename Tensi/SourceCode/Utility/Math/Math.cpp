#include "Math.h"

//----------------------------.
// �x��(�x) �� ���W�A��(rad) �ɕϊ�.
//----------------------------.
D3DXVECTOR4 Math::ToRadian( const D3DXVECTOR4& deg )
{
	return D3DXVECTOR4(
		Math::ToRadian( deg.x ),
		Math::ToRadian( deg.y ),
		Math::ToRadian( deg.z ),
		Math::ToRadian( deg.w )
	);
}
D3DXVECTOR3 Math::ToRadian( const D3DXVECTOR3& deg )
{
	return D3DXVECTOR3(
		Math::ToRadian( deg.x ),
		Math::ToRadian( deg.y ),
		Math::ToRadian( deg.z )
	);
}
D3DXVECTOR2 Math::ToRadian( const D3DXVECTOR2& deg )
{
	return D3DXVECTOR2(
		Math::ToRadian( deg.x ),
		Math::ToRadian( deg.y )
	);
}
D3DXVECTOR4 Math::ToRadian( const float degX, const float degY, const float degZ, const float degW )
{
	return D3DXVECTOR4(
		Math::ToRadian( degX ),
		Math::ToRadian( degY ),
		Math::ToRadian( degZ ),
		Math::ToRadian( degW )
	);
}
D3DXVECTOR3 Math::ToRadian( const float degX, const float degY, const float degZ )
{
	return D3DXVECTOR3(
		Math::ToRadian( degX ),
		Math::ToRadian( degY ),
		Math::ToRadian( degZ )
	);
}
D3DXVECTOR2 Math::ToRadian( const float degX, const float degY )
{
	return D3DXVECTOR2(
		Math::ToRadian( degX ),
		Math::ToRadian( degY )
	);
}
float Math::ToRadian( const float deg )
{
	return deg * ( PI / 180.0f );
}

//----------------------------.
// ���W�A��(rad) �� �x��(�x) �ɕϊ�.
//----------------------------.
D3DXVECTOR4 Math::ToDegree( const D3DXVECTOR4& red )
{
	return D3DXVECTOR4(
		Math::ToDegree( red.x ),
		Math::ToDegree( red.y ),
		Math::ToDegree( red.z ),
		Math::ToDegree( red.w )
	);
}
D3DXVECTOR3 Math::ToDegree( const D3DXVECTOR3& red )
{
	return D3DXVECTOR3(
		Math::ToDegree( red.x ),
		Math::ToDegree( red.y ),
		Math::ToDegree( red.z )
	);
}
D3DXVECTOR2 Math::ToDegree( const D3DXVECTOR2& red )
{
	return D3DXVECTOR2(
		Math::ToDegree( red.x ),
		Math::ToDegree( red.y )
	);
}
D3DXVECTOR4 Math::ToDegree( const float redX, const float redY, const float redZ, const float redW )
{
	return D3DXVECTOR4(
		Math::ToDegree( redX ),
		Math::ToDegree( redY ),
		Math::ToDegree( redZ ),
		Math::ToDegree( redW )
	);
}
D3DXVECTOR3 Math::ToDegree( const float redX, const float redY, const float redZ )
{
	return D3DXVECTOR3(
		Math::ToDegree( redX ),
		Math::ToDegree( redY ),
		Math::ToDegree( redZ )
	);
}
D3DXVECTOR2 Math::ToDegree( const float redX, const float redY )
{
	return D3DXVECTOR2(
		Math::ToDegree( redX ),
		Math::ToDegree( redY )
	);
}
float Math::ToDegree( const float red )
{
	return red * ( 180.0f / PI );
}

//---------------------------.
// ���[���h�s�񂩂���W���擾.
//---------------------------.
D3DXVECTOR3 Math::GetPosFromWorldMatrix( const D3DXMATRIX& mWorld )
{
	return D3DXVECTOR3( mWorld._41, mWorld._42, mWorld._43 );
}

//---------------------------.
// ������n�ʂŎl�̌ܓ�����.
//---------------------------.
D3DXVECTOR3 Math::Round_n( const D3DXVECTOR3& Num, const int n )
{
	return D3DXVECTOR3(
		Round_n( Num.x, n ),
		Round_n( Num.y, n ),
		Round_n( Num.z, n )
	);
}
float Math::Round_n( const float Num, const int n )
{
	float EndNum	 = Num * static_cast<float>( std::pow( 10, n - 1 ) );	// �l�̌ܓ��������l��10��(n-1)��{����.
	EndNum			 = std::round( static_cast<float>( EndNum ) );			// �����_�ȉ����l�̌ܓ�����.
	EndNum			/= static_cast<float>( std::pow( 10, n - 1 ) );			// 10��(n-1)��Ŋ���.
	return EndNum;
}

//---------------------------.
// �K��.
//---------------------------.
float Math::Factorial( const int n ) {
	float Out = 1.0f;
	for ( int i = 1; i <= n; ++i ) Out *= i;
	return Out;
}

//---------------------------.
// �񍀌W��.
//---------------------------.
float Math::Binomial( const int n, const int k ) {
	return Math::Factorial( n ) / ( Math::Factorial( k ) * Math::Factorial( n - k ) );
}

//----------------------------.
// �x�W�F�Ȑ�.
//----------------------------.
D3DXVECTOR3 Math::Evaluate( const std::vector<D3DXVECTOR3>& ControlPoints, const float NowPoint )
{
	D3DXVECTOR3 OutPos = { 0.0f, 0.0f, 0.0f };

	// �o�[���X�^�C�������g�p���Čv�Z����.
	const int n = static_cast<int>( ControlPoints.size() );
	for ( int i = 0; i < n; i++ ) 
		OutPos += ControlPoints[i] * ( Math::Binomial( n - 1, i ) * static_cast<float>( std::pow( NowPoint, i ) * std::pow( 1 - NowPoint, n - 1 - i ) ) );
	return OutPos;
}

//----------------------------.
// �x�N�g���ɑ΂��Ē����̃x�N�g�����擾.
//----------------------------.
D3DXVECTOR3 Math::GetOrthogonalVector( const D3DXVECTOR3& v )
{
	const float x = std::abs( v.x );
	const float y = std::abs( v.y );
	const float z = std::abs( v.z );

	D3DXVECTOR3 vOther = x < y ? ( x < z ? X_AXIS : Z_AXIS ) : ( y < z ? Y_AXIS : Z_AXIS );

	D3DXVECTOR3 vResult;
	D3DXVec3Cross( &vResult, &v, &vOther );

	return vResult;
}

//----------------------------.
// u�̕�����v�̕����Ɠ����ɂ��邽�߂�D3DXQUATERNION���擾.
//----------------------------.
D3DXQUATERNION Math::GetRotationQuaternion( const D3DXVECTOR3& _u, const D3DXVECTOR3& _v )
{
	D3DXQUATERNION	q;
	D3DXVECTOR3		u = _u, v = _v;
	// �x�N�g���𐳋K��.
	D3DXVec3Normalize( &u, &u );
	D3DXVec3Normalize( &v, &v );

	// �G�b�W�P�[�X.
	const float UoV = D3DXVec3Dot( &u, &v );
	if( UoV <= -1.0f + FLT_EPSILON ){ // FLT_EPSILON�ŕ��������_���ΐ��x���l��.
		// �����̎���180�x��].
		D3DXVECTOR3 vOrthogonal = Math::GetOrthogonalVector( u );
		D3DXVec3Normalize( &vOrthogonal, &vOrthogonal );

		q.x = vOrthogonal.x;
		q.y = vOrthogonal.y;
		q.z = vOrthogonal.z;
		q.w = 0.0f;
	} 
	// �ʏ�P�[�X.
	else {
		D3DXVECTOR3 vHalf;
		D3DXVECTOR3 vUV = u + v;
		D3DXVec3Normalize( &vHalf, &vUV );

		D3DXVECTOR3 vCrossUHalf;
		D3DXVec3Cross( &vCrossUHalf, &u, &vHalf );

		q.x = vCrossUHalf.x;
		q.y = vCrossUHalf.y;
		q.z = vCrossUHalf.z;
		q.w = D3DXVec3Dot( &u, &vHalf );
	}
	return q;
}

//---------------------------.
// �����_������悤�ɉ�]������s����擾.
//---------------------------.
D3DXMATRIX Math::GetLookRotationMatrix( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Look, const D3DXVECTOR3& SkyVector )
{
	D3DXVECTOR3 X, Y, Z;
	Z = Look - D3DXVECTOR3( 0.0f, 2.0f, 0.0f ) - Pos;
	D3DXVec3Normalize( &Z, &Z );
	D3DXVec3Cross( &X, D3DXVec3Normalize( &Y, &SkyVector ), &Z );
	D3DXVec3Normalize( &X, &X );
	D3DXVec3Normalize( &Y, D3DXVec3Cross( &Y, &Z, &X ) );

	D3DXMATRIX Out;
	Out._11 = X.x;	Out._12 = X.y;	Out._13 = X.z;	Out._14 = 0;
	Out._21 = Y.x;	Out._22 = Y.y;	Out._23 = Y.z;	Out._24 = 0;
	Out._31 = Z.x;	Out._32 = Z.y;	Out._33 = Z.z;	Out._34 = 0;
	Out._41 = 0.0f; Out._42 = 0.0f; Out._43 = 0.0f; Out._44 = 1.0f;
	return Out;
}

//---------------------------.
// �x�N�g���̌�_���擾.
//---------------------------.
bool Math::GetIntersection( D3DXVECTOR3* pOut, const D3DXVECTOR3& Start_A, const D3DXVECTOR3& End_A, const D3DXVECTOR3& Start_B, const D3DXVECTOR3& End_B )
{
	if ( Start_A == End_A || Start_B == End_B ) return false;
	D3DXVECTOR3 vA = End_A - Start_A;
	D3DXVECTOR3 vB = End_B - Start_B;
	D3DXVec3Normalize( &vA, &vA );
	D3DXVec3Normalize( &vB, &vB );

	// 2���������s�ȏꍇ�̋������`����.
	if ( std::abs( 1.0f - std::abs( D3DXVec3Dot( &vA, &vB ) ) ) < 1e-5f ) return false;

	D3DXVECTOR3 vAC = Start_B - Start_A;
	const float d1	= (  D3DXVec3Dot( &vA, &vAC ) - D3DXVec3Dot( &vB, &vAC ) * D3DXVec3Dot( &vA, &vB ) ) / ( 1 - D3DXVec3Dot( &vA, &vB ) * D3DXVec3Dot( &vA, &vB ) );
	const float d2	= ( -D3DXVec3Dot( &vB, &vAC ) + D3DXVec3Dot( &vA, &vAC ) * D3DXVec3Dot( &vA, &vB ) ) / ( 1 - D3DXVec3Dot( &vA, &vB ) * D3DXVec3Dot( &vA, &vB ) );

	// ��_��CD��ɂȂ��ꍇ�̋������`����.
	D3DXVECTOR3 vCD = End_B - Start_B;
	if (		d1 < 0 || d2 < 0			) return false;	// d1�܂���d2���}�C�i�X�l�Ȃ�|�C���gB��Ԃ�.
	else if (	d2 > D3DXVec3Length( &vCD )	) return false;	// d2��CD��蒷����΃|�C���gB��Ԃ�.

	// �D���Ȃ悤��p1,p2����ǂ̂悤�Ȍ`�Ō�_���Z�o���邩��`����.
	//*pOut = Start_A + d1 * vA;
	*pOut = Start_B + d2 * vB;
	return true;
}

//---------------------------.
// �x�N�g���ɑ΂��č��E�ǂ���ɂ��邩�擾.
//---------------------------.
std::string Math::GetVectorLRDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos )
{
	std::string Dir = "";

	// �O�ς��g�p���č��E�ǂ���ɂ��邩���ׂ�.
	const D3DXVECTOR3& a = VecStartPos;
	const D3DXVECTOR3& b = VecStartPos - Vec;
	const D3DXVECTOR3& c = Pos;
	const float p = ( b.x - a.x ) * ( c.z - a.z ) - ( b.z - a.z ) * ( c.x - a.x );

	// +�l�Ȃ獶���A-�l�Ȃ�E���A0�̏ꍇ�����ꏊ.
	if (		p < 0 ) Dir = "Left";
	else if (	p > 0 ) Dir = "Right";
	else				Dir = "";
	return Dir;
}

//---------------------------.
// �x�N�g���ɑ΂��đO���ǂ���ɂ��邩�擾.
//---------------------------.
std::string Math::GetVectorFBDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos )
{
	// �x�N�g�������v����90�x��]������.
	D3DXVECTOR3 SidewaysVec;
	SidewaysVec.x =  Vec.z;
	SidewaysVec.y =  Vec.y;
	SidewaysVec.z = -Vec.x;

	// �����ɂ���Ȃ�O�A�E���ɂ���Ȃ���.
	std::string Dir = GetVectorLRDirection( Pos, SidewaysVec, VecStartPos );
	if (		Dir == "Left"  ) Dir = "Front";
	else if (	Dir == "Right" ) Dir = "Back"; 
	return Dir;
}

//---------------------------.
// �x�N�g���̊p�x���擾.
//---------------------------.
float Math::GetVectorAngle( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB )
{
	// �x�N�g��A��B�̒������v�Z����.
	const float Length_vA = D3DXVec3Length( &vA );
	const float Length_vB = D3DXVec3Length( &vB );

	// ���ςƃx�N�g���������g����cos�Ƃ����߂�.
	const float CosSita = D3DXVec3Dot( &vA, &vB ) / ( Length_vA * Length_vB );

	// cos�Ƃ���Ƃ����߂�
	const float Sita = acos( CosSita );
	return Sita;
}

//---------------------------.
// �x�N�g���̉�].
//---------------------------.
void Math::VectorRotation( D3DXVECTOR3* pOut, const D3DXVECTOR3& Rotation )
{
	*pOut = VectorRotation( *pOut, Rotation );
}
D3DXVECTOR3 Math::VectorRotation( const D3DXVECTOR3& Vec, const D3DXVECTOR3& Rotation )
{
	// ��]�s��쐬.
	D3DXMATRIX mRot;
	D3DXMatrixRotationYawPitchRoll( &mRot, Rotation.y, Rotation.x, Rotation.z );

	D3DXVECTOR3 Out;
	D3DXVec3TransformCoord( &Out, &Vec, &mRot );
	if ( !std::isfinite( Out.x ) ) Out.x = 0.0f;
	if ( !std::isfinite( Out.y ) ) Out.y = 0.0f;
	if ( !std::isfinite( Out.z ) ) Out.z = 0.0f;
	return Out;
}

//---------------------------.
// �ǂ���x�N�g���̎擾.
//---------------------------.
D3DXVECTOR3 Math::GetWallScratchVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut )
{
	D3DXVECTOR3 Out, Normal_n;
	D3DXVec3Normalize( &Normal_n, &Normal );
	Out = ( Front - D3DXVec3Dot( &Front, &Normal_n ) * Normal_n );
	if ( NormalizeOut != nullptr ) {
		D3DXVec3Normalize( NormalizeOut, &Out );
	}
	return Out;
}

//---------------------------.
// ���˃x�N�g���̎擾.
//---------------------------.
D3DXVECTOR3 Math::GetReflectVector( const D3DXVECTOR3& Front, const D3DXVECTOR3& Normal, D3DXVECTOR3* NormalizeOut )
{
	D3DXVECTOR3 Out, Normal_n;
	D3DXVec3Normalize( &Normal_n, &Normal );
	Out = ( Front - 2.0f * D3DXVec3Dot( &Front, &Normal_n ) * Normal_n );
	if ( NormalizeOut != nullptr ) {
		D3DXVec3Normalize( NormalizeOut, &Out );
	}
	return Out;
}
