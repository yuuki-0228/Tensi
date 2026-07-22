#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_MASTER_DATA
#include "..\..\Global.h"
#include "MasterDatas.h"
#include <functional>
#include <typeinfo>

/************************************************
*	ƒ}ƒXƒ^[ƒf[ƒ^‚ÌƒAƒNƒZƒXƒNƒ‰ƒX.
**/
class MasterDataAccesser
{
public:
	MasterDataAccesser();
	~MasterDataAccesser();

	// “Ç‚İ‚İ.
	static HRESULT Load();

	// 1Œæ“¾‚·‚é
	//	‘¶İ‚µ‚È‚¯‚ê‚ÎƒGƒ‰[‚ª”­¶‚·‚é
	template<class T>
	static T Get( const ulong id );

	// 1Œ’T‚·
	template<class T>
	static T Find( const ulong id );

	// ˆê’v‚·‚é—v‘f‚ğ•Ô‚·
	template<class T>
	static std::vector<T> Where( const std::function<bool( T )>& );
	
	// ˆê’v‚·‚éæ“ª—v‘f‚ğ•Ô‚·
	template<class T>
	static T First( const std::function<bool( T )>& );

	// id‚ªˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
	template<class T>
	static bool Any( const ulong id );

	// ˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
	template<class T>
	static bool Any( const std::function<bool( T )>& );

	// ‘S‚Ä‚Ì—v‘f‚ğ•Ô‚·
	template<class T>
	static std::vector<T> All();

	// Œ”‚ğ•Ô‚·
	template<class T>
	static int Count();

	// ˆê’v‚·‚é—v‘f‚ÌŒ”‚ğ•Ô‚·
	template<class T>
	static int Count( const std::function<bool( T )>& );

private:
	// ƒCƒ“ƒXƒ^ƒ“ƒX‚Ìæ“¾.
	static MasterDataAccesser* GetInstance();

private:
	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> m_MasterContainer;
};


//----------------------------.
// ID‚©‚çƒ}ƒXƒ^[ƒf[ƒ^‚ğæ“¾‚·‚é
//----------------------------.
template<class T>
inline T MasterDataAccesser::Get( const ulong id )
{
	MasterDataAccesser* pI = GetInstance();
	try
	{
		return std::any_cast<T>( pI->m_MasterContainer[typeid( T ).name()].at( id ) );
	} catch ( const std::out_of_range& )
	{
		ErrorMessage( std::string( typeid( T ).name() ) + "‚ÉID:" + std::to_string( id ) + "‚ª‘¶İ‚µ‚Ü‚¹‚ñ" );
		return T();
	}
}

//----------------------------.
// 1Œ’T‚·
//----------------------------.
template<class T>
inline T MasterDataAccesser::Find( const ulong id )
{
	MasterDataAccesser* pI = GetInstance();
	try
	{
		return std::any_cast< T >( pI->m_MasterContainer[typeid( T ).name()].at( id ) );
	} catch ( const std::out_of_range& )
	{
		return T();
	}
}

//----------------------------.
// ˆê’v‚·‚é—v‘f‚ğ•Ô‚·
//----------------------------.
template<class T>
inline std::vector<T> MasterDataAccesser::Where( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	std::vector<T> out;
	out.reserve( MasterDataAccesser::Count<T>() );
	for ( auto& [key, value] : pI->m_MasterContainer[typeid( T ).name()] ) {
		auto data = std::any_cast<T>( value );
		if ( function( data ) ) {
			out.emplace_back( data );
		}
	}
	return out;
}

//----------------------------.
// ˆê’v‚·‚éæ“ª—v‘f‚ğ•Ô‚·
//----------------------------.
template<class T>
inline T MasterDataAccesser::First( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	for ( auto& [key, value] : pI->m_MasterContainer[typeid( T ).name()] ) {
		const T& data = std::any_cast<T>( value );
		if ( function( data ) ) {
			return data;
		}
	}
	return T();
}

//----------------------------.
// id‚ªˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
//----------------------------.
template<class T>
inline bool MasterDataAccesser::Any( const ulong id )
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name()].find( id ) != pI->m_MasterContainer[typeid( T ).name()].end();
}

//----------------------------.
// ˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
//----------------------------.
template<class T>
inline bool MasterDataAccesser::Any( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	for ( auto& [key, value] : pI->m_MasterContainer[typeid( T ).name()] ) {
		if ( function( std::any_cast<T>( value ) ) ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// ‘S‚Ä‚Ì—v‘f‚ğ•Ô‚·
//----------------------------.
template<class T>
inline std::vector<T> MasterDataAccesser::All()
{
	MasterDataAccesser* pI = GetInstance();

	std::vector<T> out;
	out.reserve( MasterDataAccesser::Count<T>() );
	for ( auto& [key, value] : pI->m_MasterContainer[typeid( T ).name()] ) {
		out.emplace_back( std::any_cast<T>( value ) );
	}
	return out;
}

//----------------------------.
// Œ”‚ğ•Ô‚·
//----------------------------.
template<class T>
inline int MasterDataAccesser::Count()
{
	MasterDataAccesser* pI = GetInstance();

	return static_cast<int>( pI->m_MasterContainer[typeid( T ).name()].size() );
}

//----------------------------.
// ˆê’v‚·‚é—v‘f‚ÌŒ”‚ğ•Ô‚·
//----------------------------.
template<class T>
inline int MasterDataAccesser::Count( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	int num = 0;
	for ( auto& [key, value] : pI->m_MasterContainer[typeid( T ).name()] ) {
		if ( function( std::any_cast<T>( value ) ) ) {
			num++;
		}
	}
	return num;
}
#endif