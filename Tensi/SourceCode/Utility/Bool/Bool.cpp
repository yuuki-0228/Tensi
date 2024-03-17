#include "Bool.h"

void CBool::operator=( const bool Flag )
{
	if ( Flag == true )
		BitFlagManager::OnBitFlag( m_Handle );
	else
		BitFlagManager::OffBitFlag( m_Handle );
}
void CBool::operator=( const char* sFlag )
{
	if ( sFlag == "true" )
		BitFlagManager::OnBitFlag( m_Handle );
	else if ( sFlag == "false" )
		BitFlagManager::OffBitFlag( m_Handle );
}
void CBool::operator=( const int iFlag )
{
	if ( iFlag != 0 )
		BitFlagManager::OnBitFlag( m_Handle );
	else
		BitFlagManager::OffBitFlag( m_Handle );
}

bool CBool::operator==( const bool Flag )
{
	return Flag == BitFlagManager::IsBitFlag( m_Handle );
}

bool CBool::operator!=( const bool Flag )
{
	return Flag != BitFlagManager::IsBitFlag( m_Handle );
}

bool CBool::operator!()
{
	return !BitFlagManager::IsBitFlag( m_Handle );
}

CBool::CBool( bool Flag, std::string Name, std::string Group )
	: m_Handle( 0 )
{
	// フラグの作成.
	m_Handle = BitFlagManager::CreateFlag( Name, Group );
	operator=( Flag );
}

CBool::~CBool()
{
	// このフラグは使わなくなったためフラグを削除する.
	BitFlagManager::DeleteFlag( m_Handle );
	m_Handle = 0;
}
