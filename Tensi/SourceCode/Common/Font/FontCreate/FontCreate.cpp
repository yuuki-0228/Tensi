#include "FontCreate.h"
#include "..\..\DirectX\DirectX11.h"
#include "..\..\..\Utility\StringConversion\StringConversion.h"

namespace {
	const int FONT_BMP_SIZE	= 256;
}

CFontCreate::CFontCreate( const std::string& FilePath, const std::string& FileName )
	: m_pDevice		( nullptr )
	, m_pContext	( nullptr )
	, m_wFilePath	()
	, m_wFileName	()
{
	m_pDevice	= DirectX11::GetDevice();
	m_pContext	= DirectX11::GetContext();

	// �t�@�C���p�X/�t�@�C������ۑ�����.
	m_wFilePath = StringConversion::to_wString( FilePath );
	m_wFileName = StringConversion::to_wString( FileName );

	// �t�H���g�𗘗p�\�ɂ���.
	FontAvailable();
}

CFontCreate::~CFontCreate()
{
	// ���p�\�ɂ����t�H���g��j������.
	RemoveFontResourceEx( m_wFilePath.c_str(), FR_PRIVATE, nullptr );
}

//-----------------------------------.
// �t�H���g�摜�̍쐬.
//-----------------------------------.
HRESULT CFontCreate::CreateFontTexture2D( const char* c, ID3D11ShaderResourceView** resource )
{
	if( m_pDevice	== nullptr ) return E_FAIL;
	if( m_pContext	== nullptr ) return E_FAIL;

	// �����R�[�h�擾.
	std::wstring	wString = StringConversion::to_wString( c );
	UINT			Code	= (UINT)*wString.c_str();

	//-------------------------------------------------.
	// �t�H���g�̐���.
	//-------------------------------------------------.
	LOGFONT lf = { 
		FONT_BMP_SIZE,					// �����Z���܂��͕����̍���.
		0,								// ���ϕ�����.
		0,								// ��������̕�����X���Ƃ̊p�x.
		0,								// �x�[�X���C����X���Ƃ̊p�x.
		0,								// �t�H���g�̑���.
		0,								// �C�^���b�N�̂̎w��.
		0,								// �����t���w��.
		0,								// �ŏ������t���w��.
		SHIFTJIS_CHARSET,				// �L�����N�^�Z�b�g.
		OUT_TT_ONLY_PRECIS,				// �o�͐��x.
		CLIP_DEFAULT_PRECIS,			// �N���b�s���O�̐��x.
		PROOF_QUALITY,					// �o�͕i��.
		FIXED_PITCH | FF_MODERN,		// �s�b�`�ƃt�@�~��.
	};

	// �t�H���g����ݒ�.
	const int NameSize = static_cast<int>( m_wFileName.size() );
	for ( int i = 0; i < NameSize; ++i ) {
		lf.lfFaceName[i] = m_wFileName[i];
	}

	HFONT hFont = nullptr;
	hFont		= CreateFontIndirect( &lf );
	if(  hFont == nullptr ) return E_FAIL;

	HDC		Hdc		= nullptr;
	HFONT	OldFont	= nullptr;
	// �f�o�C�X�R���e�L�X�g�擾.
	//	�f�o�C�X�Ƀt�H���g���������Ȃ���GetGlyphOutline�֐��̓G���[�ƂȂ�.
	Hdc		= GetDC( nullptr );
	OldFont = (HFONT)SelectObject( Hdc, hFont );

	// �t�H���g�r�b�g�}�b�v�擾.
	TEXTMETRIC		TM;
	GetTextMetrics( Hdc, &TM );
	GLYPHMETRICS	GM;
	CONST MAT2		Mat		= { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
	DWORD			Size	= GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, 0, nullptr, &Mat );
	BYTE*			Ptr		= new BYTE[Size];
	GetGlyphOutline( Hdc, Code, GGO_GRAY4_BITMAP, &GM, Size, Ptr, &Mat );

	// �f�o�C�X�R���e�L�X�g�ƃt�H���g�n���h���̊J��.
	SelectObject( Hdc, OldFont );
	DeleteObject( hFont );
	ReleaseDC( nullptr, Hdc );


	//-------------------------------------------------.
	// �������݉\�e�N�X�`���쐬.
	//	CPU�ŏ������݂��ł���e�N�X�`�����쐬.
	//-------------------------------------------------.
	D3D11_TEXTURE2D_DESC	Desc		= CreateDesc( GM.gmCellIncX, TM.tmHeight );
	ID3D11Texture2D*		Texture2D	= nullptr;

	if ( FAILED( m_pDevice->CreateTexture2D( &Desc, 0, &Texture2D ) ) ) {
		return E_FAIL;
	}

	D3D11_MAPPED_SUBRESOURCE hMappedResource;
	if ( FAILED( m_pContext->Map(
		Texture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &hMappedResource ) ) ) {
		SAFE_RELEASE( Texture2D );
		return E_FAIL;
	}

	// �f�[�^���擾����.
	BYTE* pBits = (BYTE*)hMappedResource.pData;
	
	// �t�H���g���̏�������.
	//	iOfs_x, iOfs_y : �����o���ʒu( ���� ).
	//	iBmp_w, iBmp_h : �t�H���g�r�b�g�}�b�v�̕���.
	//	Level : ���l�̒i�K( GGO_GRAY4_BITMAP�Ȃ̂�17�i�K ).
	int iOfs_x	= GM.gmptGlyphOrigin.x;
	int iOfs_y	= TM.tmAscent - GM.gmptGlyphOrigin.y;
	int iBmp_w	= GM.gmBlackBoxX + ( 4 - ( GM.gmBlackBoxX % 4 ) ) % 4 ;
	int iBmp_h	= GM.gmBlackBoxY;
	int Level	= 17;
	int x, y;
	DWORD Alpha, Color;
	memset( pBits, 0, hMappedResource.RowPitch * TM.tmHeight );
	for ( y = iOfs_y; y < iOfs_y + iBmp_h; y++ ) {
		for ( x = iOfs_x; x < iOfs_x + iBmp_w; x++ ) {
			Alpha = ( 255 * Ptr[x - iOfs_x + iBmp_w * ( y - iOfs_y )]) / ( Level - 1 );
			Color = 0x00ffffff | ( Alpha << 24 );

			memcpy( (BYTE*) pBits + hMappedResource.RowPitch * y + 4 * x, &Color, sizeof( DWORD ) );
		}
	}
	m_pContext->Unmap( Texture2D, 0 );

	delete[] Ptr;

	//-------------------------------------------------.
	// �e�N�X�`�������V�F�[�_�[���\�[�X�r���[�ɂ���.
	//-------------------------------------------------.
	D3D11_TEXTURE2D_DESC texDesc;
	Texture2D->GetDesc( &texDesc );

	// ShaderResourceView�̏����쐬����.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory( &srvDesc, sizeof( srvDesc ) );
	srvDesc.Format						= texDesc.Format;
	srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip	= 0;
	srvDesc.Texture2D.MipLevels			= texDesc.MipLevels;

	if ( FAILED( m_pDevice->CreateShaderResourceView( Texture2D, &srvDesc, resource ) ) ) {
		SAFE_RELEASE( Texture2D );
		return E_FAIL;
	}

	SAFE_RELEASE( Texture2D );
	return S_OK;
}

//-----------------------------------.
// �t�H���g�𗘗p�\�ɂ���.
//-----------------------------------.
int CFontCreate::FontAvailable()
{
	return AddFontResourceEx( 
		m_wFilePath.c_str(),	// �t�H���g���\�[�X��.
		FR_PRIVATE,				// �v���Z�X�I�����ɃC���X�g�[�������t�H���g���폜.
		nullptr );				// �t�H���g�\����.
}

//-----------------------------------.
// Textur2D_Desc���쐬.
//-----------------------------------.
D3D11_TEXTURE2D_DESC CFontCreate::CreateDesc( UINT width, UINT height )
{
	D3D11_TEXTURE2D_DESC desc;
	memset( &desc, 0, sizeof( desc ) );
	desc.Width				= width;
	desc.Height				= height;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;	// RGBA(255,255,255,255)�^�C�v
	desc.SampleDesc.Count	= 1;
	desc.Usage				= D3D11_USAGE_DYNAMIC;			// ���I�i�������݂��邽�߂̕K�{�����j
	desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;	// �V�F�[�_���\�[�X�Ƃ��Ďg��
	desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;		// CPU����A�N�Z�X���ď�������OK

	return desc;
}