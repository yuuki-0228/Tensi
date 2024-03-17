#include "Effect.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include <encrypt/file.h>

namespace{
	// �`��X�v���C�g�ő吔.
	const int RENDER_SPRITE_MAX		= 10000;
	// �G�t�F�N�g�Ǘ��p�C���X�^���X�ő吔.
	const int EFFECT_INSTANCE_MAX	= 10000;
}

CEffect::CEffect()
	: m_pManager	( nullptr )
	, m_pRenderer	( nullptr )
	, m_pEffect		( nullptr )
	, m_Mutex		()
{
}

CEffect::CEffect( const std::string& fileName )
	: CEffect	()
{
	if ( FAILED( Init( fileName ) ) ) {
		ErrorMessage( "Effekseer ������ : ���s" );
	}
}

CEffect::~CEffect()
{
}

//----------------------------.
// ������.
//----------------------------.
HRESULT CEffect::Init( const std::string& FileName )
{
	if ( FAILED( Create() ) ) {
		ErrorMessage( "Effekseer �\�z : ���s" );
		return E_FAIL;
	}
	if ( FAILED( LoadData( FileName ) ) ) {
		ErrorMessage( "Effekseer �ǂݍ��� : ���s" );
		return E_FAIL;
	}

	std::string Msg = FileName + " �ǂݍ��� : ����";
	Log::PushLog( Msg.c_str() );
	return S_OK;
}

//----------------------------.
// �\�z�֐�.
//----------------------------.
HRESULT CEffect::Create()
{
	// �G�t�F�N�g�̃����_���[�̍쐬.
	m_pRenderer = ::EffekseerRendererDX11::Renderer::Create(
		DirectX11::GetDevice(), DirectX11::GetContext(), RENDER_SPRITE_MAX );

	// �G�t�F�N�g�̃}�l�[�W���[�̍쐬.
	m_pManager = ::Effekseer::Manager::Create( EFFECT_INSTANCE_MAX );

	// �`�惂�W���[���̐ݒ�.
	m_pManager->SetSpriteRenderer(	m_pRenderer->CreateSpriteRenderer()	);
	m_pManager->SetRibbonRenderer(	m_pRenderer->CreateRibbonRenderer()	);
	m_pManager->SetRingRenderer(	m_pRenderer->CreateRingRenderer()	);
	m_pManager->SetTrackRenderer(	m_pRenderer->CreateTrackRenderer()	);
	m_pManager->SetModelRenderer(	m_pRenderer->CreateModelRenderer()	);

	// �e�N�X�`���A���f���A�}�e���A�����[�_�[�̐ݒ������.
	//	���[�U�[���Ǝ��Ŋg���ł���.���݂̓t�@�C������ǂݍ���ł���.
	m_pManager->SetTextureLoader(	m_pRenderer->CreateTextureLoader()	);
	m_pManager->SetModelLoader(		m_pRenderer->CreateModelLoader()	);
	m_pManager->SetMaterialLoader(	m_pRenderer->CreateMaterialLoader()	);

	return S_OK;
}

//----------------------------.
// �`��֐�.
//----------------------------.
HRESULT CEffect::Render( const Effekseer::Handle& handle )
{
	D3DXMATRIX View = CameraManager::GetViewMatrix();
	D3DXMATRIX Proj = CameraManager::GetProjMatrix();


	// �r���[�s���ݒ�.
	SetViewMatrix( View );

	// �ˉe�s���ݒ�.
	SetProjectionMatrix( Proj );

	// �G�t�F�N�g�̍X�V�������s��.
	m_pManager->BeginUpdate();

	// �G�t�F�N�g�̍X�V����.
	m_pManager->UpdateHandle( handle );
	
	// �G�t�F�N�g�̍X�V�I���������s��.
	m_pManager->EndUpdate();

	//----------------------------------------------.
	//	Effekseer�����_�����O.
	//----------------------------------------------.
	// �G�t�F�N�g�̕`��J�n�������s��.
	m_pRenderer->BeginRendering();

	// �G�t�F�N�g�̕`����s��.
	m_pManager->DrawHandle( handle );

	// �G�t�F�N�g�̕`��I���������s��.
	m_pRenderer->EndRendering();

	return S_OK;
}

//----------------------------.
// �f�[�^�Ǎ��֐�.
//----------------------------.
HRESULT CEffect::LoadData( const std::string& FileName )
{
	std::unique_lock<std::mutex> Lock( m_Mutex );

	// �G�t�F�N�g�̓Ǎ�.
	std::wstring wName = StringConversion::to_wString( FileName );
#ifdef _DEBUG
	m_pEffect = Effekseer::Effect::Create( m_pManager, ( const EFK_CHAR* ) wName.c_str() );
#else
	std::string es = encrypt::GetEncryptionFilePath( FileName );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	m_pEffect = Effekseer::Effect::Create( m_pManager, wrf.first, wrf.second );
#endif
	if ( m_pEffect == nullptr ) {
		ErrorMessage( "�G�t�F�N�g�̓ǂݍ��ݎ��s" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// DirectX Vector3 �� Effekseer Vector3 �ɕϊ�����.
//----------------------------.
::Effekseer::Vector3D CEffect::ConvertVector3ToEfk( const D3DXVECTOR3& pSrcVec3Dx )
{
	return ::Effekseer::Vector3D( pSrcVec3Dx.x, pSrcVec3Dx.y, pSrcVec3Dx.z );
}

//----------------------------.
// Effekseer Vector3 �� DirectX Vector3 �ɕϊ�����.
//----------------------------.
D3DXVECTOR3 CEffect::ConvertVector3ToDx( const::Effekseer::Vector3D& pSrcVec3Efk )
{
	return D3DXVECTOR3( pSrcVec3Efk.X, pSrcVec3Efk.Y, pSrcVec3Efk.Z );
}

//----------------------------.
// DirectX Matrix �� Effekseer Matrix �ɕϊ�����.
//----------------------------.
::Effekseer::Matrix44 CEffect::ConvertMatrixToEfk( const D3DXMATRIX& pSrcMatDx )
{
	::Effekseer::Matrix44 OutMatEfk;

	OutMatEfk.Values[0][0] = pSrcMatDx.m[0][0];	// _11.
	OutMatEfk.Values[0][1] = pSrcMatDx.m[0][1];	// _12.
	OutMatEfk.Values[0][2] = pSrcMatDx.m[0][2];	// _13.
	OutMatEfk.Values[0][3] = pSrcMatDx.m[0][3];	// _14.

	OutMatEfk.Values[1][0] = pSrcMatDx.m[1][0];	// _21.
	OutMatEfk.Values[1][1] = pSrcMatDx.m[1][1];	// _22.
	OutMatEfk.Values[1][2] = pSrcMatDx.m[1][2];	// _23.
	OutMatEfk.Values[1][3] = pSrcMatDx.m[1][3];	// _24.

	OutMatEfk.Values[2][0] = pSrcMatDx.m[2][0];	// _31.
	OutMatEfk.Values[2][1] = pSrcMatDx.m[2][1];	// _32.
	OutMatEfk.Values[2][2] = pSrcMatDx.m[2][2];	// _33.
	OutMatEfk.Values[2][3] = pSrcMatDx.m[2][3];	// _34.

	OutMatEfk.Values[3][0] = pSrcMatDx.m[3][0];	// _41.
	OutMatEfk.Values[3][1] = pSrcMatDx.m[3][1];	// _42.
	OutMatEfk.Values[3][2] = pSrcMatDx.m[3][2];	// _43.
	OutMatEfk.Values[3][3] = pSrcMatDx.m[3][3];	// _44.

	return OutMatEfk;
}

//----------------------------.
// Effekseer Matrix �� DirectX Matrix �ɕϊ�����.
//----------------------------.
D3DXMATRIX CEffect::ConvertMatrixToDx( const::Effekseer::Matrix44& pSrcMatEfk )
{
	D3DXMATRIX OutMatDx;

	OutMatDx.m[0][0] = pSrcMatEfk.Values[0][0];	// _11.
	OutMatDx.m[0][1] = pSrcMatEfk.Values[0][1];	// _12.
	OutMatDx.m[0][2] = pSrcMatEfk.Values[0][2];	// _13.
	OutMatDx.m[0][3] = pSrcMatEfk.Values[0][3];	// _14.

	OutMatDx.m[1][0] = pSrcMatEfk.Values[1][0];	// _21.
	OutMatDx.m[1][1] = pSrcMatEfk.Values[1][1];	// _22.
	OutMatDx.m[1][2] = pSrcMatEfk.Values[1][2];	// _23.
	OutMatDx.m[1][3] = pSrcMatEfk.Values[1][3];	// _24.

	OutMatDx.m[2][0] = pSrcMatEfk.Values[2][0];	// _31.
	OutMatDx.m[2][1] = pSrcMatEfk.Values[2][1];	// _32.
	OutMatDx.m[2][2] = pSrcMatEfk.Values[2][2];	// _33.
	OutMatDx.m[2][3] = pSrcMatEfk.Values[2][3];	// _34.

	OutMatDx.m[3][0] = pSrcMatEfk.Values[3][0];	// _41.
	OutMatDx.m[3][1] = pSrcMatEfk.Values[3][1];	// _42.
	OutMatDx.m[3][2] = pSrcMatEfk.Values[3][2];	// _43.
	OutMatDx.m[3][3] = pSrcMatEfk.Values[3][3];	// _44.

	return OutMatDx;
}

//----------------------------.
// DirectX D3DXVECTOR4 �� Effekseer Color �ɕϊ�����.
//----------------------------.
::Effekseer::Color CEffect::ConvertColorToEfk( const D3DXCOLOR4& pColor )
{
	return ::Effekseer::Color( 
		static_cast<uint8_t>( 255 * pColor.x ),
		static_cast<uint8_t>( 255 * pColor.y ),
		static_cast<uint8_t>( 255 * pColor.z ),
		static_cast<uint8_t>( 255 * pColor.w ) );
}

//----------------------------.
// Effekseer Color �� DirectX D3DXVECTOR4 �ɕϊ�����.
//----------------------------.
D3DXCOLOR4 CEffect::ConvertColorToDx( const::Effekseer::Color& pColor )
{
	return D3DXCOLOR4( 255.0f / pColor.A, 255.0f / pColor.G, 255.0f / pColor.B, 255.0f / pColor.A );
}

//----------------------------.
// �r���[�s���ݒ�.
//----------------------------.
void CEffect::SetViewMatrix( D3DXMATRIX& mView )
{
	// �r���[�s��.
	::Effekseer::Matrix44 tmpEfkCamMat;
	tmpEfkCamMat = ConvertMatrixToEfk( mView );

	// �r���[�s���ݒ�.
	m_pRenderer->SetCameraMatrix( tmpEfkCamMat );
}

//----------------------------.
// �ˉe�s���ݒ�.
//----------------------------.
void CEffect::SetProjectionMatrix( D3DXMATRIX mProj )
{
	// �ˉe�s��.
	::Effekseer::Matrix44 tmpEfkProjMat;
	tmpEfkProjMat = ConvertMatrixToEfk( mProj );

	// �ˉe�s���ݒ�.
	m_pRenderer->SetProjectionMatrix( tmpEfkProjMat );
}
