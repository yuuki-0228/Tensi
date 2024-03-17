#pragma once
#include "..\..\Global.h"
#include "MasterDatas.h"
#include <functional>
#include <typeinfo>

/************************************************
*	�}�X�^�[�f�[�^�̃A�N�Z�X�N���X.
*	�ucreator�v�ɂ���Ď����Łucpp�v���쐬����Ă��܂�
**/
class MasterDataAccesser
{
public:
	MasterDataAccesser();
	~MasterDataAccesser();

	// �ǂݍ���.
	static HRESULT Load();

	// 1���擾����
	//	���݂��Ȃ���΃G���[����������
	template<class T>
	static T Get( unsigned int id );

	// 1���T��
	template<class T>
	static T Find( unsigned int id );

	// ��v����v�f��Ԃ�
	template<class T>
	static std::vector<T> Where( const std::function<bool( T )>& );
	
	// ��v����擪�v�f��Ԃ�
	template<class T>
	static T First( const std::function<bool( T )>& );

	// id����v����v�f�����邩�𔻒f���ĕԂ�
	template<class T>
	static bool Any( unsigned int id );

	// ��v����v�f�����邩�𔻒f���ĕԂ�
	template<class T>
	static bool Any( const std::function<bool( T )>& );

	// �S�Ă̗v�f��Ԃ�
	template<class T>
	static std::vector<T> All();

	// ������Ԃ�
	template<class T>
	static int Count();

	// ��v����v�f�̌�����Ԃ�
	template<class T>
	static int Count( const std::function<bool( T )>& );

private:
	// �C���X�^���X�̎擾.
	static MasterDataAccesser* GetInstance();

private:
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> m_MasterContainer;
};


//----------------------------.
// ID����}�X�^�[�f�[�^���擾����
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
		ErrorMessage( std::string( typeid( T ).name ) + "��ID:" + std::to_string( id ) + "�����݂��܂���" );
		return T();
	}
}

//----------------------------.
// 1���T��
//----------------------------.
template<class T>
inline T MasterDataAccesser::Find( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return std::any_cast<T>( pI->m_MasterContainer[typeid( T ).name][id] );
}

//----------------------------.
// ��v����v�f��Ԃ�
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
// ��v����擪�v�f��Ԃ�
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
// id����v����v�f�����邩�𔻒f���ĕԂ�
//----------------------------.
template<class T>
inline bool MasterDataAccesser::Any( unsigned int id )
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name].find( id ) != pI->m_MasterContainer[typeid( T ).name].end();
}

//----------------------------.
// ��v����v�f�����邩�𔻒f���ĕԂ�
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
// �S�Ă̗v�f��Ԃ�
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
// ������Ԃ�
//----------------------------.
template<class T>
inline int MasterDataAccesser::Count()
{
	MasterDataAccesser* pI = GetInstance();

	return pI->m_MasterContainer[typeid( T ).name].size();
}

//----------------------------.
// ��v����v�f�̌�����Ԃ�
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
