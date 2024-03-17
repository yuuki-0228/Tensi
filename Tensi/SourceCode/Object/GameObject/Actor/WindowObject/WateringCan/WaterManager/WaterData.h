#pragma once
#include "..\..\..\..\..\..\Global.h"

/************************************************
*	���f�[�^.
**/
struct stWaterData
{
	D3DXPOSITION3	Pos;		// �����ꏊ.
	D3DXVECTOR3		MoveVec;	// �ړ��x�N�g��
	float			VecRotMin;	// �x�N�g���̍ŏ��̉�]�p�x(rad)
	float			VecRotMax;	// �x�N�g���̍ő�̉�]�p�x(rad)
	float			PowerMin;	// �ŏ��̃p���[
	float			PowerMax;	// �ő�̃p���[

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