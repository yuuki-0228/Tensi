/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode�Ή�.
**/
#include "SkinMeshParser.h"
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include "..\..\..\DirectX\DirectX11.h"

//----------------------------.
// �t���[�����쐬����.
//----------------------------.
HRESULT MY_HIERARCHY::CreateFrame( LPCSTR Name, LPD3DXFRAME *ppNewFrame )
{
	HRESULT hr = S_OK;
	MYFRAME *pFrame;

	*ppNewFrame = nullptr;

	pFrame = new MYFRAME();
	if ( pFrame == nullptr )
	{
		return E_OUTOFMEMORY;
	}
	int length = static_cast<int>( strlen( Name ) ) + 1;
	pFrame->Name = new CHAR[length]();
	if ( !pFrame->Name )
	{
		return E_FAIL;
	}
	strcpy_s( pFrame->Name, sizeof( CHAR )*length, Name );

	D3DXMatrixIdentity( &pFrame->TransformationMatrix );
	D3DXMatrixIdentity( &pFrame->CombinedTransformationMatrix );

	*ppNewFrame = pFrame;

	return S_OK;
}

//----------------------------.
// ���b�V���R���e�i�[���쐬����.
//----------------------------.
HRESULT MY_HIERARCHY::CreateMeshContainer(
	LPCSTR Name, CONST D3DXMESHDATA* pMeshData,
	CONST D3DXMATERIAL* pMaterials, CONST D3DXEFFECTINSTANCE* pEffectInstances,
	DWORD NumMaterials, CONST DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo,
	LPD3DXMESHCONTAINER *ppMeshContainer )
{
	*ppMeshContainer = nullptr;
	DWORD dwBoneNum = 0;

	MYMESHCONTAINER *pMeshContainer = new MYMESHCONTAINER();
	//ZeroMemory(pMeshContainer, sizeof(MYMESHCONTAINER));//ZeroMemory��Ȃ�!.
	int length = static_cast<int>( strlen( Name ) ) + 1;
	pMeshContainer->Name = new CHAR[length]();
	strcpy_s( pMeshContainer->Name, sizeof( CHAR )*length, Name );

	memcpy( &pMeshContainer->MeshData, pMeshData, sizeof( pMeshContainer->MeshData ) );
	pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

	int NumPoly = pMeshContainer->MeshData.pMesh->GetNumFaces();
	// ���b�V���𕡐�����.
	//	�Ȃ����X�L�����b�V�����ƁA���̊֐��𔲂��������D3DX�����Ń��b�V���|�C���^�[�����������Ȃ��Ă��܂��̂�.
	LPDIRECT3DDEVICE9 pDevice = nullptr;
	pMeshContainer->MeshData.pMesh->GetDevice( &pDevice );
	LPD3DXMESH pMesh = nullptr;
	pMeshContainer->MeshData.pMesh->CloneMesh( 0, nullptr, pDevice, &pMesh );
	//SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
	pMeshContainer->MeshData.pMesh = pMesh;

	// ���b�V���̃}�e���A���ݒ�.
	pMeshContainer->NumMaterials = max( 1, NumMaterials );
	pMeshContainer->pMaterials = new D3DXMATERIAL[pMeshContainer->NumMaterials]();
	pMeshContainer->pAdjacency = new DWORD[pMeshContainer->MeshData.pMesh->GetNumFaces() * 3]();
	if ( ( pMeshContainer->pAdjacency == nullptr ) || ( pMeshContainer->pMaterials == nullptr ) )
	{
		return E_FAIL;
	}
	memcpy( pMeshContainer->pAdjacency, pAdjacency, sizeof( DWORD ) * NumPoly * 3 );

	if ( NumMaterials > 0 )
	{
		for( DWORD i = 0; i < NumMaterials; i++ )
		{
			memcpy( &pMeshContainer->pMaterials[i], &pMaterials[i], sizeof( D3DXMATERIAL ) );
			if ( pMaterials[i].pTextureFilename != nullptr )
			{
				int length = 512;
				pMeshContainer->pMaterials[i].pTextureFilename = new CHAR[length]();
				strcpy_s( pMeshContainer->pMaterials[i].pTextureFilename,
					sizeof( CHAR ) * length, pMaterials[i].pTextureFilename );
			}
		}
	}
	else
	{
		pMeshContainer->pMaterials[0].pTextureFilename = nullptr;
		memset( &pMeshContainer->pMaterials[0].MatD3D, 0, sizeof( D3DMATERIAL9 ) );
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
		pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Diffuse;
	}
	// ���Y���b�V�����X�L�����������Ă���ꍇ(�X�L�����b�V���ŗL�̏���).
	if ( pSkinInfo != nullptr )
	{
		pMeshContainer->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();
		dwBoneNum = pSkinInfo->GetNumBones();
		pMeshContainer->pBoneOffsetMatrices = new D3DXMATRIX[dwBoneNum]();

		for( DWORD i = 0; i < dwBoneNum; i++ )
		{
			memcpy( &pMeshContainer->pBoneOffsetMatrices[i],
				pMeshContainer->pSkinInfo->GetBoneOffsetMatrix( i ), sizeof( D3DMATRIX ) );
		}
	}
	// ���[�J���ɐ����������b�V���R���e�i�[���Ăяo�����ɃR�s�[����.
	*ppMeshContainer = pMeshContainer;

	return S_OK;
}

//----------------------------.
// �t���[����j������.
//----------------------------.
HRESULT MY_HIERARCHY::DestroyFrame( LPD3DXFRAME pFrameToFree )
{
	if ( pFrameToFree->Name ) {
		delete[] pFrameToFree->Name;
		pFrameToFree->Name = nullptr;
	}
	if ( pFrameToFree->pMeshContainer ) {
		DestroyMeshContainer( pFrameToFree->pMeshContainer );
	}
	if ( pFrameToFree->pFrameFirstChild ) {
		DestroyFrame( pFrameToFree->pFrameFirstChild );
	}
	if ( pFrameToFree->pFrameSibling ) {
		DestroyFrame( pFrameToFree->pFrameSibling );
	}
	delete pFrameToFree;

	return S_OK;
}

//----------------------------.
// ���b�V���R���e�i�[��j������.
//----------------------------.
HRESULT MY_HIERARCHY::DestroyMeshContainer( LPD3DXMESHCONTAINER pMeshContainerBase )
{
	DWORD dwMaterial;
	MYMESHCONTAINER *pMeshContainer = (MYMESHCONTAINER*)pMeshContainerBase;

	SAFE_DELETE_ARRAY( pMeshContainer->Name );
	SAFE_RELEASE( pMeshContainer->pSkinInfo );
	SAFE_DELETE_ARRAY( pMeshContainer->pAdjacency );
	SAFE_DELETE_ARRAY( pMeshContainer->pMaterials );

	SAFE_DELETE_ARRAY( pMeshContainer->ppBoneMatrix );

	for( DWORD i = 0; i < pMeshContainer->dwBoneNum; i++ ) {
		pMeshContainer->ppBoneMatrix[i] = nullptr;
	}

	if ( pMeshContainer->ppTextures != nullptr )
	{
		for( dwMaterial = 0; dwMaterial < pMeshContainer->NumMaterials; dwMaterial++ )
		{
			SAFE_RELEASE( pMeshContainer->ppTextures[dwMaterial] );
		}
	}
	SAFE_DELETE_ARRAY( pMeshContainer->ppTextures );

	SAFE_RELEASE( pMeshContainer->MeshData.pMesh );

	if ( pMeshContainer->pBoneBuffer != nullptr )
	{
		SAFE_RELEASE( pMeshContainer->pBoneBuffer );
		SAFE_DELETE_ARRAY( pMeshContainer->pBoneOffsetMatrices );
	}

	SAFE_DELETE( pMeshContainer );

	return S_OK;
}

/*******************************************************************************************************************************************
*
*	�ȍ~�A�p�[�T�[�N���X.
*
**/
D3DXPARSER::D3DXPARSER()
	: cHierarchy		()
	, m_pHierarchy		( nullptr )
	, m_pFrameRoot		( nullptr )
	, m_pAnimController	( nullptr )
	, m_pAnimSet		()
{
#if _DEBUG
	_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
#endif
}

D3DXPARSER::~D3DXPARSER()
{
}

//----------------------------.
// �{�[���s��̗̈�m��.
//----------------------------.
HRESULT D3DXPARSER::AllocateBoneMatrix( LPD3DXMESHCONTAINER pMeshContainerBase )
{
	MYFRAME *pFrame = nullptr;
	DWORD dwBoneNum = 0;

	MYMESHCONTAINER *pMeshContainer = (MYMESHCONTAINER*)pMeshContainerBase;
	if ( pMeshContainer->pSkinInfo == nullptr )
	{
		return S_OK;
	}
	dwBoneNum = pMeshContainer->pSkinInfo->GetNumBones();
	pMeshContainer->ppBoneMatrix = new D3DXMATRIX*[dwBoneNum]();

	for( DWORD i = 0; i < dwBoneNum; i++ )
	{
		// �܂���nullptr�ŏ�����.
		pMeshContainer->ppBoneMatrix[i] = nullptr;

		pFrame = (MYFRAME*) D3DXFrameFind( m_pFrameRoot, pMeshContainer->pSkinInfo->GetBoneName( i ) );
		if ( pFrame == nullptr )
		{
			return E_FAIL;
		}
		pMeshContainer->ppBoneMatrix[i] = &pFrame->CombinedTransformationMatrix;

	}
	return S_OK;
}

//----------------------------.
// �S�Ẵ{�[���s��̗̈���m��(�ċA�֐�).
//----------------------------.
HRESULT D3DXPARSER::AllocateAllBoneMatrices( LPD3DXFRAME pFrame )
{
	if ( pFrame->pMeshContainer != nullptr )
	{
		if ( FAILED( AllocateBoneMatrix( pFrame->pMeshContainer )))
		{
			return E_FAIL;
		}
	}
	if ( pFrame->pFrameSibling != nullptr )
	{
		if ( FAILED( AllocateAllBoneMatrices( pFrame->pFrameSibling )))
		{
			return E_FAIL;
		}
	}
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		if ( FAILED( AllocateAllBoneMatrices( pFrame->pFrameFirstChild )))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

//----------------------------.
// X�t�@�C�����烁�b�V����ǂݍ���.
//----------------------------.
HRESULT D3DXPARSER::LoadMeshFromX( LPDIRECT3DDEVICE9 pDevice9, LPCTSTR fileName )
{
	// X�t�@�C������A�j���[�V�������b�V����ǂݍ��ݍ쐬����
	m_pHierarchy = new MY_HIERARCHY();
	if ( FAILED(
		DirectX11::MutexD3DXLoadMeshHierarchyFromX(
			fileName, D3DXMESH_MANAGED, pDevice9, m_pHierarchy,
			nullptr, &m_pFrameRoot, &m_pAnimController )))
	{
		ErrorMessage( "�t�@�C���̓ǂݍ��݂Ɏ��s���܂���" );
		return E_FAIL;
	}
	// �{�[�����������肠��.
	AllocateAllBoneMatrices( m_pFrameRoot );


	/*
		m_pContainer = (MYMESHCONTAINER*)GetMeshContainer( m_pFrameRoot );

		if( m_pContainer == nullptr ){
			ErrorMessage( "���b�V���R���e�i��������Ȃ�����" );
			return E_FAIL;
		}
	*/


	DWORD iAnimMax = m_pAnimController->GetNumAnimationSets();
	// �A�j���[�V�����Z�b�g�𓾂�.
	for( DWORD i = 0; i < iAnimMax; i++ )
	{
		m_pAnimController->GetAnimationSet( i, &m_pAnimSet[i] );
	}

	return S_OK;
}

//----------------------------.
// �t���[�����̃��b�V�����Ƀ��[���h�ϊ��s����X�V����.
//----------------------------.
VOID D3DXPARSER::UpdateFrameMatrices( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix )
{
	MYFRAME *pFrame = (MYFRAME*)pFrameBase;

	if ( pParentMatrix != nullptr )
	{
		D3DXMatrixMultiply( &pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, pParentMatrix );
	}
	else
	{
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;
	}
	if ( pFrame->pFrameSibling != nullptr )
	{
		UpdateFrameMatrices( pFrame->pFrameSibling, pParentMatrix );
	}
	if ( pFrame->pFrameFirstChild != nullptr )
	{
		UpdateFrameMatrices( pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix );
	}
}


int D3DXPARSER::GetNumVertices( MYMESHCONTAINER* pContainer )
{
	return pContainer->MeshData.pMesh->GetNumVertices();
}


int D3DXPARSER::GetNumFaces( MYMESHCONTAINER* pContainer )
{
	return pContainer->MeshData.pMesh->GetNumFaces();
}


int D3DXPARSER::GetNumMaterials( MYMESHCONTAINER* pContainer )
{
	return pContainer->NumMaterials;
}


int D3DXPARSER::GetNumUVs( MYMESHCONTAINER* pContainer )
{
	return pContainer->MeshData.pMesh->GetNumVertices();
}

//----------------------------.
// �w�肳�ꂽ�{�[�����e�����y�ڂ����_����Ԃ�.
//----------------------------.
int D3DXPARSER::GetNumBoneVertices( MYMESHCONTAINER* pContainer, int iBoneIndex )
{
	return pContainer->pSkinInfo->GetNumBoneInfluences( iBoneIndex );
}

//----------------------------.
// �w�肳�ꂽ�{�[�����e�����y�ڂ����_�̃C���f�b�N�X��Ԃ� ��2�����́A�e�����󂯂钸�_�̃C���f�b�N�X�O���[�v���̃C���f�b�N�X�i�C���f�b�N�X���Ⴂ���j
//	�Ⴆ�΃{�[���ɉe���������钸�_���S���Ƃ��āA���̃{�[���ɉe����������S�̒��_�̂���2�Ԗڂ̃C���f�b�N�X��m�肽���ꍇ�́AiIndexInGroup��1���w�肷��i0�X�^�[�g�Ȃ̂Łj
//----------------------------.
DWORD D3DXPARSER::GetBoneVerticesIndices(MYMESHCONTAINER* pContainer, int iBoneIndex, int iIndexInGroup)
{
	int Num = pContainer->pSkinInfo->GetNumBoneInfluences(iBoneIndex);
	DWORD* pVerts = new DWORD[Num]();
	float* pWights = new float[Num]();

	DWORD dwRslt = 0;

	if ( FAILED(
		pContainer->pSkinInfo->GetBoneInfluence( iBoneIndex, pVerts, pWights )))
	{
		MessageBox( nullptr, _T( "�{�[���e�����󂯂钸�_������Ȃ�" ), _T( "error" ), MB_OK );
	}
	else {
		dwRslt = pVerts[iIndexInGroup];
	}

	delete[] pVerts;
	delete[] pWights;

	return dwRslt;
}

//----------------------------.
// �w�肳�ꂽ�{�[�����e�����y�ڂ����_�̃{�[���E�F�C�g��Ԃ� ��2�����́A�e�����󂯂钸�_�̃C���f�b�N�X�O���[�v���̃C���f�b�N�X�i�C���f�b�N�X���Ⴂ���j
//	�Ⴆ�΃{�[���ɉe���������钸�_���S���Ƃ��āA���̃{�[���ɉe����������S�̒��_�̂���2�Ԗڂ̒��_�̃{�[���E�F�C�g��m�肽���ꍇ�́AiIndexInGroup��1���w�肷��i0�X�^�[�g�Ȃ̂Łj
//----------------------------.
double D3DXPARSER::GetBoneVerticesWeights( MYMESHCONTAINER* pContainer, int iBoneIndex, int iIndexInGroup )
{
	int Num = pContainer->pSkinInfo->GetNumBoneInfluences( iBoneIndex );
	DWORD* pVerts = new DWORD[Num]();
	float* pWights = new float[Num]();

	double dRslt = 0.0f;

	if (FAILED(
		pContainer->pSkinInfo->GetBoneInfluence( iBoneIndex, pVerts, pWights )))
	{
		MessageBox( nullptr, _T( "�{�[���e�����󂯂钸�_������Ȃ�" ), _T( "error" ), MB_OK );
	}
	else
	{
		dRslt = (double)pWights[iIndexInGroup];
	}
	delete[] pVerts;
	delete[] pWights;

	return dRslt;
}

//----------------------------.
// ���W.
//----------------------------.
D3DXVECTOR3 D3DXPARSER::GetVertexCoord( MYMESHCONTAINER* pContainer, DWORD Index )
{
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	pContainer->MeshData.pMesh->GetVertexBuffer( &pVB );
	DWORD Stride = pContainer->MeshData.pMesh->GetNumBytesPerVertex();
	BYTE *pVertices = nullptr;
	D3DXVECTOR3* pCoord = nullptr;

	if ( SUCCEEDED( pVB->Lock( 0, 0, (VOID**)&pVertices, 0 )))
	{
		pVertices += Index * Stride;
		pCoord = (D3DXVECTOR3*)pVertices;
		pVB->Unlock();
	}
	SAFE_RELEASE( pVB );

	return *pCoord;
}

//----------------------------.
// �@��.
//----------------------------.
D3DXVECTOR3 D3DXPARSER::GetNormal( MYMESHCONTAINER* pContainer, DWORD Index )
{
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	pContainer->MeshData.pMesh->GetVertexBuffer( &pVB );
	DWORD Stride = pContainer->MeshData.pMesh->GetNumBytesPerVertex();
	BYTE *pVertices = nullptr;
	D3DXVECTOR3* pNormal = nullptr;

	if ( SUCCEEDED( pVB->Lock( 0, 0, (VOID**)&pVertices, 0 )))
	{
		pVertices += Index * Stride;
		pVertices += sizeof( D3DXVECTOR3 );	//���W���i�߂�.
		pNormal = (D3DXVECTOR3*)pVertices;
		pVB->Unlock();
	}
	SAFE_RELEASE( pVB );

	return *pNormal;
}

//----------------------------.
// �e�N�X�`���[���W.
//----------------------------.
D3DXVECTOR2 D3DXPARSER::GetUV( MYMESHCONTAINER* pContainer, DWORD Index )
{
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	pContainer->MeshData.pMesh->GetVertexBuffer( &pVB );
	DWORD Stride = pContainer->MeshData.pMesh->GetNumBytesPerVertex();
	BYTE *pVertices = nullptr;
	D3DXVECTOR2* pUV = nullptr;

	if ( SUCCEEDED( pVB->Lock( 0, 0, (VOID**)&pVertices, 0 )))
	{
		pVertices += Index * Stride;
		pVertices += sizeof( D3DXVECTOR3 );	//���W���i�߂�.
		pVertices += sizeof( D3DXVECTOR3 );	//�@�����i�߂�.
		pUV = (D3DXVECTOR2*)pVertices;
		pVB->Unlock();
	}
	SAFE_RELEASE(pVB);

	return *pUV;
}

//----------------------------.
// �C���f�b�N�X�o�b�t�@���̎w�肵���ʒu�i�C���f�b�N�X�C���f�b�N�X�j�ɂ��钸�_�C���f�b�N�X��Ԃ�.
//----------------------------.
int D3DXPARSER::GetIndex( MYMESHCONTAINER* pContainer, DWORD Index )
{
	WORD VertIndex = 0;
	WORD* pIndex = nullptr;
	pContainer->MeshData.pMesh->LockIndexBuffer( D3DLOCK_READONLY, (VOID**) &pIndex );

	VertIndex = pIndex[Index];

	pContainer->MeshData.pMesh->UnlockIndexBuffer();

	return VertIndex;
}


D3DXVECTOR4 D3DXPARSER::GetAmbient( MYMESHCONTAINER* pContainer, int iIndex )
{
	D3DXCOLOR color;
	color = pContainer->pMaterials[iIndex].MatD3D.Ambient;
	return D3DXVECTOR4( color.a, color.r, color.g, color.b );
}


D3DXVECTOR4 D3DXPARSER::GetDiffuse( MYMESHCONTAINER* pContainer, int iIndex )
{
	D3DXCOLOR color;
	color = pContainer->pMaterials[iIndex].MatD3D.Diffuse;
	return D3DXVECTOR4( color.a, color.r, color.g, color.b );
}


D3DXVECTOR4 D3DXPARSER::GetSpecular( MYMESHCONTAINER* pContainer, int iIndex )
{
	D3DXCOLOR color;
	color = pContainer->pMaterials[iIndex].MatD3D.Specular;
	return D3DXVECTOR4( color.a, color.r, color.g, color.b );
}


LPSTR D3DXPARSER::GetTexturePath( MYMESHCONTAINER* pContainer, int index )
{
	return pContainer->pMaterials[index].pTextureFilename;
}



float D3DXPARSER::GetSpecularPower( MYMESHCONTAINER* pContainer, int iIndex )
{
	return pContainer->pMaterials[iIndex].MatD3D.Power;
}

//----------------------------.
// ���̃|���S�����A�ǂ̃}�e���A���ł��邩��Ԃ�.
//----------------------------.
int D3DXPARSER::GeFaceMaterialIndex( MYMESHCONTAINER* pContainer, int iFaceIndex )
{
	int MaterialIndex = 0;
	DWORD* pBuf = nullptr;
	if ( SUCCEEDED(
		pContainer->MeshData.pMesh->LockAttributeBuffer( D3DLOCK_READONLY, &pBuf )))
	{
		MaterialIndex = pBuf[iFaceIndex];
	}
	else
	{
		MessageBox( nullptr, _T( "�����o�b�t�@�̃��b�N���s" ), _T( "error" ), MB_OK );
	}
	return MaterialIndex;
}

//----------------------------.
// iFaceIndex�Ԗڂ̃|���S�����`������3���_�̒��ŁAiIndexInFace�Ԗ�[0,2]�̒��_�̃C���f�b�N�X��Ԃ�.
//----------------------------.
int D3DXPARSER::GetFaceVertexIndex( MYMESHCONTAINER* pContainer, int iFaceIndex, int iIndexInFace )
{
	// �C���f�b�N�X�o�b�t�@�[�𒲂ׂ�Ε�����.
	WORD VertIndex = 0;
	WORD* pIndex = nullptr;
	pContainer->MeshData.pMesh->LockIndexBuffer( D3DLOCK_READONLY, (VOID**) &pIndex );

	VertIndex = pIndex[iFaceIndex * 3 + iIndexInFace];

	pContainer->MeshData.pMesh->UnlockIndexBuffer();

	return VertIndex;
}


D3DXMATRIX D3DXPARSER::GetBindPose( MYMESHCONTAINER* pContainer, int iBoneIndex )
{
	return *pContainer->pSkinInfo->GetBoneOffsetMatrix( iBoneIndex );
}

//----------------------------.
// ���̊֐�����Ԃ��Ƃ�Update����Ɩ��ʂȂ̂ŁARender�֐����s����Update������̂Ƃ��Ă��̂܂ܓn��.
//	�e�N���X�����̃N���X��UpdateMatrices�����s���Ȃ��ꍇ�́AUpdateMatrices���Ȃ��ƍs�񂪍ŐV�ɂȂ�Ȃ��̂ŗ���.
//----------------------------.
D3DXMATRIX D3DXPARSER::GetNewPose( MYMESHCONTAINER* pContainer, int iBoneIndex )
{
	MYMESHCONTAINER* pMyContaiber = (MYMESHCONTAINER*)pContainer;
	return *pMyContaiber->ppBoneMatrix[iBoneIndex];
}



LPCSTR D3DXPARSER::GetBoneName( MYMESHCONTAINER* pContainer, int iBoneIndex )
{
	return pContainer->pSkinInfo->GetBoneName( iBoneIndex );
}



int D3DXPARSER::GetNumBones( MYMESHCONTAINER* pContainer )
{
	return pContainer->pSkinInfo->GetNumBones();
}



//----------------------------.
// ���b�V���R���e�i���Ăяo��(�ċA����)
//----------------------------.
LPD3DXMESHCONTAINER D3DXPARSER::GetMeshContainer( LPD3DXFRAME pFrame )
{
	LPD3DXMESHCONTAINER pCon = nullptr;

	// ���b�V���R���e�i����ΕԂ�.
	if ( pFrame->pMeshContainer ) {
		return pFrame->pMeshContainer;
	}

	// ����������A�q�̃t���[�����`�F�b�N.
	// ���������q�t���[������H
	if ( pFrame->pFrameFirstChild ) {
		// ����΃`�F�b�N����.
		pCon = GetMeshContainer( pFrame->pFrameFirstChild );
	}

	// �q�̃t���[���ŉ��w�܂Ń`�F�b�N������������Ȃ�����.
	if ( pCon == nullptr ) {
		// �Z��̃t���[�����T��.
		//	���������Z��t���[������H
		if ( pFrame->pFrameSibling ) {
			// �����΃`�F�b�N����.
			pCon = GetMeshContainer( pFrame->pFrameSibling );
		}
	}

	// ������Ȃ��ꍇ��nullptr������.
	return pCon;
}

//----------------------------.
// �A�j���[�V�����Z�b�g�̐؂�ւ�.
//----------------------------.
void D3DXPARSER::ChangeAnimSet( int index, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// �A�j���[�V�����g���b�N�\����.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ���ȉ�3�́A�قڌŒ��OK.
	TrackDesc.Weight	= 1.0f;	// �d��.
	TrackDesc.Speed		= 1.0f;	// ����.
	TrackDesc.Enable	= TRUE;	// �L��.

	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = 0.0;		// �t���[���ʒu(�J�n�ʒu���w��ł���)

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// �w��(index�j�̃A�j���[�V�����g���b�N�ɕύX.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( 0, TRUE );
	pTmpAC->SetTrackEnable( 1, FALSE );
}

//----------------------------.
// �A�j���[�V�����Z�b�g�̐؂�ւ�(�J�n�t���[���w��\��)
//	��Q�����ɊJ�n�������t���[�����w�肷��.
//	���S��~�������ꍇ�́A�O��ŃA�j���[�V�������x��0.0f�Ɏw�肵�Ă��K�v������.
//----------------------------.
void D3DXPARSER::ChangeAnimSet_StartPos(
	int index, double dStartFramePos, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// �A�j���[�V�����g���b�N�\����.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ���ȉ�3�́A�قڌŒ��OK.
	TrackDesc.Weight	= 1.0f;	// �d��.
	TrackDesc.Speed		= 1.0f;	// ����.
	TrackDesc.Enable	= TRUE;	// �L��.

	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = dStartFramePos;

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// �w��(index�j�̃A�j���[�V�����g���b�N�ɕύX.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( index, TRUE );
}

//----------------------------.
// �A�j���[�V�������u�����h���Đ؂�ւ�.
//----------------------------.
void D3DXPARSER::ChangeAnimBlend( int index, int oldIndex, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// �A�j���[�V�����g���b�N�\����.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ���ȉ�3�́A�قڌŒ��OK.
	TrackDesc.Weight	= 1.0f;	// �d��.
	TrackDesc.Speed		= 1.0f;	// ����.
	TrackDesc.Enable	= TRUE;	// �L��.
	
	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = 0.0;		// �t���[���ʒu(�J�n�ʒu���w��ł���)

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// ���݂̃A�j���[�V�����g���b�N���擾.
	D3DXTRACK_DESC nowTrackDesc;
	pTmpAC->GetTrackDesc( 0, &nowTrackDesc );

	// �g���b�N1 �Ɍ��݂̃A�j���[�V������ݒ�.
	pTmpAC->SetTrackDesc( 1, &nowTrackDesc );
	pTmpAC->SetTrackAnimationSet( 1, m_pAnimSet[oldIndex] );
	pTmpAC->SetTrackEnable( 1, true );

	// �g���b�N0 �ɐV�����A�j���[�V������ݒ�.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( 0, true );
}

//----------------------------.
// �A�j���[�V������~���Ԃ��擾.
//----------------------------.
double D3DXPARSER::GetAnimPeriod( int index )
{
	if ( index < 0 || MAX_ANIM_SET <= index ) {
		return 0.0;
	}
	return m_pAnimSet[index]->GetPeriod();
}

//----------------------------.
// �ő�A�j���[�V���������擾.
//----------------------------.
int D3DXPARSER::GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC )
{
	if (pAC) {
		return pAC->GetNumAnimationSets();
	}
	return m_pAnimController->GetNumAnimationSets();
}

//----------------------------.
// �w�肵���{�[�����(�s��)���擾����֐�.
//----------------------------.
bool D3DXPARSER::GetMatrixFromBone( LPCSTR sBoneName, D3DXMATRIX* pOutMat )
{
	LPD3DXFRAME pFrame;
	pFrame = (MYFRAME*)D3DXFrameFind( m_pFrameRoot, sBoneName );

	if ( pFrame == nullptr ) {
		return false;
	}

	MYFRAME* pMyFrame = ( MYFRAME* )pFrame;
	// �ʒu�����擾.
	*pOutMat = pMyFrame->CombinedTransformationMatrix;

	return true;
}

//----------------------------.
// �w�肵���{�[�����(���W)���擾����֐�.
//----------------------------.
bool D3DXPARSER::GetPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos )
{
	D3DXMATRIX mBone;
	if ( GetMatrixFromBone( sBoneName, &mBone ) == false ) {
		return false;
	}
	pOutPos->x = mBone._41;
	pOutPos->y = mBone._42;
	pOutPos->z = mBone._43;

	return true;
}


//----------------------------.
// �ꊇ�������.
//----------------------------.
HRESULT D3DXPARSER::Release()
{
	// �쐬�������̂��Ō�ɍ�������̂��珇������Ă���.
	// ��{�I�ɂ� new �������̂� delete ���Ă���.

	// �A�j���[�V�����Z�b�g�̉��.
	DWORD iAnimMax = m_pAnimController->GetNumAnimationSets();
	for( DWORD i = 0; i < iAnimMax; i++ )
	{
		SAFE_RELEASE( m_pAnimSet[i] );
	}

	// �A�j���[�V�����R���g���[���폜.
	SAFE_RELEASE( m_pAnimController );

	// �t���[���ƃ��b�V���R���e�i�̍폜.
	m_pHierarchy->DestroyFrame( m_pFrameRoot );

	// �����������H
	if ( m_pFrameRoot != nullptr ) {
		m_pFrameRoot = nullptr;
	}

	// Hierarchy�폜.
	if ( m_pHierarchy != nullptr ) {
		delete m_pHierarchy;
		m_pHierarchy = nullptr;
	}

	return S_OK;
}
