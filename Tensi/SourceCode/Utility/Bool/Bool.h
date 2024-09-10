#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_CLASS_BOOL
#include "BitFlagManager/BitFlagManager.h"
#include <string>

/************************************************
*	オリジナルboolクラス.
*	BitFlagManager で一括管理が可能.
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

	// boolの状態を文字列で取得.
	inline std::string ToString() {
		return BitFlagManager::IsBitFlag( m_Handle ) ? "true" : "false";
	};

	// bool型で取得.
	inline bool get() {
		return BitFlagManager::IsBitFlag( m_Handle );
	}

	// 名前を取得.
	inline std::string GetName() {
		return BitFlagManager::GetName( m_Handle );
	};
	// 名前の設定.
	inline void SetName( std::string Name ) {
		BitFlagManager::SetName( m_Handle, Name );
	};

	// フラグを反転させる.
	inline void Inversion() {
		BitFlagManager::BitInversion( m_Handle );
	}

private:
	BitFlagManager::Handle m_Handle; // ハンドル.
};
#endif