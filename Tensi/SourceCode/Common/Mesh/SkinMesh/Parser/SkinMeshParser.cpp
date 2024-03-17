/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode対応.
**/
#include "SkinMeshParser.h"
#include <stdlib.h>
#include <string.h>
#include <crtdbg.h>
#include "..\..\..\DirectX\DirectX11.h"

//----------------------------.
// フレームを作成する.
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
// メッシュコンテナーを作成する.
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
	//ZeroMemory(pMeshContainer, sizeof(MYMESHCONTAINER));//ZeroMemory危ない!.
	int length = static_cast<int>( strlen( Name ) ) + 1;
	pMeshContainer->Name = new CHAR[length]();
	strcpy_s( pMeshContainer->Name, sizeof( CHAR )*length, Name );

	memcpy( &pMeshContainer->MeshData, pMeshData, sizeof( pMeshContainer->MeshData ) );
	pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

	int NumPoly = pMeshContainer->MeshData.pMesh->GetNumFaces();
	// メッシュを複製する.
	//	なぜかスキンメッシュだと、この関数を抜けた直後にD3DX内部でメッシュポインターがおかしくなってしまうので.
	LPDIRECT3DDEVICE9 pDevice = nullptr;
	pMeshContainer->MeshData.pMesh->GetDevice( &pDevice );
	LPD3DXMESH pMesh = nullptr;
	pMeshContainer->MeshData.pMesh->CloneMesh( 0, nullptr, pDevice, &pMesh );
	//SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
	pMeshContainer->MeshData.pMesh = pMesh;

	// メッシュのマテリアル設定.
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
	// 当該メッシュがスキン情報を持っている場合(スキンメッシュ固有の処理).
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
	// ローカルに生成したメッシュコンテナーを呼び出し側にコピーする.
	*ppMeshContainer = pMeshContainer;

	return S_OK;
}

//----------------------------.
// フレームを破棄する.
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
// メッシュコンテナーを破棄する.
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
*	以降、パーサークラス.
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
// ボーン行列の領域確保.
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
		// まずはnullptrで初期化.
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
// 全てのボーン行列の領域を確保(再帰関数).
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
// Xファイルからメッシュを読み込む.
//----------------------------.
HRESULT D3DXPARSER::LoadMeshFromX( LPDIRECT3DDEVICE9 pDevice9, LPCTSTR fileName )
{
	// Xファイルからアニメーションメッシュを読み込み作成する
	m_pHierarchy = new MY_HIERARCHY();
	if ( FAILED(
		DirectX11::MutexD3DXLoadMeshHierarchyFromX(
			fileName, D3DXMESH_MANAGED, pDevice9, m_pHierarchy,
			nullptr, &m_pFrameRoot, &m_pAnimController )))
	{
		ErrorMessage( "ファイルの読み込みに失敗しました" );
		return E_FAIL;
	}
	// ボーンメモリ割りあて.
	AllocateAllBoneMatrices( m_pFrameRoot );


	/*
		m_pContainer = (MYMESHCONTAINER*)GetMeshContainer( m_pFrameRoot );

		if( m_pContainer == nullptr ){
			ErrorMessage( "メッシュコンテナが見つからなかった" );
			return E_FAIL;
		}
	*/


	DWORD iAnimMax = m_pAnimController->GetNumAnimationSets();
	// アニメーションセットを得る.
	for( DWORD i = 0; i < iAnimMax; i++ )
	{
		m_pAnimController->GetAnimationSet( i, &m_pAnimSet[i] );
	}

	return S_OK;
}

//----------------------------.
// フレーム内のメッシュ毎にワールド変換行列を更新する.
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
// 指定されたボーンが影響を及ぼす頂点数を返す.
//----------------------------.
int D3DXPARSER::GetNumBoneVertices( MYMESHCONTAINER* pContainer, int iBoneIndex )
{
	return pContainer->pSkinInfo->GetNumBoneInfluences( iBoneIndex );
}

//----------------------------.
// 指定されたボーンが影響を及ぼす頂点のインデックスを返す 第2引数は、影響を受ける頂点のインデックスグループ内のインデックス（インデックスが若い順）
//	例えばボーンに影響をうける頂点が４つだとして、そのボーンに影響をうける４つの頂点のうち2番目のインデックスを知りたい場合は、iIndexInGroupに1を指定する（0スタートなので）
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
		MessageBox( nullptr, _T( "ボーン影響を受ける頂点見つからない" ), _T( "error" ), MB_OK );
	}
	else {
		dwRslt = pVerts[iIndexInGroup];
	}

	delete[] pVerts;
	delete[] pWights;

	return dwRslt;
}

//----------------------------.
// 指定されたボーンが影響を及ぼす頂点のボーンウェイトを返す 第2引数は、影響を受ける頂点のインデックスグループ内のインデックス（インデックスが若い順）
//	例えばボーンに影響をうける頂点が４つだとして、そのボーンに影響をうける４つの頂点のうち2番目の頂点のボーンウェイトを知りたい場合は、iIndexInGroupに1を指定する（0スタートなので）
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
		MessageBox( nullptr, _T( "ボーン影響を受ける頂点見つからない" ), _T( "error" ), MB_OK );
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
// 座標.
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
// 法線.
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
		pVertices += sizeof( D3DXVECTOR3 );	//座標分進める.
		pNormal = (D3DXVECTOR3*)pVertices;
		pVB->Unlock();
	}
	SAFE_RELEASE( pVB );

	return *pNormal;
}

//----------------------------.
// テクスチャー座標.
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
		pVertices += sizeof( D3DXVECTOR3 );	//座標分進める.
		pVertices += sizeof( D3DXVECTOR3 );	//法線分進める.
		pUV = (D3DXVECTOR2*)pVertices;
		pVB->Unlock();
	}
	SAFE_RELEASE(pVB);

	return *pUV;
}

//----------------------------.
// インデックスバッファ内の指定した位置（インデックスインデックス）にある頂点インデックスを返す.
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
// そのポリゴンが、どのマテリアルであるかを返す.
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
		MessageBox( nullptr, _T( "属性バッファのロック失敗" ), _T( "error" ), MB_OK );
	}
	return MaterialIndex;
}

//----------------------------.
// iFaceIndex番目のポリゴンを形成する3頂点の中で、iIndexInFace番目[0,2]の頂点のインデックスを返す.
//----------------------------.
int D3DXPARSER::GetFaceVertexIndex( MYMESHCONTAINER* pContainer, int iFaceIndex, int iIndexInFace )
{
	// インデックスバッファーを調べれば分かる.
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
// この関数をよぶごとにUpdateすると無駄なので、Render関数実行時にUpdateするものとしてそのまま渡す.
//	親クラスがこのクラスのUpdateMatricesを実行しない場合は、UpdateMatricesしないと行列が最新にならないので留意.
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
// メッシュコンテナを呼び出す(再帰処理)
//----------------------------.
LPD3DXMESHCONTAINER D3DXPARSER::GetMeshContainer( LPD3DXFRAME pFrame )
{
	LPD3DXMESHCONTAINER pCon = nullptr;

	// メッシュコンテナあれば返す.
	if ( pFrame->pMeshContainer ) {
		return pFrame->pMeshContainer;
	}

	// 無かったら、子のフレームをチェック.
	// そもそも子フレームある？
	if ( pFrame->pFrameFirstChild ) {
		// あればチェックする.
		pCon = GetMeshContainer( pFrame->pFrameFirstChild );
	}

	// 子のフレーム最下層までチェックしたが見つからなかった.
	if ( pCon == nullptr ) {
		// 兄弟のフレームも探す.
		//	そもそも兄弟フレームある？
		if ( pFrame->pFrameSibling ) {
			// あるればチェックする.
			pCon = GetMeshContainer( pFrame->pFrameSibling );
		}
	}

	// 見つからない場合はnullptrが入る.
	return pCon;
}

//----------------------------.
// アニメーションセットの切り替え.
//----------------------------.
void D3DXPARSER::ChangeAnimSet( int index, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// アニメーショントラック構造体.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ※以下3つは、ほぼ固定でOK.
	TrackDesc.Weight	= 1.0f;	// 重み.
	TrackDesc.Speed		= 1.0f;	// 速さ.
	TrackDesc.Enable	= TRUE;	// 有効.

	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = 0.0;		// フレーム位置(開始位置を指定できる)

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// 指定(index）のアニメーショントラックに変更.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( 0, TRUE );
	pTmpAC->SetTrackEnable( 1, FALSE );
}

//----------------------------.
// アニメーションセットの切り替え(開始フレーム指定可能版)
//	第２引数に開始したいフレームを指定する.
//	完全停止したい場合は、前後でアニメーション速度も0.0fに指定してやる必要がある.
//----------------------------.
void D3DXPARSER::ChangeAnimSet_StartPos(
	int index, double dStartFramePos, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// アニメーショントラック構造体.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ※以下3つは、ほぼ固定でOK.
	TrackDesc.Weight	= 1.0f;	// 重み.
	TrackDesc.Speed		= 1.0f;	// 速さ.
	TrackDesc.Enable	= TRUE;	// 有効.

	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = dStartFramePos;

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// 指定(index）のアニメーショントラックに変更.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( index, TRUE );
}

//----------------------------.
// アニメーションをブレンドして切り替え.
//----------------------------.
void D3DXPARSER::ChangeAnimBlend( int index, int oldIndex, LPD3DXANIMATIONCONTROLLER pAC )
{
	D3DXTRACK_DESC TrackDesc;		// アニメーショントラック構造体.
	ZeroMemory( &TrackDesc, sizeof( TrackDesc ) );

	// ※以下3つは、ほぼ固定でOK.
	TrackDesc.Weight	= 1.0f;	// 重み.
	TrackDesc.Speed		= 1.0f;	// 速さ.
	TrackDesc.Enable	= TRUE;	// 有効.
	
	TrackDesc.Priority = D3DXPRIORITY_LOW;

	TrackDesc.Position = 0.0;		// フレーム位置(開始位置を指定できる)

	LPD3DXANIMATIONCONTROLLER pTmpAC;
	if ( pAC ) {
		pTmpAC = pAC;
	}
	else {
		pTmpAC = m_pAnimController;
	}

	// 現在のアニメーショントラックを取得.
	D3DXTRACK_DESC nowTrackDesc;
	pTmpAC->GetTrackDesc( 0, &nowTrackDesc );

	// トラック1 に現在のアニメーションを設定.
	pTmpAC->SetTrackDesc( 1, &nowTrackDesc );
	pTmpAC->SetTrackAnimationSet( 1, m_pAnimSet[oldIndex] );
	pTmpAC->SetTrackEnable( 1, true );

	// トラック0 に新しいアニメーションを設定.
	pTmpAC->SetTrackDesc( 0, &TrackDesc );
	pTmpAC->SetTrackAnimationSet( 0, m_pAnimSet[index] );
	pTmpAC->SetTrackEnable( 0, true );
}

//----------------------------.
// アニメーション停止時間を取得.
//----------------------------.
double D3DXPARSER::GetAnimPeriod( int index )
{
	if ( index < 0 || MAX_ANIM_SET <= index ) {
		return 0.0;
	}
	return m_pAnimSet[index]->GetPeriod();
}

//----------------------------.
// 最大アニメーション数を取得.
//----------------------------.
int D3DXPARSER::GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC )
{
	if (pAC) {
		return pAC->GetNumAnimationSets();
	}
	return m_pAnimController->GetNumAnimationSets();
}

//----------------------------.
// 指定したボーン情報(行列)を取得する関数.
//----------------------------.
bool D3DXPARSER::GetMatrixFromBone( LPCSTR sBoneName, D3DXMATRIX* pOutMat )
{
	LPD3DXFRAME pFrame;
	pFrame = (MYFRAME*)D3DXFrameFind( m_pFrameRoot, sBoneName );

	if ( pFrame == nullptr ) {
		return false;
	}

	MYFRAME* pMyFrame = ( MYFRAME* )pFrame;
	// 位置情報を取得.
	*pOutMat = pMyFrame->CombinedTransformationMatrix;

	return true;
}

//----------------------------.
// 指定したボーン情報(座標)を取得する関数.
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
// 一括解放処理.
//----------------------------.
HRESULT D3DXPARSER::Release()
{
	// 作成したものを最後に作ったものから順解放していく.
	// 基本的には new したものを delete していく.

	// アニメーションセットの解放.
	DWORD iAnimMax = m_pAnimController->GetNumAnimationSets();
	for( DWORD i = 0; i < iAnimMax; i++ )
	{
		SAFE_RELEASE( m_pAnimSet[i] );
	}

	// アニメーションコントローラ削除.
	SAFE_RELEASE( m_pAnimController );

	// フレームとメッシュコンテナの削除.
	m_pHierarchy->DestroyFrame( m_pFrameRoot );

	// 解放処理いる？
	if ( m_pFrameRoot != nullptr ) {
		m_pFrameRoot = nullptr;
	}

	// Hierarchy削除.
	if ( m_pHierarchy != nullptr ) {
		delete m_pHierarchy;
		m_pHierarchy = nullptr;
	}

	return S_OK;
}
