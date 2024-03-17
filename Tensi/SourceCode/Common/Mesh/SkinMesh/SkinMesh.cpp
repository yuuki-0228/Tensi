/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode�Ή�.
**/
#include "SkinMesh.h"
#include "..\..\DirectX\DirectX9.h"
#include "..\..\Fog\Fog.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include "..\..\..\Object\Light\Light.h"
#include "..\..\..\Utility\Transform\PositionRenderer\PositionRenderer.h"

#include <stdlib.h>	// �}���`�o�C�g������Unicode�����ϊ��ŕK�v.
#include <locale.h>
#include <crtdbg.h>
#include <filesystem>

// �V�F�[�_��(�f�B���N�g�����܂�)
const TCHAR SHADER_NAME[] = _T( "Data\\Shader\\SkinMesh.hlsl" );

CSkinMesh::CSkinMesh()
	: m_pSampleLinear			( nullptr )
	, m_pVertexShader			( nullptr )
	, m_pPixelShader			( nullptr )
	, m_pVertexLayout			( nullptr )
	, m_pCBufferPerMesh			( nullptr )
	, m_pCBufferPerMaterial		( nullptr )
	, m_pCBufferPerFrame		( nullptr )
	, m_pCBufferPerBone			( nullptr )
	, m_mWorld					()
	, m_mRotation				()
	, m_mView					()
	, m_mProj					()
	, m_pRotMatrix				( nullptr )
	, m_vLight					()
	, m_vCamPos					()
	, m_dAnimSpeed				( 0.001f )	// ��悸�A���̒l.
	, m_dBlendSpeed				( 0.003f )	// ��悸�A���̒l.
	, m_dAnimTime				()
	, m_IsChangeAnim			( false )
	, m_pD3dxMesh				( nullptr )
	, m_pMeshForRay				( nullptr )
	, m_FilePath				()
	, m_iFrame					()
	, m_PointLightIntensity		( 3.0f )
	, m_pFog					( nullptr )
	, m_IsFog					( false )
	, m_DitherFlag				( false )
	, m_AlphaBlockFlag			( true )
	, m_OrLessDistanceAlphaFlag	( false )
	, m_OrMoreDistanceAlphaFlag	( false )
	, m_OrLessStartDistance		( 0.0f )
	, m_OrMoreStartDistance		( 0.0f )
	, m_AlphaDistance			( 0.0f )
	, m_NowIndex				( 0 )
	, m_NewIndex				( 0 )
	, m_MaterialTextureNo		()
{
	m_pFog = std::make_unique<CFog>();
}

CSkinMesh::~CSkinMesh()
{
	// �������.
	Release();

	// �V�F�[�_��T���v���֌W.
	SAFE_RELEASE( m_pSampleLinear		);
	SAFE_RELEASE( m_pVertexShader		);
	SAFE_RELEASE( m_pPixelShader		);
	SAFE_RELEASE( m_pVertexLayout		);

	// �R���X�^���g�o�b�t�@�֌W.
	SAFE_RELEASE( m_pCBufferPerBone		);
	SAFE_RELEASE( m_pCBufferPerFrame	);
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh		);

	SAFE_RELEASE( m_pD3dxMesh			);
}

//----------------------------.
// ������.
//----------------------------.
HRESULT CSkinMesh::Init( LPCTSTR fileName )
{
	const std::wstring	wFileName	= fileName;
	const std::string	FileName	= StringConversion::to_String( wFileName );

	// ���f���ǂݍ���.
	if( FAILED( LoadXMesh( fileName ) )														) return E_FAIL;
	// �V�F�[�_�̍쐬.
	if( FAILED( CreateShader() )															) return E_FAIL;
	// �R���X�^���g�o�b�t�@(���b�V������).
	if( FAILED( CreateCBuffer( &m_pCBufferPerMesh, sizeof( CBUFFER_PER_MESH ) ) )			) return E_FAIL;
	// �R���X�^���g�o�b�t�@(���b�V������).
	if( FAILED( CreateCBuffer( &m_pCBufferPerMaterial, sizeof( CBUFFER_PER_MATERIAL ) ) )	) return E_FAIL;
	// �R���X�^���g�o�b�t�@(���b�V������).
	if( FAILED( CreateCBuffer( &m_pCBufferPerFrame, sizeof( CBUFFER_PER_FRAME ) ) )			) return E_FAIL;
	// �R���X�^���g�o�b�t�@(���b�V������).
	if( FAILED( CreateCBuffer( &m_pCBufferPerBone, sizeof( CBUFFER_PER_BONES ) ) )			) return E_FAIL;
	// �e�N�X�`���[�p�T���v���[�쐬.
	if( FAILED( CreateSampler( &m_pSampleLinear ) )											) return E_FAIL;

	// �{�[�����W��ݒ肷��.
	D3DXMATRIX m;
	D3DXMatrixIdentity( &m );
	m_pD3dxMesh->UpdateFrameMatrices( m_pD3dxMesh->m_pFrameRoot, &m );

	// �t�H�O�̏�����.
	m_pFog->Init();

	return S_OK;
}

//----------------------------.
// �V�F�[�_������.
//----------------------------.
HRESULT	CSkinMesh::CreateShader()
{
	// D3D11�֘A�̏�����
	ID3DBlob*		pCompiledShader = nullptr;
	ID3DBlob*		pErrors			= nullptr;
	UINT			uCompileFlag	= 0;
#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif// #ifdef _DEBUG

	// HLSL����o�[�e�b�N�X�V�F�[�_�̃u���u���쐬.
	if( FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME, nullptr, nullptr,
			"VS_Main", "vs_5_0",
			uCompileFlag, 0, nullptr,
			&pCompiledShader, &pErrors, nullptr ) ) )
	{
		int size = static_cast<int>( pErrors->GetBufferSize() );
//		char* ch = (char*)pErrors->GetBufferPointer();
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE( pErrors );

	// ��L�ō쐬�����u���u����o�[�e�b�N�X�V�F�[�_���쐬.
	if( FAILED(
		DirectX11::MutexDX11CreateVertexShader(
			pCompiledShader,
			nullptr, &m_pVertexShader ) ) )
	{
		SAFE_RELEASE(pCompiledShader);
		ErrorMessage( "�o�[�e�b�N�X�V�F�[�_�쐬���s" );
		return E_FAIL;
	}
	// ���_�C���v�b�g���C�A�E�g���`	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT,	0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT",0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	// ���_�C���v�b�g���C�A�E�g���쐬
	if( FAILED(
		DirectX11::MutexDX11CreateInputLayout(
			layout, numElements, pCompiledShader, &m_pVertexLayout ) ) )
	{
		ErrorMessage( "���_�C���v�b�g���C�A�E�g�쐬���s" );
		return E_FAIL;
	}
	// ���_�C���v�b�g���C�A�E�g���Z�b�g
	DirectX11::GetContext()->IASetInputLayout( m_pVertexLayout );

	// HLSL����s�N�Z���V�F�[�_�̃u���u���쐬.
	if(FAILED(
		DirectX11::MutexD3DX11CompileFromFile(
			SHADER_NAME, nullptr, nullptr,
			"PS_Main", "ps_5_0",
			uCompileFlag, 0, nullptr,
			&pCompiledShader, &pErrors, nullptr ) ) )
	{
		ErrorMessage( "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE( pErrors );
	// ��L�ō쐬�����u���u����s�N�Z���V�F�[�_���쐬.
	if( FAILED(
		DirectX11::MutexDX11CreatePixelShader(
			pCompiledShader,
			nullptr, &m_pPixelShader ) ) )
	{
		SAFE_RELEASE( pCompiledShader );
		ErrorMessage( "�s�N�Z���V�F�[�_�쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE( pCompiledShader );

	return S_OK;
}

//----------------------------.
// X�t�@�C������X�L���֘A�̏���ǂݏo���֐�.
//----------------------------.
HRESULT CSkinMesh::ReadSkinInfo(
	MYMESHCONTAINER* pContainer, VERTEX* pvVB, SKIN_PARTS_MESH* pParts )
{
	// X�t�@�C�����璊�o���ׂ����́A
	//	�u���_���Ƃ̃{�[���C���f�b�N�X�v�u���_���Ƃ̃{�[���E�F�C�g�v.
	//	�u�o�C���h�s��v�u�|�[�Y�s��v��4����.

	int i, k, m, n;			// �e�탋�[�v�ϐ�.
	int iNumVertex	= 0;	// ���_��.
	int iNumBone	= 0;	// �{�[����.

	// ���_��.
	iNumVertex	= m_pD3dxMesh->GetNumVertices( pContainer );
	// �{�[����.
	iNumBone	= m_pD3dxMesh->GetNumBones( pContainer );

	// ���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�.
	//	��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���.
	for( i=0; i<iNumBone; i++ )
	{
		// ���̃{�[���ɉe�����󂯂钸�_��.
		int iNumIndex
			= m_pD3dxMesh->GetNumBoneVertices( pContainer, i );

		int*	pIndex = new int[iNumIndex]();
		double*	pWeight = new double[iNumIndex]();

		for( k=0; k<iNumIndex; k++ )
		{
			pIndex[k]
				= m_pD3dxMesh->GetBoneVerticesIndices( pContainer, i, k );
			pWeight[k]
				= m_pD3dxMesh->GetBoneVerticesWeights( pContainer, i, k );
		}

		// ���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������.
		for( k=0; k<iNumIndex; k++ )
		{
			// X�t�@�C����CG�\�t�g���{�[��4�{�ȓ��Ƃ͌���Ȃ�.
			//	5�{�ȏ�̏ꍇ�́A�d�݂̑傫������4�{�ɍi��.

			// �E�F�C�g�̑傫�����Ƀ\�[�g(�o�u���\�[�g).
			for( m=4; m>1; m-- )
			{
				for( n=1; n<m; n++ )
				{
					if( pvVB[pIndex[k]].bBoneWeight[n-1] < pvVB[pIndex[k]].bBoneWeight[n] )
					{
						float tmp = pvVB[pIndex[k]].bBoneWeight[n - 1];
						pvVB[pIndex[k]].bBoneWeight[n - 1]	= pvVB[pIndex[k]].bBoneWeight[n];
						pvVB[pIndex[k]].bBoneWeight[n]		= tmp;
						int itmp = pvVB[pIndex[k]].bBoneIndex[n - 1];
						pvVB[pIndex[k]].bBoneIndex[n - 1]	= pvVB[pIndex[k]].bBoneIndex[n];
						pvVB[pIndex[k]].bBoneIndex[n]		= itmp;
					}
				}
			}
			// �\�[�g��́A�Ō�̗v�f�Ɉ�ԏ������E�F�C�g�������Ă�͂�.
			bool flag = false;
			for( m=0; m<4; m++ )
			{
				if( pvVB[pIndex[k]].bBoneWeight[m] == 0 )
				{
					flag = true;
					pvVB[pIndex[k]].bBoneIndex[ m] = i;
					pvVB[pIndex[k]].bBoneWeight[m] = (float)pWeight[k];
					break;
				}
			}
			if( flag == false )
			{
				pvVB[pIndex[k]].bBoneIndex[ 3] = i;
				pvVB[pIndex[k]].bBoneWeight[3] = (float)pWeight[k];
				break;
			}
		}
		// �g���I���΍폜����.
		delete[] pIndex;
		delete[] pWeight;
	}

	// �{�[������.
	pParts->iNumBone	= iNumBone;
	pParts->pBoneArray	= new BONE[iNumBone]();
	// �|�[�Y�s��𓾂�(�����|�[�Y).
	for( i=0; i<pParts->iNumBone; i++ )
	{
		pParts->pBoneArray[i].mBindPose
			= m_pD3dxMesh->GetBindPose( pContainer, i );
	}

	return S_OK;
}

//----------------------------.
// UV�X�N���[������l��ݒ�.
//----------------------------.
void CSkinMesh::SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv )
{
	ChangeUV( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// x������UV�X�N���[������l��ݒ�.
//----------------------------.
void CSkinMesh::SetUVScrollX( const int MaterialNo, const float uv )
{
	ChangeUVX( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// y������UV�X�N���[������l��ݒ�.
//----------------------------.
void CSkinMesh::SetUVScrollY( const int MaterialNo, const float uv )
{
	ChangeUVY( MaterialNo, uv, m_pD3dxMesh->m_pFrameRoot );
}

//----------------------------.
// X����X�L�����b�V�����쐬����.
//	���Ӂj�f�ށiX)�̂ق��́A�O�p�|���S���ɂ��邱��.
//----------------------------.
HRESULT CSkinMesh::LoadXMesh( LPCTSTR lpFileName )
{
	// �t�@�C�������p�X���Ǝ擾.
	lstrcpy( m_FilePath, lpFileName );

	// X�t�@�C���ǂݍ���.
	m_pD3dxMesh = new D3DXPARSER();
	m_pD3dxMesh->LoadMeshFromX( DirectX9::GetDevice(), lpFileName );


	// �S�Ẵ��b�V�����쐬����.
	BuildAllMesh( m_pD3dxMesh->m_pFrameRoot );

	return S_OK;
}

//----------------------------.
// Direct3D�̃C���f�b�N�X�o�b�t�@�[�쐬.
//----------------------------.
HRESULT CSkinMesh::CreateIndexBuffer(
	DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer )
{
	D3D11_BUFFER_DESC bd;
	bd.Usage					= D3D11_USAGE_DEFAULT;
	bd.ByteWidth				= dwSize;
	bd.BindFlags				= D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags			= 0;
	bd.MiscFlags				= 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem			= pIndex;
	InitData.SysMemPitch		= 0;
	InitData.SysMemSlicePitch	= 0;
	if( FAILED( m_pDevice->CreateBuffer( &bd, &InitData, ppIndexBuffer ) ) )
	{
		return E_FAIL;
	}
	
	return S_OK;
}

//----------------------------.
// �����_�����O.
//----------------------------.
void CSkinMesh::Render( STransform* pTransform, SAnimationController* pAC, D3DXMATRIX* pRot )
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

	m_pRotMatrix = pRot;

	m_mView		= CameraManager::GetViewMatrix();
	m_mProj		= CameraManager::GetProjMatrix();
	m_vCamPos	= CameraManager::GetPosition();
	m_vLight	= Light::GetDirection();

	// �A�j���[�V�����̍X�V.
	AnimUpdate( pAC );

	D3DXMATRIX m;
	D3DXMatrixIdentity( &m );
	m_pD3dxMesh->UpdateFrameMatrices( m_pD3dxMesh->m_pFrameRoot, &m );
	DrawFrame( Transform, m_pD3dxMesh->m_pFrameRoot );

}

//----------------------------.
// �S�Ẵ��b�V�����쐬����.
//----------------------------.
void CSkinMesh::BuildAllMesh( D3DXFRAME* pFrame )
{
	if( pFrame && pFrame->pMeshContainer ) {
		CreateAppMeshFromD3DXMesh( pFrame );
	}

	// �ċA�֐�.
	if( pFrame->pFrameSibling != nullptr ) {
		BuildAllMesh( pFrame->pFrameSibling );
	}
	if( pFrame->pFrameFirstChild != nullptr ) {
		BuildAllMesh( pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// ���b�V���쐬.
//----------------------------.
HRESULT CSkinMesh::CreateAppMeshFromD3DXMesh( LPD3DXFRAME p )
{
	MYFRAME* pFrame	= (MYFRAME*)p;

	//LPD3DXMESH pD3DXMesh = pFrame->pMeshContainer->MeshData.pMesh;//D3DX���b�V��(��������E�E�E).
	m_pMeshForRay = pFrame->pMeshContainer->MeshData.pMesh; //D3DXү��(��������E�E�E).
	MYMESHCONTAINER* pContainer = (MYMESHCONTAINER*)pFrame->pMeshContainer;

	// �A�v�����b�V��(�E�E�E�����Ƀ��b�V���f�[�^���R�s�[����).
	SKIN_PARTS_MESH* pAppMesh = new SKIN_PARTS_MESH();
	pAppMesh->bTex = false;

	// ���O�ɒ��_���A�|���S�������𒲂ׂ�.
	pAppMesh->dwNumVert	= m_pD3dxMesh->GetNumVertices( pContainer );
	pAppMesh->dwNumFace	= m_pD3dxMesh->GetNumFaces( pContainer );
	pAppMesh->dwNumUV	= m_pD3dxMesh->GetNumUVs( pContainer );
	// Direct3D�ł�UV�̐��������_���K�v.
	if( pAppMesh->dwNumVert < pAppMesh->dwNumUV ){
		// ���L���_���ŁA���_������Ȃ��Ƃ�.
		ErrorMessage( "Direct3D�́AUV�̐��������_���K�v�ł�(UV��u���ꏊ���K�v�ł�)�e�N�X�`���͐������\���Ȃ��Ǝv���܂�" );
		return E_FAIL;
	}
	// �ꎞ�I�ȃ������m��(���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@).
	VERTEX* pvVB = new VERTEX[pAppMesh->dwNumVert]();
	int* piFaceBuffer	= new int[pAppMesh->dwNumFace*3]();
	// 3���_�|���S���Ȃ̂ŁA1�t�F�C�X=3���_(3�C���f�b�N�X).

	// ���_�ǂݍ���.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR3 v	= m_pD3dxMesh->GetVertexCoord( pContainer, i );
		pvVB[i].vPos.x	= v.x;
		pvVB[i].vPos.y	= v.y;
		pvVB[i].vPos.z	= v.z;
	}
	// �|���S�����(���_�C���f�b�N�X)�ǂݍ���.
	for( DWORD i=0; i<pAppMesh->dwNumFace*3; i++ ){
		piFaceBuffer[i] = m_pD3dxMesh->GetIndex( pContainer, i );
	}
	// �@���ǂݍ���.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR3 v	= m_pD3dxMesh->GetNormal( pContainer, i );
		pvVB[i].vNorm.x	= v.x;
		pvVB[i].vNorm.y	= v.y;
		pvVB[i].vNorm.z	= v.z;
	}
	// �e�N�X�`�����W�ǂݍ���.
	for( DWORD i=0; i<pAppMesh->dwNumVert; i++ ){
		D3DXVECTOR2 v	= m_pD3dxMesh->GetUV( pContainer, i );
		pvVB[i].vTex.x	= v.x;
		pvVB[i].vTex.y	= v.y;
	}

	// �}�e���A���ǂݍ���.
	pAppMesh->dwNumMaterial	= m_pD3dxMesh->GetNumMaterials( pContainer );
	pAppMesh->pMaterial		= new MY_SKINMATERIAL[pAppMesh->dwNumMaterial]();
	m_MaterialTextureNo.resize( pAppMesh->dwNumMaterial );

	// �}�e���A���̐������C���f�b�N�X�o�b�t�@���쐬.
	pAppMesh->ppIndexBuffer = new ID3D11Buffer*[pAppMesh->dwNumMaterial]();
	// �|���Z�ł͂Ȃ��uID3D11Buffer*�v�̔z��Ƃ����Ӗ�.
	for( DWORD i=0; i<pAppMesh->dwNumMaterial; i++ )
	{
		// ����(�A���r�G���g).
		pAppMesh->pMaterial[i].Ka.x	= m_pD3dxMesh->GetAmbient( pContainer, i ).y;
		pAppMesh->pMaterial[i].Ka.y	= m_pD3dxMesh->GetAmbient( pContainer, i ).z;
		pAppMesh->pMaterial[i].Ka.z	= m_pD3dxMesh->GetAmbient( pContainer, i ).w;
		pAppMesh->pMaterial[i].Ka.w	= m_pD3dxMesh->GetAmbient( pContainer, i ).x;
		// �g�U���ˌ�(�f�B�t���[�Y).
		pAppMesh->pMaterial[i].Kd.x	= m_pD3dxMesh->GetDiffuse( pContainer, i ).y;
		pAppMesh->pMaterial[i].Kd.y	= m_pD3dxMesh->GetDiffuse( pContainer, i ).z;
		pAppMesh->pMaterial[i].Kd.z	= m_pD3dxMesh->GetDiffuse( pContainer, i ).w;
		pAppMesh->pMaterial[i].Kd.w	= m_pD3dxMesh->GetDiffuse( pContainer, i ).x;
		// ���ʔ��ˌ�(�X�y�L����).
		pAppMesh->pMaterial[i].Ks.x	= m_pD3dxMesh->GetSpecular( pContainer, i ).y;
		pAppMesh->pMaterial[i].Ks.y	= m_pD3dxMesh->GetSpecular( pContainer, i ).z;
		pAppMesh->pMaterial[i].Ks.z	= m_pD3dxMesh->GetSpecular( pContainer, i ).w;
		pAppMesh->pMaterial[i].Ks.w	= m_pD3dxMesh->GetSpecular( pContainer, i ).x;

#if 0
		// �e�N�X�`��(�f�B�t���[�Y�e�N�X�`���̂�).
#ifdef UNICODE
		WCHAR TexFilename_w[32] = L"";
		// �e�N�X�`�����̃T�C�Y���擾.
		size_t charSize = strlen( m_pD3dxMesh->GetTexturePath( pContainer, i ) ) + 1;
		size_t ret;	// �ϊ����ꂽ������.

		// �}���`�o�C�g�����̃V�[�P���X��Ή����郏�C�h�����̃V�[�P���X�ɕϊ����܂�.
		errno_t err = mbstowcs_s(
			&ret,
			TexFilename_w,
			charSize,
			m_pD3dxMesh->GetTexturePath( pContainer, i ),
			_TRUNCATE );

		LPTSTR name = TexFilename_w;
#else// #ifdef UNICODE
		LPTSTR name = d3dxMaterials[No].pTextureFilename;
#endif// #ifdef UNICODE
#endif

		// �e�N�X�`��(�f�B�t���[�Y�e�N�X�`���̂�).
#ifdef UNICODE
		WCHAR TextureName_w[32] = L"";
		// �����ϊ�.
		ConvertCharaMultiByteToUnicode( TextureName_w, 32, m_pD3dxMesh->GetTexturePath( pContainer, i ));
		LPTSTR name = TextureName_w;
#else// #ifdef UNICODE
		LPTSTR name = m_pD3dxMesh->GetTexturePath( pContainer, i );
#endif// #ifdef UNICODE

		if( name != nullptr ){
			LPTSTR ret = _tcsrchr( m_FilePath, _T( '\\' ) );
			if( ret != nullptr ){
				int check = static_cast<int>( ret - m_FilePath );
				TCHAR path[512];
				lstrcpy( path, m_FilePath );
				path[check + 1] = '\0';

				// �p�X�̃R�s�[.
				lstrcpy( pAppMesh->pMaterial[i].TextureName, path );
				// �e�N�X�`���t�@�C������A��.
				lstrcat( pAppMesh->pMaterial[i].TextureName, name );
			}
		}
		// �e�N�X�`�����쐬.
		if ( pAppMesh->pMaterial[i].TextureName != nullptr ) {
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
					pAppMesh->pMaterial[i].TextureName,
					nullptr, nullptr, &pAppMesh->pMaterial[i].pTexture[0], nullptr ) ) )
			{
				ErrorMessage( "�e�N�X�`���쐬���s" );
				return E_FAIL;
			}

			// �����ւ��e�N�X�`�����擾.
			int TexNo = 1;
			while ( 1 ) {
				// �����ւ��e�N�X�`���p�X�̍쐬.
				std::string TexName	 = StringConversion::to_String( pAppMesh->pMaterial[i].TextureName );
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
				pAppMesh->pMaterial[i].pTexture.emplace_back( pTexture );
			}

			// ���C�g�e�N�X�`���p�X�̍쐬.
			std::string LightTexName = StringConversion::to_String( pAppMesh->pMaterial[i].TextureName );
			size_t		LightIndex	 = LightTexName.find( "." );
			LightTexName.insert( LightIndex, "_light" );

			// ���C�g�e�N�X�`�����쐬�ł��邩.
			if ( FAILED(
				DirectX11::MutexD3DX11CreateShaderResourceViewFromFile(
				StringConversion::to_wString( LightTexName ).c_str(),
				nullptr, nullptr, &pAppMesh->pMaterial[i].pLightTexture, nullptr ) ) )
			{
				pAppMesh->pMaterial[i].pLightTexture = nullptr;
			}
		}
		// ���̃}�e���A���ł���C���f�b�N�X�z����̊J�n�C���f�b�N�X�𒲂ׂ�.
		//	����ɃC���f�b�N�X�̌��𒲂ׂ�.
		int iCount = 0;
		int* pIndex = new int[pAppMesh->dwNumFace*3]();
			// �Ƃ肠�����A���b�V�����̃|���S�����Ń������m��.
			//	(�����̂ۂ育�񂮂�[�Ղ͕K������ȉ��ɂȂ�).

		for( DWORD k=0; k<pAppMesh->dwNumFace; k++ )
		{
			// ���� i==k �Ԗڂ̃|���S���̃}�e���A���ԍ��Ȃ�.
			if( i == m_pD3dxMesh->GeFaceMaterialIndex( pContainer, k ) )
			{
				// k�Ԗڂ̃|���S������钸�_�̃C���f�b�N�X.
				pIndex[iCount]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 0 );	// 1��.
				pIndex[iCount+1]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 1 );	// 2��.
				pIndex[iCount+2]
					= m_pD3dxMesh->GetFaceVertexIndex( pContainer, k, 2 );	// 3��.
				iCount += 3;
			}
		}
		if( iCount > 0 ){
			// �C���f�b�N�X�o�b�t�@�쐬.
			CreateIndexBuffer( iCount*sizeof(int),
				pIndex, &pAppMesh->ppIndexBuffer[i] );
		}
		else{
			// ������̏����ɕs����o������.
			//	�J�E���g����0�ȉ��̏ꍇ�́A�C���f�b�N�X�o�b�t�@�� nullptr �ɂ��Ă���.
			pAppMesh->ppIndexBuffer[i] = nullptr;
		}

		// ���̃}�e���A�����̃|���S����.
		pAppMesh->pMaterial[i].dwNumFace = iCount / 3;
		// �s�v�ɂȂ����̂ō폜.
		delete[] pIndex;
	}

	// �X�L����񂠂�H
	if( pContainer->pSkinInfo == nullptr ){
#ifdef _DEBUG
		// �s���ȃX�L������΂����ŋ�����.�s�v�Ȃ�R�����g�A�E�g���Ă�������.
		TCHAR strDbg[128];
		WCHAR str[64] = L"";
		ConvertCharaMultiByteToUnicode( str, 64, pContainer->Name );
		_stprintf( strDbg, _T( "ContainerName:[%s]" ), str );
		MessageBox( nullptr, strDbg, _T( "Not SkinInfo" ), MB_OK );
#endif// #ifdef _DEBUG
		pAppMesh->bEnableBones = false;
	}
	else{
		// �X�L�����(�W���C���g�A�E�F�C�g)�ǂݍ���.
		ReadSkinInfo( pContainer, pvVB, pAppMesh );
	}

	// �o�[�e�b�N�X�o�b�t�@���쐬.
	D3D11_BUFFER_DESC bd;
	bd.Usage	= D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( VERTEX ) * ( pAppMesh->dwNumVert );
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags= 0;
	bd.MiscFlags	= 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pvVB;

	HRESULT hRslt = S_OK;
	if( FAILED(
		m_pDevice->CreateBuffer(
			&bd, &InitData, &pAppMesh->pVertexBuffer ) ) )
	{
		hRslt = E_FAIL;
	}

	// �p�[�c���b�V���ɐݒ�.
	pFrame->pPartsMesh = pAppMesh;
//	m_pReleaseMaterial = pAppMesh;

	// �ꎞ�I�ȓ��ꕨ�͕s�v�Ȃ�̂ō폜.
	if( piFaceBuffer ){
		delete[] piFaceBuffer;
	}
	if( pvVB ){
		delete[] pvVB;
	}

	return hRslt;
}

//----------------------------.
//�{�[�������̃|�[�Y�ʒu�ɃZ�b�g����֐�.
//----------------------------.
void CSkinMesh::SetNewPoseMatrices(
	SKIN_PARTS_MESH* pParts, int frame, MYMESHCONTAINER* pContainer )
{
	// �]�ރt���[����Update���邱��.
	//	���Ȃ��ƍs�񂪍X�V����Ȃ�.
	//	m_pD3dxMesh->UpdateFrameMatrices(
	//	m_pD3dxMesh->m_pFrameRoot)�������_�����O���Ɏ��s���邱��.

	// �܂��A�A�j���[�V�������ԂɌ��������s����X�V����̂�D3DXMESH�ł�
	//	�A�j���[�V�����R���g���[����(����)����Ă������̂Ȃ̂ŁA
	//	�A�j���[�V�����R���g���[�����g���ăA�j����i�s�����邱�Ƃ��K�v.
	//	m_pD3dxMesh->m_pAnimController->AdvanceTime(...)��.
	//	�����_�����O���Ɏ��s���邱��.

	if( pParts->iNumBone <= 0 ){
		// �{�[���� 0�@�ȉ��̏ꍇ.
	}

	for( int i=0; i<pParts->iNumBone; i++ )
	{
		pParts->pBoneArray[i].mNewPose
			= m_pD3dxMesh->GetNewPose( pContainer, i );
	}
}

//----------------------------.
// ����(���݂�)�|�[�Y�s���Ԃ��֐�.
//----------------------------.
D3DXMATRIX CSkinMesh::GetCurrentPoseMatrix( SKIN_PARTS_MESH* pParts, int index )
{
	D3DXMATRIX ret =
		pParts->pBoneArray[index].mBindPose * pParts->pBoneArray[index].mNewPose;
	return ret;
}

//----------------------------.
// �t���[���̕`��.
//----------------------------.
VOID CSkinMesh::DrawFrame( STransform* pTransform, LPD3DXFRAME p )
{
	MYFRAME*			pFrame	= (MYFRAME*)p;
	SKIN_PARTS_MESH*	pPartsMesh	= pFrame->pPartsMesh;
	MYMESHCONTAINER*	pContainer	= (MYMESHCONTAINER*)pFrame->pMeshContainer;

	if( pPartsMesh != nullptr )
	{
		DrawPartsMesh(
			pTransform,
			pPartsMesh, 
			pFrame->CombinedTransformationMatrix,
			pContainer );
	}

	// �ċA�֐�(�Z��).
	if( pFrame->pFrameSibling != nullptr )
	{
		DrawFrame( pTransform, pFrame->pFrameSibling );
	}
	// (�e�q)
	if( pFrame->pFrameFirstChild != nullptr )
	{
		DrawFrame( pTransform, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// �p�[�c���b�V����`��.
//----------------------------.
void CSkinMesh::DrawPartsMesh(
	STransform* pTransform, SKIN_PARTS_MESH* pMesh, D3DXMATRIX World, MYMESHCONTAINER* pContainer )
{
	D3D11_MAPPED_SUBRESOURCE pData;

	// �g�p����V�F�[�_�̃Z�b�g.
	m_pContext->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext->PSSetShader( m_pPixelShader, nullptr, 0 );

	// ���[���h�s��̎擾.
	m_mWorld = pTransform->GetWorldMatrix( m_pRotMatrix );

	// �A�j���[�V�����t���[����i�߂� �X�L�����X�V.
	m_iFrame++;
	if( m_iFrame >= 3600 ){
		m_iFrame = 0;
	}
	SetNewPoseMatrices( pMesh, m_iFrame, pContainer );

	//------------------------------------------------.
	//	�R���X�^���g�o�b�t�@�ɏ��𑗂�(�{�[��).
	//------------------------------------------------.
	if( SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerBone, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		CBUFFER_PER_BONES cb;
		for( int i=0; i<pMesh->iNumBone; i++ )
		{
			D3DXMATRIX mat = GetCurrentPoseMatrix( pMesh, i );
			D3DXMatrixTranspose( &mat, &mat );
			cb.mBone[i] = mat;
		}
		memcpy_s( pData.pData, pData.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerBone, 0 );
	}
	m_pContext->VSSetConstantBuffers( 3, 1, &m_pCBufferPerBone );
	m_pContext->PSSetConstantBuffers( 3, 1, &m_pCBufferPerBone );
	
	// �o�[�e�b�N�X�o�b�t�@���Z�b�g.
	UINT stride = sizeof( VERTEX );
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(
		0, 1, &pMesh->pVertexBuffer, &stride, & offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext->IASetInputLayout(	m_pVertexLayout );

	// �v���~�e�B�u�E�g�|���W�[���Z�b�g.
	m_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//------------------------------------------------.
	//	�R���X�^���g�o�b�t�@�ɏ���ݒ�(�t���[������).
	//------------------------------------------------.
	if( SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerFrame, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		CBUFFER_PER_FRAME cb;
		cb.vCameraPos				= D3DXVECTOR4( m_vCamPos.x, m_vCamPos.y, m_vCamPos.z, 0.0f );
		cb.vLightDir				= D3DXVECTOR4( m_vLight.x, m_vLight.y, m_vLight.z, 0.0f );
		cb.vLightIntensity.x		= Light::GetIntensity();
		cb.vPointLightIntensity.x	= m_PointLightIntensity;
		cb.vFlag.x					= m_DitherFlag				== true ? 1.0f : 0.0f;
		cb.vFlag.y					= m_AlphaBlockFlag			== true ? 1.0f : 0.0f;
		cb.vFlag.z					= m_OrLessDistanceAlphaFlag	== true ? 1.0f : 0.0f;
		cb.vFlag.w					= m_OrMoreDistanceAlphaFlag	== true ? 1.0f : 0.0f;
		cb.vAlphaDistance.x			= m_OrLessStartDistance;
		cb.vAlphaDistance.y			= m_OrMoreStartDistance;
		cb.vAlphaDistance.z			= m_AlphaDistance;
		memcpy_s( pData.pData, pData.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerFrame, 0 );
	}
	m_pContext->VSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );
	m_pContext->PSSetConstantBuffers( 2, 1, &m_pCBufferPerFrame );


	//------------------------------------------------.
	//	�R���X�^���g�o�b�t�@�ɏ���ݒ�(���b�V������).
	//------------------------------------------------.
	D3D11_MAPPED_SUBRESOURCE pDat;
	if (SUCCEEDED(
		m_pContext->Map(
			m_pCBufferPerMesh, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &pDat )))
	{
		CBUFFER_PER_MESH cb;
		cb.mW = m_mWorld;
		D3DXMatrixTranspose( &cb.mW, &cb.mW );
		cb.mWVP = m_mWorld * m_mView * m_mProj;
		D3DXMatrixTranspose( &cb.mWVP, &cb.mWVP );
		cb.vColor = m_Color;
		memcpy_s( pDat.pData, pDat.RowPitch, (void*)&cb, sizeof( cb ) );
		m_pContext->Unmap(m_pCBufferPerMesh, 0);
	}
	m_pContext->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
	m_pContext->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );


	// �}�e���A���̐������A
	//	���ꂼ��̃}�e���A���̃C���f�b�N�X�o�b�t�@��`��.
	for( DWORD i=0; i<pMesh->dwNumMaterial; i++ )
	{
		// �g�p����Ă��Ȃ��}�e���A���΍�.
		if( pMesh->pMaterial[i].dwNumFace == 0 )
		{
			continue;
		}
		// �C���f�b�N�X�o�b�t�@���Z�b�g.
		stride	= sizeof( int );
		offset	= 0;
		m_pContext->IASetIndexBuffer(
			pMesh->ppIndexBuffer[i],
			DXGI_FORMAT_R32_UINT, 0 );

		//------------------------------------------------.
		//	�}�e���A���̊e�v�f�ƕϊ��s����V�F�[�_�ɓn��.
		//------------------------------------------------.
		D3D11_MAPPED_SUBRESOURCE pDat;
		if( SUCCEEDED(
			m_pContext->Map(
				m_pCBufferPerMaterial, 0,
				D3D11_MAP_WRITE_DISCARD, 0, &pDat )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.vAmbient	= pMesh->pMaterial[i].Ka;
			cb.vDiffuse	= pMesh->pMaterial[i].Kd;
			cb.vSpecular= pMesh->pMaterial[i].Ks;
			cb.vUV		= pMesh->pMaterial[i].UV;
			memcpy_s( pDat.pData, pDat.RowPitch, (void*)&cb, sizeof( cb ) );
			m_pContext->Unmap( m_pCBufferPerMaterial, 0 );
		}
		m_pContext->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );

		// �e�N�X�`�����V�F�[�_�ɓn��.
		if( pMesh->pMaterial[i].TextureName != nullptr )
		{
			m_pContext->PSSetSamplers( 0, 1, &m_pSampleLinear );
			m_pContext->PSSetShaderResources( 0, 1, &pMesh->pMaterial[i].pTexture[m_MaterialTextureNo[i]] );
			m_pContext->PSSetShaderResources( 1, 1, &pMesh->pMaterial[i].pLightTexture );
		}
		else
		{
			ID3D11ShaderResourceView* Nothing[1] = { 0 };
			m_pContext->PSSetShaderResources( 0, 1, Nothing );
		}

		// �t�H�O�̕`��.
		if ( m_IsFog == true ) m_pFog->Render( pMesh->pMaterial[i].dwNumFace * 3, m_mWorld, pMesh->pMaterial[i].pTexture[m_MaterialTextureNo[i]], 0.0f, 100.0f );

		// Draw.
		m_pContext->DrawIndexed( pMesh->pMaterial[i].dwNumFace * 3, 0, 0 );
	}
}

//----------------------------.
// �A�j���[�V�����̍X�V.
//----------------------------.
void CSkinMesh::AnimUpdate( SAnimationController* pAC )
{
	if ( pAC == nullptr ) {
		if ( m_pD3dxMesh->m_pAnimController ) {
			BlendAnimUpdate();
			m_pD3dxMesh->m_pAnimController->AdvanceTime( m_dAnimSpeed, nullptr );
		}
	}
	else {
		pAC->BlendAnimUpdate(  m_dBlendSpeed );
		pAC->pAC->AdvanceTime( m_dAnimSpeed, nullptr );
	}
}

//----------------------------.
// �u�����h�A�j���[�V�����̍X�V.
//----------------------------.
void CSkinMesh::BlendAnimUpdate()
{
	// �A�j���[�V�����؂�ւ��t���O������Ă���I��.
	if ( m_IsChangeAnim == false ) return;

	m_dAnimTime += m_dAnimSpeed;
	float Weight = static_cast<float>( m_dBlendSpeed ) / 1.0f;			// �E�F�C�g�̌v�Z.
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 0, Weight );		// �g���b�N0 �ɃE�F�C�g��ݒ�.
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 1, 1.0f - Weight );	// �g���b�N1 �ɃE�F�C�g��ݒ�.

	if ( m_dAnimTime < 1.0 ) return;
	// �A�j���[�V�����^�C�������l�ɒB������.
	m_IsChangeAnim	= false;	// �t���O������.
	m_NowIndex		= m_NewIndex;
	m_NewIndex		= -1;
	m_pD3dxMesh->m_pAnimController->SetTrackWeight( 0, 1.0f );			// �E�F�C�g��1�ɌŒ肷��.
	m_pD3dxMesh->m_pAnimController->SetTrackEnable( 1, false );			// �g���b�N1�𖳌��ɂ���.
}

//----------------------------.
//����֐�.
//----------------------------.
HRESULT CSkinMesh::Release()
{
	if( m_pD3dxMesh != nullptr ){
		// �S�Ẵ��b�V���폜.
		DestroyAllMesh( m_pD3dxMesh->m_pFrameRoot );

		// �p�[�T�[�N���X�������.
		m_pD3dxMesh->Release();
		SAFE_DELETE( m_pD3dxMesh );
	}

	return S_OK;
}

//----------------------------.
// �S�Ẵ��b�V�����폜.
//----------------------------.
void CSkinMesh::DestroyAllMesh( D3DXFRAME* pFrame )
{
	if ((pFrame != nullptr) && (pFrame->pMeshContainer != nullptr))
	{
		DestroyAppMeshFromD3DXMesh( pFrame );
	}

	// �ċA�֐�.
	if( pFrame->pFrameSibling != nullptr )
	{
		DestroyAllMesh( pFrame->pFrameSibling );
	}
	if( pFrame->pFrameFirstChild != nullptr )
	{
		DestroyAllMesh( pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// ���b�V���̍폜.
//----------------------------.
HRESULT CSkinMesh::DestroyAppMeshFromD3DXMesh( LPD3DXFRAME p )
{
	MYFRAME* pFrame = (MYFRAME*)p;

	MYMESHCONTAINER* pMeshContainerTmp = (MYMESHCONTAINER*)pFrame->pMeshContainer;

	// MYMESHCONTAINER�̒��g�̉��.
	if ( pMeshContainerTmp != nullptr )
	{
		if ( pMeshContainerTmp->pBoneBuffer != nullptr )
		{
			pMeshContainerTmp->pBoneBuffer->Release();
			pMeshContainerTmp->pBoneBuffer = nullptr;
		}

		if ( pMeshContainerTmp->pBoneOffsetMatrices != nullptr )
		{
			delete pMeshContainerTmp->pBoneOffsetMatrices;
			pMeshContainerTmp->pBoneOffsetMatrices = nullptr;
		}
		
		if ( pMeshContainerTmp->ppBoneMatrix != nullptr )
		{
			int iMax = static_cast<int>( pMeshContainerTmp->pSkinInfo->GetNumBones() );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				if ( pMeshContainerTmp->ppBoneMatrix[i] != nullptr )
				{
					pMeshContainerTmp->ppBoneMatrix[i] = nullptr;
				}
			}

			delete[] pMeshContainerTmp->ppBoneMatrix;
			pMeshContainerTmp->ppBoneMatrix = nullptr;
		}

		if ( pMeshContainerTmp->ppTextures != nullptr )
		{
			int iMax = static_cast<int>( pMeshContainerTmp->NumMaterials );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				if ( pMeshContainerTmp->ppTextures[i] != nullptr )
				{
					pMeshContainerTmp->ppTextures[i]->Release();
					pMeshContainerTmp->ppTextures[i] = nullptr;
				}
			}

			delete[] pMeshContainerTmp->ppTextures;
			pMeshContainerTmp->ppTextures = nullptr;
		}
	}

	pMeshContainerTmp = nullptr;

	// MYMESHCONTAINER�������.

	// LPD3DXMESHCONTAINER�̉��.
	if ( pFrame->pMeshContainer->pAdjacency != nullptr )
	{
		delete[] pFrame->pMeshContainer->pAdjacency;
		pFrame->pMeshContainer->pAdjacency = nullptr;
	}

	if ( pFrame->pMeshContainer->pEffects != nullptr )
	{
		if ( pFrame->pMeshContainer->pEffects->pDefaults != nullptr )
		{
			if ( pFrame->pMeshContainer->pEffects->pDefaults->pParamName != nullptr )
			{
				delete pFrame->pMeshContainer->pEffects->pDefaults->pParamName;
				pFrame->pMeshContainer->pEffects->pDefaults->pParamName = nullptr;
			}

			if ( pFrame->pMeshContainer->pEffects->pDefaults->pValue != nullptr )
			{
				delete pFrame->pMeshContainer->pEffects->pDefaults->pValue;
				pFrame->pMeshContainer->pEffects->pDefaults->pValue = nullptr;
			}

			delete pFrame->pMeshContainer->pEffects->pDefaults;
			pFrame->pMeshContainer->pEffects->pDefaults = nullptr;
		}

		if ( pFrame->pMeshContainer->pEffects->pEffectFilename != nullptr )
		{
			delete pFrame->pMeshContainer->pEffects->pEffectFilename;
			pFrame->pMeshContainer->pEffects->pEffectFilename = nullptr;
		}

		delete pFrame->pMeshContainer->pEffects;
		pFrame->pMeshContainer->pEffects = nullptr;
	}

	if ( pFrame->pMeshContainer->pMaterials != nullptr )
	{
		int iMax = static_cast<int>( pFrame->pMeshContainer->NumMaterials );

		for ( int i = iMax - 1; i >= 0; i-- )
		{
			delete[] pFrame->pMeshContainer->pMaterials[i].pTextureFilename;
			pFrame->pMeshContainer->pMaterials[i].pTextureFilename = nullptr;
		}

		delete[] pFrame->pMeshContainer->pMaterials;
		pFrame->pMeshContainer->pMaterials = nullptr;
	}

	if ( pFrame->pMeshContainer->pNextMeshContainer != nullptr )
	{
		// ���̃��b�V���R���e�i�[�̃|�C���^�[�����̂��Ƃ�����.
		//	new�ō���邱�Ƃ͂Ȃ��͂��Ȃ̂ŁA����ōs����Ǝv��.
		pFrame->pMeshContainer->pNextMeshContainer = nullptr;
	}

	if ( pFrame->pMeshContainer->pSkinInfo != nullptr )
	{
		pFrame->pMeshContainer->pSkinInfo->Release();
		pFrame->pMeshContainer->pSkinInfo = nullptr;
	}

	// LPD3DXMESHCONTAINER�̉������.

	// MYFRAME�̉��.

	if ( pFrame->pPartsMesh != nullptr )
	{
		// �{�[�����̉��.
		if ( pFrame->pPartsMesh->pBoneArray )
		{
			delete[] pFrame->pPartsMesh->pBoneArray;
			pFrame->pPartsMesh->pBoneArray = nullptr;
		}

		if ( pFrame->pPartsMesh->pMaterial != nullptr )
		{
			int iMax = static_cast<int>( pFrame->pPartsMesh->dwNumMaterial );

			for ( int i = iMax - 1; i >= 0; i-- )
			{
				const int Size = static_cast<int>( pFrame->pPartsMesh->pMaterial[i].pTexture.size() );
				for ( int n = Size - 1; n >= 0; --n ) {
					if ( pFrame->pPartsMesh->pMaterial[i].pTexture[n] )
					{
						pFrame->pPartsMesh->pMaterial[i].pTexture[n]->Release();
						pFrame->pPartsMesh->pMaterial[i].pTexture[n] = nullptr;
					}
				}
			}

			delete[] pFrame->pPartsMesh->pMaterial;
			pFrame->pPartsMesh->pMaterial = nullptr;
		}
		

		if ( pFrame->pPartsMesh->ppIndexBuffer )
		{
			// �C���f�b�N�X�o�b�t�@���.
			for ( DWORD i = 0; i<pFrame->pPartsMesh->dwNumMaterial; i++ ){
				if ( pFrame->pPartsMesh->ppIndexBuffer[i] != nullptr ){
					pFrame->pPartsMesh->ppIndexBuffer[i]->Release();
					pFrame->pPartsMesh->ppIndexBuffer[i] = nullptr;
				}
			}
			delete[] pFrame->pPartsMesh->ppIndexBuffer;
		}

		// ���_�o�b�t�@�J��.
		pFrame->pPartsMesh->pVertexBuffer->Release();
		pFrame->pPartsMesh->pVertexBuffer = nullptr;
	}

	// �p�[�c�}�e���A���J��.
	delete[] pFrame->pPartsMesh;
	pFrame->pPartsMesh = nullptr;

	// SKIN_PARTS_MESH�������.

	// MYFRAME��SKIN_PARTS_MESH�ȊO�̃����o�[�|�C���^�[�ϐ��͕ʂ̊֐��ŉ������Ă��܂���.

	return S_OK;
}

//----------------------------.
// �A�j���[�V�����Z�b�g�̐؂�ւ�.
//----------------------------.
void CSkinMesh::ChangeAnimSet( int index, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr ) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex ) return;

	m_pD3dxMesh->ChangeAnimSet( index, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= false;
		pAC->NowIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= false;
	m_NowIndex		= index;
	m_dAnimTime		= 0.0;
}

//----------------------------.
// �A�j���[�V�����Z�b�g�̐؂�ւ�(�J�n�t���[���w��\��).
//----------------------------.
void CSkinMesh::ChangeAnimSet_StartPos( int index, double dStartFramePos, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr		) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex ) return;

	m_pD3dxMesh->ChangeAnimSet_StartPos( index, dStartFramePos, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= false;
		pAC->NowIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= false;
	m_NowIndex		= index;
	m_dAnimTime		= 0.0;
}

//----------------------------.
// �A�j���[�V�������u�����h���Đ؂�ւ�.
//----------------------------.
void CSkinMesh::ChangeAnimBlend( int index, int oldIndex, SAnimationController* pAC )
{
	if ( pAC == nullptr ? m_IsChangeAnim : pAC->IsAnimChangeBlend ) return;

	ChangeAnimBlend_Skip( index, oldIndex, pAC );
}
void CSkinMesh::ChangeAnimBlend( int index, SAnimationController* pAC )
{
	if ( pAC != nullptr ) {
		ChangeAnimBlend( index, pAC->NowIndex, pAC );
		return;
	}
	ChangeAnimBlend( index, m_NowIndex, pAC );
}

//----------------------------.
// �A�j���[�V�������u�����h���Đ؂�ւ�(���̃u�����h�����Ă��Ă��s��).
//----------------------------.
void CSkinMesh::ChangeAnimBlend_Skip( int index, int oldIndex, SAnimationController* pAC )
{
	if ( m_pD3dxMesh == nullptr		) return;
	if ( pAC == nullptr ? index == m_NewIndex : index == pAC->NewIndex	) return;
	if ( pAC == nullptr ? index == m_NowIndex : index == pAC->NowIndex	) return;

	m_pD3dxMesh->ChangeAnimBlend( index, oldIndex, pAC == nullptr ? nullptr : pAC->pAC );
	if ( pAC != nullptr ) {
		pAC->IsAnimChangeBlend	= true;
		pAC->NewIndex			= index;
		pAC->AnimTime			= 0.0;
	}
	m_IsChangeAnim	= true;
	m_NewIndex		= index;
	m_dAnimTime		= 0.0;
}
void CSkinMesh::ChangeAnimBlend_Skip( int index, SAnimationController* pAC )
{
	if ( pAC != nullptr ) {
		ChangeAnimBlend_Skip( index, pAC->NowIndex, pAC );
		return;
	}
	ChangeAnimBlend_Skip( index, m_NowIndex, pAC );
}

//----------------------------.
// �A�j���[�V������~���Ԃ��擾.
//----------------------------.
double CSkinMesh::GetAnimPeriod( int index )
{
	if( m_pD3dxMesh == nullptr ){
		return 0.0f;
	}
	return m_pD3dxMesh->GetAnimPeriod( index );
}

//----------------------------.
// �A�j���[�V���������擾.
//----------------------------.
int CSkinMesh::GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC )
{
	if( m_pD3dxMesh != nullptr ){
		return m_pD3dxMesh->GetAnimMax( pAC );
	}
	return -1;
}

//----------------------------.
// �w�肵���{�[�����(�s��)���擾����֐�.
//----------------------------.
D3DXMATRIX CSkinMesh::GetMatrixFromBone(
	LPCSTR sBoneName )
{
	D3DXMATRIX OutMatrix = {};
	GetMatrixFromBone( sBoneName, &OutMatrix );
	return OutMatrix;
}
bool CSkinMesh::GetMatrixFromBone(
	LPCSTR sBoneName, D3DXMATRIX* pOutMat )
{
	if( m_pD3dxMesh != nullptr ){
		if( m_pD3dxMesh->GetMatrixFromBone( sBoneName, pOutMat ) ){
			return true;
		}
	}
	return false;
}

//----------------------------.
// �w�肵���{�[�����(���W)���擾����֐�.
//----------------------------.
D3DXVECTOR3 CSkinMesh::GetPosFromBone(
	LPCSTR sBoneName )
{
	D3DXVECTOR3 OutPos = {};
	GetPosFromBone( sBoneName, &OutPos );
	return OutPos;
}
bool CSkinMesh::GetPosFromBone(
	LPCSTR sBoneName, D3DXVECTOR3* pOutPos )
{
	if( m_pD3dxMesh != nullptr ){
		D3DXVECTOR3 tmpPos;
		if( m_pD3dxMesh->GetPosFromBone( sBoneName, &tmpPos ) ){
			D3DXMATRIX mWorld, mTran, mScale, mRot;
			D3DXMatrixScaling(				&mScale,	m_Transform.Scale.x,	m_Transform.Scale.y,	m_Transform.Scale.z );
			D3DXMatrixRotationYawPitchRoll( &mRot,		m_Transform.Rotation.y, m_Transform.Rotation.x, m_Transform.Rotation.z );
			D3DXMatrixTranslation(			&mTran,		tmpPos.x, tmpPos.y, tmpPos.z );
			mWorld	= mTran * mScale * mRot;

			pOutPos->x = mWorld._41 + m_Transform.Position.x;
			pOutPos->y = mWorld._42 + m_Transform.Position.y;
			pOutPos->z = mWorld._43 + m_Transform.Position.z;

			return true;
		}
	}
	return false;
}
D3DXVECTOR3 CSkinMesh::GetDeviaPosFromBone(
	LPCSTR sBoneName, D3DXVECTOR3 vSpecifiedPos )
{
	D3DXVECTOR3 OutPos = {};
	GetDeviaPosFromBone( sBoneName, &OutPos, vSpecifiedPos );
	return OutPos;
}
bool CSkinMesh::GetDeviaPosFromBone( 
	LPCSTR sBoneName, D3DXVECTOR3* pOutPos, D3DXVECTOR3 vSpecifiedPos )
{
	if ( m_pD3dxMesh != nullptr )
	{
		D3DXMATRIX mtmp;
		if ( m_pD3dxMesh->GetMatrixFromBone( sBoneName, &mtmp )){
			D3DXMATRIX mWorld, mScale, mTran, mDevia;
			D3DXMATRIX mRot, mYaw, mPitch, mRoll;
			//�����炭�{�[���̏����̌��������ʒu�ɂȂ��Ă���͂��ł�.
			D3DXMatrixTranslation( &mDevia, vSpecifiedPos.x, vSpecifiedPos.y, vSpecifiedPos.z );//���炵���������̍s����쐬.
			D3DXMatrixMultiply( &mtmp, &mDevia, &mtmp );//�{�[���ʒu�s��Ƃ��炵���������s����|�����킹��.
			D3DXVECTOR3 tmpPos = D3DXVECTOR3( mtmp._41, mtmp._42, mtmp._43 );//�ʒu�̂ݎ擾.

			D3DXMatrixScaling(		&mScale,	m_Transform.Scale.x, m_Transform.Scale.y, m_Transform.Scale.z );
			D3DXMatrixRotationY(	&mYaw,		m_Transform.Rotation.y );
			D3DXMatrixRotationX(	&mPitch,	m_Transform.Rotation.x );
			D3DXMatrixRotationZ(	&mRoll,		m_Transform.Rotation.z );
			D3DXMatrixTranslation(	&mTran,		tmpPos.x, tmpPos.y, tmpPos.z );

			mRot = mYaw * mPitch * mRoll;
			mWorld = mTran * mScale * mRot;

			pOutPos->x = mWorld._41 + m_Transform.Position.x;
			pOutPos->y = mWorld._42 + m_Transform.Position.y;
			pOutPos->z = mWorld._43 + m_Transform.Position.z;

			return true;
		}
	}
	return false;
}

//----------------------------.
// �R���X�^���g�o�b�t�@�쐬�֐�.
//----------------------------.
HRESULT CSkinMesh::CreateCBuffer(
	ID3D11Buffer** pConstantBuffer,
	UINT size)
{
	D3D11_BUFFER_DESC cb;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth			= size;
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags			= 0;
	cb.StructureByteStride	= 0;
	cb.Usage				= D3D11_USAGE_DYNAMIC;
	if (FAILED(
		m_pDevice->CreateBuffer( &cb, nullptr, pConstantBuffer )))
	{
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �T���v���[�쐬�֐�.
//----------------------------.
HRESULT CSkinMesh::CreateSampler( ID3D11SamplerState** pSampler )
{
	// �e�N�X�`���[�p�T���v���[�쐬.
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory( &SamDesc, sizeof( D3D11_SAMPLER_DESC ) );

	SamDesc.Filter	= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(
		m_pDevice->CreateSamplerState( &SamDesc, &m_pSampleLinear )))
	{
		return E_FAIL;
	}
	return S_OK;
}

//----------------------------.
// �}���`�o�C�g������Unicode�����ɕϊ�����.
//----------------------------.
void CSkinMesh::ConvertCharaMultiByteToUnicode(
	WCHAR* Dest,			// (out)�ϊ���̕�����(Unicode������).
	size_t DestArraySize,	// �ϊ���̕�����̔z��̗v�f�ő吔.
	const CHAR* str )		// (in)�ϊ��O�̕�����(�}���`�o�C�g������).
{
	// �e�N�X�`�����̃T�C�Y���擾.
	size_t charSize = strlen( str ) + 1;
	size_t ret;	// �ϊ����ꂽ������.

	// �}���`�o�C�g�����̃V�[�P���X��Ή����郏�C�h�����̃V�[�P���X�ɕϊ����܂�.
	errno_t err = mbstowcs_s(
		&ret,
		Dest,
		charSize,
		str,
		_TRUNCATE );
}

//----------------------------.
// UV�̕ύX.
//----------------------------.
void CSkinMesh::ChangeUV( const int MaterialNo, const D3DXVECTOR2& uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.x = uv.x;
		pPartsMesh->pMaterial[MaterialNo].UV.y = uv.y;
	}

	// �ċA�֐�(�Z��).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUV( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (�e�q)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUV( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// x������UV�̕ύX.
//----------------------------.
void CSkinMesh::ChangeUVX( const int MaterialNo, const float uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.x = uv;
	}

	// �ċA�֐�(�Z��).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUVX( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (�e�q)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUVX( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}

//----------------------------.
// y������UV�̕ύX.
//----------------------------.
void CSkinMesh::ChangeUVY( const int MaterialNo, const float uv, LPD3DXFRAME p )
{
	MYFRAME*		 pFrame		= ( MYFRAME* )p;
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;

	if ( pPartsMesh != nullptr )
	{
		pPartsMesh->pMaterial[MaterialNo].UV.y = uv;
	}

	// �ċA�֐�(�Z��).
	if ( pFrame->pFrameSibling != nullptr )
	{
		ChangeUVY( MaterialNo, uv, pFrame->pFrameSibling );
	}
	// (�e�q)
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		ChangeUVY( MaterialNo, uv, pFrame->pFrameFirstChild );
	}
}
