#include "Fog.h"
#include "..\..\Object\Camera\CameraManager\CameraManager.h"

namespace{
	constexpr TCHAR SHADER_NAME[]	= _T( "Data\\Shader\\Fog.hlsl" );	// �V�F�[�_�t�@�C����( �f�B���N�g�����܂� ).
	constexpr float FOG_E			= 2.71828f;							// �w���ΐ��̒�.
}

CFog::CFog()
	: m_vertexShader	( 0 )
	, m_pixelShader		( 0 )
	, m_layout			( 0 )
	, m_constantBuffer	( 0 )
	, m_sampleState		( 0 )
	, m_fogBuffer		( 0 )
	, m_Density			( 0.111f )
{
}

CFog::~CFog()
{
	Release();
}

//---------------------------.
// ������.
//---------------------------.
bool CFog::Init()
{
	bool result;

	// ���_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[��������.
	result = InitializeShader();
	if ( !result ) return false;

	return true;
}

//---------------------------.
// �`��.
//---------------------------.
bool CFog::Render( int indexCount, D3DXMATRIX worldMatrix,
	ID3D11ShaderResourceView* texture, float fogStart, float fogEnd )
{
	bool result;

	// �����_�����O�Ɏg�p����V�F�[�_�[�p�����[�^��ݒ�.
	result = SetShaderParameters( worldMatrix, texture, fogStart, fogEnd );
	if ( !result ) return false;

	// �o�b�t�@���V�F�[�_�[�Ń����_�����O.
	RenderShader( indexCount );

	return true;
}

//---------------------------.
// �V�F�[�_�̏�����.
//---------------------------.
bool CFog::InitializeShader()
{
	HRESULT				result;
	D3D11_BUFFER_DESC	constantBufferDesc	= {};
	D3D11_SAMPLER_DESC	samplerDesc			= {};
	D3D11_BUFFER_DESC	fogBufferDesc		= {};

	ID3DBlob*			pCompiledShader		= nullptr;
	ID3DBlob*			pErrors				= nullptr;
	UINT				uCompileFlag		= 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	if ( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// �V�F�[�_�[�t�@�C����.
		nullptr,			// �}�N����`�̔z��ւ̃|�C���^(���g�p).
		nullptr,			// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		"VS_Main",			// �V�F�[�_�[�G���g���[�|�C���g�֐��̖��O.
		"vs_5_0",			// �V�F�[�_�[�̃��f�����w�肷�镶����(�v���t�@�C��).
		uCompileFlag,		// �V�F�[�_�[�R���p�C���t���O.
		0,					// �G�t�F�N�g�R���p�C���t���O(���g�p).
		nullptr,			// �X���b�h�|���v�C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		&pCompiledShader,	// �u���u���i�[���郁�����ւ̃|�C���^.
		&pErrors,			// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
		nullptr ) ) )		// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return false;
	}
	SAFE_RELEASE( pErrors );

	// ��L�ō쐬�����u���u����u�o�[�e�b�N�X�V�F�[�_�v���쐬.
	if ( FAILED(
		DirectX11::MutexDX11CreateVertexShader(
		pCompiledShader,
		nullptr,
		&m_vertexShader ) ) )	//(out)�o�[�e�b�N�X�V�F�[�_.
	{
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�쐬���s" );
		return false;
	}

	// ���_�C���v�b�g���C�A�E�g���`.
	D3D11_INPUT_ELEMENT_DESC layout[2];
	// ���_�C���v�b�g���C�A�E�g�̔z��v�f�����Z�o.
	UINT numElements = 0;
	D3D11_INPUT_ELEMENT_DESC tmp[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = sizeof( tmp ) / sizeof( tmp[0] );	// �v�f���Z�o.
	memcpy_s( layout, sizeof( layout ),
		tmp, sizeof( D3D11_INPUT_ELEMENT_DESC )*numElements );
	// ���_�C���v�b�g���C�A�E�g���쐬.
	if ( FAILED(
		DirectX11::MutexDX11CreateInputLayout(
		layout,
		numElements,
		pCompiledShader,
		&m_layout ) ) )	// (out)���_�C���v�b�g���C�A�E�g.
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s" );
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// HLSL����s�N�Z���V�F�[�_�̃u���u���쐬.
	if ( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
		SHADER_NAME,		// �V�F�[�_�[�t�@�C����.
		nullptr,			// �}�N����`�̔z��ւ̃|�C���^(���g�p).
		nullptr,			// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		"PS_Main",			// �V�F�[�_�[�G���g���[�|�C���g�֐��̖��O.
		"ps_5_0",			// �V�F�[�_�[�̃��f�����w�肷�镶����(�v���t�@�C��).
		uCompileFlag,		// �V�F�[�_�[�R���p�C���t���O.
		0,					// �G�t�F�N�g�R���p�C���t���O(���g�p).
		nullptr,			// �X���b�h�|���v�C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
		&pCompiledShader,	// �u���u���i�[���郁�����ւ̃|�C���^.
		&pErrors,			// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
		nullptr ) ) )		// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return false;
	}
	SAFE_RELEASE( pErrors );

	// ��L�ō쐬��������ނ���u�s�N�Z���V�F�[�_�v���쐬.
	if ( FAILED(
		DirectX11::MutexDX11CreatePixelShader(
		pCompiledShader,
		nullptr,
		&m_pixelShader ) ) )	// (out)�s�N�Z���V�F�[�_.
	{
		ErrorMessage( "�s�N�Z���V�F�[�_�쐬���s" );
		return false;
	}
	SAFE_RELEASE( pCompiledShader );

	// ���_�V�F�[�_�[�̐ݒ�.
	constantBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
	constantBufferDesc.ByteWidth			= sizeof( ConstantBufferType );
	constantBufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.MiscFlags			= 0;
	constantBufferDesc.StructureByteStride	= 0;

	// �萔�o�b�t�@�[�|�C���^�[���쐬.
	result = m_pDevice->CreateBuffer( &constantBufferDesc, NULL, &m_constantBuffer );
	if ( FAILED( result ) ) return false;

	// �e�N�X�`���T���v���[�̐ݒ�.
	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias		= 0.0f;
	samplerDesc.MaxAnisotropy	= 1;
	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0]	= 0;
	samplerDesc.BorderColor[1]	= 0;
	samplerDesc.BorderColor[2]	= 0;
	samplerDesc.BorderColor[3]	= 0;
	samplerDesc.MinLOD			= 0;
	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	// �e�N�X�`���T���v���[�̍쐬.
	result = m_pDevice->CreateSamplerState( &samplerDesc, &m_sampleState );
	if ( FAILED( result ) ) return false;

	// ���_�V�F�[�_�[�ɂ���_�C�i�~�b�N�t�H�O�萔�o�b�t�@�[�̐ݒ�.
	fogBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
	fogBufferDesc.ByteWidth				= sizeof( FogBufferType );
	fogBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	fogBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	fogBufferDesc.MiscFlags				= 0;
	fogBufferDesc.StructureByteStride	= 0;

	// �t�H�O�o�b�t�@�[�|�C���^�[���쐬.
	result = m_pDevice->CreateBuffer( &fogBufferDesc, NULL, &m_fogBuffer );
	if ( FAILED( result ) ) return false;

	return true;
}

//---------------------------.
// ������.
//---------------------------.
void CFog::Release()
{
	// �t�H�O�萔�o�b�t�@�����.
	if ( m_fogBuffer ) {
		m_fogBuffer->Release();
		m_fogBuffer = 0;
	}

	// �T���v���[�̏�Ԃ��J��.
	if ( m_sampleState ) {
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// �萔�o�b�t�@�����.
	if ( m_constantBuffer ) {
		m_constantBuffer->Release();
		m_constantBuffer = 0;
	}

	// ���C�A�E�g�����.
	if ( m_layout ) {
		m_layout->Release();
		m_layout = 0;
	}

	// �s�N�Z���V�F�[�_�[�����.
	if ( m_pixelShader ) {
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// ���_�V�F�[�_�[�����.
	if ( m_vertexShader ) {
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}

//---------------------------.
// �G���[���b�Z�[�W���o��.
//---------------------------.
void CFog::OutputShaderErrorMessage( ID3D10Blob* errorMessage, WCHAR* shaderFilename )
{
	char*			compileErrors;
	unsigned long	bufferSize, i;
	std::ofstream	fout;

	// �G���[���b�Z�[�W�e�L�X�g�o�b�t�@�ւ̃|�C���^���擾.
	compileErrors = (char*) ( errorMessage->GetBufferPointer() );

	// ���b�Z�[�W�̒������擾.
	bufferSize = static_cast<unsigned long>( errorMessage->GetBufferSize() );

	// �t�@�C�����J��.
	fout.open( "shader-error.txt" );

	// �G���[���b�Z�[�W�������o��.
	for ( i = 0; i < bufferSize; i++ ) {
		fout << compileErrors[i];
	}

	// �t�@�C�������.
	fout.close();

	// �G���[���b�Z�[�W���J��.
	errorMessage->Release();
	errorMessage = 0;

	// �G���[���b�Z�[�W��\��.
	ErrorMessage( 
		"�V�F�[�_�[�̃R���p�C�����ɃG���[���������܂����B\n"
		"���b�Z�[�W�ɂ��Ắushader-error.txt�v���m�F���Ă��������B" );
}

//---------------------------.
// �V�F�[�_�[�p�����[�^��ݒ�.
//---------------------------.
bool CFog::SetShaderParameters( D3DXMATRIX worldMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd )
{
	HRESULT						result;
	D3D11_MAPPED_SUBRESOURCE	mappedResource;
	ConstantBufferType*			dataPtr;
	unsigned int				bufferNumber;
	FogBufferType*				dataPtr2;

	// �萔�o�b�t�@�����b�N���āA�������݉\�ɂ���.
	result = m_pContext->Map( m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( FAILED( result ) ) return false;

	// �萔�o�b�t�@���̃f�[�^�ւ̃|�C���^���擾.
	dataPtr = (ConstantBufferType*) mappedResource.pData;

	// �}�g���b�N�X��]�u���āA�V�F�[�_�[�p�ɏ���.
	D3DXMATRIX viewMatrix		= CameraManager::GetViewMatrix();
	D3DXMATRIX projectionMatrix = CameraManager::GetProjMatrix();
	D3DXMatrixTranspose( &worldMatrix, &worldMatrix				);
	D3DXMatrixTranspose( &viewMatrix, &viewMatrix				);
	D3DXMatrixTranspose( &projectionMatrix, &projectionMatrix	);

	// �s���萔�o�b�t�@�[�ɃR�s�[.
	dataPtr->world		= worldMatrix;
	dataPtr->view		= viewMatrix;
	dataPtr->projection = projectionMatrix;

	// �R���X�^���g�o�b�t�@�̃��b�N������.
	m_pContext->Unmap( m_constantBuffer, 0 );

	// ���_�V�F�[�_�[�Œ萔�o�b�t�@�[�̈ʒu��ݒ�.
	bufferNumber = 0;

	// �X�V���ꂽ�l���g�p���āA���_�V�F�[�_�[�ɒ萔�o�b�t�@�[��ݒ�.
	m_pContext->VSSetConstantBuffers( bufferNumber, 1, &m_constantBuffer );

	// �s�N�Z���V�F�[�_�[�ŃV�F�[�_�[�e�N�X�`�����\�[�X��ݒ�.
	m_pContext->PSSetShaderResources( 0, 1, &texture );

	// �t�H�O�萔�o�b�t�@�����b�N���āA���̂悤�ɏ������߂�悤�ɂ���.
	result = m_pContext->Map( m_fogBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( FAILED( result ) ) return false;

	// �萔�o�b�t�@���̃f�[�^�ւ̃|�C���^���擾.
	dataPtr2 = (FogBufferType*) mappedResource.pData;

	// ���`�t�H�O�̐ݒ�.
	dataPtr2->fogStart	= fogStart;
	dataPtr2->fogEnd	= fogEnd;

	// �w���t�H�O�̐ݒ�.
	dataPtr2->e			= FOG_E;
	dataPtr2->density	= m_Density;

	// �R���X�^���g�o�b�t�@�̃��b�N������.
	m_pContext->Unmap( m_fogBuffer, 0 );

	// ���_�V�F�[�_�[�Ńt�H�O�R���X�^���g�o�b�t�@�̈ʒu��ݒ�.
	bufferNumber = 1;

	// �X�V���ꂽ�l���g�p���āA���_�V�F�[�_�[�Ƀt�H�O�o�b�t�@��ݒ�.
	m_pContext->VSSetConstantBuffers( bufferNumber, 1, &m_fogBuffer );

	return true;
}

//---------------------------.
// �V�F�[�_�̕`��.
//---------------------------.
void CFog::RenderShader( int indexCount )
{
	// ���_���̓��C�A�E�g��ݒ�.
	m_pContext->IASetInputLayout( m_layout );

	// �O�p�`�̃����_�����O�Ɏg�p����钸�_�V�F�[�_�[�ƃs�N�Z���V�F�[�_�[��ݒ�.
	m_pContext->VSSetShader( m_vertexShader, NULL, 0 );
	m_pContext->PSSetShader( m_pixelShader,	NULL, 0 );

	// �s�N�Z���V�F�[�_�[�ŃT���v���[�̏�Ԃ�ݒ�.
	m_pContext->PSSetSamplers( 0, 1, &m_sampleState );

	// �O�p�`�������_�����O.
	m_pContext->DrawIndexed( indexCount, 0, 0 );
}