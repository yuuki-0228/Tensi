/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode�Ή�.
**/
#pragma once
#include "..\..\..\..\Global.h"

//�x���ɂ��ẴR�[�h���͂𖳌��ɂ���B4005�F�Ē�`.
#pragma warning( disable : 4005 )

#include <d3dx9.h>
#include <d3d11.h>
#include <d3dx10.h>
#include <d3dx11.h>

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"d3dx9.lib")



// �}�N��.
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=nullptr; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=nullptr; } }


// �I���W�i���@�}�e���A���\����.
struct MY_SKINMATERIAL
{
	TCHAR									Name[110];
	D3DXVECTOR4								Ka;					// �A���r�G���g.
	D3DXVECTOR4								Kd;					// �f�B�t���[�Y.
	D3DXVECTOR4								Ks;					// �X�y�L�����[.
	TCHAR									TextureName[512];	// �e�N�X�`���[�t�@�C����.
	std::vector<ID3D11ShaderResourceView*>	pTexture;			// �e�N�X�`��.
	ID3D11ShaderResourceView*				pLightTexture;		// ���C�g�e�N�X�`��.
	DWORD									dwNumFace;			// ���̃}�e���A���ł���|���S����.
	D3DXVECTOR4								UV;					// UV(x,y�̂ݎg�p����).

	MY_SKINMATERIAL()
		: Name			()
		, Ka			()
		, Kd			()
		, Ks			()
		, TextureName	()
		, pTexture		( 1 )
		, pLightTexture	( nullptr )
		, dwNumFace		()
		, UV			( INIT_FLOAT4 )
	{
	}
	~MY_SKINMATERIAL()
	{
		const int Size = static_cast<int>( pTexture.size() );
		for ( int i = Size - 1; i >= 0; --i ) {
			if ( pTexture[i] != nullptr )
			{
				pTexture[i]->Release();
				pTexture[i] = nullptr;
			}
		}
		if ( pLightTexture != nullptr )
		{
			pLightTexture->Release();
			pLightTexture = nullptr;
		}
	}
};

// �{�[���\����.
struct BONE
{
	D3DXMATRIX mBindPose;	// �����|�[�Y�i�����ƕς��Ȃ��j.
	D3DXMATRIX mNewPose;	// ���݂̃|�[�Y�i���̓s�x�ς��j.
	DWORD dwNumChild;		// �q�̐�.
	int iChildIndex[50];	// �����̎q�́g�C���f�b�N�X�h50�܂�.
	char Name[256];

	BONE()
		: mBindPose()
		, mNewPose()
		, dwNumChild()
		, iChildIndex()
		, Name()
	{
		D3DXMatrixIdentity( &mBindPose );
		D3DXMatrixIdentity( &mNewPose );
	}
};

// �p�[�c���b�V���\����.
struct SKIN_PARTS_MESH
{
	DWORD	dwNumVert;
	DWORD	dwNumFace;
	DWORD	dwNumUV;
	DWORD				dwNumMaterial;
	MY_SKINMATERIAL*	pMaterial;
	char				TextureFileName[8][256];	// �e�N�X�`���[�t�@�C����(8���܂�).
	bool				bTex;

	ID3D11Buffer*	pVertexBuffer;
	ID3D11Buffer**	ppIndexBuffer;

	// �{�[��.
	int		iNumBone;
	BONE*	pBoneArray;

	bool	bEnableBones;	// �{�[���̗L���t���O.

	SKIN_PARTS_MESH()
		: dwNumVert()
		, dwNumFace()
		, dwNumUV()
		, dwNumMaterial()
		, pMaterial( nullptr )
		, TextureFileName()
		, bTex()
		, pVertexBuffer( nullptr )
		, ppIndexBuffer( nullptr )
		, iNumBone()
		, pBoneArray( nullptr )
		, bEnableBones()
	{}
};

// �h���t���[���\����.
//	���ꂼ��̃��b�V���p�̍ŏI���[���h�s���ǉ�����.
struct MYFRAME : public D3DXFRAME
{
	D3DXMATRIX CombinedTransformationMatrix;
	SKIN_PARTS_MESH* pPartsMesh;
	MYFRAME()
		: D3DXFRAME()
		, CombinedTransformationMatrix()
		, pPartsMesh( nullptr )
	{
	}
};
// �h�����b�V���R���e�i�[�\����.
//	�R���e�i�[���e�N�X�`���𕡐����Ă�悤�Ƀ|�C���^�[�̃|�C���^�[��ǉ�����
struct MYMESHCONTAINER : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9*  ppTextures;
	DWORD dwWeight;					// �d�݂̌��i�d�݂Ƃ͒��_�ւ̉e���B�j.
	DWORD dwBoneNum;				// �{�[���̐�.
	LPD3DXBUFFER pBoneBuffer;		// �{�[���E�e�[�u��.
	D3DXMATRIX** ppBoneMatrix;		// �S�Ẵ{�[���̃��[���h�s��̐擪�|�C���^.
	D3DXMATRIX*	pBoneOffsetMatrices;// �t���[���Ƃ��Ẵ{�[���̃��[���h�s��̃|�C���^.
	MYMESHCONTAINER()
		: D3DXMESHCONTAINER()
		, ppTextures( nullptr )
		, dwWeight()
		, dwBoneNum()
		, pBoneBuffer( nullptr )
		, ppBoneMatrix( nullptr )
		, pBoneOffsetMatrices( nullptr )
	{}
};
// X�t�@�C�����̃A�j���[�V�����K�w��ǂ݉����Ă����N���X��h��������.
//	ID3DXAllocateHierarchy�͔h�����邱�Ƒz�肵�Đ݌v����Ă���.
class MY_HIERARCHY : public ID3DXAllocateHierarchy
{
public:
	MY_HIERARCHY()
		: ID3DXAllocateHierarchy() {}
	STDMETHOD( CreateFrame )( THIS_ LPCSTR, LPD3DXFRAME * );
	STDMETHOD( CreateMeshContainer )( THIS_ LPCSTR, CONST D3DXMESHDATA*, CONST D3DXMATERIAL*,
		CONST D3DXEFFECTINSTANCE*, DWORD, CONST DWORD *, LPD3DXSKININFO, LPD3DXMESHCONTAINER * );
	STDMETHOD( DestroyFrame )( THIS_ LPD3DXFRAME );
	STDMETHOD( DestroyMeshContainer )( THIS_ LPD3DXMESHCONTAINER );
};

//==================================================================================================
//
//	�p�[�T�[�N���X.
//
//==================================================================================================
class D3DXPARSER final
{
public:
	// �ő�{�[����.
	static const int MAX_BONES = 255;
	// �ő�A�j���[�V�����Z�b�g��.
	static const int MAX_ANIM_SET = 100;


	MY_HIERARCHY cHierarchy;
	MY_HIERARCHY* m_pHierarchy;
	LPD3DXFRAME m_pFrameRoot;

	LPD3DXANIMATIONCONTROLLER m_pAnimController;	// �f�t�H���g�ň��.
	LPD3DXANIMATIONSET m_pAnimSet[MAX_ANIM_SET];

	D3DXPARSER();
	~D3DXPARSER();

	HRESULT LoadMeshFromX( LPDIRECT3DDEVICE9, LPCTSTR fileName );
	HRESULT AllocateBoneMatrix( LPD3DXMESHCONTAINER );
	HRESULT AllocateAllBoneMatrices( LPD3DXFRAME );
	VOID UpdateFrameMatrices( LPD3DXFRAME, LPD3DXMATRIX );


	int GetNumVertices( MYMESHCONTAINER* pContainer );
	int GetNumFaces( MYMESHCONTAINER* pContainer );
	int GetNumMaterials( MYMESHCONTAINER* pContainer );
	int GetNumUVs( MYMESHCONTAINER* pContainer );
	int GetNumBones( MYMESHCONTAINER* pContainer );
	int GetNumBoneVertices( MYMESHCONTAINER* pContainer, int iBoneIndex );
	DWORD GetBoneVerticesIndices( MYMESHCONTAINER* pContainer, int iBoneIndex, int iIndexInGroup );
	double GetBoneVerticesWeights( MYMESHCONTAINER* pContainer, int iBoneIndex, int iIndexInGroup );
	D3DXVECTOR3 GetVertexCoord( MYMESHCONTAINER* pContainer, DWORD iIndex );
	D3DXVECTOR3 GetNormal( MYMESHCONTAINER* pContainer, DWORD iIndex );
	D3DXVECTOR2 GetUV( MYMESHCONTAINER* pContainer, DWORD iIndex );
	int GetIndex( MYMESHCONTAINER* pContainer, DWORD iIndex );
	D3DXVECTOR4 GetAmbient( MYMESHCONTAINER* pContainer, int iIndex );
	D3DXVECTOR4 GetDiffuse( MYMESHCONTAINER* pContainer, int iIndex );
	D3DXVECTOR4 GetSpecular( MYMESHCONTAINER* pContainer, int iIndex );
	LPSTR GetTexturePath( MYMESHCONTAINER* pContainer, int index );
	float GetSpecularPower( MYMESHCONTAINER* pContainer, int iIndex );
	int GeFaceMaterialIndex( MYMESHCONTAINER* pContainer, int iFaceIndex );
	int GetFaceVertexIndex( MYMESHCONTAINER* pContainer, int iFaceIndex, int iIndexInFace );
	D3DXMATRIX GetBindPose( MYMESHCONTAINER* pContainer, int iBoneIndex );
	D3DXMATRIX GetNewPose( MYMESHCONTAINER* pContainer, int iBoneIndex );
	LPCSTR GetBoneName( MYMESHCONTAINER* pContainer, int iBoneIndex );

	// ���b�V���R���e�i���擾����.
	LPD3DXMESHCONTAINER GetMeshContainer( LPD3DXFRAME pFrame );

	// �A�j���[�V�����Z�b�g�̐؂�ւ�.
	void ChangeAnimSet( int index, LPD3DXANIMATIONCONTROLLER pAC = nullptr );
	// �A�j���[�V�����Z�b�g�̐؂�ւ�(�J�n�t���[���w��\��).
	void ChangeAnimSet_StartPos( int index, double dStartFramePos, LPD3DXANIMATIONCONTROLLER pAC = nullptr );
	// �A�j���[�V�������u�����h���Đ؂�ւ�.
	void ChangeAnimBlend( int index, int oldIndex, LPD3DXANIMATIONCONTROLLER pAC = nullptr );

	// �A�j���[�V������~���Ԃ��擾.
	double GetAnimPeriod( int index );
	// �A�j���[�V���������擾.
	int GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC = nullptr );

	// �w�肵���{�[�����(���W�E�s��)���擾����֐�.
	bool GetMatrixFromBone( LPCSTR sBoneName, D3DXMATRIX* pOutMat );
	bool GetPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos );

	// �ꊇ�������.
	HRESULT Release();

	// ���b�V�����.
//	HRESULT ReleaseMesh( LPD3DXFRAME pFrame );
};

