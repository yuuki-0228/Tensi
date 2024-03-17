#pragma once
#include <string>
#include <nameof\nameof.hpp>

// �����R�[�h.
enum class enCodePage {
	ANSI,
	Macintosh,
	OEM,
	Symbol,
	UTF7,
	UTF8,

	None = 0
} typedef ECodePage;

/************************************************
*	������ϊ�
*		���c�F�P.
**/
namespace StringConversion {
	// std::string �� std::wstring �ɕϊ�.
	std::wstring to_wString( const std::string& s, const ECodePage& CodePage = ECodePage::None );

	// std::wstring �� std::string �ɕϊ�.
	std::string to_String( const std::wstring& ws, const ECodePage& CodePage = ECodePage::None );

	// �w�肵�����ɂȂ�悤�ɍ����ɕ����𖄂߂�������ɕϊ�.
	std::string	 to_LeftPaddingString( const std::string&	s,	const int n, const char PaddedChar = ' ' );
	std::wstring to_LeftPaddingString( const std::wstring&	ws,	const int n, const char PaddedChar = ' ' );

	// �w�肵�����ɂȂ�悤�ɉE���ɕ����𖄂߂�������ɕϊ�.
	std::string  to_RightPaddingString( const std::string&  s,	const int n, const char PaddedChar = ' ' );
	std::wstring to_RightPaddingString( const std::wstring& ws,	const int n, const char PaddedChar = ' ' );

	// �ϐ��̌^�����擾(�S�ďȗ�).
	template<class T>
	inline std::string ShotType( const T& Var );

	// �ϐ��̌^�����擾(�Q��/�C���q���ȗ�).
	template<class T>
	inline std::string Type( const T& Var );

	// �ϐ��̌^�����擾(�S��).
	template<class T>
	inline std::string FullType( T Var );

	// �񋓑̖̂��O�̎擾.
	template<class T>
	inline std::string Enum( const T& Enum );

	// �񋓑̂��錾����Ă��邩�擾.
	template<class T>
	inline bool EnumCheck( int i );

	// �񋓑̗̂v�f�����擾.
	template<class T>
	inline int EnumNum();
}

//---------------------------.
// �ϐ��̌^�����擾(�S�ďȗ�).
//---------------------------.
template<class T>
std::string StringConversion::ShotType( const T& Var )
{
	return std::string( NAMEOF_SHORT_TYPE_EXPR( Var ) );
}

//---------------------------.
// �ϐ��̌^�����擾(�Q��/�C���q���ȗ�).
//---------------------------.
template<class T>
inline std::string StringConversion::Type( const T& Var ) {
	return std::string( NAMEOF_TYPE_EXPR( Var ) );
}

//---------------------------.
// �ϐ��̌^�����擾(�S��).
//---------------------------.
template<class T>
inline std::string StringConversion::FullType( T Var ) {
	return std::string( NAMEOF_FULL_TYPE_EXPR( Var ) );
}

//---------------------------.
// �񋓑̖̂��O�̎擾.
//---------------------------.
template<class T>
inline std::string StringConversion::Enum( const T& Enum ) {
	return std::string( NAMEOF_ENUM( Enum ) );
}

//---------------------------.
// �񋓑̂��錾����Ă��邩�擾.
//---------------------------.
template<class T>
inline bool StringConversion::EnumCheck( int i ) {
	return NAMEOF_ENUM_CHECK( static_cast<T>( i ) );
}

//---------------------------.
// �񋓑̗̂v�f�����擾.
//---------------------------.
template<class T>
inline int StringConversion::EnumNum() {
	int		Cnt = 0;
	while ( EnumCheck<T>( Cnt ) ) Cnt++;
	return	Cnt;
}
