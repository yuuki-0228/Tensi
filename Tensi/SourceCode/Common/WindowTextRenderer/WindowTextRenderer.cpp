#include "WindowTextRenderer.h"

namespace {
	constexpr float RECT_W			= 30.0f;
	constexpr float RECT_H			= 36.0f;
	constexpr float TEX_DIMENSION	= 384.0f;

	// �V�F�[�_�[�t�@�C���p�X.
	constexpr TCHAR SHADER_NAME[] = _T( "Data\\Shader\\WindowText.hlsl" );
}

WindowTextRenderer::WindowTextRenderer()
	: m_pVertexShader	( nullptr )
	, m_pVertexLayout	( nullptr )
	, m_pPixelShader	( nullptr )
	, m_pConstantBuffer	( nullptr )
	, m_pVertexBuffer	()
	, m_pSampleLinear	( nullptr )
	, m_pTexture		( nullptr )
	, m_Kerning			()
	, m_FontSize		( 1.0f )
{
}

WindowTextRenderer::~WindowTextRenderer()
{
	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pTexture );
	for ( int i = 100 - 1; i >= 0; i-- ) {
		SAFE_RELEASE( m_pVertexBuffer[i] );
	}
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pVertexShader );
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
WindowTextRenderer* WindowTextRenderer::GetInstance()
{
	static std::unique_ptr<WindowTextRenderer> pInstance = std::make_unique<WindowTextRenderer>();
	return pInstance.get();
}

//----------------------------.
// ������.
//----------------------------.
HRESULT WindowTextRenderer::Init()
{
	WindowTextRenderer* pI = GetInstance();

	// 100�������J��Ԃ�.
	for ( int i = 0; i < 100; i++ ) {
		pI->m_Kerning[i] = 30.0f;
	}

	if( FAILED( CreateShader()	) ) return E_FAIL;
	if( FAILED( CreateModel()	) ) return E_FAIL;
	if( FAILED( CreateSampler() ) ) return E_FAIL;
	if( FAILED( CreateTexture( _T( "Data\\Sprite\\WindowText\\ascii.png" ) ) ) ) return E_FAIL;

	Log::PushLog( "�E�B���h�E�e�L�X�g�̏����� : ����" );
	return S_OK;
}

//----------------------------.
// HLSĻ�ق�ǂݍ��ݼ���ނ��쐬����.
//----------------------------.
HRESULT WindowTextRenderer::CreateShader()
{
	WindowTextRenderer*	pI				= GetInstance();
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�[�̃u���u���쐬.
	if (FAILED(
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
			nullptr)))			// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�o�[�e�b�N�X�V�F�[�_�[�v���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&pI->m_pVertexShader)))	// (out)�o�[�e�b�N�X�V�F�[�_�[.
	{
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�[�쐬���s" );
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
	if (FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout,
			numElements,
			pCompiledShader,
			&pI->m_pVertexLayout)))	// (out)���_�C���v�b�g���C�A�E�g.
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSL����s�N�Z���V�F�[�_�[������ނ��쐬.
	if (FAILED(
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
			nullptr)))			// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�s�N�Z���V�F�[�_�[�v���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr,
			&pI->m_pPixelShader)))	// (out)�s�N�Z���V�F�[�_�[.
	{
		ErrorMessage( "�s�N�Z���V�F�[�_�[�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// �R���X�^���g(�萔)�o�b�t�@�쐬.
	// �V�F�[�_�[�ɓ���̐��l�𑗂�o�b�t�@.
	// �����ł͕ϊ��s��n���p.
	// �V�F�[�_�[�� World, View, Projection �s���n��.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof(SHADER_CONSTANT_BUFFER);	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;								// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;								// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if (FAILED(
		pI->m_pDevice->CreateBuffer(
			&cb,
			nullptr,
			&pI->m_pConstantBuffer)))
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// ���f���쐬.
//----------------------------.
HRESULT WindowTextRenderer::CreateModel()
{
	WindowTextRenderer* pI = GetInstance();

	// �t�H���g���ɋ�`�쐬.
	float	left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
	int		count = 0;

	// 2�d���[�v��1�������w�肷��.
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 10; y++) {

			// �������̑傫��.
			left	= x * RECT_W;
			top		= y * RECT_H;
			right	= left + pI->m_Kerning[count];
			bottom	= top + RECT_H;

			// �e�N�X�`���̑傫������䗦�v�Z.
			left	/= TEX_DIMENSION;
			top		/= TEX_DIMENSION;
			right	/= TEX_DIMENSION;
			bottom	/= TEX_DIMENSION;

			// �|��(�l�p�`)�̒��_���쐬.
			VERTEX vertices[] =
			{
				// ���_���W(x,y,z)					 UV���W(u,v)
				D3DXVECTOR3(   0.0f, RECT_H, 0.0f ), D3DXVECTOR2(  left, bottom ),	// ���_�P(����).
				D3DXVECTOR3(   0.0f,   0.0f, 0.0f ), D3DXVECTOR2(  left,    top ),	// ���_�Q(����).
				D3DXVECTOR3( RECT_W, RECT_H, 0.0f ), D3DXVECTOR2( right, bottom ),	// ���_�R(�E��).
				D3DXVECTOR3( RECT_W,   0.0f, 0.0f ), D3DXVECTOR2( right,    top )	// ���_�S(�E��).
			};
			// �ő�v�f�����Z�o����.
			UINT uVerMax = sizeof(vertices) / sizeof(vertices[0]);

			// �o�b�t�@�\����.
			D3D11_BUFFER_DESC bd;
			bd.Usage				= D3D11_USAGE_DEFAULT;		// �g�p���@(�f�t�H���g).
			bd.ByteWidth			= sizeof(VERTEX) * uVerMax;	// ���_�̃T�C�Y.
			bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;	// ���_�o�b�t�@�Ƃ��Ĉ���.
			bd.CPUAccessFlags		= 0;						// CPU����̓A�N�Z�X���Ȃ�.
			bd.MiscFlags			= 0;						// ���̑��̃t���O(���g�p).
			bd.StructureByteStride	= 0;						// �\���̂̃T�C�Y(���g�p).

			// �T�u���\�[�X�f�[�^�\����.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = vertices;	// �|���̒��_���Z�b�g.

			// ���_�o�b�t�@�̍쐬.
			if ( FAILED( pI->m_pDevice->CreateBuffer(
				&bd, &InitData, &pI->m_pVertexBuffer[count])))
			{
				ErrorMessage( "���_�o�b�t�@�쐬���s" );
				return E_FAIL;
			}

			count++;// 1�����쐬�����̂Ŏ���.
		}
	}

	return S_OK;
}

//----------------------------.
// �e�N�X�`���쐬.
//----------------------------.
HRESULT WindowTextRenderer::CreateTexture( LPCTSTR lpFileName )
{
	WindowTextRenderer* pI = GetInstance();

	// �e�N�X�`���쐬.
	if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
		lpFileName,			// �t�@�C����.
		nullptr, nullptr,
		&pI->m_pTexture,	// (out)�e�N�X�`��.
		nullptr ) ) )
	{
		ErrorMessage( "�e�N�X�`���쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// �T���v���쐬.
//----------------------------.
HRESULT WindowTextRenderer::CreateSampler()
{
	WindowTextRenderer* pI = GetInstance();

	// �e�N�X�`���p�̃T���v���\����.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof( samDesc ) );
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// ���j�A�t�B���^(���`���).
						// POINT:���������e��.
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		// ���b�s���O���@(WRAP:�J��Ԃ�).
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// MIRROR: ���]�J��Ԃ�.
	// CLAMP : �[�̖͗l�������L�΂�.
	// BORDER: �ʓr���E�F�����߂�.
	// �T���v���쐬.
	if( FAILED( pI->m_pDevice->CreateSamplerState(
		&samDesc, &pI->m_pSampleLinear ) ) ) //(out)�T���v��.
	{
		ErrorMessage( "�T���v���쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// �`��.
//----------------------------.
void WindowTextRenderer::Render( const std::string& text, const D3DXPOSITION3& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer::Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
}
void WindowTextRenderer::Render( const std::string& text, const D3DXPOSITION2& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer::Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
}
void WindowTextRenderer::Render( const std::string& text, const int x, const int y, const float FontSize, const D3DXCOLOR4& Color )
{
	WindowTextRenderer* pI = GetInstance();

	// �t�H���g�T�C�Y�̐ݒ�.
	pI->m_FontSize	= FontSize;

	// �F�̐ݒ�.
	pI->m_Color = Color;

	// �g�p����V�F�[�_�[�̓o�^.
	pI->m_pContext->VSSetShader( pI->m_pVertexShader, nullptr, 0 );
	pI->m_pContext->PSSetShader( pI->m_pPixelShader, nullptr, 0 );

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg�����H.
	pI->m_pContext->VSSetConstantBuffers( 0, 1, &pI->m_pConstantBuffer );
	pI->m_pContext->PSSetConstantBuffers( 0, 1, &pI->m_pConstantBuffer );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	pI->m_pContext->IASetInputLayout( pI->m_pVertexLayout );
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	pI->m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �e�N�X�`�����V�F�[�_�[�ɓn��.
	pI->m_pContext->PSSetSamplers( 0, 1, &pI->m_pSampleLinear );
	pI->m_pContext->PSSetShaderResources( 0, 1, &pI->m_pTexture );

	// �����������[�v.
	int				RenderX = x;
	int				RenderY = y;
	std::wstring	wString	= StringConversion::to_wString( text );
	LPCTSTR			wText	= wString.c_str();
	for ( int i = 0; i < lstrlen( wText ); i++ ) {
		TCHAR	font	= text[i];
		int		index	= font - 32;	// �t�H���g�C���f�b�N�X�쐬.

		// ���s.
		if ( font == '\n' ) {
			RenderX  = x;
			RenderY += static_cast<int>( RECT_H * pI->m_FontSize );
			continue;
		}

		// �t�H���g�����_�����O.
		RenderFont( index, RenderX, RenderY );

		// ������`�悷��ʒu�𓮂���.
		RenderX += static_cast<int>( pI->m_Kerning[index] * pI->m_FontSize );
	}
}
void WindowTextRenderer::DebugRender( const std::string& text, const D3DXPOSITION3& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
#endif	// #ifdef _DEBUG.
}
void WindowTextRenderer::DebugRender( const std::string& text, const D3DXPOSITION2& Pos, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ), FontSize, Color );
#endif	// #ifdef _DEBUG.
}
void WindowTextRenderer::DebugRender( const std::string& text, const int x, const int y, const float FontSize, const D3DXCOLOR4& Color )
{
#ifdef _DEBUG
	Render( text, x, y, FontSize, Color );
#endif	// #ifdef _DEBUG.
}

//----------------------------.
// �t�H���g�����_�����O
//----------------------------.
void WindowTextRenderer::RenderFont(
	int FontIndex, int x, int y )
{
	if ( FontIndex == 0 ) return;

	WindowTextRenderer* pI = GetInstance();

	// ���[���h�s��.
	D3DXMATRIX	mWorld;
	D3DXMATRIX	mTrans, mScale;

	// �g��k���s��.
	D3DXMatrixScaling( &mScale, pI->m_FontSize, pI->m_FontSize, pI->m_FontSize );

	// ���s�ړ��s��.
	D3DXMatrixTranslation( &mTrans,
		static_cast<float>(x), static_cast<float>(y), 0.0f );

	// ���[���h���W�ϊ�.
	//	�d�v: �g�k�s�� * ���s�s��.
	mWorld = mScale * mTrans;

	// �V�F�[�_�[�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	// �R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if (SUCCEEDED(
		pI->m_pContext->Map( pI->m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		// ���[���h�s���n��(�r���[,�v���W�F�N�V�����͎g��Ȃ�).
		D3DXMATRIX m	= mWorld;
		D3DXMatrixTranspose(&m, &m);//�s���]�u����.
		cb.mWVP			= m;

		// �J���[.
		cb.vColor = pI->m_Color;

		// �r���[�|�[�g�̕��A������n��.
		cb.fViewPortWidth	= FWND_W;
		cb.fViewPortHeight	= FWND_H;

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		pI->m_pContext->Unmap( pI->m_pConstantBuffer, 0 );
	}

	// ���_�o�t�@���Z�b�g.
	UINT stride = sizeof(VERTEX);// �f�[�^�̊Ԋu.
	UINT offset = 0;
	pI->m_pContext->IASetVertexBuffers( 0, 1,
		&pI->m_pVertexBuffer[FontIndex], &stride, &offset );

	// �A���t�@�u�����h�L���ɂ���.
	DirectX11::SetAlphaBlend( true );

	// �v���~�e�B�u�������_�����O.
	pI->m_pContext->Draw( 4, 0 );// �|��(���_4��).

	// �A���t�@�u�����h�����ɂ���.
	DirectX11::SetAlphaBlend( false );
}
