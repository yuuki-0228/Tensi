#include "Math.h"

//----------------------------.
// 度数(度) を ラジアン(rad) に変換.
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
// ラジアン(rad) を 度数(度) に変換.
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
// ワールド行列から座標を取得.
//---------------------------.
D3DXVECTOR3 Math::GetPosFromWorldMatrix( const D3DXMATRIX& mWorld )
{
	return D3DXVECTOR3( mWorld._41, mWorld._42, mWorld._43 );
}

//---------------------------.
// 少数第n位で四捨五入する.
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
	float EndNum	 = Num * static_cast<float>( std::pow( 10, n - 1 ) );	// 四捨五入したい値を10の(n-1)乗倍する.
	EndNum			 = std::round( static_cast<float>( EndNum ) );			// 小数点以下を四捨五入する.
	EndNum			/= static_cast<float>( std::pow( 10, n - 1 ) );			// 10の(n-1)乗で割る.
	return EndNum;
}

//---------------------------.
// 階乗.
//---------------------------.
float Math::Factorial( const int n ) {
	float Out = 1.0f;
	for ( int i = 1; i <= n; ++i ) Out *= i;
	return Out;
}

//---------------------------.
// 二項係数.
//---------------------------.
float Math::Binomial( const int n, const int k ) {
	return Math::Factorial( n ) / ( Math::Factorial( k ) * Math::Factorial( n - k ) );
}

//----------------------------.
// ベジェ曲線.
//----------------------------.
D3DXVECTOR3 Math::Evaluate( const std::vector<D3DXVECTOR3>& ControlPoints, const float NowPoint )
{
	D3DXVECTOR3 OutPos = { 0.0f, 0.0f, 0.0f };

	// バーンスタイン基底を使用して計算する.
	const int n = static_cast<int>( ControlPoints.size() );
	for ( int i = 0; i < n; i++ ) 
		OutPos += ControlPoints[i] * ( Math::Binomial( n - 1, i ) * static_cast<float>( std::pow( NowPoint, i ) * std::pow( 1 - NowPoint, n - 1 - i ) ) );
	return OutPos;
}

//----------------------------.
// ベクトルに対して直交のベクトルを取得.
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
// uの方向をvの方向と同じにするためのD3DXQUATERNIONを取得.
//----------------------------.
D3DXQUATERNION Math::GetRotationQuaternion( const D3DXVECTOR3& _u, const D3DXVECTOR3& _v )
{
	D3DXQUATERNION	q;
	D3DXVECTOR3		u = _u, v = _v;
	// ベクトルを正規化.
	D3DXVec3Normalize( &u, &u );
	D3DXVec3Normalize( &v, &v );

	// エッジケース.
	const float UoV = D3DXVec3Dot( &u, &v );
	if( UoV <= -1.0f + FLT_EPSILON ){ // FLT_EPSILONで浮動小数点相対精度を考慮.
		// 直交の軸で180度回転.
		D3DXVECTOR3 vOrthogonal = Math::GetOrthogonalVector( u );
		D3DXVec3Normalize( &vOrthogonal, &vOrthogonal );

		q.x = vOrthogonal.x;
		q.y = vOrthogonal.y;
		q.z = vOrthogonal.z;
		q.w = 0.0f;
	} 
	// 通常ケース.
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
// 注視点を見るように回転させる行列を取得.
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
// ベクトルの交点を取得.
//---------------------------.
bool Math::GetIntersection( D3DXVECTOR3* pOut, const D3DXVECTOR3& Start_A, const D3DXVECTOR3& End_A, const D3DXVECTOR3& Start_B, const D3DXVECTOR3& End_B )
{
	if ( Start_A == End_A || Start_B == End_B ) return false;
	D3DXVECTOR3 vA = End_A - Start_A;
	D3DXVECTOR3 vB = End_B - Start_B;
	D3DXVec3Normalize( &vA, &vA );
	D3DXVec3Normalize( &vB, &vB );

	// 2直線が平行な場合の挙動を定義する.
	if ( std::abs( 1.0f - std::abs( D3DXVec3Dot( &vA, &vB ) ) ) < 1e-5f ) return false;

	D3DXVECTOR3 vAC = Start_B - Start_A;
	const float d1	= (  D3DXVec3Dot( &vA, &vAC ) - D3DXVec3Dot( &vB, &vAC ) * D3DXVec3Dot( &vA, &vB ) ) / ( 1 - D3DXVec3Dot( &vA, &vB ) * D3DXVec3Dot( &vA, &vB ) );
	const float d2	= ( -D3DXVec3Dot( &vB, &vAC ) + D3DXVec3Dot( &vA, &vAC ) * D3DXVec3Dot( &vA, &vB ) ) / ( 1 - D3DXVec3Dot( &vA, &vB ) * D3DXVec3Dot( &vA, &vB ) );

	// 交点がCD上にない場合の挙動を定義する.
	D3DXVECTOR3 vCD = End_B - Start_B;
	if (		d1 < 0 || d2 < 0			) return false;	// d1またはd2がマイナス値ならポイントBを返す.
	else if (	d2 > D3DXVec3Length( &vCD )	) return false;	// d2がCDより長ければポイントBを返す.

	// 好きなようにp1,p2からどのような形で交点を算出するか定義する.
	//*pOut = Start_A + d1 * vA;
	*pOut = Start_B + d2 * vB;
	return true;
}

//---------------------------.
// ベクトルに対して左右どちらにいるか取得.
//---------------------------.
std::string Math::GetVectorLRDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos )
{
	std::string Dir = "";

	// 外積を使用して左右どちらにいるか調べる.
	const D3DXVECTOR3& a = VecStartPos;
	const D3DXVECTOR3& b = VecStartPos - Vec;
	const D3DXVECTOR3& c = Pos;
	const float p = ( b.x - a.x ) * ( c.z - a.z ) - ( b.z - a.z ) * ( c.x - a.x );

	// +値なら左側、-値なら右側、0の場合同じ場所.
	if (		p < 0 ) Dir = "Left";
	else if (	p > 0 ) Dir = "Right";
	else				Dir = "";
	return Dir;
}

//---------------------------.
// ベクトルに対して前後ろどちらにいるか取得.
//---------------------------.
std::string Math::GetVectorFBDirection( const D3DXVECTOR3& Pos, const D3DXVECTOR3& Vec, const D3DXVECTOR3& VecStartPos )
{
	// ベクトルを時計回りに90度回転させる.
	D3DXVECTOR3 SidewaysVec;
	SidewaysVec.x =  Vec.z;
	SidewaysVec.y =  Vec.y;
	SidewaysVec.z = -Vec.x;

	// 左側にあるなら前、右側にあるなら後ろ.
	std::string Dir = GetVectorLRDirection( Pos, SidewaysVec, VecStartPos );
	if (		Dir == "Left"  ) Dir = "Front";
	else if (	Dir == "Right" ) Dir = "Back"; 
	return Dir;
}

//---------------------------.
// ベクトルの角度を取得.
//---------------------------.
float Math::GetVectorAngle( const D3DXVECTOR3& vA, const D3DXVECTOR3& vB )
{
	// ベクトルAとBの長さを計算する.
	const float Length_vA = D3DXVec3Length( &vA );
	const float Length_vB = D3DXVec3Length( &vB );

	// 内積とベクトル長さを使ってcosθを求める.
	const float CosSita = D3DXVec3Dot( &vA, &vB ) / ( Length_vA * Length_vB );

	// cosθからθを求める
	const float Sita = acos( CosSita );
	return Sita;
}

//---------------------------.
// ベクトルの回転.
//---------------------------.
void Math::VectorRotation( D3DXVECTOR3* pOut, const D3DXVECTOR3& Rotation )
{
	*pOut = VectorRotation( *pOut, Rotation );
}
D3DXVECTOR3 Math::VectorRotation( const D3DXVECTOR3& Vec, const D3DXVECTOR3& Rotation )
{
	// 回転行列作成.
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
// 壁ずりベクトルの取得.
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
// 反射ベクトルの取得.
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
