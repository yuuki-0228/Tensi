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
// �C���X�^���X�̎擾.
//----------------------------.
BitFlagManager* BitFlagManager::GetInstance()
{
	static std::unique_ptr<BitFlagManager> pInstance = std::make_unique<BitFlagManager>();
	return pInstance.get();
}

//----------------------------.
// �t���O�̍쐬.
//----------------------------.
BitFlagManager::Handle BitFlagManager::CreateFlag( const std::string& Name, const std::string& Group )
{
	BitFlagManager* pI = GetInstance();

	// �S�Ẵt���O���g�p���Ă��邩.
	if ( pI->m_FlagNum == std::pow( 2, sizeof( Handle ) * 8 ) - 1 ) {
		MessageBox( NULL, TEXT(
			"�t���O�̍쐬�Ɏ��s���܂����B\n"
			"Handle�̃T�C�Y��傫�����Ă��������B\n"
			"���ڂ�����BitFlagManager.h���m�F���Ă��������B"
		), TEXT( "�t���O�̍쐬���s" ), MB_OK );
		return 0;
	}

	// �폜�����n���h�����ė��p����.
	if ( pI->m_DeleteHandleList.size() != 0 ) {
		const Handle handle = pI->m_DeleteHandleList.back();
		pI->m_DeleteHandleList.pop_back();
		pI->m_SaveDataList[handle].Name		= Name;
		pI->m_SaveDataList[handle].Group	= Group;
		return handle;
	}

	// �V�����n���h�����쐬����.
	const int Size = static_cast<int>( pI->m_FlagList.size() );
	for ( int i = 0; i < Size + 1; i++ ) {
		// �V�����z��Ȃ�쐬����.
		if ( i == Size ) {
			pI->m_FlagList.emplace_back( 0 );
			pI->m_BitNum.emplace_back( 0 );
		}
		// �r�b�g�S�Ďg�p���Ă��邽�ߎ��̔z����m�F����.
		if ( pI->m_BitNum[i] == sizeof( pI->m_FlagList[0] ) * 8 ) continue;

		// �ۑ�����r�b�g�̏ꏊ��ۑ�.
		const Handle handle = pI->m_FlagNum;
		pI->m_SaveDataList[handle] = SSaveData( i, pI->m_BitNum[i], Name, Group );

		// �ۑ���������ǉ�.
		pI->m_BitNum[i]++;
		pI->m_FlagNum++;

		// �n���h����Ԃ�.
		return handle;
	}
	return 0;	// ��O��Ԃ�.
}

//----------------------------.
// �t���O�̍폜.
//----------------------------.
void BitFlagManager::DeleteFlag( const Handle & handle )
{
	BitFlagManager* pI = GetInstance();

	// �폜�����n���h����ۑ�.
	pI->m_DeleteHandleList.emplace_back( handle );

	// �폜����r�b�g�̏�����.
	OffBitFlag( handle );
	pI->m_SaveDataList[handle].Name = "";
}

//----------------------------.
// �r�b�g�𗧂Ă�.
//----------------------------.
void BitFlagManager::OnBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] |= ( 1 << SaveList.BitNo );
}

//----------------------------.
// �r�b�g�����낷.
//----------------------------.
void BitFlagManager::OffBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] &= ~( 1 << SaveList.BitNo );
}

//----------------------------.
// �r�b�g�𔽓]������.
//----------------------------.
void BitFlagManager::BitInversion( const Handle & handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	pI->m_FlagList[SaveList.VectorNo] ^= ( 1 << SaveList.BitNo );
}

//----------------------------.
// �r�b�g�������Ă��邩.
//----------------------------.
bool BitFlagManager::IsBitFlag( const Handle& handle )
{
	BitFlagManager* pI = GetInstance();

	const SSaveData& SaveList = pI->m_SaveDataList[handle];
	return pI->m_FlagList[SaveList.VectorNo] & ( 1 << SaveList.BitNo );
}

//----------------------------.
// �t���O�̐ݒ�.
//----------------------------.
void BitFlagManager::SetBitFlag( const Handle & handle, const bool Flag )
{
	if ( Flag ) OnBitFlag(  handle );
	else		OffBitFlag( handle );
}

//----------------------------.
// ���O�̎擾.
//----------------------------.
std::string BitFlagManager::GetName( const Handle& handle )
{
	return GetInstance()->m_SaveDataList[handle].Name;
}

//----------------------------.
// �O���[�v�̎擾.
//----------------------------.
std::string BitFlagManager::GetGroup( const Handle & handle )
{
	return GetInstance()->m_SaveDataList[handle].Group;
}

//----------------------------.
// ���O��ݒ�.
//----------------------------.
void BitFlagManager::SetName( const Handle& handle, const std::string & name )
{
	GetInstance()->m_SaveDataList[handle].Name = name;
}

//----------------------------.
// �O���[�v��ݒ�.
//----------------------------.
void BitFlagManager::SetGroup( const Handle & handle, const std::string & group )
{
	GetInstance()->m_SaveDataList[handle].Group = group;
}
