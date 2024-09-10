#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_CLASS_BOOL
#include "BitFlagManager/BitFlagManager.h"
#include <string>

/************************************************
*	�I���W�i��bool�N���X.
*	BitFlagManager �ňꊇ�Ǘ����\.
**/
class CBool final
{
public:
	void operator=(  const bool	 Flag );
	void operator=(  const char* sFlag );
	void operator=(  const int	 iFlag );
	bool operator==( const bool  Flag );
	bool operator!=( const bool  Flag );
	bool operator!();

public:
	CBool( bool Flag = false, std::string Name = u8"", std::string Group = u8"Object" );
	~CBool();

	// bool�̏�Ԃ𕶎���Ŏ擾.
	inline std::string ToString() {
		return BitFlagManager::IsBitFlag( m_Handle ) ? "true" : "false";
	};

	// bool�^�Ŏ擾.
	inline bool get() {
		return BitFlagManager::IsBitFlag( m_Handle );
	}

	// ���O���擾.
	inline std::string GetName() {
		return BitFlagManager::GetName( m_Handle );
	};
	// ���O�̐ݒ�.
	inline void SetName( std::string Name ) {
		BitFlagManager::SetName( m_Handle, Name );
	};

	// �t���O�𔽓]������.
	inline void Inversion() {
		BitFlagManager::BitInversion( m_Handle );
	}

private:
	BitFlagManager::Handle m_Handle; // �n���h��.
};
#endif