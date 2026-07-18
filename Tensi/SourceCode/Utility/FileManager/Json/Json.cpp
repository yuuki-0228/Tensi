#include "Json.h"
#ifdef ENABLE_FILE
#include "..\..\Log\Log.h"

//---------------------------.
// コンストラクタ.
//---------------------------.
Json::Json()
	: m_Data	()
{
}
Json::Json( const NJson& Data )
	: m_Data	( Data )
{
}
Json::Json( NJson&& Data )
	: m_Data	( std::move( Data ) )
{
}

//---------------------------.
// 階層参照.
//---------------------------.
Json::Ref Json::operator [] ( const Key& Key )
{
	return Ref( &m_Data, { Key } );
}
Json::Ref Json::operator [] ( const Key& Key ) const
{
	// 読み取り専用で使用するためconstを外す.
	return Ref( const_cast<NJson*>( &m_Data ), { Key } );
}

//---------------------------.
// std::unordered_map への変換.
//---------------------------.
std::unordered_map<std::string, std::string> Json::ToMap() const
{
	std::unordered_map<std::string, std::string> Out;
	for ( auto& [Key, Value] : m_Data.items() ) Out[Key] = Value;
	return Out;
}

//---------------------------.
// std::unordered_map から作成.
//---------------------------.
Json Json::FromMap( const std::unordered_map<std::string, std::string>& Map )
{
	NJson Out;
	for ( auto& [Key, Value] : Map ) Out[Key] = ToValue( Value );
	return Json( std::move( Out ) );
}
Json Json::FromMap( const std::unordered_map<std::string, std::vector<std::string>>& Map )
{
	NJson Out;
	for ( auto& [Key, vValue] : Map ) {
		for ( auto& Value : vValue ) Out[Key].emplace_back( ToValue( Value ) );
	}
	return Json( std::move( Out ) );
}

//---------------------------.
// 文字列から型を推測して json の値に変換.
//---------------------------.
Json::NJson Json::ToValue( const std::string& Value )
{
	if (		Value == "nullptr"						) return nullptr;
	else if (	Value == "true"							) return true;
	else if (	Value == "false"						) return false;
	else if (	Value.find_first_not_of( "0123456789.f" ) == std::string::npos ) {
		if (	Value.find( "." ) != std::string::npos	) return std::stof( Value );
		else											  return std::stoi( Value );
	}
	return Value;
}

//---------------------------.
// コンストラクタ (プロキシ).
//---------------------------.
Json::Ref::Ref( NJson* pRoot, std::vector<Key> Keys )
	: m_pRoot	( pRoot )
	, m_Keys	( std::move( Keys ) )
{
}

//---------------------------.
// 階層参照 (プロキシ).
//---------------------------.
Json::Ref Json::Ref::operator [] ( const Key& Key ) const
{
	std::vector<Json::Key> Keys = m_Keys;
	Keys.emplace_back( Key );
	return Ref( m_pRoot, std::move( Keys ) );
}

//---------------------------.
// 値の代入.
//---------------------------.
Json::Ref& Json::Ref::operator = ( const NJson& Value )
{
	Make() = Value;
	return *this;
}

//---------------------------.
// json のコピー取得.
//---------------------------.
Json::Ref::operator Json::NJson () const
{
	const NJson* pNode = Find();
	if ( pNode == nullptr ) return NJson();
	return *pNode;
}

//---------------------------.
// ノードを検索する.
//---------------------------.
const Json::NJson* Json::Ref::Find() const
{
	const NJson* pNode = m_pRoot;
	for ( const Key& k : m_Keys ) {
		// 配列インデックスの場合.
		if ( k.IsIndex ) {
			if ( pNode->is_array() == false	) return nullptr;
			if ( k.Index >= pNode->size()	) return nullptr;
			pNode = &( ( *pNode )[k.Index] );
			continue;
		}

		// オブジェクトでなければキーは存在しない.
		if ( pNode->is_object() == false ) return nullptr;
		const auto itr = pNode->find( k.Name );
		if ( itr == pNode->end() ) return nullptr;
		pNode = &( *itr );
	}
	return pNode;
}

//---------------------------.
// ノードを作成しながら辿る.
//---------------------------.
Json::NJson& Json::Ref::Make() const
{
	NJson* pNode = m_pRoot;
	for ( const Key& k : m_Keys ) {
		// 配列インデックスの場合.
		if ( k.IsIndex ) {
			// 配列以外の階層に書き込もうとした場合は例外を投げる.
			if ( pNode->is_array() == false && pNode->is_null() == false ) {
				ThrowError( "配列以外の階層にインデックスで書き込もうとしました" );
			}
			pNode = &( ( *pNode )[k.Index] );
			continue;
		}

		// オブジェクト以外の階層に書き込もうとした場合は例外を投げる.
		if ( pNode->is_object() == false && pNode->is_null() == false ) {
			ThrowError( "オブジェクト以外の階層に書き込もうとしました" );
		}
		pNode = &( ( *pNode )[k.Name] );
	}
	return *pNode;
}

//---------------------------.
// ルートからのキーのパスを取得.
//---------------------------.
std::string Json::Ref::GetPath() const
{
	std::string Out = "";
	for ( const Key& k : m_Keys ) {
		if ( k.IsIndex ) {
			Out += "[" + std::to_string( k.Index ) + "]";
			continue;
		}
		if ( Out.empty() == false ) Out += "/";
		Out += k.Name;
	}
	return Out;
}

//---------------------------.
// エラーをログに出して例外を投げる.
//---------------------------.
void Json::Ref::ThrowError( const std::string& Reason ) const
{
	const std::string Error = "Json error : [" + GetPath() + "] " + Reason;
	Log::PushLogError( Error );
	throw std::runtime_error( Error );
}
#endif
