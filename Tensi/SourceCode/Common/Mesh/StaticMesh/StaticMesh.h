#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

class CFog;

/************************************************
*	�X�^�e�B�b�N���b�V���N���X	
**/
class CStaticMesh final
	: public CCommon
{
public:
	// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
	//	���V�F�[�_���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
	//	���b�V���P�ʂœn�����.
	struct CBUFFER_PER_MESH
	{
		D3DXMATRIX	mW;			// ���[���h�s��.
		D3DXMATRIX	mWVP;		// ���[���h,�r���[,�v���W�F�N�V�����̍����ϊ��s��.	
		D3DXCOLOR4	vColor;		// �J���[.
	};
	// �}�e���A���P�ʂœn�����.
	struct CBUFFER_PER_MATERIAL
	{
		D3DXVECTOR4 vAmbient;	// �A���r�G���g�F(���F).
		D3DXVECTOR4	vDiffuse;	// �f�B�t���[�Y�F(�g�U���ːF).
		D3DXVECTOR4	vSpecular;	// �X�y�L�����F(���ʔ��ːF).
		D3DXVECTOR4	vUV;		// �X�y�L�����F(���ʔ��ːF).
	};
	// �t���[���P�ʂœn�����.
	//	�J����,���C�g���͂����ɓ����.
	struct CBUFFER_PER_FRAME
	{
		D3DXPOSITION4	vCameraPos;			// �J�����ʒu(���_�ʒu).
		D3DXVECTOR4		vLightDir;			// ���C�g����.
		D3DXVECTOR4		vLightIntensity;	// ���C�g�̋���.
		D3DXVECTOR4		vFlag;				// x�F�޲�ޔ������g�p���邩�A	y�F��̧��ۯ����g�p���邩�A	z�F�����œ����ɂ��邩(1�ȏ�Ȃ�g�p����).
		D3DXVECTOR4		vAlphaDistance;		// x�F�J�n���鋗��(�ȉ�)�A	y�F�J�n���鋗��(�ȏ�)�A	z�F�����ɂ��Ă����Ԋu.
	};

	// ���_�̍\����.
	struct VERTEX
	{
		D3DXPOSITION3	Pos;	// ���_���W.
		D3DXVECTOR3		Normal;	// �@��(�A�e�v�Z�ɕK�{).
		D3DXVECTOR2		UV;		// UV(x,y�̂ݎg�p����).
	};
	// �}�e���A���\����.
	struct MY_MATERIAL
	{
		D3DXVECTOR4								Ambient;			// �A���r�G���g.
		D3DXVECTOR4								Diffuse;			// �f�B�t���[�Y.
		D3DXVECTOR4								Specular;			// �X�y�L����.
		TCHAR									TextureName[64];	// �e�N�X�`���t�@�C����.
		std::vector<ID3D11ShaderResourceView*>	pTexture;			// �e�N�X�`��.
		ID3D11ShaderResourceView*				pLightTexture;		// ���C�g�e�N�X�`��.
		DWORD									dwNumFace;			// ���̃}�e���A���̃|���S����.
		D3DXVECTOR4								UV;					// UV(x,y�̂ݎg�p����).

		// �R���X�g���N�^.
		MY_MATERIAL()
			: Ambient		()
			, Diffuse		()
			, Specular		()
			, TextureName	()
			, pTexture		( 1 )
			, pLightTexture	( nullptr )
			, dwNumFace		( 0 )
			, UV			( INIT_FLOAT4 )
		{
		}
		// �f�X�g���N�^.
		~MY_MATERIAL() {
			const int Size = static_cast<int>( pTexture.size() );
			for ( int i = Size - 1; i >= 0; --i ) {
				SAFE_RELEASE( pTexture[i] );
			}
			if ( pLightTexture != nullptr )
			{
				pLightTexture->Release();
				pLightTexture = nullptr;
			}
		}
	};

	// X�t�@�C���̃��[�h�ŕK�v�󂯎�����.
	struct MY_MODEL
	{
		TCHAR			FileName[64];		// �t�@�C����.
		LPD3DXMESH		pMesh;				// ���b�V���I�u�W�F�N�g.
		LPD3DXBUFFER	pD3DXMtrlBuffer;	// �}�e���A���o�b�t�@.
		DWORD			NumMaterials;		// �}�e���A����.
		MY_MODEL()
			: FileName			()
			, pMesh				( nullptr )
			, pD3DXMtrlBuffer	( nullptr )
			, NumMaterials		()
		{}
		~MY_MODEL(){
			SAFE_RELEASE( pMesh );
			SAFE_RELEASE( pD3DXMtrlBuffer );
		}
	};

public:
	CStaticMesh();
	~CStaticMesh();

	HRESULT Init( LPCTSTR lpFileName );

	// ����֐�.
	void Release();

	// �����_�����O�p.
	void Render( STransform* pTransform = nullptr, const D3DXMATRIX* pRot = nullptr );

	// ���b�V�����X�g���擾.
	inline std::pair<LPD3DXMESH, LPD3DXMESH> GetMeshList() const { return std::make_pair( m_Model.pMesh, m_ModelForRay.pMesh ); }

	// ���b�V���̎擾.
	inline LPD3DXMESH GetMesh()			const { return m_Model.pMesh; }
	// ���C�Ƃ̓����蔻��p�̃��b�V���̎擾.
	inline LPD3DXMESH GetModelForRay()	const { return m_ModelForRay.pMesh; }

	// �t�H�O�̗L��/����.
	inline void SetIsFog( const bool Flag ) { m_IsFog = Flag; }
	// �f�B�U�������g�p���邩.
	inline void SetDither( const bool Flag ) { m_DitherFlag = Flag; }
	// �A���t�@�u���b�N���g�p���邩.
	inline void SetAlphaBlock( const bool Flag ) { m_AlphaBlockFlag = Flag; }
	// �A���t�@�u�����h���g�p���邩.
	inline void SetAlphaBlend( const bool Flag ) { DirectX11::SetAlphaBlend( Flag ); }
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
	inline void SetUVScroll( const int MaterialNo, const D3DXVECTOR2& uv ) {
		m_pMaterials[MaterialNo].UV.x = uv.x;
		m_pMaterials[MaterialNo].UV.y = uv.y;
	}
	// x������UV�X�N���[������l��ݒ�.
	inline void SetUVScrollX( const int MaterialNo, const float uv ) {
		m_pMaterials[MaterialNo].UV.x = uv;
	}
	// y������UV�X�N���[������l��ݒ�.
	inline void SetUVScrollY( const int MaterialNo, const float uv ) {
		m_pMaterials[MaterialNo].UV.y = uv;
	}

private:
	// ���b�V���ǂݍ���.
	HRESULT LoadXMesh( LPCTSTR lpFileName );
	// �V�F�[�_�쐬.
	HRESULT CreateShader();
	// ���f���쐬.
	HRESULT CreateModel();
	// �}�e���A���쐬.
	HRESULT CreateMaterials();
	// �C���f�b�N�X�o�b�t�@�쐬.
	HRESULT CreateIndexBuffer();
	// ���_�o�b�t�@�쐬.
	HRESULT CreateVertexBuffer();
	// �R���X�^���g�o�b�t�@�쐬.
	HRESULT CreateConstantBuffer();
	// �T���v���쐬.
	HRESULT CreateSampler();

	// �����_�����O�֐�(�N���X���ł̂ݎg�p����).
	void RenderMesh( D3DXMATRIX& mWorld );

private:
	ID3D11VertexShader*		m_pVertexShader;				// ���_�V�F�[�_.
	ID3D11InputLayout*		m_pVertexLayout;				// ���_���C�A�E�g.
	ID3D11PixelShader*		m_pPixelShader;					// �s�N�Z���V�F�[�_.

	ID3D11Buffer*			m_pCBufferPerMesh;				// �R���X�^���g�o�b�t�@(���b�V����).
	ID3D11Buffer*			m_pCBufferPerMaterial;			// �R���X�^���g�o�b�t�@(�}�e���A����).
	ID3D11Buffer*			m_pCBufferPerFrame;				// �R���X�^���g�o�b�t�@(�t���[����).

	ID3D11Buffer*			m_pVertexBuffer;				// ���_�o�b�t�@.
	ID3D11Buffer**			m_ppIndexBuffer;				// �C���f�b�N�X�o�b�t�@.
	ID3D11SamplerState*		m_pSampleLinear;				// �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	MY_MODEL				m_Model;
	MY_MODEL				m_ModelForRay;					// ���C�Ƃ̓����蔻��Ŏg�p����.

	MY_MATERIAL*			m_pMaterials;					// �}�e���A���\����.
	DWORD					m_NumAttr;						// ������.
	DWORD					m_AttrID[300];					// ����ID ��300�����܂�.

	bool					m_EnableTexture;				// �e�N�X�`������.

	std::unique_ptr<CFog>	m_pFog;							// �t�H�O.
	bool					m_IsFog;						// �t�H�O��L���ɂ��邩.
	bool					m_DitherFlag;					// �f�B�U�������g�p���邩.
	bool					m_AlphaBlockFlag;				// �A���t�@�u���b�N���g�p���邩.
	bool					m_OrLessDistanceAlphaFlag;		// �����œ����ɂ��邩(�ȉ�).
	bool					m_OrMoreDistanceAlphaFlag;		// �����œ����ɂ��邩(�ȏ�).

	float					m_OrLessStartDistance;			// �J�n���鋗��(�ȉ�).
	float					m_OrMoreStartDistance;			// �J�n���鋗��(�ȏ�).
	float					m_AlphaDistance;				// �����ɂ��Ă����Ԋu.

	std::vector<std::string> m_TexturePathList;				// �����ւ��e�N�X�`���p�X���X�g.
	std::vector<int>		m_MaterialTextureNo;			// �}�e���A���̃e�N�X�`����No.
 };
