#pragma once
#include "..\..\..\..\..\Global.h"

struct stWeedData
{
	STransform	Transform;		// �g�����X�t�H�[��
	bool		IsDisp;			// �\�����Ă��邩

	stWeedData()
		: stWeedData( STransform(), false )
	{}
	stWeedData( const STransform& t, const bool d )
		: Transform	( t )
		, IsDisp	( d )
	{}
} typedef SWeedData;