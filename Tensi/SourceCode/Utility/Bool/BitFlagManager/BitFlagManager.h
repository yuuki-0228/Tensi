#pragma once
#include <string>
#include <unordered_map>

/************************************************
*	�t���O�}�l�[�W���[�N���X.
**/
class BitFlagManager final
{
public:
	// �n���h��.
	//	�t���O�̍쐬�����s�����ꍇ�� "std::uint�Z�Z_t" �̐����𑝂₵�Ă�������.
	//	�g����t���O�̐��͈ȉ��̒ʂ�ł�.
	//	 | uint8_t	= 255��.
	//	 | uint16_t	= 65,535��.
	//	 | uint32_t	= 4,294,967,295��.
	//	 | uint64_t	= 18,446,744,073,709,551,615��.
	using Handle = std::uint16_t;

private:
	// �ۑ��f�[�^�\����.
	struct stSaveData {
		int			VectorNo;	// �z��̕ۑ��ꏊ.
		int			BitNo;		// �r�b�g�̕ۑ��ꏊ.
		std::string	Name;		// ���O.
		std::string Group;		// �t���O�̃O���[�v.

		stSaveData()
			: stSaveData( 0, 0, "", "" )
		{}
		stSaveData( const int vNo, const int bNo, const std::string& name, const std::string& group )
			: VectorNo	( vNo )
			, BitNo		( bNo )
			, Name		( name )
			, Group		( group )
		{}
	} typedef SSaveData;

	// �ۑ��f�[�^�̃}�b�v.
	using DataSave_map = std::unordered_map<Handle, SSaveData>;

public:
	BitFlagManager();
	~BitFlagManager();

	// �t���O�̍쐬.
	//	���O�̎��ʔԍ���Ԃ�.
	//	�쐬�ł��Ȃ������ꍇ�� "0" ��Ԃ�.
	static Handle CreateFlag( const std::string& Name, const std::string& Group );
	// �t���O�̍폜.
	static void DeleteFlag( const Handle& handle );

	// �r�b�g�𗧂Ă�.
	static void OnBitFlag( const Handle& handle );
	// �r�b�g�����낷.
	static void OffBitFlag( const Handle& handle );
	// �r�b�g�𔽓]������.
	static void BitInversion( const Handle& handle );
	// �r�b�g�������Ă��邩.
	static bool IsBitFlag( const Handle& handle );

	// �t���O���̎擾.
	static Handle GetFlagNum() { return GetInstance()->m_FlagNum; }

	// ���O�̎擾.
	static std::string GetName( const Handle& handle );
	// �O���[�v�̎擾.
	static std::string GetGroup( const Handle& handle );

	// �t���O�̐ݒ�.
	static void SetBitFlag( const Handle& handle, const bool Flag );
	// ���O��ݒ�.
	static void SetName( const Handle& handle, const std::string& name );
	// �O���[�v��ݒ�.
	static void SetGroup( const Handle& handle, const std::string& group );

private:
	// �C���X�^���X�̎擾.
	static BitFlagManager* GetInstance();

private:
	std::vector<std::uint32_t>	m_FlagList;			// �t���O���X�g.
	DataSave_map				m_SaveDataList;		// �ۑ��f�[�^���X�g.
	std::vector<Handle>			m_DeleteHandleList;	// �폜���Ďg�p���Ă��Ȃ��n���h�����X�g.
	std::vector<unsigned int>	m_BitNum;			// �r�b�g�̕ۑ���.
	Handle						m_FlagNum;			// �ۑ������t���O��.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	BitFlagManager( const BitFlagManager & )				= delete;
	BitFlagManager& operator = ( const BitFlagManager & )	= delete;
	BitFlagManager( BitFlagManager && )						= delete;
	BitFlagManager& operator = ( BitFlagManager && )		= delete;
};
