#include "Font.h"
#include "..\DirectX\DirectX11.h"
#include "..\Sprite\Sprite.h"
#include "..\..\Resource\FontResource\FontResource.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	// �V�F�[�_�t�@�C����(�f�B���N�g�����܂�).
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Font.hlsl" );

	// �t�H���g�f�[�^�t�@�C���p�X.
	constexpr char FONT_DATA_FILE_PATH[] = "Data\\Sprite\\Font\\FontData.json";
}

CFont::CFont()
	: m_pDevice					( nullptr )
	, m_pContext				( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pVertexBufferUI			( nullptr )
	, m_pVertexBuffer3D			( nullptr )
	, m_pConstantBuffer			( nullptr )
	, m_Vertices				()
	, m_FontState				()
	, m_FontRenderState			()
	, m_pSampleLinears			()
	, m_FileName				( "" )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
{
}

CFont::~CFont()
{
	for ( auto& s : m_pSampleLinears )	SAFE_RELEASE( s );
	SAFE_RELEASE( m_pVertexBuffer3D	);
	SAFE_RELEASE( m_pVertexBufferUI	);
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader	);
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//---------------------------------.
// ������.
//---------------------------------.
HRESULT CFont::Init( const std::string& FilePath, const std::string& FileName )
{
	m_pContext	= DirectX11::GetContext();
	m_pDevice	= DirectX11::GetDevice();
	m_FileName	= FileName;

	// �X�v���C�g���̎擾.
	if ( FAILED( FontStateDataLoad( FilePath ) ) ) return E_FAIL;

	// ������.
	if ( FAILED( CreateShader()	 ) ) return E_FAIL;
	if ( FAILED( CreateModelUI() ) ) return E_FAIL;
	if ( FAILED( CreateModel3D() ) ) return E_FAIL;
	if ( FAILED( CreateSampler() ) ) return E_FAIL;
	return S_OK;
}


//---------------------------------.
// UI�ŕ`��.
//---------------------------------.
void CFont::RenderUI( SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	RenderUI( RenderState->Text, RenderState );
}
void CFont::RenderUI( const std::string& Text, SFontRenderState* pRenderState )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	// ��\���̏ꍇ�͏������s��Ȃ�.
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	// �����̑傫���擾.
	const float			TextSizeW = m_FontState.Disp.w * RenderState->Transform.GetWorldScale().x;
	const float			TextSizeH = m_FontState.Disp.h * RenderState->Transform.GetWorldScale().x;

	// �������̎擾.
	const std::wstring	wText	= StringConversion::to_wString( Text.substr( 0, Text.find( "\n" ) ) );
	const int			TextNum = static_cast<int>( wText.size() );

	// �e�L�X�g�̕\���`���ɑΉ������J�n�ʒu�̕ύX.
	const D3DXPOSITION3 Pos		= RenderState->Transform.Position;
	bool				IsMove	= false;
	if ( RenderState->TextAlign == ETextAlign::Right ) {
		RenderState->Transform.Position.x -= TextSizeW * TextNum;
		IsMove = true;
	}
	else if ( RenderState->TextAlign == ETextAlign::Center ) {
		RenderState->Transform.Position.x -= TextSizeW * ( TextNum / 2.0f );
		IsMove = true;
	}

	// ���S���W�ɂ�Ĉʒu�𒲐�����.
	if ( IsMove ){
		if ( m_FontState.LocalPosNo == ELocalPosition::Up ||
			 m_FontState.LocalPosNo == ELocalPosition::Center ||
			 m_FontState.LocalPosNo == ELocalPosition::Up )
		{
			RenderState->Transform.Position.x += TextSizeW / 2.0f;
		}
		if ( m_FontState.LocalPosNo == ELocalPosition::RightUp ||
			 m_FontState.LocalPosNo == ELocalPosition::Right ||
			 m_FontState.LocalPosNo == ELocalPosition::RightDown )
		{
			RenderState->Transform.Position.x += TextSizeW;
		}
	}

	// �����������[�v.
	for ( int i = 0; i < static_cast<int>( Text.length() ); ++i ){
		// �\�����镶���̎擾.
		std::string f = Text.substr( i, 1 );
		if( IsDBCSLeadByte( Text[i] ) == TRUE ){
			f = Text.substr( i++, 2 );	// �S�p����.
		} else {
			f = Text.substr( i, 1 );	// ���p����.
		}

		// ���s.
		if ( f == "\n" ) {
			// �������̎擾.
			const int			NextNo	= i + 1;
			const std::wstring	wText	= StringConversion::to_wString( Text.substr( NextNo, Text.find( "\n", NextNo ) - NextNo ) );
			const int			TextNum = static_cast<int>( wText.size() );

			// �e�L�X�g�̕\���`���ɑΉ������J�n�ʒu�̕ύX.
			RenderState->Transform.Position.x = Pos.x;
			if ( RenderState->TextAlign == ETextAlign::Right ) {
				RenderState->Transform.Position.x -= TextSizeW * TextNum;
			}
			else if ( RenderState->TextAlign == ETextAlign::Center ) {
				RenderState->Transform.Position.x -= TextSizeW * static_cast< float >( TextNum ) / 2.0f;
			}
			RenderState->Transform.Position.y += TextSizeH + RenderState->TextInterval.h;
			continue;
		}

		// �A�E�g���C���̕`��( 4�����ɂ��炵���t�H���g��`�悷�� ).
		if ( RenderState->IsOutLine || RenderState->IsBold ) {
			for ( int i = 0; i < 4; ++i ) {
				SFontRenderState OutLineState = *RenderState;

				// �A�E�g���C���̏ꍇ�A�E�g���C���̐F�𔽉f������.
				if ( OutLineState.IsOutLine ) OutLineState.Color = OutLineState.OutLineColor;

				// �A�E�g���C���̕`��.
				OutLineState.Transform.Position.x += -( OutLineState.OutLineSize / 2 ) + OutLineState.OutLineSize * ( i / 2 );
				OutLineState.Transform.Position.y += -( OutLineState.OutLineSize / 2 ) + OutLineState.OutLineSize * ( i % 2 );
				RenderFontUI( f.c_str(), &OutLineState );
			}
		}

		// �\�����镶���̕`��.
		RenderFontUI( f.c_str(), RenderState );
		RenderState->Transform.Position.x += TextSizeW + +RenderState->TextInterval.w;
	}
	RenderState->Transform.Position = Pos;
}

//---------------------------------.
// 3D�ŕ`��.
//---------------------------------.
void CFont::Render3D( SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	Render3D( RenderState->Text, RenderState );
}
void CFont::Render3D( const std::string& Text, SFontRenderState* pRenderState, const bool& IsBillBoard )
{
	SFontRenderState* RenderState = pRenderState == nullptr ? &m_FontRenderState : pRenderState;

	// ��\���̏ꍇ�͏������s��Ȃ�.
	if ( RenderState->IsDisp	== false	) return;
	if ( RenderState->Color.w	== 0.0f		) return;

	// �����������[�v.
	const D3DXPOSITION3 Pos = RenderState->Transform.Position;
	for( int i = 0; i < static_cast<int>( Text.length() ); ++i ){
		// �\�����镶���̎擾.
		std::string f = Text.substr( i, 1 );
		if( IsDBCSLeadByte( Text[i] ) == TRUE ){
			f = Text.substr( i++, 2 );	// �S�p����.
		} else {
			f = Text.substr( i, 1 );	// ���p����.
		}

		// ���s.
		if ( f == "\n" ) {
			RenderState->Transform.Position.x = Pos.x;
			RenderState->Transform.Position.y += m_FontState.Disp.h * RenderState->Transform.Scale.y + 12.0f;
			continue;
		}

		// �A�E�g���C���̕`��.
		if ( RenderState->IsOutLine || RenderState->IsBold ) {
			const D3DXPOSITION3& FontPos = RenderState->Transform.Position;

			// 4�����ɂ��炵���t�H���g��`�悷��.
			for ( int i = 0; i < 4; ++i ) {
				RenderState->Transform.Position.x += -(RenderState->OutLineSize / 2) + RenderState->OutLineSize * (i / 2);
				RenderState->Transform.Position.y += -(RenderState->OutLineSize / 2) + RenderState->OutLineSize * (i % 2);
				RenderFontUI( f.c_str(), RenderState );
			}

			// �t�H���g�̈ʒu�����ɖ߂�.
			RenderState->Transform.Position = FontPos;
		}

		// �\�����镶���̕`��.
		RenderFont3D( f.c_str(), RenderState, IsBillBoard );
		RenderState->Transform.Position.x += m_FontState.Disp.w * RenderState->Transform.Scale.x;
	}
	RenderState->Transform.Position = Pos;
}

//----------------------------.
// �t�H���g���̓ǂݍ���.
//----------------------------.
HRESULT CFont::FontStateDataLoad( const std::string& FilePath )
{
	// �������O�̃e�L�X�g�̓ǂݍ���.
	std::string TextPath = FilePath;
	TextPath.erase( TextPath.find( "." ), TextPath.size() ) += ".json";

#ifndef _DEBUG
	// �Í�������Ă��邽�ߌ��̃p�X�ɖ߂�
	auto dp = TextPath.rfind( "." );
	auto fp = TextPath.rfind( "\\" ) + 1;
	if ( dp != std::string::npos && fp != std::string::npos ) {
		std::string n = TextPath.substr( fp, dp - fp );
		n.erase( 0, 1 );
		n.erase( n.length() - 2 );
		TextPath.erase( fp, dp - fp );
		TextPath.insert( fp, n );
	}
#endif
	json j = FileManager::JsonLoad( TextPath );

	// �������O�̃e�L�X�g�������������ߋ��L�̃t�@�C����ǂݍ���.
	if ( j.empty() ) {
		// �t�H���g�f�[�^�t�@�C���̓ǂݍ���.
		j = FileManager::JsonLoad( FONT_DATA_FILE_PATH );

		// �t�@�C�����������߃t�@�C�����쐬����.
		if ( j.empty() ) return E_FAIL;
	}

	// �X�v���C�g���̎擾.
	m_FontState.LocalPosNo	= static_cast<ELocalPosition>( j["LocalPosition"] );
	m_FontState.Disp.w		= j["Disp"]["w"];
	m_FontState.Disp.h		= j["Disp"]["h"];
	return S_OK;
}

//----------------------------.
// �V�F�[�_�쐬.
//----------------------------.
HRESULT CFont::CreateShader()
{
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	auto result = DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// �V�F�[�_�t�@�C����(HLSL�t�@�C��).
		nullptr,			// �}�N����`�̔z��ւ̃|�C���^(���g�p).
		nullptr,			// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		"VS_Main",			// �V�F�[�_�G���g���[�|�C���g�֐��̖��O.
		"vs_5_0",			// �V�F�[�_�̃��f�����w�肷�镶����(�v���t�@�C��).
		uCompileFlag,		// �V�F�[�_�R���p�C���t���O.
		0,					// �G�t�F�N�g�R���p�C���t���O(���g�p).
		nullptr,			// �X���b�h �|���v �C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		&pCompiledShader,	// �u���u���i�[���郁�����ւ̃|�C���^.
		&pErrors,			// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
		nullptr );			// �߂�l�ւ̃|�C���^(���g�p).
	if ( FAILED( result ) )
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�o�[�e�b�N�X�V�F�[�_�v���쐬.
	result = DirectX11::MutexDX11CreateVertexShader(
		pCompiledShader,
		nullptr,
		&m_pVertexShader );	// (out)�o�[�e�b�N�X�V�F�[�_.
	if ( FAILED( result ) )
	{
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�쐬���s",result );
		return E_FAIL;
	}

	// ���_�C���v�b�g���C�A�E�g���`.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						// �ʒu.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	// DXGI�̃t�H�[�}�b�g(32bit float�^*3).
			0,
			0,								// �f�[�^�̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD",						// �e�N�X�`���ʒu.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		// DXGI�̃t�H�[�}�b�g(32bit float�^*2).
			0,
			12,								// �f�[�^�̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	// ���_�C���v�b�g���C�A�E�g�̔z��v�f�����Z�o.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// ���_�C���v�b�g���C�A�E�g���쐬.
	result = DirectX11::MutexDX11CreateInputLayout(
		layout,
		numElements,
		pCompiledShader,
		&m_pVertexLayout );	//(out)���_�C���v�b�g���C�A�E�g.
	if (FAILED( result ) )
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSL����s�N�Z���V�F�[�_�̃u���u���쐬.
	 DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,				// �V�F�[�_�t�@�C����(HLSL�t�@�C��).
		nullptr,					// �}�N����`�̔z��ւ̃|�C���^(���g�p).
		nullptr,					// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		"PS_Main",					// �V�F�[�_�G���g���[�|�C���g�֐��̖��O.
		"ps_5_0",					// �V�F�[�_�̃��f�����w�肷�镶����(�v���t�@�C��).
		uCompileFlag,				// �V�F�[�_�R���p�C���t���O.
		0,							// �G�t�F�N�g�R���p�C���t���O(���g�p).
		nullptr,					// �X���b�h �|���v �C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		&pCompiledShader,			// �u���u���i�[���郁�����ւ̃|�C���^.
		&pErrors,					// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
		nullptr );					// �߂�l�ւ̃|�C���^(���g�p).
	if (FAILED( result ) )
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�s�N�Z���V�F�[�_�v���쐬.
	result = DirectX11::MutexDX11CreatePixelShader(
		pCompiledShader,
		nullptr,
		&m_pPixelShader );	// (out)�s�N�Z���V�F�[�_.
	if (FAILED( result ) )
	{
		ErrorMessage( "�s�N�Z���V�F�[�_�쐬���s", result );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// �R���X�^���g(�萔)�o�b�t�@�쐬.
	//	�V�F�[�_�ɓ���̐��l�𑗂�o�b�t�@.
	//	�����ł͕ϊ��s��n���p.
	//	�V�F�[�_�� World, View, Projection �s���n��.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;			// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof(SFontShaderConstantBuffer);	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;				// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;									// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;									// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;					// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	result = m_pDevice->CreateBuffer(
		&cb,
		nullptr,
		&m_pConstantBuffer );
	if (FAILED( result ) )
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// ���f���쐬(UI).
//----------------------------.
HRESULT CFont::CreateModelUI()
{
	float w = m_FontState.Disp.w;
	float h = m_FontState.Disp.h;

	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	CreateVertex( w, h, 1.0f, 1.0f );
	// �ő�v�f�����Z�o����.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// �o�b�t�@�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// �g�p���@(�f�t�H���g).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// ���_�̃T�C�Y.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// ���_�o�b�t�@�Ƃ��Ĉ���.
	bd.CPUAccessFlags		= 0;							// CPU����̓A�N�Z�X���Ȃ�.
	bd.MiscFlags			= 0;							// ���̑��̃t���O(���g�p).
	bd.StructureByteStride	= 0;							// �\���̂̃T�C�Y(���g�p).

	// �T�u���\�[�X�f�[�^�\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;	// �|���̒��_���Z�b�g.

	// ���_�o�b�t�@�̍쐬.
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBufferUI );
	if ( FAILED( result ) )
	{
		ErrorMessage( "���_�o�b�t�@UI�쐬���s" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// ���f���쐬(3D).
//----------------------------.
HRESULT CFont::CreateModel3D()
{
	// �摜�̔䗦�����߂�.
	int		as = myGcd( static_cast<int>( m_FontState.Disp.w ), static_cast<int>( m_FontState.Disp.h ) );
	float	w = ( m_FontState.Disp.w / as ) * m_FontState.Disp.w * 0.1f;
	float	h = ( m_FontState.Disp.h / as ) * m_FontState.Disp.h * 0.1f;

	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	CreateVertex( w, -h, 1.0f, 1.0f );
	// �ő�v�f�����Z�o����.
	UINT uVerMax = sizeof( m_Vertices ) / sizeof( m_Vertices[0] );

	// �o�b�t�@�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;			// �g�p���@(�f�t�H���g).
	bd.ByteWidth			= sizeof(SVertex) * uVerMax;	// ���_�̃T�C�Y.
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;		// ���_�o�b�t�@�Ƃ��Ĉ���.
	bd.CPUAccessFlags		= 0;							// CPU����̓A�N�Z�X���Ȃ�.
	bd.MiscFlags			= 0;							// ���̑��̃t���O(���g�p).
	bd.StructureByteStride	= 0;							// �\���̂̃T�C�Y(���g�p).

	// �T�u���\�[�X�\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Vertices;

	// ���_�o�b�t�@�̍쐬.
	auto result = m_pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer3D );
	if (FAILED( result ) )
	{
		ErrorMessage( "���_�o�b�t�@3D�쐬���s" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �T���v���쐬.
//----------------------------.
HRESULT CFont::CreateSampler()
{
	// �e�N�X�`���p�̃T���v���\����.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof(samDesc) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;

	// �T���v���쐬.
	for( int i = 0; i < static_cast<int>( ESamplerState::Max ); i++ ){
		samDesc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		samDesc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>( i + 1 );
		auto result = m_pDevice->CreateSamplerState( &samDesc, &m_pSampleLinears[i] );
		if( FAILED( result ) ){
			ErrorMessage( "�T���v���쐬 : ���s" );
			return E_FAIL;
		}
	}
	return S_OK;
}

//---------------------------.
// ���_���̍쐬.
//---------------------------.
HRESULT CFont::CreateVertex( const float w, const float h, const float u, const float v )
{
	// �|��(�l�p�`)�̒��_���쐬.
	//	���_���W(x,y,z), UV���W(u,v).
	switch ( m_FontState.LocalPosNo ) {
	case ELocalPosition::LeftUp:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Left:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::LeftDown:
		m_Vertices[0] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(    w, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(    w,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Down:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::RightDown:
		m_Vertices[0] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w,   -h, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f,   -h, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Right:
		m_Vertices[0] = { D3DXPOSITION3(   -w,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::RightUp:
		m_Vertices[0] = { D3DXPOSITION3(   -w,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3(   -w, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3( 0.0f,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3( 0.0f, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Up:
		m_Vertices[0] = { D3DXPOSITION3( -w / 2,    h, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXPOSITION3( -w / 2, 0.0f, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXPOSITION3(  w / 2,    h, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXPOSITION3(  w / 2, 0.0f, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	case ELocalPosition::Center:
	default: 
		m_Vertices[0] = { D3DXVECTOR3( -w / 2,  h / 2, 0.0f ),	D3DXVECTOR2( 0.0f,    v ) };	// ���_�P(����).
		m_Vertices[1] = { D3DXVECTOR3( -w / 2, -h / 2, 0.0f ),	D3DXVECTOR2( 0.0f, 0.0f ) };	// ���_�P(����).
		m_Vertices[2] = { D3DXVECTOR3(  w / 2,  h / 2, 0.0f ),	D3DXVECTOR2(    u,    v ) };	// ���_�P(����).
		m_Vertices[3] = { D3DXVECTOR3(  w / 2, -h / 2, 0.0f ),	D3DXVECTOR2(    u, 0.0f ) };	// ���_�P(����).
		break;
	}
	return S_OK;
}


//---------------------------------.
// �e�N�X�`���̕`��.
//---------------------------------.
void CFont::RenderFontUI( const char* c, SFontRenderState* pRenderState )
{
	// �e�N�X�`���̎擾.
	ID3D11ShaderResourceView* pTexture = FontResource::GetFontTexture( m_FileName, c );

	// ���[���h�s����擾.
	D3DXMATRIX mWorld = pRenderState->Transform.GetWorldMatrix();

	// �g�p����V�F�[�_�̓o�^.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE	pData;
	SFontShaderConstantBuffer	cb;	// �R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ���[���h�s���n��(�r���[,�v���W�F�N�V�����͎g��Ȃ�).
		D3DXMATRIX m = mWorld;
		D3DXMatrixTranspose( &m, &m );// �s���]�u����.
		cb.mWVP = m;

		// �J���[.
		cb.vColor = pRenderState->Color;

		// �e�N�X�`�����W(UV���W).
		// 1�}�X������̊����Ƀp�^�[���ԍ�(�}�X��)�������č��W��ݒ肷��.
		cb.vUV.x = 0.0f;
		cb.vUV.y = 0.0f;

		// �`�悷��G���A.
		D3DXVECTOR4 RenderArea = pRenderState->RenderArea;
		const float WndScalseW = DirectX11::GetWndWidth()  / FWND_W;
		const float WndScalseH = DirectX11::GetWndHeight() / FWND_H;
		RenderArea.x	*= WndScalseW;
		RenderArea.z	*= WndScalseW;
		RenderArea.y	*= WndScalseH;
		RenderArea.w	*= WndScalseH;
		cb.vRenderArea	 = RenderArea;

		// �r���[�|�[�g�̕��A������n��.
		cb.fViewPortWidth	= FWND_W;
		cb.fViewPortHeight	= FWND_H;

		// �f�B�U�������g�p���邩.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof( SVertex );// �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBufferUI, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// �e�N�X�`�����V�F�[�_�ɓn��.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( pRenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &pTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}

//---------------------------------.
// �e�N�X�`���̕`��.
//---------------------------------.
void CFont::RenderFont3D( const char* c, SFontRenderState* pRenderState, const bool& IsBillBoard  )
{
	// �e�N�X�`���̎擾.
	ID3D11ShaderResourceView* pTexture = FontResource::GetFontTexture( m_FileName, c );

	// ���[���h�s����擾.
	D3DXMATRIX mWorld = pRenderState->Transform.GetWorldMatrix();

	// �r���{�[�h�p.
	if ( IsBillBoard == true ) {
		D3DXMATRIX CancelRotation = CameraManager::GetViewMatrix();	// �r���[�s��.
		CancelRotation._41
			= CancelRotation._42 = CancelRotation._43 = 0.0f;			// xyz��0�ɂ���.
		// CancelRotation�̋t�s������߂܂�.
		D3DXMatrixInverse( &CancelRotation, nullptr, &CancelRotation );
		mWorld = CancelRotation * mWorld;
	}

	// �g�p����V�F�[�_�̓o�^.
	m_pContext->VSSetShader( m_pVertexShader,	nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader,	nullptr, 0 );

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE	pData;
	SFontShaderConstantBuffer	cb;	// �R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if ( SUCCEEDED(
		m_pContext->Map( m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		// ���[���h,�r���[,�v���W�F�N�V�����s���n��.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose( &m, &m );// �s���]�u����.
		cb.mWVP = m;

		// �J���[.
		cb.vColor = pRenderState->Color;

		// �e�N�X�`�����W(UV���W).
		cb.vUV.x = 0.0f;
		cb.vUV.y = 0.0f;

		// �`�悷��G���A.
		D3DXVECTOR4 RenderArea = pRenderState->RenderArea;
		const float WndScalseW = DirectX11::GetWndWidth()  / FWND_W;
		const float WndScalseH = DirectX11::GetWndHeight() / FWND_H;
		RenderArea.x	*= WndScalseW;
		RenderArea.z	*= WndScalseW;
		RenderArea.y	*= WndScalseH;
		RenderArea.w	*= WndScalseH;
		cb.vRenderArea	 = RenderArea;

		// �r���[�|�[�g�̕��A�����͎g�p���Ȃ����ߏ�����.
		cb.fViewPortWidth	= 0.0f;
		cb.fViewPortHeight	= 0.0f;

		// �f�B�U�������g�p���邩.
		cb.vDitherFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vAlphaBlockFlag.x = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		memcpy_s( pData.pData, pData.RowPitch,
			(void*) ( &cb ), sizeof( cb ) );

		m_pContext->Unmap( m_pConstantBuffer, 0 );
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof( SVertex );// �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext->IASetVertexBuffers( 0, 1,
		&m_pVertexBuffer3D, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout( m_pVertexLayout );
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// �e�N�X�`�����V�F�[�_�ɓn��.
	m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinears[static_cast<Sampler>( pRenderState->SmaplerNo )] );
	m_pContext->PSSetShaderResources( 0, 1, &pTexture );

	// �A���t�@�u�����h�L���ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	if ( !m_DitherFlag ) DirectX11::SetAlphaBlend( false );
}