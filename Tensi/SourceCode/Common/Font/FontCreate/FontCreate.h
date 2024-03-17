#pragma once
#include "..\..\..\Global.h"
#include <Shlwapi.h>

/***********************************
*	�t�H���g�X�v���C�g�쐬�N���X.
*		���c�F�P.
**/
class CFontCreate final
{
public:
	CFontCreate( const std::string& FilePath, const std::string& FileName );
	~CFontCreate();

	// �t�H���g�摜�̍쐬.
	HRESULT CreateFontTexture2D( const char* c, ID3D11ShaderResourceView** textur2D );

private:
	// �t�H���g�𗘗p�\�ɂ���.
	int FontAvailable();
	// Textur2D_Desc���쐬.
	D3D11_TEXTURE2D_DESC CreateDesc( UINT width, UINT height );

private:
	ID3D11Device*			m_pDevice;		// �f�o�C�X.
	ID3D11DeviceContext*	m_pContext;		// �f�o�C�X�R���e�L�X�g.
	std::wstring			m_wFilePath;	// �t�@�C���p�X.
	std::wstring			m_wFileName;	// �t�@�C����.

private:
	// �f�t�H���g�R���X�g���N�^���֎~����.
	CFontCreate() = delete;
	CFontCreate( const CFontCreate& ) = delete;
	CFontCreate& operator = ( const CFontCreate& rhs ) = default;
};
