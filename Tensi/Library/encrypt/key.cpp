#include "key.h"

//---------------------------.
// �Ή������t�@�C��ID���擾.
//---------------------------.
std::string SecretKey::GetFileId( const std::string& Ext )
{
	std::string e = Ext;
	if ( e.substr( 0, 1 ) == "." ) e.erase( 0, 1 );

	for ( auto [List, id] : FILE_LIST ) {
		auto [e1, e2] = List;
		if ( e == e1 || e == e2 ) {
			return id;
		}
	}
	return ErrorId;
}

//---------------------------.
// �Ή������p�X�̎擾.
//---------------------------.
std::string SecretKey::GetFileExt( const std::string& FId )
{
	for ( auto [List, id] : FILE_LIST ) {
		if ( FId == id ) {
			return "." + List.first;
		}
	}
	return ErrorId;
}

//---------------------------.
// �Í���.
//---------------------------.
void SecretKey::Encryption( char* data, const DWORD& size )
{
	for ( int i = 0; i < static_cast<int>( size ); i++ ) {
		int keyIndex = 0;
		for ( int j = KEY_NUM; j >= 2; j-- ) {
			if ( ( i + 1 ) % j == 0 ) {
				keyIndex = j - 1;
				break;
			}
		}
		data[i] ^= SECRET_KEY[keyIndex];
	}
}

//---------------------------.
// �Í��������ɖ߂�
//---------------------------.
void SecretKey::Restore( char* data, const DWORD& size )
{
	for ( int i = 0; i < static_cast<int>( size ); i++ ) {
		int keyIndex = 0;
		for ( int j = KEY_NUM; j >= 2; j-- ) {
			if ( ( i + 1 ) % j == 0 ) {
				keyIndex = j - 1;
				break;
			}
		}
		data[i] ^= SECRET_KEY[keyIndex];
	}
}
