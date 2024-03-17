#pragma once
#include "..\..\..\Global.h"

/************************************************
*	�摜�t�@�C���̕��A�������擾����.
*		���c�F�P.
**/
namespace ImageSize {
	// �摜�t�@�C���̕��A�������擾����.
	D3DXVECTOR2 GetImageSize( const std::string& FilePath );


	// jpg�t�@�C���̕��A�������擾����.
	D3DXVECTOR2 GetJpgImageSize( const std::string& FilePath );

	// png�t�@�C���̕��A�������擾����.
	D3DXVECTOR2 GetPngImageSize( const std::string& FilePath );

	// bmp�t�@�C���̕��A�������擾����.
	D3DXVECTOR2 GetBmpImageSize( const std::string& FilePath );
}