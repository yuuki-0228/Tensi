/***************************************************************************************************
*	SkinMeshCode Version 2.10
*	LastUpdate	: 2020/10/06.
*		Unicode�Ή�.
**/
#pragma once

#include "..\..\Common.h"
#include "Parser\SkinMeshParser.h"

class CFog;
class CStaticMesh;

// �A�j���[�V�����R���g���[���\����.
struct stAnimationController {
	LPD3DXANIMATIONCONTROLLER	pAC;
	double						AnimTime;
	bool						IsAnimChangeBlend;
	int							NowIndex;
	int							NewIndex;

	stAnimationController()
		: pAC				( nullptr )
		, AnimTime			( 0.0 )
		, IsAnimChangeBlend	( false )
		, NowIndex			( 0 )
		, NewIndex			( 0 )
	{}

	// �A�j���[�V�����u�����h�̍X�V.
	void BlendAnimUpdate( const double& Speed ) {
		if( IsAnimChangeBlend == false ) return;
		AnimTime += Speed;

		float weight = static_cast<float>( AnimTime ) / 1.0f;	// �E�F�C�g�̌v�Z.
		pAC->SetTrackWeight( 0, weight );						// �g���b�N0 �ɃE�F�C�g��ݒ�.
		pAC->SetTrackWeight( 1, 1.0f - weight );				// �g���b�N1 �ɃE�F�C�g��ݒ�.

		if( AnimTime < 1.0 ) return;
		// �A�j���[�V�����^�C�������l�ɒB������.
		IsAnimChangeBlend	= false;								// �t���O������.
		NowIndex			= NewIndex;
		NewIndex			= -1;
		pAC->SetTrackWeight( 0, 1.0f );							// �E�F�C�g��1�ɌŒ肷��.
		pAC->SetTrackEnable( 1, false );						// �g���b�N1�𖳌��ɂ���.
	}

	// �A�j���[�V�����R���g���[���̐ݒ�.
	HRESULT SetAnimController( LPD3DXANIMATIONCONTROLLER pAc ) {
		if( FAILED( pAc->CloneAnimationController(
			pAc->GetMaxNumAnimationOutputs(),
			pAc->GetMaxNumAnimationSets(),
			pAc->GetMaxNumTracks(),
			pAc->GetMaxNumEvents(),
			&pAC ) ) )
		{
			ErrorMessage( "�A�j���[�V�����R���g���[���̃N���[���쐬���s" );
			return E_FAIL;
		}
		return S_OK;
	}
} typedef SAnimationController;


/************************************************
*	�X�L�����b�V���N���X
**/
class CSkinMesh final
	: public CCommon
{
public:
	// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
	//	���V�F�[�_���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
	struct CBUFFER_PER_MESH
	{
		D3DXMATRIX	mW;				// ���[���h�s��.
		D3DXMATRIX	mWVP;			// ���[���h����ˉe�܂ł̕ϊ��s��.
		D3DXCOLOR4	vColor;			// �J���[.

	};

	// �}�e���A���P��.
	struct CBUFFER_PER_MATERIAL
	{
		D3DXVECTOR4 vAmbient;		// �A���r�G���g��.
		D3DXVECTOR4 vDiffuse;		// �f�B�t���[�Y�F.
		D3DXVECTOR4 vSpecular;		// ���ʔ���.
		D3DXVECTOR4	vUV;			// UV(x,y�̂ݎg�p����).
	};

	// �V�F�[�_�[�ɓn���l.
	struct CBUFFER_PER_FRAME
	{
		D3DXPOSITION4	vCameraPos;				// �J�����ʒu.
		D3DXVECTOR4		vLightDir;				// ���C�g����.
		D3DXVECTOR4		vLightIntensity;		// ���C�g�̋���.
		D3DXVECTOR4		vPointLightIntensity;	// ���b�V���ɑ΂��Ẵ|�C���g���C�g�̋���.
		D3DXVECTOR4		vFlag;					// x�F�޲�ޔ������g�p���邩�A	y�F��̧��ۯ����g�p���邩�A	z�F�����œ����ɂ��邩(1�ȏ�Ȃ�g�p����).
		D3DXVECTOR4		vAlphaDistance;			// x�F�J�n���鋗��(�ȉ�)�A	y�F�J�n���鋗��(�ȏ�)�A	z�F�����ɂ��Ă����Ԋu.
	};

	// �{�[���P��.
	struct CBUFFER_PER_BONES
	{
		D3DXMATRIX mBone[D3DXPARSER::MAX_BONES];
		CBUFFER_PER_BONES()
		{
			for ( int i = 0; i < D3DXPARSER::MAX_BONES; i++ )
			{
				D3DXMatrixIdentity( &mBone[i] );
			}
		}
	};


	// ���_�\����.
	struct VERTEX
	{
		D3DXPOSITION3	vPos;		// ���_�ʒu.
		D3DXVECTOR3		vNorm;		// ���_�@��.
		D3DXVECTOR2		vTex;		// UV���W.
		UINT bBoneIndex[4];			// �{�[�� �ԍ�.
		float bBoneWeight[4];		// �{�[�� �d��.
		VERTEX()
			: vPos			()
			, vNorm			()
			, vTex			()
			, bBoneIndex	()
			, bBoneWeight	()
		{}
	};

public:
	CSkinMesh();
	~CSkinMesh();

	HRESULT Init( LPCTSTR fileName );

	// ����֐�.
	HRESULT Release();

	// �`��֐�.
	void Render( STransform* pTransform = nullptr, SAnimationController* pAC = nullptr, D3DXMATRIX* pRot = nullptr );

	double GetAnimSpeed()				{ return m_dAnimSpeed;		}
	void SetAnimSpeed( double dSpeed )	{ m_dAnimSpeed = dSpeed;	}

	double GetBlendSpeed()				{ return m_dBlendSpeed;		}
	void SetBlendSpeed( double dSpeed )	{ m_dBlendSpeed = dSpeed;	}

	double GetAnimTime()				{ return m_dAnimTime;		}
	void SetAnimTime( double dTime )	{ m_dAnimTime = dTime;		}

	// ���b�V���ɑ΂��Ẵ|�C���g���C�g�̋����̐ݒ�.
	void	SetPointLightIntensity( float Intensity ) { m_PointLightIntensity = Intensity; }
	float	GetPointLightIntensity() { return m_PointLightIntensity; }

	// �p�[�T�[�N���X����A�j���[�V�����R���g���[�����擾����.
	LPD3DXANIMATIONCONTROLLER GetAnimationController() { return m_pD3dxMesh->m_pAnimController; }

	// �A�j���[�V�����Z�b�g�̐؂�ւ�.
	void ChangeAnimSet( int index, SAnimationController* pAC=nullptr );
	// �A�j���[�V�����Z�b�g�̐؂�ւ�(�J�n�t���[���w��\��).
	void ChangeAnimSet_StartPos( int index, double dStartFramePos, SAnimationController* pAC=nullptr );
	// �A�j���[�V�������u�����h���Đ؂�ւ�.
	void ChangeAnimBlend( int index, int oldIndex, SAnimationController* pAC = nullptr );
	void ChangeAnimBlend( int index, SAnimationController* pAC = nullptr );
	// �A�j���[�V�������u�����h���Đ؂�ւ�(���̃u�����h�����Ă��Ă��s��).
	void ChangeAnimBlend_Skip( int index, int oldIndex, SAnimationController* pAC = nullptr );
	void ChangeAnimBlend_Skip( int index, SAnimationController* pAC = nullptr );

	// �A�j���[�V������~���Ԃ��擾.
	double GetAnimPeriod( int index );
	// �A�j���[�V���������擾.
	int GetAnimMax( LPD3DXANIMATIONCONTROLLER pAC=nullptr );
	// ���b�V�����擾.
	LPD3DXMESH GetMesh() const { return m_pMeshForRay; }

	// �w�肵���{�[�����(���W�E�s��)���擾����֐�.
	D3DXMATRIX GetMatrixFromBone( LPCSTR sBoneName );
	bool GetMatrixFromBone( LPCSTR sBoneName, D3DXMATRIX* pOutMat );
	D3DXVECTOR3 GetPosFromBone( LPCSTR sBoneName );
	bool GetPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos);
	D3DXVECTOR3 GetDeviaPosFromBone( LPCSTR sBoneName, D3DXVECTOR3 vSpecifiedPos = { 0.0f, 0.0f, 0.0f });
	bool GetDeviaPosFromBone( LPCSTR sBoneName, D3DXVECTOR3* pOutPos, D3DXVECTOR3 vSpecifiedPos = { 0.0f, 0.0f, 0.0f });

	// �t�H�O�̗L��/����.
	inline void SetIsFog( const bool& Flag ) { m_IsFog = Flag; }
	// �f�B�U�������g�p���邩.
	inline void SetDither( const bool& Flag ) { m_DitherFlag = Flag; }
	// �A���t�@�u���b�N���g�p���邩.
	inline void SetAlphaBlock( const bool& Flag ) { m_AlphaBlockFlag = Flag; }
	// �A���t�@�u�����h���g�p���邩.
	inline void SetAlphaBlend( const bool& Flag ) { DirectX11::SetAlphaBlend( Flag ); }
	// �����œ����ɂ��邩(�ȉ�).
	inline void SetDistanceAlphaOrLess( const bool Flag, const float StartDistance = 0.0f, const float AlphaDistance = 0.0f ) {
		m_OrLessDistanceAlphaFlag	= Flag;
		m_OrLessStartDistance		= StartDistance;
		m_AlphaDistance				= AlphaDistance;
	}
	// �����œ����ɂ��邩(�ȏ�).
	inline void SetDistanceAlphaOrMore( const bool Flag, const float StartDistance = 0.0f, const float AlphaDistance = 0.0f ) {
		m_OrMoreDistanceAlphaFlag	= Flag;
		m_OrMoreStartDistance		= StartDistance;
		m_AlphaDistance				= AlphaDistance;
	}

	// �e�N�X�`���̕ύX.
	inline void ChangeTexture( const int MaterialNo, const int TextureNo ) { m_MaterialTextureNo[MaterialNo] = TextureNo; }

	// UV�X�N���[������l��ݒ�.
	void SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv );
	// x������UV�X�N���[������l��ݒ�.
	void SetUVScrollX( const int MaterialNo, const float uv );
	// y������UV�X�N���[������l��ݒ�.
	void SetUVScrollY( const int MaterialNo, const float uv );

private:
	// X�t�@�C������X�L�����b�V�����쐬����.
	HRESULT LoadXMesh( LPCTSTR lpFileName );

	// �V�F�[�_���쐬����.
	HRESULT CreateShader();
	// �C���f�b�N�X�o�b�t�@���쐬����.
	HRESULT CreateIndexBuffer( DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer );
//	void RecursiveSetNewPoseMatrices( BONE* pBone,D3DXMATRIX* pmParent );

	// ���b�V�����쐬����.
	HRESULT CreateAppMeshFromD3DXMesh( LPD3DXFRAME pFrame );

	// �R���X�^���g�o�b�t�@�쐬����.
	HRESULT CreateCBuffer( ID3D11Buffer** pConstantBuffer, UINT size );
	// �T���v�����쐬����.
	HRESULT CreateSampler( ID3D11SamplerState** pSampler );

	// �S�Ẵ��b�V�����쐬����.
	void BuildAllMesh( D3DXFRAME* pFrame );

	// X�t�@�C������X�L���֘A�̏���ǂݏo���֐�.
	HRESULT ReadSkinInfo( MYMESHCONTAINER* pContainer, VERTEX* pvVB, SKIN_PARTS_MESH* pParts );

	// �{�[�������̃|�[�Y�ʒu�ɃZ�b�g����֐�.
	void SetNewPoseMatrices( SKIN_PARTS_MESH* pParts, int frame, MYMESHCONTAINER* pContainer );
	// ����(���݂�)�|�[�Y�s���Ԃ��֐�.
	D3DXMATRIX GetCurrentPoseMatrix( SKIN_PARTS_MESH* pParts, int index );

	// �t���[���`��.
	void DrawFrame( STransform* pTransform, LPD3DXFRAME pFrame );
	// �p�[�c�`��.
	void DrawPartsMesh( STransform* pTransform, SKIN_PARTS_MESH* p, D3DXMATRIX World, MYMESHCONTAINER* pContainer );

	// �A�j���[�V�����̍X�V.
	void AnimUpdate( SAnimationController* pAC );
	// �u�����h�A�j���[�V�����̍X�V.
	void BlendAnimUpdate();

	// �S�Ẵ��b�V�����폜.
	void DestroyAllMesh( D3DXFRAME* pFrame );
	HRESULT DestroyAppMeshFromD3DXMesh( LPD3DXFRAME p );

	// �}���`�o�C�g������Unicode�����ɕϊ�����.
	void ConvertCharaMultiByteToUnicode( WCHAR* Dest, size_t DestArraySize, const CHAR* str );

	// UV�̕ύX.
	void ChangeUV( const int MaterialNo, const D3DXVECTOR2& uv, LPD3DXFRAME p );
	// x������UV�̕ύX.
	void ChangeUVX( const int MaterialNo, const float uv, LPD3DXFRAME p );
	// y������UV�̕ύX.
	void ChangeUVY( const int MaterialNo, const float uv, LPD3DXFRAME p );

private:
	ID3D11SamplerState*		m_pSampleLinear;
	ID3D11VertexShader*		m_pVertexShader;
	ID3D11PixelShader*		m_pPixelShader;
	ID3D11InputLayout*		m_pVertexLayout;
	ID3D11Buffer*			m_pCBufferPerMesh;		// �R���X�^���g�o�b�t�@(���b�V����).
	ID3D11Buffer*			m_pCBufferPerMaterial;	// �R���X�^���g�o�b�t�@(�}�e���A����).
	ID3D11Buffer*			m_pCBufferPerFrame;		// �R���X�^���g�o�b�t�@(�t���[����).
	ID3D11Buffer*			m_pCBufferPerBone;		// �R���X�^���g�o�b�t�@(�{�[����).

	D3DXMATRIX				m_mWorld;
	D3DXMATRIX				m_mRotation;

	D3DXMATRIX				m_mView;
	D3DXMATRIX				m_mProj;

	D3DXMATRIX*				m_pRotMatrix;

	D3DXVECTOR3				m_vLight;
	D3DXPOSITION3			m_vCamPos;

	double					m_dAnimSpeed;					// �A�j���[�V�������x.
	double					m_dBlendSpeed;					// �A�j���[�V�����u�����h���x.
	double					m_dAnimTime;					// �A�j���[�V�������x.

	bool					m_IsChangeAnim;

	D3DXPARSER*				m_pD3dxMesh;					// ���b�V��.
	LPD3DXMESH				m_pMeshForRay;					// ڲ��ү���p.

	TCHAR					m_FilePath[256];				// XFile�̃p�X.

	int						m_iFrame;						// �A�j���[�V�����t���[��.

	float					m_PointLightIntensity;			// ���b�V���ɑ����Ẵ|�C���g���C�g�̋���.

	std::unique_ptr<CFog>	m_pFog;							// �t�H�O.
	bool					m_IsFog;						// �t�H�O��L���ɂ��邩.
	bool					m_DitherFlag;					// �f�B�U�������g�p���邩.
	bool					m_AlphaBlockFlag;				// �A���t�@�u���b�N���g�p���邩.
	bool					m_OrLessDistanceAlphaFlag;		// �����œ����ɂ��邩(�ȉ�).
	bool					m_OrMoreDistanceAlphaFlag;		// �����œ����ɂ��邩(�ȏ�).

	float					m_OrLessStartDistance;			// �J�n���鋗��(�ȉ�).
	float					m_OrMoreStartDistance;			// �J�n���鋗��(�ȏ�).
	float					m_AlphaDistance;				// �����ɂ��Ă����Ԋu.

	int						m_NowIndex;						// ���݂̃A�j���[�V����No.
	int						m_NewIndex;						// ���̃A�j���[�V����No.

	std::vector<int>		m_MaterialTextureNo;			// �}�e���A���̃e�N�X�`����No.
};
