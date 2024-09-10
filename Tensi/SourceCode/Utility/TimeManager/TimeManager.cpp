#include "TimeManager.h"
#ifdef ENABLE_TIME

//---------------------------.
// ���݂̓������擾.
//---------------------------.
std::tm TimeManager::GetTime()
{
	std::time_t t = std::time( nullptr );
	std::tm* now = std::localtime( &t );

	// �l�𒲐�����.
	now->tm_year	+= 1900;
	now->tm_mon		+= 1;
	return *now;
}

//---------------------------.
// �����̍����擾����
//---------------------------.
std::tm TimeManager::GetTimediff( std::tm tm1, std::tm tm2 )
{
	// �l�𒲐�����
	tm1.tm_year -= 1900;
	tm2.tm_year -= 1900;
	tm1.tm_mon  -= 1;
	tm2.tm_mon  -= 1;

	// �����擾
	const std::time_t t1 = std::mktime( &tm1 );
	const std::time_t t2 = std::mktime( &tm2 );
	int dt = static_cast<int>( std::difftime( t1, t2 ) );

	// �l�𐮂���
	std::tm diff;
	diff.tm_mday = dt / ( 3600 * 24 );
	diff.tm_hour = ( dt % ( 3600 * 24 ) ) / 3600;
	diff.tm_min	 = ( ( dt % ( 3600 * 24 ) ) % 3600 ) % 60;
	return diff;
}

//---------------------------.
// ���������ǂ����擾.
//---------------------------.
bool TimeManager::GetIsSameDay( const std::tm& tm1, const std::tm& tm2 )
{
	return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday;
}

//---------------------------.
// ������擾.
//---------------------------.
std::tm TimeManager::Yesterday( std::tm tm )
{
	tm.tm_mday -= 1;
	time_t tomorrow = std::mktime( &tm );

	if ( tomorrow = -1 ) return GetTime();
	return *std::localtime( &tomorrow );
}

//---------------------------.
// �������擾.
//---------------------------.
std::tm TimeManager::Tomorrow( std::tm tm )
{
	tm.tm_mday += 1;
	time_t tomorrow = std::mktime( &tm );

	if ( tomorrow = -1 ) return GetTime();
	return *std::localtime( &tomorrow );
}

#endif