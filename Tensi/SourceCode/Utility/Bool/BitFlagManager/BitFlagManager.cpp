#include "BitFlagManager.h"
#include <memory>
#include <Windows.h>

BitFlagManager::BitFlagManager()
	: m_FlagList			()
	, m_SaveDataList		()
	, m_DeleteHandleList	()
	, m_BitNum				()
	, m_FlagNum				()
{
}

BitFlagManager::~BitFlagManager()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
BitFlagManager* BitFlagManager::GetInstance()
{
	static std::unique_ptr<BitFlagManager> pInstance = std::make_unique<BitFlagManager>();
	return pInstance.get();
}

//----------------------------.
// フラグの作成.
//----------------------------.
BitFlagManager::Handle BitFlagManager::CreateFlag( const std::string& Name, const std::string& Group )
{
	BitFlagManager* pI = GetInstance();

	// 全てのフラグを使用しているか.
	if ( pI->m_FlagNum == std::pow( 2, sizeof( Handle ) * 8 ) - 1 ) {
		MessageBox( NULL, TEXT(
			"フラグの作成に失敗しました。\n"
			"Handleのサイズを大きくしてください。\n"
			"※詳しくはBitFlagManager.hを確認してください。"
		), TEXT( "フラグの作成失敗" ), MB_OK );
		return 0;
	}

	// 削除したハンドルを再利用する.
	if ( pI->m_DeleteHandleList.size() != 0 ) {
		const Handle handle = pI->m_DeleteHandleList.back();
		pI->m_DeleteHandleList.pop_back();
		pI->m_SaveDataList[handle].Name		= Name;
		pI->m_SaveDataList[handle].Group	= Group;
		return handle;
	}

	// 新しくハンドルを作成する.
	const int Size = static_cast<int>( pI->m_FlagList.size() );
	for ( int i = 0; i < Size + 1; i++ ) {
		// 新しい配列なら作成する.
		if ( i == Size ) {
			pI->m_FlagList.emplace_back( 0 );
			pI->m_BitNum.emplace_back( 0 );
		}
		// ビット全て使用しているため次の配列を確認する.
		if ( pI->m_BitNum[i] == sizeof( pI->m_FlagList[0] ) * 8 ) continue;

		// 保存するビットの場所を保存.
		const Handle handle = pI->m_FlagNum;
		pI->m_SaveDataList[handle] = SSaveData( i, pI->m_BitNum[i], Name, Group );

		// 保存した個数を追加.
		pI->m_BitNum[i]++;
		pI->m_FlagNum++;

		// ハンドルを返す.
		return handle;
	}
	return 0;	// 例外を返す.
}

//----------------------------.
// フラグの削除.
//----------------------------.
void BitFlagManager::DeleteFlag( const Handle & handle )
{
	BitFlagManager* pI = GetInstance();

	// 削除したハンドルを保存.
	pI->m_DeleteHandleList.emplace_back( handle );

	// 削除するビットの初期化.
	OffBitFlag( handle );
	pI->m_SaveDataList[handle].Name = "";
}

//----------------------------.
// ビットを立てる.
//----------------------------.
void BitFlagManager::OnBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] |= ( 1 << SaveList.BitNo );
}

//----------------------------.
// ビットを下ろす.
//----------------------------.
void BitFlagManager::OffBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] &= ~( 1 << SaveList.BitNo );
}

//----------------------------.
// ビットを反転させる.
//----------------------------.
void BitFlagManager::BitInversion( const Handle & handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] ^= ( 1 << SaveList.BitNo );
}

//----------------------------.
// ビットが立っているか.
//----------------------------.
bool BitFlagManager::IsBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	return pI->m_FlagList[SaveList.VectorNo] & ( 1 << SaveList.BitNo );
}

//----------------------------.
// フラグの設定.
//----------------------------.
void BitFlagManager::SetBitFlag( const Handle & handle, const bool Flag )
{
	if ( Flag ) OnBitFlag(  handle );
	else		OffBitFlag( handle );
}

//----------------------------.
// 名前の取得.
//----------------------------.
std::string BitFlagManager::GetName( const Handle& handle )
{
	return GetInstance()->m_SaveDataList[handle].Name;
}

//----------------------------.
// グループの取得.
//----------------------------.
std::string BitFlagManager::GetGroup( const Handle & handle )
{
	return GetInstance()->m_SaveDataList[handle].Group;
}

//----------------------------.
// 名前を設定.
//----------------------------.
void BitFlagManager::SetName( const Handle& handle, const std::string & name )
{
	GetInstance()->m_SaveDataList[handle].Name = name;
}

//----------------------------.
// グループを設定.
//----------------------------.
void BitFlagManager::SetGroup( const Handle & handle, const std::string & group )
{
	GetInstance()->m_SaveDataList[handle].Group = group;
}
