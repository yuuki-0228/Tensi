#pragma once
#include "..\..\Global.h"
#include "MasterDatas.h"
#include <functional>
#include <typeinfo>

/************************************************
*	ƒ}ƒXƒ^[ƒf[ƒ^‚ÌƒAƒNƒZƒXƒNƒ‰ƒX.
*	ucreatorv‚É‚æ‚Á‚Ä©“®‚Åucppv‚ªì¬‚³‚ê‚Ä‚¢‚Ü‚·
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
	static T Get( unsigned int id );

	// 1Œ’T‚·
	template<class T>
	static T Find( unsigned int id );

	// ˆê’v‚·‚é—v‘f‚ğ•Ô‚·
	template<class T>
	static std::vector<T> Where( const std::function<bool( T )>& );
	
	// ˆê’v‚·‚éæ“ª—v‘f‚ğ•Ô‚·
	template<class T>
	static T First( const std::function<bool( T )>& );

	// id‚ªˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
	template<class T>
	static bool Any( unsigned int id );

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
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> m_MasterContainer;
};


//----------------------------.
// ID‚©‚çƒ}ƒXƒ^[ƒf[ƒ^‚ğæ“¾‚·‚é
//----------------------------.
template<class T>
inline T MasterDataAccesser::Get( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();
	try
	{
		return std::any_cast<T>( pI->m_MasterContainer[typeid( T ).name].at( id ) );
	} catch ( const std::out_of_range& out )
	{
		ErrorMessage( std::string( typeid( T ).name ) + "‚ÉID:" + std::to_string( id ) + "‚ª‘¶İ‚µ‚Ü‚¹‚ñ" );
		return T();
	}
}

//----------------------------.
// 1Œ’T‚·
//----------------------------.
template<class T>
inline T MasterDataAccesser::Find( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return std::any_cast<T>( pI->m_MasterContainer[typeid( T ).name][id] );
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
	for ( auto& m : pI->m_MasterContainer[typeid( T ).name] ) {
		const T& data = std::any_cast<T>( m );
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

	for ( auto& m : pI->m_MasterContainer[typeid( T ).name] ) {
		const T& data = std::any_cast<T>( m );
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
inline bool MasterDataAccesser::Any( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name].find( id ) != pI->m_MasterContainer[typeid( T ).name].end();
}

//----------------------------.
// ˆê’v‚·‚é—v‘f‚ª‚ ‚é‚©‚ğ”»’f‚µ‚Ä•Ô‚·
//----------------------------.
template<class T>
inline bool MasterDataAccesser::Any( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	for ( auto& m : pI->m_MasterContainer[typeid( T ).name] ) {
		if ( function( std::any_cast<T>( m ) ) ) {
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
	for ( auto& m : pI->m_MasterContainer[typeid( T ).name] ) {
		out.emplace_back( std::any_cast<T>( m ) );
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

	return pI->m_MasterContainer[typeid( T ).name].size();
}

//----------------------------.
// ˆê’v‚·‚é—v‘f‚ÌŒ”‚ğ•Ô‚·
//----------------------------.
template<class T>
inline int MasterDataAccesser::Count( const std::function<bool( T )>& function )
{
	MasterDataAccesser* pI = GetInstance();

	int num = 0;
	for ( auto& m : pI->m_MasterContainer[typeid( T ).name] ) {
		if ( function( std::any_cast<T>( m ) ) ) {
			num++;
		}
	}
	return num;
}
