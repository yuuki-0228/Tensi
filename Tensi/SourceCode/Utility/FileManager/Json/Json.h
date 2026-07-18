#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_FILE
#include <nlohmnn\json.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

/************************************************
*	nlohmann::json のラッパークラス.
*	 存在しないキーを参照しても例外にならず、
*	 Get( Default ) で安全に値を取得できる.
*	 引数無しの Get() はキーが無い場合に例外を投げる.
**/
class Json final
{
public:
	using NJson = nlohmann::json;

	/************************************************
	*	参照キー (文字列キー or 配列インデックス).
	**/
	struct Key final
	{
		Key( const std::string& Name )	: Name( Name ), Index( 0 ), IsIndex( false )						{}
		Key( const char* Name )			: Name( Name ), Index( 0 ), IsIndex( false )						{}
		Key( const size_t Index )		: Name(), Index( Index ), IsIndex( true )							{}
		Key( const int Index )			: Name(), Index( static_cast<size_t>( Index ) ), IsIndex( true )	{}

		std::string	Name;		// 文字列キー.
		size_t		Index;		// 配列インデックス.
		bool		IsIndex;	// 配列インデックスか.
	};

	/************************************************
	*	json の階層参照用プロキシ.
	*	 読み込みではキーを作成せず、代入時のみ階層を自動で作成する.
	**/
	class Ref final
	{
	public:
		Ref( NJson* pRoot, std::vector<Key> Keys );

		// 階層参照.
		Ref operator [] ( const Key& Key ) const;

		// 値の代入 (存在しない階層は自動で作成する).
		Ref& operator = ( const NJson& Value );
		template<class T>
		Ref& operator = ( const T& Value )
		{
			Make() = Value;
			return *this;
		}

		// 値の安全な取得 (キーが無い・型が違う場合は Default を返す).
		template<class T>
		T Get( const T& Default ) const
		{
			const NJson* pNode = Find();
			if ( pNode == nullptr || pNode->is_null() ) return Default;
			try {
				return pNode->get<T>();
			}
			catch ( ... ) {
				return Default;
			}
		}

		// 値の取得 (キーが無い・型が違う場合は std::runtime_error を投げる).
		template<class T>
		T Get() const
		{
			const NJson* pNode = Find();
			if ( pNode == nullptr || pNode->is_null() ) ThrowError( "キーが存在しません" );
			try {
				return pNode->get<T>();
			}
			catch ( ... ) {
				ThrowError( "型変換に失敗しました" );
			}
		}

		// 配列に要素を追加する (存在しない階層は自動で作成する).
		template<class T>
		void emplace_back( const T& Value ) { Make().emplace_back( Value ); }
		template<class T>
		void push_back( const T& Value )	{ Make().push_back( Value ); }

		// キーが存在するか.
		bool Contains() const { return Find() != nullptr; }

		// json のコピー取得 (存在しない場合は null).
		operator NJson () const;

	private:
		// ノードを検索する (見つからなければ nullptr).
		const NJson* Find() const;
		// ノードを作成しながら辿る.
		NJson& Make() const;
		// ルートからのキーのパスを取得.
		std::string GetPath() const;
		// エラーをログに出して例外を投げる.
		[[noreturn]] void ThrowError( const std::string& Reason ) const;

	private:
		NJson*				m_pRoot;	// ルートの json.
		std::vector<Key>	m_Keys;		// ルートからのキーの並び.
	};

public:
	Json();
	Json( const NJson& Data );
	Json( NJson&& Data );

	// nlohmann::json への暗黙変換.
	operator NJson& ()				{ return m_Data; }
	operator const NJson& () const	{ return m_Data; }

	// 階層参照 (存在しないキーを参照しても例外にならない).
	Ref operator [] ( const Key& Key );
	// const版 (読み取り専用で使うこと・代入した場合の動作は未保証).
	Ref operator [] ( const Key& Key ) const;

	// ルート自体の安全な取得 (型が違う場合は Default を返す).
	template<class T>
	T Get( const T& Default ) const
	{
		if ( m_Data.is_null() ) return Default;
		try {
			return m_Data.get<T>();
		}
		catch ( ... ) {
			return Default;
		}
	}

	// std::unordered_map への変換.
	std::unordered_map<std::string, std::string> ToMap() const;

	// std::unordered_map から作成.
	static Json FromMap( const std::unordered_map<std::string, std::string>& Map );
	static Json FromMap( const std::unordered_map<std::string, std::vector<std::string>>& Map );

	// 文字列から型を推測して json の値に変換.
	static NJson ToValue( const std::string& Value );

	// nlohmann::json 本体の取得.
	NJson&			Data()			{ return m_Data; }
	const NJson&	Data() const	{ return m_Data; }

	// nlohmann::json と同じ書き味の関数.
	auto		items()										{ return m_Data.items();		}
	auto		items() const								{ return m_Data.items();		}
	std::string	dump( const int Indent = -1 ) const			{ return m_Data.dump( Indent );	}
	bool		is_null() const								{ return m_Data.is_null();		}
	bool		is_object() const							{ return m_Data.is_object();	}
	bool		is_array() const							{ return m_Data.is_array();		}
	bool		contains( const std::string& Key ) const	{ return m_Data.contains( Key );}
	size_t		size() const								{ return m_Data.size();			}
	bool		empty() const								{ return m_Data.empty();		}
	void		clear()										{ m_Data.clear();				}
	auto		begin()										{ return m_Data.begin();		}
	auto		end()										{ return m_Data.end();			}
	auto		begin() const								{ return m_Data.begin();		}
	auto		end() const									{ return m_Data.end();			}

private:
	NJson m_Data;	// json 本体.
};
#endif
