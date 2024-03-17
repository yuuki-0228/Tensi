#pragma once
#include "..\..\Global.h"

/************************************************
*	時間マネージャー.
**/
namespace TimeManager {
	// 初期値.
	const std::tm None = std::tm();

	// 現在の日時を取得.
	std::tm GetTime();
	
	// 日時の差を取得する(return: tm1 - tm2)
	//	日:tm_mday, 時:tm_hour, 分:tm_min
	std::tm GetTimediff( std::tm tm1, std::tm tm2 );

	// 同じ日かどうか取得.
	bool GetIsSameDay( const std::tm& tm1, const std::tm& tm2 );

	// 昨日を取得.
	std::tm Yesterday( std::tm tm = GetTime() );
	// 明日を取得.
	std::tm Tomorrow( std::tm tm = GetTime() );
};
