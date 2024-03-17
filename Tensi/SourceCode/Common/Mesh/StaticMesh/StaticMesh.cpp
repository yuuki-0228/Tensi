#include "StaticMesh.h"
#include "..\..\DirectX\DirectX9.h"
#include "..\..\Fog\Fog.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Light\Light.h"
#include "..\..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

#include <stdlib.h>	// �}���`�o�C�g������Unicode�����ϊ��ŕK�v.
#include <locale.h>
#include <filesystem>

namespace {
	// �V�F�[�_�t�@�C����(�f�B���N�g�����܂�).
	const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\Mesh.hlsl" );
}

CStaticMesh::CStaticMesh()
	: m_pVertexShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pCBufferPerMesh			( nullptr )
	, m_pCBufferPerMaterial		( nullptr )
	, m_pCBufferPerFrame		( nullptr )
	, m_pVertexBuffer			( nullptr )
	, m_ppIndexBuffer			( nullptr )
	, m_pSampleLinear			( nullptr )
	, m_Model					()
	, m_ModelForRay				()
	, m_pMaterials				( nullptr )
	, m_NumAttr					()
	, m_AttrID					()
	, m_EnableTexture			( false )
	, m_pFog					( nullptr )
	, m_IsFog					( false )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_OrLessDistanceAlphaFlag	( false )
	, m_OrMoreDistanceAlphaFlag	( false )
	, m_OrLessStartDistance		( 0.0f )
	, m_OrMoreStartDistance		( 0.0f )
	, m_AlphaDistance			( 0.0f )
	, m_TexturePathList			()
	, m_MaterialTextureNo		()
{
	m_pFog = std::make_unique<CFog>();
}

CStaticMesh::~CStaticMesh()
{
	// �������.
	Release();
}

//----------------------------.
// �������֐�.
//----------------------------.
HRESULT CStaticMesh::Init( LPCTSTR lpFileName )
{
	const std::wstring	wFileName	= lpFileName;
	const std::string	FileName	= StringConversion::to_String( wFileName );

	// �t�@�C���ǂݍ���.
	if (FAILED(LoadXMesh( lpFileName ) )			) return E_FAIL;
	// ���f���쐬.
	if( FAILED( CreateModel() )						) return E_FAIL;
	// �V�F�[�_�쐬.
	if (FAILED(CreateShader() )						) return E_FAIL;
	// �R���X�^���g�o�b�t�@�쐬.
	if( FAILED( CreateConstantBuffer() )			) return E_FAIL;
	// �T���v���쐬.
	if( FAILED( CreateSampler() )					) return E_FAIL;

	// �t�H�O�̏�����.
	m_pFog->Init();

	return S_OK;
}

//----------------------------.
// ���b�V���ǂݍ���.
//----------------------------.
HRESULT CStaticMesh::LoadXMesh( LPCTSTR lpFileName )
{
	// �t�@�C�������R�s�[.
	lstrcpy( m_Model.FileName, lpFileName );
	lstrcpy( m_ModelForRay.FileName, lpFileName );

	// �}�e���A���o�b�t�@.
	LPD3DXBUFFER pD3DXMtrlBuffer = nullptr;

	// X�t�@�C���̃��[�h.
	if( FAILED( DirectX11::MutexD3DXLoadMeshFromX(
		lpFileName,					// �t�@�C����.
		D3DXMESH_SYSTEMMEM			// �V�X�e���������ɓǂݍ���.
		| D3DXMESH_32BIT,			// 32bit.
		DirectX9::GetDevice(), nullptr,
		&m_Model.pD3DXMtrlBuffer,	// (out)�}�e���A�����.
		nullptr,
		&m_Model.NumMaterials,		// (out)�}�e���A����.
		&m_Model.pMesh ) ) )		// (out)���b�V���I�u�W�F�N�g.
	{
		ErrorMessage( "X�t�@�C���Ǎ����s" );
		return E_FAIL;
	}

	// X�t�@�C���̃��[�h(���C�Ƃ̔���p�ɕʐݒ�œǂݍ���).
	if( FAILED( DirectX11::MutexD3DXLoadMeshFromX(
		lpFileName,						// �t�@�C����.
		D3DXMESH_SYSTEMMEM,				// �V�X�e���������ɓǂݍ���.
		DirectX9::GetDevice(), nullptr,
		&m_ModelForRay.pD3DXMtrlBuffer,	// (out)�}�e���A�����.
		nullptr,
		&m_ModelForRay.NumMaterials,	// (out)�}�e���A����.
		&m_ModelForRay.pMesh ) ) )		// (out)���b�V���I�u�W�F�N�g.
	{
		ErrorMessage( "X�t�@�C���Ǎ����s" );
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// ���f���쐬.
//----------------------------.
HRESULT CStaticMesh::CreateModel()
{
	// �}�e���A���쐬.
	if( FAILED( CreateMaterials() )		) return E_FAIL;
	// �C���f�b�N�X�o�b�t�@�쐬.
	if( FAILED( CreateIndexBuffer() )	) return E_FAIL;
	// ���_�o�b�t�@�쐬.
	if( FAILED( CreateVertexBuffer() )	) return E_FAIL;
	return S_OK;
}

//----------------------------.
// �}�e���A���쐬.
//----------------------------.
HRESULT CStaticMesh::CreateMaterials()
{
	// �t�@�C���̃p�X(�f�B���N�g��)���m��.
	const int PATH_MAX = 64;
	TCHAR path[PATH_MAX] = _T( "" );
	int path_count = lstrlen( m_Model.FileName );
	for( int k = path_count; k >= 0; k-- ) {
		if( m_Model.FileName[k] == '\\' ) {
			for( int j = 0; j <= k; j++ ) {
				path[j] = m_Model.FileName[j];
			}
			path[k + 1] = '\0';
			break;
		}
	}

	// �ǂݍ��񂾏�񂩂�K�v�ȏ��𔲂��o��.
	D3DXMATERIAL* d3dxMaterials
		= static_cast<D3DXMATERIAL*>( m_Model.pD3DXMtrlBuffer->GetBufferPointer() );
	// �}�e���A�������̗̈���m��.
	m_pMaterials = new MY_MATERIAL[m_Model.NumMaterials]();
	m_MaterialTextureNo.resize( m_Model.NumMaterials );
	// �}�e���A�������J��Ԃ�.
	for( DWORD No = 0; No < m_Model.NumMaterials; No++ )
	{
		// �}�e���A�����̃R�s�[.
		//	�A���r�G���g.
		m_pMaterials[No].Ambient.x = d3dxMaterials[No].MatD3D.Ambient.r;
		m_pMaterials[No].Ambient.y = d3dxMaterials[No].MatD3D.Ambient.g;
		m_pMaterials[No].Ambient.z = d3dxMaterials[No].MatD3D.Ambient.b;
		m_pMaterials[No].Ambient.w = d3dxMaterials[No].MatD3D.Ambient.a;
		// �f�B�t���[�Y.
		m_pMaterials[No].Diffuse.x = d3dxMaterials[No].MatD3D.Diffuse.r;
		m_pMaterials[No].Diffuse.y = d3dxMaterials[No].MatD3D.Diffuse.g;
		m_pMaterials[No].Diffuse.z = d3dxMaterials[No].MatD3D.Diffuse.b;
		m_pMaterials[No].Diffuse.w = d3dxMaterials[No].MatD3D.Diffuse.a;
		// �X�y�L����.
		m_pMaterials[No].Specular.x = d3dxMaterials[No].MatD3D.Specular.r;
		m_pMaterials[No].Specular.y = d3dxMaterials[No].MatD3D.Specular.g;
		m_pMaterials[No].Specular.z = d3dxMaterials[No].MatD3D.Specular.b;
		m_pMaterials[No].Specular.w = d3dxMaterials[No].MatD3D.Specular.a;

		// (���̖ʂ�)�e�N�X�`�����\���Ă��邩�H.
		if( d3dxMaterials[No].pTextureFilename != nullptr
			&& strlen( d3dxMaterials[No].pTextureFilename ) > 0 )
		{
#ifdef UNICODE
			WCHAR TexFilename_w[32] = L"";
			// �e�N�X�`�����̃T�C�Y���擾.
			size_t charSize = strlen( d3dxMaterials[No].pTextureFilename ) + 1;
			size_t ret;	// �ϊ����ꂽ������.

			// �}���`�o�C�g�����̃V�[�P���X��Ή����郏�C�h�����̃V�[�P���X�ɕϊ����܂�.
			errno_t err = mbstowcs_s(
				&ret,
				TexFilename_w,
				charSize,
				d3dxMaterials[No].pTextureFilename,
				_TRUNCATE );

			LPTSTR filename = TexFilename_w;
#else// #ifdef UNICODE
			LPTSTR filename = d3dxMaterials[No].pTextureFilename;
#endif// #ifdef UNICODE

			// �e�N�X�`������̃t���O�𗧂Ă�.
			m_EnableTexture = true;

			// �p�X���R�s�[.
			lstrcpy( m_pMaterials[No].TextureName, path );
			// �e�N�X�`���t�@�C������A��.
			lstrcat( m_pMaterials[No].TextureName, filename );

			// �e�N�X�`���쐬.
			if( FAILED( DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				m_pMaterials[No].TextureName,	// �e�N�X�`���t�@�C����.
				nullptr, nullptr,
				&m_pMaterials[No].pTexture[0],		// (out)�e�N�X�`���I�u�W�F�N�g.
				nullptr ) ) )
			{
				ErrorMessage( "�e�N�X�`���쐬���s" );
				return E_FAIL;
			}

			// �����ւ��e�N�X�`�����擾.
			int TexNo = 1;
			while ( 1 ) {
				// �����ւ��e�N�X�`���p�X�̍쐬.
				std::string TexName	 = StringConversion::to_String( m_pMaterials[No].TextureName );
				size_t		TexIndex = TexName.find( "." );
				TexName.insert( TexIndex, "_Tex" + std::to_string( TexNo ) );

				// �e�N�X�`�����쐬�ł��邩.
				ID3D11ShaderResourceView* pTexture = nullptr;
				if ( FAILED(
					DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
					StringConversion::to_wString( TexName ).c_str(),
					nullptr, nullptr, &pTexture, nullptr ) ) )
				{
					// �쐬�ł��Ȃ��������ߍ����ւ��e�N�X�`���̍쐬���I������.
					break;
				}

				// ���̃e�N�X�`�������邩�m���߂�.
				TexNo++;
				m_pMaterials[No].pTexture.emplace_back( pTexture );
			}

			// ���C�g�e�N�X�`���p�X�̍쐬.
			std::string LightTexName = StringConversion::to_String( m_pMaterials[No].TextureName );
			size_t		LightIndex	 = LightTexName.find( "." );
			LightTexName.insert( LightIndex, "_light" );

			// ���C�g�e�N�X�`�����쐬�ł��邩.
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				StringConversion::to_wString( LightTexName ).c_str(),
				nullptr, nullptr, &m_pMaterials[No].pLightTexture, nullptr ) ) )
			{
				m_pMaterials[No].pLightTexture = nullptr;
			}
		}
	}

	return S_OK;
}

//----------------------------.
// �C���f�b�N�X�o�b�t�@�쐬.
//----------------------------.
HRESULT CStaticMesh::CreateIndexBuffer()
{
	D3D11_BUFFER_DESC		bd;			// Dx11�o�b�t�@�\����.
	D3D11_SUBRESOURCE_DATA	InitData;	// �������f�[�^.

	// �}�e���A�������̗̈���m��.
	m_ppIndexBuffer = new ID3D11Buffer*[m_Model.NumMaterials]();
	for( DWORD No = 0; No < m_Model.NumMaterials; No++ ){
		m_ppIndexBuffer[No] = nullptr;
	}

	// ���b�V���̑������𓾂�.
	//	�������ŃC���f�b�N�X�o�b�t�@����ׂ����}�e���A�����Ƃ̃C���f�b�N�X�o�b�t�@�𕪗��ł���.
	D3DXATTRIBUTERANGE* pAttrTable = nullptr;

	// ���b�V���̖ʂ���ђ��_�̏��ԕύX�𐧌䂵�A�p�t�H�[�}���X���œK������.
	//	D3DXMESHOPT_COMPACT : �ʂ̏��Ԃ�ύX���A�g�p����Ă��Ȃ����_�Ɩʂ��폜����.
	//	D3DXMESHOPT_ATTRSORT : �p�t�H�[�}���X���グ��ׁA�ʂ̏��Ԃ�ύX���čœK�����s��.
	m_Model.pMesh->OptimizeInplace( D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT,
		nullptr, nullptr, nullptr, nullptr );
	// �����e�[�u���̎擾.
	m_Model.pMesh->GetAttributeTable( nullptr, &m_NumAttr );
	pAttrTable = new D3DXATTRIBUTERANGE[m_NumAttr];
	if( FAILED( m_Model.pMesh->GetAttributeTable( pAttrTable, &m_NumAttr ) ) )
	{
		ErrorMessage( "�����e�[�u���擾���s" );
		return E_FAIL;
	}

	// ������Lock���Ȃ��Ǝ��o���Ȃ�.
	int* pIndex = nullptr;
	m_Model.pMesh->LockIndexBuffer(
		D3DLOCK_READONLY, (void**)&pIndex );
	// �������Ƃ̃C���f�b�N�X�o�b�t�@���쐬.
	for( DWORD No = 0; No < m_NumAttr; No++ )
	{
		m_AttrID[No] = pAttrTable[No].AttribId;
		// Dx9�̃C���f�b�N�X�o�b�t�@����̏��ŁADx11�̃C���f�b�N�X�o�b�t�@���쐬.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth
			= sizeof( int )*pAttrTable[No].FaceCount * 3;//�ʐ��~3�Œ��_��.
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		// �傫���C���f�b�N�X�o�b�t�@���̃I�t�Z�b�g(�~3����).
		InitData.pSysMem = &pIndex[pAttrTable[No].FaceStart * 3];

		if( FAILED( m_pDevice->CreateBuffer(
			&bd, &InitData, &m_ppIndexBuffer[No] ) ) )
		{
			ErrorMessage( "�C���f�b�N�X�o�b�t�@�쐬���s" );
			return E_FAIL;
		}
		// �ʂ̐����R�s�[.
		m_pMaterials[m_AttrID[No]].dwNumFace = pAttrTable[No].FaceCount;
	}
	// �����e�[�u���̍폜.
	delete[] pAttrTable;
	// �g�p�ς݂̃C���f�b�N�X�o�b�t�@�̉��.
	m_Model.pMesh->UnlockIndexBuffer();

	return S_OK;
}

//----------------------------.
// ���_�o�b�t�@�쐬.
//----------------------------.
HRESULT CStaticMesh::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC		bd;			// Dx11�o�b�t�@�\����.
	D3D11_SUBRESOURCE_DATA	InitData;	// �������f�[�^.

	// Dx9�̏ꍇ�Amap�ł͂Ȃ�Lock�Œ��_�o�b�t�@����f�[�^�����o��.
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	m_Model.pMesh->GetVertexBuffer( &pVB );
	DWORD	dwStride	= m_Model.pMesh->GetNumBytesPerVertex();
	BYTE*	pVertices	= nullptr;
	VERTEX* pVertex		= nullptr;
	if( SUCCEEDED(
		pVB->Lock( 0, 0, (VOID**)&pVertices, 0 ) ) )
	{
		pVertex				= (VERTEX*)pVertices;
		// Dx9�̒��_�o�b�t�@����̏��ŁADx11���_�o�b�t�@���쐬.
		bd.Usage			= D3D11_USAGE_DEFAULT;
		// ���_���i�[����̂ɕK�v�ȃo�C�g��.
		bd.ByteWidth		= m_Model.pMesh->GetNumBytesPerVertex()*m_Model.pMesh->GetNumVertices();
		bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= 0;
		bd.MiscFlags		= 0;
		InitData.pSysMem	= pVertex;
		if( FAILED( m_pDevice->CreateBuffer(
			&bd, &InitData, &m_pVertexBuffer ) ) )
		{
			ErrorMessage( "���_�o�b�t�@�쐬���s" );
			return E_FAIL;
		}
		pVB->Unlock();
	}
	SAFE_RELEASE( pVB );	// ���_�o�b�t�@���.
	return S_OK;
}

// �T���v���쐬.
HRESULT CStaticMesh::CreateSampler()
{
	// �e�N�X�`���p�̃T���v���\����.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof( samDesc ) );
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// ���j�A�t�B���^(���`���).
						// POINT:���������e��.
	samDesc.AddressU
		= D3D11_TEXTURE_ADDRESS_WRAP;// ���b�s���O���[�h(WRAP:�J��Ԃ�).
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// MIRROR: ���]�J��Ԃ�.
	// CLAMP : �[�̖͗l�������L�΂�.
	// BORDER: �ʓr���E�F�����߂�.
	// �T���v���쐬.
	if( FAILED( m_pDevice->CreateSamplerState(
		&samDesc, &m_pSampleLinear ) ) )//(out)�T���v��.
	{
		ErrorMessage( "�T���v���쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// ����֐�.
//----------------------------.
void CStaticMesh::Release()
{
	// �C���f�b�N�X�o�b�t�@���.	���o�O��̂Œ����K�v.
	if (m_ppIndexBuffer != nullptr) {
		for (int No = m_Model.NumMaterials-1; No >= 0; No--) {
			if (m_ppIndexBuffer[No] != nullptr) {
				SAFE_RELEASE(m_ppIndexBuffer[No]);
			}
		}
		delete[] m_ppIndexBuffer;
		m_ppIndexBuffer = nullptr;
	}
	// �}�e���A�����.
	if (m_pMaterials != nullptr) {
		delete[] m_pMaterials;
		m_pMaterials = nullptr;
	}
	// ���b�V���f�[�^�̉��.
	SAFE_RELEASE( m_Model.pMesh );
	SAFE_RELEASE( m_Model.pD3DXMtrlBuffer );

	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pVertexShader );
}

//----------------------------.
//	HLSL�t�@�C����ǂݍ��݃V�F�[�_���쐬����.
//	HLSL: High Level Shading Language �̗�.
//----------------------------.
HRESULT CStaticMesh::CreateShader()
{
	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrors = nullptr;
	UINT uCompileFlag = 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	if (m_EnableTexture == true){
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "VS_Main", "vs_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
			return E_FAIL;
		}
	}
	else {
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "VS_NoTex", "vs_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
			return E_FAIL;
		}
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
	D3D11_INPUT_ELEMENT_DESC layout[3];
	// ���_�C���v�b�g���C�A�E�g�̔z��v�f�����Z�o.
	UINT numElements = 0;
	if (m_EnableTexture == true)
	{
		D3D11_INPUT_ELEMENT_DESC tmp[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(tmp) / sizeof(tmp[0]);	// �v�f���Z�o.
		memcpy_s( layout, sizeof(layout),
			tmp, sizeof(D3D11_INPUT_ELEMENT_DESC)*numElements);
	}
	else
	{
		D3D11_INPUT_ELEMENT_DESC tmp[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		numElements = sizeof(tmp) / sizeof(tmp[0]);	// �v�f���Z�o.
		memcpy_s(layout, sizeof(layout),
			tmp, sizeof(D3D11_INPUT_ELEMENT_DESC)*numElements);
	}

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
	if (m_EnableTexture == true){
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "PS_Main", "ps_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
			return E_FAIL;
		}
	}
	else {
		if (FAILED(
			DirectX11::MutexD3DX11CompileFromFile(
				SHADER_NAME, nullptr, nullptr, "PS_NoTex", "ps_5_0",
				uCompileFlag, 0, nullptr, &pCompiledShader, &pErrors, nullptr)))
		{
			ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
			return E_FAIL;
		}
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

	return S_OK;
}

//----------------------------.
// �R���X�^���g�o�b�t�@�쐬.
//----------------------------.
HRESULT CStaticMesh::CreateConstantBuffer()
{
	// �R���X�^���g�o�b�t�@(���b�V���p).
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;	// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof( CBUFFER_PER_MESH );	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;		// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;							// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;							// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;			// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerMesh ) ) )
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@(���b�V��)�쐬���s" );
		return E_FAIL;
	}

	// �R���X�^���g�o�b�t�@(�}�e���A���p).
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof( CBUFFER_PER_MATERIAL );	// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;								// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;								// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerMaterial ) ) )
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@(�}�e���A���p)�쐬���s" );
		return E_FAIL;
	}

	// �R���X�^���g�o�b�t�@(�t���[���p).
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;		// �R���X�^���g�o�b�t�@���w��.
	cb.ByteWidth			= sizeof( CBUFFER_PER_FRAME );		// �R���X�^���g�o�b�t�@�̃T�C�Y.
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;			// �������݂ŃA�N�Z�X.
	cb.MiscFlags			= 0;								// ���̑��̃t���O(���g�p).
	cb.StructureByteStride	= 0;								// �\���̂̃T�C�Y(���g�p).
	cb.Usage				= D3D11_USAGE_DYNAMIC;				// �g�p���@:���ڏ�������.

	// �R���X�^���g�o�b�t�@�̍쐬.
	if( FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, &m_pCBufferPerFrame ) ) )
	{
		ErrorMessage( "�R���X�^���g�o�b�t�@(�t���[���p)�쐬���s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// �����_�����O�p.
//	��DirectX���̃����_�����O�֐�.
//	�ŏI�I�ɉ�ʂɏo�͂���͕̂ʃN���X�̃����_�����O�֐������.
//----------------------------.
void CStaticMesh::Render( STransform* pTransform, const D3DXMATRIX* pRot )
{
	STransform* Transform = &m_Transform;
	if ( pTransform != nullptr ) {
		m_Transform = *pTransform;
		Transform	= pTransform;
	}
#ifdef _DEBUG
	PositionRenderer::Render( *Transform );
#endif
	if ( m_Color.w == 0.0f ) return;

	// ���[���h�s��̎擾.
	D3DXMATRIX mWorld = Transform->GetWorldMatrix( pRot );

	// �g�p����V�F�[�_�̃Z�b�g.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );	// ���_�V�F�[�_.
	m_pContext->PSSetShader( m_pPixelShader,  nullptr, 0 );	// �s�N�Z���V�F�[�_.

	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	// �o�b�t�@���̃f�[�^�̏��������J�n����Map.
	if (SUCCEEDED( m_pContext->Map(
		m_pCBufferPerFrame, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// �R���X�^���g�o�b�t�@(�t���[���p).
		CBUFFER_PER_FRAME cb;

		// �J�����ʒu.
		const D3DXVECTOR3 CamPos = CameraManager::GetPosition();
		cb.vCameraPos = D3DXVECTOR4(CamPos.x, CamPos.y, CamPos.z, 0.0f);

		// ���C�g����.
		const D3DXVECTOR3 Direction = Light::GetDirection();
		cb.vLightDir			= D3DXVECTOR4( Direction.x, Direction.y, Direction.z, 0.0f );
		cb.vLightIntensity.x	= Light::GetIntensity();

		// �f�B�U�������g�p���邩��n��.
		cb.vFlag.x = m_DitherFlag == true ? 1.0f : 0.0f;

		// �A���t�@�u���b�N���g�p���邩��n��.
		cb.vFlag.y = m_AlphaBlockFlag == true ? 1.0f : 0.0f;

		// �����œ����ɂ��Ă�������n��.
		cb.vFlag.z			= m_OrLessDistanceAlphaFlag == true ? 1.0f : 0.0f;
		cb.vFlag.w			= m_OrMoreDistanceAlphaFlag == true ? 1.0f : 0.0f;
		cb.vAlphaDistance.x = m_OrLessStartDistance;
		cb.vAlphaDistance.y = m_OrMoreStartDistance;
		cb.vAlphaDistance.z = m_AlphaDistance;

		// ���C�g�����̐��K��(�m�[�}���C�Y�j.
		//	�����f�����烉�C�g�֌���������. �f�B���N�V���i�����C�g�ŏd�v�ȗv�f.
		D3DXVec4Normalize(&cb.vLightDir, &cb.vLightDir);

		memcpy_s(
			pData.pData,	// �R�s�[��̃o�b�t�@.
			pData.RowPitch,	// �R�s�[��̃o�b�t�@�T�C�Y.
			(void*)(&cb),	// �R�s�[���̃o�b�t�@.
			sizeof(cb));	// �R�s�[���̃o�b�t�@�T�C�Y.

		// �o�b�t�@���̃f�[�^�̏��������I������Unmap.
		m_pContext->Unmap(m_pCBufferPerFrame, 0);
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�p���邩�H.
	m_pContext->VSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );// ���_�V�F�[�_.
	m_pContext->PSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );// �s�N�Z���V�F�[�_.

	// ���b�V���̃����_�����O.
	RenderMesh(mWorld);
}

//----------------------------.
// �����_�����O�֐�(�N���X���ł̂ݎg�p����).
//----------------------------.
void CStaticMesh::RenderMesh(
	D3DXMATRIX& mWorld )
{
	// �V�F�[�_�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	// �o�b�t�@���̃f�[�^�̏��������J�n����Map.
	if (SUCCEEDED( m_pContext->Map(
		m_pCBufferPerMesh, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// �R���X�^���g�o�b�t�@(���b�V���p).
		CBUFFER_PER_MESH cb;

		// ���[���h�s���n��.
		cb.mW = mWorld;
		D3DXMatrixTranspose(&cb.mW, &cb.mW);

		// ���[���h,�r���[,�v���W�F�N�V�����s���n��.
		D3DXMATRIX mWVP = mWorld * CameraManager::GetViewProjMatrix();
		D3DXMatrixTranspose(&mWVP, &mWVP);// �s���]�u����.
		// ���s��̌v�Z���@��DirectX��GPU�ňقȂ邽�ߓ]�u���K�v.
		cb.mWVP = mWVP;

		// �J���[.
		cb.vColor = m_Color;

		memcpy_s(
			pData.pData,	// �R�s�[��̃o�b�t�@.
			pData.RowPitch,	// �R�s�[��̃o�b�t�@�T�C�Y.
			(void*)(&cb),	// �R�s�[���̃o�b�t�@.
			sizeof(cb));	// �R�s�[���̃o�b�t�@�T�C�Y.

		// �o�b�t�@���̃f�[�^�̏��������I������Unmap.
		m_pContext->Unmap(m_pCBufferPerMesh, 0);
	}

	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�p���邩�H.
	m_pContext->VSSetConstantBuffers(	0, 1, &m_pCBufferPerMesh );	// ���_�V�F�[�_.
	m_pContext->PSSetConstantBuffers(	0, 1, &m_pCBufferPerMesh );	// �s�N�Z���V�F�[�_.

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout(m_pVertexLayout);

	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = m_Model.pMesh->GetNumBytesPerVertex();
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0, 1, &m_pVertexBuffer, &stride, &offset);

	// �����̐������A���ꂼ��̑����̃C���f�b�N�X�o�b�t�@��`��.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		// �g�p����Ă��Ȃ��}�e���A���΍�.
		if (m_pMaterials[m_AttrID[No]].dwNumFace == 0) {
			continue;
		}
		// �C���f�b�N�X�o�b�t�@���Z�b�g.
		m_pContext->IASetIndexBuffer(
			m_ppIndexBuffer[No], DXGI_FORMAT_R32_UINT, 0);
		// �}�e���A���̊e�v�f���V�F�[�_�ɓn��.
		D3D11_MAPPED_SUBRESOURCE pDataMat;
		if (SUCCEEDED(
			m_pContext->Map(m_pCBufferPerMaterial,
				0, D3D11_MAP_WRITE_DISCARD, 0, &pDataMat)))
		{
			// �R���X�^���g�o�b�t�@(�}�e���A���p).
			CBUFFER_PER_MATERIAL cb;
			// �A���r�G���g,�f�B�t���[�Y,�X�y�L�������V�F�[�_�ɓn��.
			cb.vAmbient = m_pMaterials[m_AttrID[No]].Ambient;
			cb.vDiffuse = m_pMaterials[m_AttrID[No]].Diffuse;
			cb.vSpecular = m_pMaterials[m_AttrID[No]].Specular;

			// UV����n��.
			cb.vUV = m_pMaterials[m_AttrID[No]].UV;

			memcpy_s(pDataMat.pData, pDataMat.RowPitch,
				(void*)&cb, sizeof(cb));

			m_pContext->Unmap(m_pCBufferPerMaterial, 0);
		}

		// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�����H.
		m_pContext->VSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);
		m_pContext->PSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);

		// �e�N�X�`�����V�F�[�_�ɓn��.
		if (m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]] != nullptr) {
			// �e�N�X�`��������Ƃ�.
			m_pContext->PSSetSamplers(0, 1, &m_pSampleLinear);
			m_pContext->PSSetShaderResources(
				0, 1, &m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]] );
		}
		else {
			// �e�N�X�`�����Ȃ��Ƃ�.
			ID3D11ShaderResourceView* pNothing[1] = { 0 };
			m_pContext->PSSetShaderResources(0, 1, pNothing);
		}

		// �t�H�O�̕`��.
		if ( m_IsFog == true ) m_pFog->Render( m_pMaterials[m_AttrID[No]].dwNumFace * 3, mWorld, m_pMaterials[m_AttrID[No]].pTexture[m_MaterialTextureNo[m_AttrID[No]]], 0.0f, 100.0f );

		// �v���~�e�B�u(�|���S��)�������_�����O.
		m_pContext->DrawIndexed(
			m_pMaterials[m_AttrID[No]].dwNumFace * 3, 0, 0);
	}
}
