#pragma once
#include "..\..\Global.h"
#include "MasterDatas.h"
#include <functional>
#include <typeinfo>

/************************************************
*	マスターデータのアクセスクラス.
*	「creator」によって自動で「cpp」が作成されています
**/
class MasterDataAccesser
{
public:
	MasterDataAccesser();
	~MasterDataAccesser();

	// 読み込み.
	static HRESULT Load();

	// 1件取得する
	//	存在しなければエラーが発生する
	template<class T>
	static T Get( unsigned int id );

	// 1件探す
	template<class T>
	static T Find( unsigned int id );

	// 一致する要素を返す
	template<class T>
	static std::vector<T> Where( const std::function<bool( T )>& );
	
	// 一致する先頭要素を返す
	template<class T>
	static T First( const std::function<bool( T )>& );

	// idが一致する要素があるかを判断して返す
	template<class T>
	static bool Any( unsigned int id );

	// 一致する要素があるかを判断して返す
	template<class T>
	static bool Any( const std::function<bool( T )>& );

	// 全ての要素を返す
	template<class T>
	static std::vector<T> All();

	// 件数を返す
	template<class T>
	static int Count();

	// 一致する要素の件数を返す
	template<class T>
	static int Count( const std::function<bool( T )>& );

private:
	// インスタンスの取得.
	static MasterDataAccesser* GetInstance();

private:
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> m_MasterContainer;
};


//----------------------------.
// IDからマスターデータを取得する
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
		ErrorMessage( std::string( typeid( T ).name ) + "にID:" + std::to_string( id ) + "が存在しません" );
		return T();
	}
}

//----------------------------.
// 1件探す
//----------------------------.
template<class T>
inline T MasterDataAccesser::Find( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return std::any_cast<T>( pI->m_MasterContainer[typeid( T ).name][id] );
}

//----------------------------.
// 一致する要素を返す
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
// 一致する先頭要素を返す
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
// idが一致する要素があるかを判断して返す
//----------------------------.
template<class T>
inline bool MasterDataAccesser::Any( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name].find( id ) != pI->m_MasterContainer[typeid( T ).name].end();
}

//----------------------------.
// 一致する要素があるかを判断して返す
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
// 全ての要素を返す
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
// 件数を返す
//----------------------------.
template<class T>
inline int MasterDataAccesser::Count()
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name].size();
}

//----------------------------.
// 一致する要素の件数を返す
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
