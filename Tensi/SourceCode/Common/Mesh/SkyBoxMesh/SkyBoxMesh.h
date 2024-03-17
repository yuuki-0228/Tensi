#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

/************************************************
*	�X�J�C�{�b�N�X���b�V���N���X	
**/
class CSkyBoxMesh final
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

	// ���_�̍\����.
	struct VERTEX
	{
		D3DXPOSITION3	Pos;	// ���_���W.
		D3DXVECTOR3		Normal;	// �@��(�A�e�v�Z�ɕK�{).
		D3DXVECTOR2		UV;		// �e�N�X�`�����W.
	};
	// �}�e���A���\����.
	struct MY_MATERIAL
	{
		TCHAR TextureName[64];	// �e�N�X�`���t�@�C����.
		ID3D11ShaderResourceView* pTexture;// �e�N�X�`��.
		DWORD dwNumFace;		// ���̃}�e���A���̃|���S����.
		// �R���X�g���N�^.
		MY_MATERIAL()
			: TextureName	()
			, pTexture		( nullptr )
			, dwNumFace		( 0 )
		{
		}
		// �f�X�g���N�^.
		~MY_MATERIAL() {
			SAFE_RELEASE( pTexture );
		}
	};
	// �t���[���P�ʂœn�����.
	//	�J����,���C�g���͂����ɓ����.
	struct CBUFFER_PER_FRAME
	{
		D3DXVECTOR4 vFlag;		// x�F�޲�ޔ������g�p���邩�A	y�F��̧��ۯ����g�p���邩.
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
	CSkyBoxMesh();
	~CSkyBoxMesh();

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

	// �f�B�U�������g�p���邩.
	inline void SetDither( const bool& Flag ) { m_DitherFlag = Flag; }
	// �A���t�@�u���b�N���g�p���邩.
	inline void SetAlphaBlock( const bool& Flag ) { m_AlphaBlockFlag = Flag; }
	// �A���t�@�u�����h���g�p���邩.
	inline void SetAlphaBlend( const bool& Flag ) { DirectX11::SetAlphaBlend( Flag ); }

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
	ID3D11VertexShader*		m_pVertexShader;		// ���_�V�F�[�_.
	ID3D11InputLayout*		m_pVertexLayout;		// ���_���C�A�E�g.
	ID3D11PixelShader*		m_pPixelShader;			// �s�N�Z���V�F�[�_.

	ID3D11Buffer*			m_pCBufferPerMesh;		// �R���X�^���g�o�b�t�@(���b�V����).
	ID3D11Buffer*			m_pCBufferPerFrame;		// �R���X�^���g�o�b�t�@(�t���[����).

	ID3D11Buffer*			m_pVertexBuffer;		// ���_�o�b�t�@.
	ID3D11Buffer**			m_ppIndexBuffer;		// �C���f�b�N�X�o�b�t�@.
	ID3D11SamplerState*		m_pSampleLinear;		// �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	MY_MODEL				m_Model;
	MY_MODEL				m_ModelForRay;			// ���C�Ƃ̓����蔻��Ŏg�p����.

	MY_MATERIAL*			m_pMaterials;			// �}�e���A���\����.
	DWORD					m_NumAttr;				// ������.
	DWORD					m_AttrID[300];			// ����ID ��300�����܂�.

	bool					m_EnableTexture;		// �e�N�X�`������.
	bool					m_DitherFlag;			// �f�B�U�������g�p���邩.
	bool					m_AlphaBlockFlag;		// �A���t�@�u���b�N���g�p���邩.
 };
