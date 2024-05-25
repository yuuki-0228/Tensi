#pragma once
#include "..\..\..\..\..\..\Global.h"

/************************************************
*	水データ.
**/
struct stWaterData
{
	D3DXPOSITION3	Pos;		// 生成場所.
	D3DXVECTOR3		MoveVec;	// 移動ベクトル
	float			VecRotMin;	// ベクトルの最小の回転角度(rad)
	float			VecRotMax;	// ベクトルの最大の回転角度(rad)
	float			PowerMin;	// 最小のパワー
	float			PowerMax;	// 最大のパワー

	stWaterData( const D3DXPOSITION3& p, const D3DXVECTOR3& v, const float vmin, const float vmax, const float pmin, const float pmax )
		: Pos		( p )
		, MoveVec	( v )
		, VecRotMin	( vmin )
		, VecRotMax	( vmax )
		, PowerMin	( pmin )
		, PowerMax	( pmax )
	{}
	stWaterData()
		: stWaterData( INIT_FLOAT3, INIT_FLOAT3, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT, INIT_FLOAT )
	{}
} typedef SWaterData;