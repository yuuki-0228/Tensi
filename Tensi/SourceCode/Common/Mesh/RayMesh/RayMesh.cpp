#include "RayMesh.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Collision\Ray\Ray.h"

namespace {
	// �V�F�[�_�t�@�C����(�f�B���N�g�����܂�).
	const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Ray.hlsl" );
}

CRayMesh::CRayMesh()
	: m_pVertexShader	( nullptr )
	, m_pVertexLayout	( nullptr )
	, m_pPixelShader	( nullptr )
	, m_pConstantBuffer	( nullptr )
	, m_pVertexBuffer	( nullptr )
{
}

CRayMesh::~CRayMesh()
{
	SAFE_RELEASE( m_pVertexBuffer	);
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader	);
	SAFE_RELEASE( m_pVertexLayout	);
	SAFE_RELEASE( m_pVertexShader	);
}

//---------------------------.
// ������.
//---------------------------.
HRESULT CRayMesh::Init()
{
	if ( FAILED( CreateShader() ) ) return E_FAIL;
	return S_OK;
}

//---------------------------.
// �V�F�[�_�쐬.
//---------------------------.
HRESULT CRayMesh::CreateShader()
{
	ID3D11Device*	pDevice			= DirectX11::GetDevice();
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG.

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
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
			nullptr)))			// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�o�[�e�b�N�X�V�F�[�_�v���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr,
			&m_pVertexShader)))	// (out)�o�[�e�b�N�X�V�F�[�_.
	{
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�쐬���s" );
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
			&m_pVertexLayout)))	// (out)���_�C���v�b�g���C�A�E�g.
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// HLSL����s�N�Z���V�F�[�_�̃u���u���쐬.
	if (FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME,		// �V�F�[�_�t�@�C����(HLSL�t�@�C��).
			nullptr,			// �}�N����`�̔z��ւ̃|�C���^(���g�p).
			nullptr,			// �C���N���[�h�t�@�C���������C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
			"PS_Main",			// �V�F�[�_�G���g���[�|�C���g�֐��̖��O.
			"ps_5_0",			// �V�F�[�_�̃��f�����w�肷�镶����(�v���t�@�C��).
			uCompileFlag,		// �V�F�[�_�R���p�C���t���O.
			0,					// �G�t�F�N�g�R���p�C���t���O(���g�p).
			nullptr,			// �X���b�h �|���v �C���^�[�t�F�C�X�ւ̃|�C���^(���g�p).
			&pCompiledShader,	// �u���u���i�[���郁�����ւ̃|�C���^.
			&pErrors,			// �G���[�ƌx���ꗗ���i�[���郁�����ւ̃|�C���^.
			nullptr)))			// �߂�l�ւ̃|�C���^(���g�p).
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	// ��L�ō쐬�����u���u����u�s�N�Z���V�F�[�_�v���쐬.
	if (FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr,
			&m_pPixelShader)))	// (out)�s�N�Z���V�F�[�_.
	{
		ErrorMessage( "�s�N�Z���V�F�[�_�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	// �R���X�^���g(�萔)�o�b�t�@�쐬.
	//	�V�F�[�_�ɓ���̐��l�𑗂�o�b�t�@.
	//	�����ł͕ϊ��s��n���p.
	//	�V�F�[�_�� World, View, Projection �s���n��.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof(SHADER_CONSTANT_BUFFER);	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;								// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;								// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if (FAILED(
		pDevice->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//---------------------------.
// ���f���쐬.
//---------------------------.
HRESULT CRayMesh::CreateModel( CRay* pRay )
{
	// �o�b�t�@�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage				= D3D11_USAGE_DEFAULT;		// �g�p���@(�f�t�H���g).
	bd.ByteWidth			= sizeof(VERTEX) * 2;		// ���_�̃T�C�Y(�����Ȃ̂Ŏn�_�ƏI�_�̂Q�_��).
	bd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;	// ���_�o�b�t�@�Ƃ��Ĉ���.
	bd.CPUAccessFlags		= 0;						// CPU����̓A�N�Z�X���Ȃ�.
	bd.MiscFlags			= 0;						// ���̑��̃t���O(���g�p).
	bd.StructureByteStride	= 0;						// �\���̂̃T�C�Y(���g�p).

	// ���C�\���̂���n�_�ƏI�_���Z�o.
	D3DXVECTOR3 vertices[2];
	vertices[0] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);			// ���C�̎n�_.
	vertices[1] = pRay->GetVector() * pRay->GetLength();	// ���C�̏I�_.

	// �T�u���\�[�X�f�[�^�\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;	// ���C�̒��_���Z�b�g.

	// ���_�o�b�t�@�̍쐬.
	if ( FAILED( DirectX11::GetDevice()->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer ) ) )
	{
		ErrorMessage( "���_�o�b�t�@�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//---------------------------.
// �����_�����O�p.
//---------------------------.
void CRayMesh::Render( CRay* Ray )
{
	ID3D11DeviceContext* pContext = DirectX11::GetContext();

	// �O���̃��C�\���̂𔽉f.
	m_Transform.Position = Ray->GetPosition();
	m_Transform.Rotation = Ray->GetRotation();

	// ���[���h���W�̎擾.
	D3DXMATRIX mWorld = m_Transform.GetWorldMatrix();

	// �g�p����V�F�[�_�̓o�^.
	pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	//�R���X�^���g�o�b�t�@.
	// �o�b�t�@���̃f�[�^�̏��������J�n����map.
	if (SUCCEEDED(
		pContext->Map(m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		// ���[���h,�r���[,�v���W�F�N�V�����s���n��.
		D3DXMATRIX m = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose(&m, &m);//�s���]�u����.
		cb.mWVP = m;

		// �J���[.
		cb.vColor = m_Color;

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		pContext->Unmap(m_pConstantBuffer, 0);
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
	pContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	pContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// ���_�o�b�t�@���Z�b�g.
	if ( FAILED( CreateModel( Ray ) ) ) return;
	UINT stride = sizeof(VERTEX);// �f�[�^�̊Ԋu.
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1,
		&m_pVertexBuffer, &stride, &offset);
	SAFE_RELEASE( m_pVertexBuffer );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	pContext->IASetInputLayout(m_pVertexLayout);
	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �v���~�e�B�u�������_�����O.
	pContext->Draw( 2, 0 );// ���C(�����Ȃ̂Œ��_2��).
}
