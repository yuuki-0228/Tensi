#include "ImageSize.h"
#include <iostream>
#include <fstream>

namespace {
	constexpr int	_1BYTE_BIT_NUM		= 8;				// 1�o�C�g�̃r�b�g��.
	constexpr int	_3BYTE_BIT_NUM		= 24;				// 2�o�C�g�̃r�b�g��.
	constexpr int	_1BYTE_MAX_10BASE	= 256;				// 1�o�C�g�ŕ\����ő�10�i��.
	constexpr int	JPG_END_DQTMARKER	= 0xC0;				// jpg�̈ړ����I�����鐔.
	constexpr int	SKIP_2BIT			= 2;				// �g�p���Ȃ���񂪂��邽�ߔ�΂���.
	constexpr int	SKIP_5BIT			= 5;				// �g�p���Ȃ���񂪂��邽�ߔ�΂���.
	constexpr int	SKIP_16BIT			= 16;				// �g�p���Ȃ���񂪂��邽�ߔ�΂���.
	constexpr int	SKIP_18BIT			= 18;				// �g�p���Ȃ���񂪂��邽�ߔ�΂���.
	constexpr float ERROR_SIZE[2]		= { -1.0f, -1.0f };	// �G���[���o���Ƃ��Ɏ��ɕԂ����A����.

	// jpg�p�̉摜�f�[�^.
	struct stJpgData {
		std::uint16_t	Height;		// �摜�̍���.
		std::uint16_t	Width;		// �摜�̕�.
	} typedef SJpgData;

	// jpg�ȊO�̉摜�f�[�^.
	struct stData {
		std::uint32_t	Width;		// �摜�̕�.
		std::uint32_t	Height;		// �摜�̍���.
	} typedef SData;
}

//---------------------------.
// �摜�t�@�C���̕��A�������擾����.
//---------------------------.
D3DXVECTOR2 ImageSize::GetImageSize( const std::string& FilePath )
{
	// �g���q�����Ĕ��f���đΉ������֐����g�p����.
	std::string Extension = FilePath;
	Extension.erase( 0, FilePath.rfind( "." ) );
	if ( Extension == ".jpg" || Extension == ".JPG" ) return GetJpgImageSize( FilePath );
	if ( Extension == ".png" || Extension == ".PNG" ) return GetPngImageSize( FilePath );
	if ( Extension == ".bmp" || Extension == ".BMP" ) return GetBmpImageSize( FilePath );

	// jpg/png/bmp�t�@�C���ł͂Ȃ�.
	ErrorMessage( FilePath + "�͓ǂݍ��߂�摜�t�@�C���ł͖����������߁A�ǂݍ��߂܂���ł����B" );
	return ERROR_SIZE;
}

//---------------------------.
// jpg�t�@�C���̕��A�������擾����.
//---------------------------.
D3DXVECTOR2 ImageSize::GetJpgImageSize( const std::string& FilePath )
{
	SJpgData Image;

	// �t�@�C�����J��.
	std::ifstream file( FilePath.data(), std::ios_base::binary );
	if ( !file.is_open() ) {
		ErrorMessage( FilePath + "���J���܂���ł����B" );
		return ERROR_SIZE;
	};

	// �s�v�ȃZ�O�����g���ړ�����.
	int SeekPoint = SKIP_2BIT;
	std::uint8_t DQTMarker;
	do{
		SeekPoint += SKIP_2BIT;

		// �Z�O�����g�̒���(�o�C�g��)�̎擾.
		std::uint16_t ByteNum = 0;
		file.seekg( SeekPoint );
		if ( !file.read( ( char * )&ByteNum, sizeof( ByteNum ) ) ) {
			ErrorMessage( FilePath + "�̕��A�����̎擾�Ɏ��s���܂����B\njpg�t�@�C�����m�F���Ă��������B" );
			return ERROR_SIZE;
		}

		/*
		*	260�̏ꍇ�ȉ��̃r�b�g�̂悤�ɂȂ�.
		*	0000 0100	| 0000 0001
		*	��0 ~ 255(�l),	��256������(�ׂ���),
		*	���̂���8�r�b�g�E�ɃV�t�g���Ēl���擾.
		*	8�r�b�g���ɃV�t�g���Ēl�������Ă���8�r�b�g�E�ɃV�t�g���Ăׂ�����擾.
		**/
		const int Value	= ByteNum >> _1BYTE_BIT_NUM;
		ByteNum			<<= _1BYTE_BIT_NUM;
		const int Pow	= ByteNum >> _1BYTE_BIT_NUM;

		// �t�@�C���ɉ����ăr�b�g�����قȂ邽�ߎ擾���ړ����ɉ�����.
		SeekPoint += ( _1BYTE_MAX_10BASE * Pow ) + Value;

		// while�𔲂��邩���ׂ�.
		const int& s = SeekPoint + 1;
		file.seekg( s );
		if ( !file.read( ( char * )&DQTMarker, sizeof( DQTMarker ) ) ) {
			ErrorMessage( FilePath + "�̕��A�����̎擾�Ɏ��s���܂����B\njpg�t�@�C�����m�F���Ă��������B" );
			return ERROR_SIZE;
		}
	} while ( DQTMarker != JPG_END_DQTMARKER );

	// ���A�����̑O�ɂ���Ȃ���񂪂��邽�߈ړ����ēǂݍ���.
	SeekPoint += SKIP_5BIT;
	file.seekg( SeekPoint );
	if ( !file.read( ( char * )&Image, sizeof( Image ) ) ) {
		ErrorMessage( FilePath + "�̕��A�����̎擾�Ɏ��s���܂����B\njpg�t�@�C�����m�F���Ă��������B" );
		return ERROR_SIZE;
	}

	/*
	*	260�̏ꍇ�ȉ��̃r�b�g�̂悤�ɂȂ�.
	*	0000 0100	| 0000 0001
	*	��0 ~ 255(�l),	��256������(�ׂ���),	
	*	���̂���8�r�b�g�E�ɃV�t�g���Ēl���擾.
	*	8�r�b�g���ɃV�t�g���Ēl�������Ă���8�r�b�g�E�ɃV�t�g���Ăׂ�����擾.
	**/
	// ��.
	const int w_Value	= Image.Width >> _1BYTE_BIT_NUM;
	Image.Width			<<= _1BYTE_BIT_NUM;
	const int w_Pow		= Image.Width >> _1BYTE_BIT_NUM;
	// ����.
	const int h_Value	= Image.Height >> _1BYTE_BIT_NUM;
	Image.Height		<<= _1BYTE_BIT_NUM;
	const int h_Pow		= Image.Height >> _1BYTE_BIT_NUM;

	// �t�@�C�������.
	file.close();

	return D3DXVECTOR2( 
		static_cast<float>( _1BYTE_MAX_10BASE * w_Pow + w_Value ),
		static_cast<float>( _1BYTE_MAX_10BASE * h_Pow + h_Value )
	);
}

//---------------------------.
// png�t�@�C���̕��A�������擾����.
//---------------------------.
D3DXVECTOR2 ImageSize::GetPngImageSize( const std::string& FilePath )
{
	SData Image;

	// �t�@�C�����J��.
	std::ifstream file( FilePath.data(), std::ios_base::binary );
	if ( !file.is_open() ) {
		ErrorMessage( FilePath + "���J���܂���ł����B" );
		return ERROR_SIZE;
	}

	// ���A�����̑O�ɂ���Ȃ���񂪂��邽�߈ړ����ēǂݍ���.
	file.seekg( SKIP_16BIT );
	if ( !file.read( ( char * )&Image, sizeof( Image ) ) ) {
		ErrorMessage( FilePath + "�̕��A�����̎擾�Ɏ��s���܂����B\npng�t�@�C�����m�F���Ă��������B" );
		return ERROR_SIZE;
	}

	/*
	*	260�̏ꍇ�ȉ��̃r�b�g�̂悤�ɂȂ�.
	*	0000 0100	| 0000 0001				| 0000 0000 | 0000 0000.	
	*	��0 ~ 255(�l),	��256������(�ׂ���),		����������ȍ~�g��Ȃ�.
	*	���̂���24�r�b�g�E�ɃV�t�g���Ēl���擾.
	*	8�r�b�g���ɃV�t�g���Ēl�������Ă���24�r�b�g�E�ɃV�t�g���Ăׂ�����擾.
	**/
	// ��.
	const int w_Value	= Image.Width >> _3BYTE_BIT_NUM;
	Image.Width			<<= _1BYTE_BIT_NUM;
	const int w_Pow		= Image.Width >> _3BYTE_BIT_NUM;
	// ����.
	const int h_Value	= Image.Height >> _3BYTE_BIT_NUM;
	Image.Height		<<= _1BYTE_BIT_NUM;
	const int h_Pow		= Image.Height >> _3BYTE_BIT_NUM;

	return D3DXVECTOR2(
		static_cast<float>( _1BYTE_MAX_10BASE * w_Pow + w_Value ),
		static_cast<float>( _1BYTE_MAX_10BASE * h_Pow + h_Value )
	);
}

//---------------------------.
// jpg�t�@�C���̕��A�������擾����.
//---------------------------.
D3DXVECTOR2 ImageSize::GetBmpImageSize( const std::string& FilePath )
{
	SData Image;

	// �t�@�C�����J��.
	std::ifstream file( FilePath.data(), std::ios_base::binary );
	if ( !file.is_open() ) {
		ErrorMessage( FilePath + "���J���܂���ł����B" );
		return ERROR_SIZE;
	}

	// ���A�����̑O�ɂ���Ȃ���񕪈ړ����ēǂݍ���.
	file.seekg( SKIP_18BIT );
	if ( !file.read( (char*) &Image, sizeof( Image ) ) ) {
		ErrorMessage( FilePath + "�̕��A�����̎擾�Ɏ��s���܂����B\nbmp�t�@�C�����m�F���Ă��������B" );
		return ERROR_SIZE;
	}

	return D3DXVECTOR2( 
		static_cast<float>( Image.Width ),
		static_cast<float>( Image.Height )
	);
}
