#pragma once
#include "..\..\SystemSettings.h"
#ifdef ENABLE_CASH
#include "..\..\Global.h"
#include "..\FileManager\FileManager.h"
#include <any>

namespace CashKey {
//	constexpr char Test[] = "Test";
};

/************************************************
*	キャッシュマネージャー.
**/
class CashManager
{
public:
	CashManager();
	~CashManager();

	// キャッシュデータを読み込む
	static HRESULT Load();

	// キャッシュデータを保存
	template<class T>
	static void Set( const std::string& key, T value, bool isSave = false );

	// キャッシュデータを取得
	template<class T>
	static T Get( const std::string& key );

	// キャッシュデータを一度だけ取得
	template<class T>
	static T GetOnce( const std::string& key );

	// キーが存在するか
	static bool ContainsKey( const std::string& key );

	// キャッシュデータを削除
	static void Remove( const std::string& key );

	// キャッシュデータを全て削除
	static void Clear();

private:
	// インスタンスの取得.
	static CashManager* GetInstance();

	// キャッシュデータの書き出し
	template<class T>
	static std::string CashSave( const std::string& key, T value );

private:
	const std::string CAHS_FILE_PATH = "Data/Cash/";

private:
	std::unordered_map<std::string, std::pair<std::any, std::string>> m_Cashs;
};

//---------------------------.
// キャッシュデータを保存
//---------------------------.
template<class T>
void CashManager::Set( const std::string& key, T value, bool isSave )
{
	std::string savePath = "";
	if ( isSave ) {
		savePath = CashManager::CashSave( key, value );
	}
	GetInstance()->m_Cashs[key] = std::make_pair( value, savePath );
}

//---------------------------.
// キャッシュデータを取得
//---------------------------.
template<class T>
inline T CashManager::Get( const std::string& key )
{
	CashManager* pI = GetInstance();

	if ( pI->m_Cashs[key].first.has_value() == false && pI->m_Cashs[key].second != "" ) {
		T cash;
		FileManager::BinaryLoad( std::string( pI->CAHS_FILE_PATH + key + ".bin" ).c_str(), cash );
		pI->m_Cashs[key].first = cash;
	}
	return std::any_cast<T>( pI->m_Cashs[key].first );
}

//---------------------------.
// キャッシュデータを一度だけ取得
//---------------------------.
template<class T>
inline T CashManager::GetOnce( const std::string& key )
{
	const T cash = CashManager::Get<T>( key );
	CashManager::Remove( key );
	return cash;
}

//---------------------------.
// キャッシュデータの書き出し
//---------------------------.
template<class T>
inline std::string CashManager::CashSave( const std::string& key, T value )
{
	std::string filePath = GetInstance()->CAHS_FILE_PATH + key + ".bin";
	FileManager::BinarySave( filePath.c_str(), value );

	Log::PushLogInfo( filePath + " 保存 : 成功" );
	return filePath;
}

#endif