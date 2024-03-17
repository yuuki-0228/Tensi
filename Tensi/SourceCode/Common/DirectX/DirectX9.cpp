#include "DirectX9.h"

DirectX9::DirectX9()
	: m_pDevice9	( nullptr )
{
}

DirectX9::~DirectX9()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
DirectX9* DirectX9::GetInstance()
{
	static std::unique_ptr<DirectX9> pInstance = std::make_unique<DirectX9>();
	return pInstance.get();
}

//----------------------------.
// Dx9������.
//----------------------------.
HRESULT DirectX9::Create( HWND hWnd )
{
	DirectX9* pI = GetInstance();

	// �uDirect3D�v�I�u�W�F�N�g�̍쐬.
	LPDIRECT3D9 pD3d9 = Direct3DCreate9( D3D_SDK_VERSION );
	if( pD3d9 == nullptr ) {
		ErrorMessage( "Dx9�I�u�W�F�N�g�쐬���s" );
		return E_FAIL;
	}

	// Diret3D�f�o�C�X�I�u�W�F�N�g�̍쐬.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// �o�b�N�o�b�t�@�̃t�H�[�}�b�g(�f�t�H���g).
	d3dpp.BackBufferCount = 1;					// �o�b�N�o�b�t�@�̐�.
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// �X���b�v�G�t�F�N�g(�f�t�H���g).
	d3dpp.Windowed = true;						// �E�B���h�E���[�h.
	d3dpp.EnableAutoDepthStencil = true;		// �X�e���V���L��.
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// �X�e���V���̃t�H�[�}�b�g(16bit).

	// �f�o�C�X�쐬(HAL���[�h:�`��ƒ��_������GPU�ōs��).
	if( FAILED( pD3d9->CreateDevice(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &pI->m_pDevice9 ) ) )
	{
		// �f�o�C�X�쐬(HAL���[�h:�`���GPU�A���_������CPU�ōs��).
		if( FAILED( pD3d9->CreateDevice(
			D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &pI->m_pDevice9 ) ) )
		{
			//Log::PushLog( "HAL���[�h��DIRECT3D�f�o�C�X�쐬�ł��܂���BREF���[�h�ōĎ��s���܂�" );
			//MessageBox( nullptr,
			//	_T( "HAL���[�h��DIRECT3D�f�o�C�X�쐬�ł��܂���\nREF���[�h�ōĎ��s���܂�" ),
			//	_T( "�x�� "), MB_OK );

			// �f�o�C�X�쐬(REF���[�h:�`���CPU�A���_������GPU�ōs��).
			if( FAILED( pD3d9->CreateDevice(
				D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING,
				&d3dpp, &pI->m_pDevice9 ) ) )
			{
				// �f�o�C�X�쐬(REF���[�h:�`��ƒ��_������CPU�ōs��).
				auto result = pD3d9->CreateDevice(
					D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING,
					&d3dpp, &pI->m_pDevice9 );
				if( FAILED( result ) )
				{
					SAFE_RELEASE( pD3d9 );
					ErrorMessage( "DIRECT3D�f�o�C�X�쐬���s", result );
					return E_FAIL;
				}
			}
		}
	}

	SAFE_RELEASE( pD3d9 );
	return S_OK;
}

//----------------------------.
// DirectX9���.
//----------------------------.
void DirectX9::Release()
{
	SAFE_RELEASE( GetInstance()->m_pDevice9 );
}