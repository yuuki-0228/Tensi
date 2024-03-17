#include "Effect.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\Object\Camera\CameraManager\CameraManager.h"
#include <encrypt/file.h>

namespace{
	// 描画スプライト最大数.
	const int RENDER_SPRITE_MAX		= 10000;
	// エフェクト管理用インスタンス最大数.
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
		ErrorMessage( "Effekseer 初期化 : 失敗" );
	}
}

CEffect::~CEffect()
{
}

//----------------------------.
// 初期化.
//----------------------------.
HRESULT CEffect::Init( const std::string& FileName )
{
	if ( FAILED( Create() ) ) {
		ErrorMessage( "Effekseer 構築 : 失敗" );
		return E_FAIL;
	}
	if ( FAILED( LoadData( FileName ) ) ) {
		ErrorMessage( "Effekseer 読み込み : 失敗" );
		return E_FAIL;
	}

	std::string Msg = FileName + " 読み込み : 成功";
	Log::PushLog( Msg.c_str() );
	return S_OK;
}

//----------------------------.
// 構築関数.
//----------------------------.
HRESULT CEffect::Create()
{
	// エフェクトのレンダラーの作成.
	m_pRenderer = ::EffekseerRendererDX11::Renderer::Create(
		DirectX11::GetDevice(), DirectX11::GetContext(), RENDER_SPRITE_MAX );

	// エフェクトのマネージャーの作成.
	m_pManager = ::Effekseer::Manager::Create( EFFECT_INSTANCE_MAX );

	// 描画モジュールの設定.
	m_pManager->SetSpriteRenderer(	m_pRenderer->CreateSpriteRenderer()	);
	m_pManager->SetRibbonRenderer(	m_pRenderer->CreateRibbonRenderer()	);
	m_pManager->SetRingRenderer(	m_pRenderer->CreateRingRenderer()	);
	m_pManager->SetTrackRenderer(	m_pRenderer->CreateTrackRenderer()	);
	m_pManager->SetModelRenderer(	m_pRenderer->CreateModelRenderer()	);

	// テクスチャ、モデル、マテリアルローダーの設定をする.
	//	ユーザーが独自で拡張できる.現在はファイルから読み込んでいる.
	m_pManager->SetTextureLoader(	m_pRenderer->CreateTextureLoader()	);
	m_pManager->SetModelLoader(		m_pRenderer->CreateModelLoader()	);
	m_pManager->SetMaterialLoader(	m_pRenderer->CreateMaterialLoader()	);

	return S_OK;
}

//----------------------------.
// 描画関数.
//----------------------------.
HRESULT CEffect::Render( const Effekseer::Handle& handle )
{
	D3DXMATRIX View = CameraManager::GetViewMatrix();
	D3DXMATRIX Proj = CameraManager::GetProjMatrix();


	// ビュー行列を設定.
	SetViewMatrix( View );

	// 射影行列を設定.
	SetProjectionMatrix( Proj );

	// エフェクトの更新処理を行う.
	m_pManager->BeginUpdate();

	// エフェクトの更新処理.
	m_pManager->UpdateHandle( handle );
	
	// エフェクトの更新終了処理を行う.
	m_pManager->EndUpdate();

	//----------------------------------------------.
	//	Effekseerレンダリング.
	//----------------------------------------------.
	// エフェクトの描画開始処理を行う.
	m_pRenderer->BeginRendering();

	// エフェクトの描画を行う.
	m_pManager->DrawHandle( handle );

	// エフェクトの描画終了処理を行う.
	m_pRenderer->EndRendering();

	return S_OK;
}

//----------------------------.
// データ読込関数.
//----------------------------.
HRESULT CEffect::LoadData( const std::string& FileName )
{
	std::unique_lock<std::mutex> Lock( m_Mutex );

	// エフェクトの読込.
	std::wstring wName = StringConversion::to_wString( FileName );
#ifdef _DEBUG
	m_pEffect = Effekseer::Effect::Create( m_pManager, ( const EFK_CHAR* ) wName.c_str() );
#else
	std::string es = encrypt::GetEncryptionFilePath( FileName );
	auto wrf = encrypt::GetRestoreFile( StringConversion::to_wString( es ) );
	m_pEffect = Effekseer::Effect::Create( m_pManager, wrf.first, wrf.second );
#endif
	if ( m_pEffect == nullptr ) {
		ErrorMessage( "エフェクトの読み込み失敗" );
		return E_FAIL;
	}

	return S_OK;
}

//----------------------------.
// DirectX Vector3 を Effekseer Vector3 に変換する.
//----------------------------.
::Effekseer::Vector3D CEffect::ConvertVector3ToEfk( const D3DXVECTOR3& pSrcVec3Dx )
{
	return ::Effekseer::Vector3D( pSrcVec3Dx.x, pSrcVec3Dx.y, pSrcVec3Dx.z );
}

//----------------------------.
// Effekseer Vector3 を DirectX Vector3 に変換する.
//----------------------------.
D3DXVECTOR3 CEffect::ConvertVector3ToDx( const::Effekseer::Vector3D& pSrcVec3Efk )
{
	return D3DXVECTOR3( pSrcVec3Efk.X, pSrcVec3Efk.Y, pSrcVec3Efk.Z );
}

//----------------------------.
// DirectX Matrix を Effekseer Matrix に変換する.
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
// Effekseer Matrix を DirectX Matrix に変換する.
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
// DirectX D3DXVECTOR4 を Effekseer Color に変換する.
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
// Effekseer Color を DirectX D3DXVECTOR4 に変換する.
//----------------------------.
D3DXCOLOR4 CEffect::ConvertColorToDx( const::Effekseer::Color& pColor )
{
	return D3DXCOLOR4( 255.0f / pColor.A, 255.0f / pColor.G, 255.0f / pColor.B, 255.0f / pColor.A );
}

//----------------------------.
// ビュー行列を設定.
//----------------------------.
void CEffect::SetViewMatrix( D3DXMATRIX& mView )
{
	// ビュー行列.
	::Effekseer::Matrix44 tmpEfkCamMat;
	tmpEfkCamMat = ConvertMatrixToEfk( mView );

	// ビュー行列を設定.
	m_pRenderer->SetCameraMatrix( tmpEfkCamMat );
}

//----------------------------.
// 射影行列を設定.
//----------------------------.
void CEffect::SetProjectionMatrix( D3DXMATRIX mProj )
{
	// 射影行列.
	::Effekseer::Matrix44 tmpEfkProjMat;
	tmpEfkProjMat = ConvertMatrixToEfk( mProj );

	// 射影行列を設定.
	m_pRenderer->SetProjectionMatrix( tmpEfkProjMat );
}
