#pragma once
#include "..\..\..\..\..\Global.h"

struct stWeedData
{
	STransform	Transform;		// トランスフォーム
	bool		IsDisp;			// 表示しているか

	stWeedData()
		: stWeedData( STransform(), false )
	{}
	stWeedData( const STransform& t, const bool d )
		: Transform	( t )
		, IsDisp	( d )
	{}
} typedef SWeedData;