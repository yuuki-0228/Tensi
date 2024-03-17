#pragma once
#include "..\..\Global.h"

/************************************************
*	���ԃ}�l�[�W���[.
**/
namespace TimeManager {
	// �����l.
	const std::tm None = std::tm();

	// ���݂̓������擾.
	std::tm GetTime();
	
	// �����̍����擾����(return: tm1 - tm2)
	//	��:tm_mday, ��:tm_hour, ��:tm_min
	std::tm GetTimediff( std::tm tm1, std::tm tm2 );

	// ���������ǂ����擾.
	bool GetIsSameDay( const std::tm& tm1, const std::tm& tm2 );

	// ������擾.
	std::tm Yesterday( std::tm tm = GetTime() );
	// �������擾.
	std::tm Tomorrow( std::tm tm = GetTime() );
};
