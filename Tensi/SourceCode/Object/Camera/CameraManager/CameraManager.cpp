#include "CameraManager.h"
#include "..\FreeCamera\FreeCamera.h"
#include "..\..\..\Scene\SceneManager\SceneManager.h"
#include "..\..\..\Utility\Input\Input.h"
#include "..\..\..\Utility\ImGuiManager\DebugWindow\DebugWindow.h"
#include "..\..\..\Utility\Math\Math.h"
#include "..\..\..\Utility\Random\Random.h"

#include "..\..\..\Utility\FrameRate\FrameRate.h"

namespace {
	// カメラの振動の種類.
	enum enShakingType : unsigned char {
		None,

		Pitch,		// 縦揺れ.
		Rolling,	// 横揺れ.
		Mix,		// ミックス.
	} typedef EShakingType;
}

CameraManager::CameraManager()
	: m_pNowCamera			( nullptr )
	, m_pTmpCamera			( nullptr )
	, m_pFreeCamera			( nullptr )
	, m_pRay				( nullptr )
	, m_FreeCameraState		()
	, m_TmpCameraState		()
	, m_DireVec				( INIT_FLOAT3 )
	, m_HitStopEndProc		( {} )
	, m_HitStopSlowProc		( {} )
	, m_ShakingNo			( EShakingType::None )
	, m_ShakingNum			( INIT_INT )
	, m_DeltaTime			( INIT_FLOAT )
	, m_ShakingTimeCnt		( INIT_FLOAT )
	, m_ShakingTimeMax		( INIT_FLOAT )
	, m_ShakingMax			( INIT_FLOAT )
	, m_ShakingMin			( INIT_FLOAT )
	, m_IsHitStop			( false )
	, m_IsHitStopSlow		( false )
	, m_IsFreeCamera		( false )
#ifdef _DEBUG
	, m_IsCameraChangeLock	( false )
#else
	, m_IsCameraChangeLock	( true )
#endif
{
	m_pFreeCamera	= std::make_unique<CFreeCamera>();
	m_pRay			= std::make_unique<CRay>();
}

CameraManager::~CameraManager()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
CameraManager* CameraManager::GetInstance()
{
	static std::unique_ptr<CameraManager> pInstance = std::make_unique<CameraManager>();
	return pInstance.get();
}

//---------------------------.
// 更新
//	カメラが設定されていない場合は処理は行わない.
//---------------------------.
void CameraManager::Update( float DeltaTime )
{
	CameraManager* pI = GetInstance();

	pI->m_DeltaTime = DeltaTime;

	if ( pI->m_pNowCamera == nullptr ) return;

	// カメラの切り替え.
	if ( pI->m_IsCameraChangeLock == false ) {
		if ( DebugKeyInput::IsANDKeyDown( 'C', 'M' ) ) {
			if ( pI->m_IsFreeCamera )	ChangeSetCamera();
			else						ChangeFreeCamera();
		}
	}

	// 現在のカメラの更新.
	ShakingUpdate();
	if ( SceneManager::GetIsSceneStop() == false ) {
		pI->m_pNowCamera->Update( DeltaTime );
	}
	pI->m_pNowCamera->ViewProjUpdate();

	// ヒットストップの停止.
	if ( pI->m_IsHitStop && pI->m_ShakingNo == EShakingType::None ) {
		if ( pI->m_IsHitStopSlow ) {
			// シーンの停止を解除する.
			SceneManager::SetIsSceneStop( false );

			// スローにさせる.
			CFrameRate::SetSlow( 5 );
			SetMixShaking( pI->m_ShakingTimeMax, 5, 0.2f, 0.05f );
			pI->m_IsHitStopSlow = false;

			// スローに変更時に行う関数を呼び出す.
			pI->m_HitStopSlowProc();
			pI->m_HitStopSlowProc = {};
		}
		else {
			// シーンの停止を解除する.
			SceneManager::SetIsSceneStop( false );

			// ヒットストップの停止.
			pI->m_IsHitStop = false;
			CFrameRate::ResetSpeed();

			// ヒットストップ終了時に行う関数を呼び出す.
			pI->m_HitStopEndProc();
			pI->m_HitStopEndProc = {};
		}
	}

	// 向きベクトルの更新.
	const D3DXROTATION3& CamRot = pI->m_pNowCamera->GetRotation();
	pI->m_DireVec.x = sinf( Math::ToRadian( CamRot.y ) );
	pI->m_DireVec.y = sinf( Math::ToRadian( CamRot.x ) );
	pI->m_DireVec.z = cosf( Math::ToRadian( CamRot.y ) );

	// カメラの向いている方向のベクトルを求める.
	const D3DXPOSITION3& Pos	= CameraManager::GetPosition();
	const D3DXPOSITION3& Look	= CameraManager::GetLookPosition();
	D3DXVECTOR3 Vec				= Look - Pos;

	// レイの更新.
	pI->m_pRay->SetVector( Vec );
	pI->m_pRay->SetPosition( Pos );

#ifdef _DEBUG
	DebugWindow::PushProc( "Camera", [&]() {
		const D3DXPOSITION3 Pos		= CameraManager::GetPosition();
		const D3DXPOSITION3 Look	= CameraManager::GetLookPosition();
		const D3DXROTATION3 Rot		= CameraManager::GetRotation();

		ImGui::Text( "------------------------------" );
		ImGui::Text( "'C + M' Change Camera( SetCamera <-> FreeCamera )" );
		ImGui::Text( "FreeCameraMove 'Ctrl + '" );
		ImGui::Text( "'A or D'         : x Move" );
		ImGui::Text( "'SPACE or SHIFT' : y Move" );
		ImGui::Text( "'W or S'         : z Move " );
		ImGui::Text( "'R or F'         : x Rotation" );
		ImGui::Text( "'Q or E'         : y Rotation" );
		ImGui::Text( "'Z or X'         : Zoom" );
		ImGui::Text( "------------------------------" );
		ImGui::Text( "Position         : x[%.3f], y[%.3f], z[%.3f]", Pos.x,  Pos.y,  Pos.z );
		ImGui::Text( "LookPosition     : x[%.3f], y[%.3f], z[%.3f]", Look.x, Look.y, Look.z );
		ImGui::Text( "Rotation         : x[%.3f], y[%.3f], z[%.3f]", Rot.x,  Rot.y,  Rot.z );
		ImGui::Text( "IsFreeCamera     : %s", GetInstance()->m_IsFreeCamera ? "true" : "false" );
		ImGui::Text( "------------------------------" );
	} );
#endif	// #ifdef _DEBUG.
}

//---------------------------.
// カメラの設定.
//	設定するカメラが null の場合は処理は行わない.
//---------------------------.
void CameraManager::SetCamera( CCameraBase* pCamera )
{
	CameraManager* pI = GetInstance();

	if ( pCamera == nullptr ) return;

	// 現在フリーカメラを設定されている場合外す.
	if ( pI->m_IsFreeCamera ){
		pI->m_FreeCameraState	= pI->m_pNowCamera->GetCameraState();
		pI->m_IsFreeCamera		= false;
	}

	// カメラの設定.
	pI->m_pNowCamera = pCamera;
}

//---------------------------.
// カメラの振動(縦揺れ)の設定.
//---------------------------.
void CameraManager::SetPitch( const float Time, const int Num, const float MaxValue, const float MinValue )
{
	CameraManager* pI = GetInstance();

	// 揺れの設定.
	pI->m_ShakingNo			= EShakingType::Pitch;
	pI->m_ShakingNum		= Num		< 1		? 1			: Num; 
	pI->m_ShakingMax		= MaxValue	< 0.0f	? 0.0f		: MaxValue;
	pI->m_ShakingMin		= MinValue	< 0.0f	? MaxValue	: MinValue;
	pI->m_ShakingTimeMax	= Time;
	pI->m_ShakingTimeCnt	= INIT_FLOAT;
}

//---------------------------.
// カメラの振動(横揺れ)の設定.
//---------------------------.
void CameraManager::SetRolling( const float Time, const int Num, const float MaxValue, const float MinValue )
{
	CameraManager* pI = GetInstance();

	// 揺れの設定.
	pI->m_ShakingNo			= EShakingType::Rolling;
	pI->m_ShakingNum		= Num		< 1		? 1			: Num; 
	pI->m_ShakingMax		= MaxValue	< 0.0f	? 0.0f		: MaxValue;
	pI->m_ShakingMin		= MinValue	< 0.0f	? MaxValue	: MinValue;
	pI->m_ShakingTimeMax	= Time;
	pI->m_ShakingTimeCnt	= INIT_FLOAT;
}

//---------------------------.
// カメラの振動(ミックス)の設定.
//---------------------------.
void CameraManager::SetMixShaking( const float Time, const int Num, const float MaxValue, const float MinValue )
{
	CameraManager* pI = GetInstance();

	// 揺れの設定.
	pI->m_ShakingNo			= EShakingType::Mix;
	pI->m_ShakingNum		= Num		< 1		? 1			: Num; 
	pI->m_ShakingMax		= MaxValue	< 0.0f	? 0.0f		: MaxValue;
	pI->m_ShakingMin		= MinValue	< 0.0f	? MaxValue	: MinValue;
	pI->m_ShakingTimeMax	= Time;
	pI->m_ShakingTimeCnt	= INIT_FLOAT;
}

//---------------------------.
// ヒットストップの設定.
//---------------------------.
void CameraManager::HitStop( const float Time, const bool IsSlow, const std::function<void()>& EndProc, const std::function<void()>& SlowProc )
{
	CameraManager* pI = GetInstance();

	// ヒットストップを行うように設定.
	pI->m_IsHitStop		= true;
	pI->m_IsHitStopSlow = IsSlow;

	// 関数を保存する.
	pI->m_HitStopEndProc	= EndProc;
	pI->m_HitStopSlowProc	= SlowProc;

	// 画面の揺れの設定.
	SetMixShaking( Time, 5 * static_cast<int>( Time ), 0.2f, 0.05f );

	// シーンを停止させる.
	SceneManager::SetIsSceneStop( true );
}

//---------------------------.
// フリーカメラに変更.
//	フリーカメラが null / 現在フリーカメラ の場合処理は行わない.
//---------------------------.
void CameraManager::ChangeFreeCamera()
{
	CameraManager* pI = GetInstance();

	if ( pI->m_pFreeCamera  == nullptr	) return;
	if ( pI->m_IsFreeCamera == true		) return;

	// 現在設定されているカメラを保管する.
	pI->m_pTmpCamera		= pI->m_pNowCamera;
	pI->m_TmpCameraState	= pI->m_pNowCamera->GetCameraState();

	// フリーカメラに設定.
	pI->m_pNowCamera = pI->m_pFreeCamera.get();
	pI->m_pNowCamera->SetCameraState( pI->m_FreeCameraState );

	pI->m_IsFreeCamera = true;
}

//---------------------------.
// 設定されているカメラに変更.
//	保管中のカメラが null / 現在設定中のカメラ の場合処理は行わない.
//---------------------------.
void CameraManager::ChangeSetCamera()
{
	CameraManager* pI = GetInstance();

	if ( pI->m_pFreeCamera  == nullptr	) return;
	if ( pI->m_IsFreeCamera == false	) return;

	// フリーカメラの状態を保管する.
	pI->m_FreeCameraState	= pI->m_pNowCamera->GetCameraState();

	// 設定されたカメラに設定.
	pI->m_pNowCamera = pI->m_pTmpCamera;
	pI->m_pNowCamera->SetCameraState( pI->m_TmpCameraState );

	pI->m_IsFreeCamera = false;
}

//---------------------------.
// 揺れの更新.
//	揺れされない場合は処理を行わない.
//---------------------------.
void CameraManager::ShakingUpdate()
{
	CameraManager* pI = GetInstance();

	if ( pI->m_ShakingNo == EShakingType::None ) return;

	// 揺れのタイプに応じた更新を行う.
	D3DXVECTOR3 ShakingAddPos;
	switch ( pI->m_ShakingNo ) {
	case EShakingType::Pitch:
		ShakingAddPos = PitchUpdate();
		break;
	case EShakingType::Rolling:
		ShakingAddPos = RollingUpdate();
		break;
	case EShakingType::Mix:
	{
		D3DXVECTOR3 PitchAddPos;
		D3DXVECTOR3 RollingAddPos;

		PitchAddPos		= PitchUpdate();
		RollingAddPos	= RollingUpdate();

		ShakingAddPos = PitchAddPos + RollingAddPos;
		break;
	}
	default:
		break;
	}

	// 揺れによる追加の座標を設定.
	pI->m_pNowCamera->SetShaking( ShakingAddPos );
}

//---------------------------.
// 縦揺れの更新.
//---------------------------.
D3DXPOSITION3 CameraManager::PitchUpdate()
{
	CameraManager* pI = GetInstance();

	// 揺れを設定.
	D3DXPOSITION3	ShakingAddPos	= INIT_FLOAT3;
	const float		ShakingValue	= Random::GetRand( pI->m_ShakingMin, pI->m_ShakingMax );
	ShakingAddPos.y = sinf( Math::RADIAN_MAX * ( pI->m_ShakingTimeCnt / pI->m_ShakingTimeMax ) * pI->m_ShakingNum ) * ShakingValue;

	// 揺れの時間を増やす.
	pI->m_ShakingTimeCnt += pI->m_DeltaTime;
	if ( pI->m_ShakingTimeCnt >= pI->m_ShakingTimeMax ) {
		// 揺れを停止させる.
		pI->m_ShakingNo			= EShakingType::None;
		pI->m_ShakingTimeCnt	= INIT_FLOAT;
		ShakingAddPos			= INIT_FLOAT3;
	};
	return ShakingAddPos;
}

//---------------------------.
// 横揺れの更新.
//---------------------------.
D3DXPOSITION3 CameraManager::RollingUpdate()
{
	CameraManager* pI = GetInstance();

	// 移動ベクトルの作成.
	const D3DXVECTOR3&	LookVector = pI->m_pNowCamera->GetLookVector();
	D3DXVECTOR3			MoveVector = Math::X_AXIS;
	D3DXMATRIX			mRot;
	D3DXMatrixRotationYawPitchRoll( &mRot, atan2f( LookVector.x, LookVector.z ), 0.0f, 0.0f );
	D3DXVec3TransformCoord( &MoveVector, &MoveVector, &mRot );

	// 揺れを設定.
	D3DXPOSITION3	ShakingAddPos	= INIT_FLOAT3;
	const float		ShakingValue	= Random::GetRand( pI->m_ShakingMin, pI->m_ShakingMax );
	ShakingAddPos = MoveVector * ( sinf( Math::RADIAN_MAX * ( pI->m_ShakingTimeCnt / pI->m_ShakingTimeMax ) * pI->m_ShakingNum ) * ShakingValue );

	// 揺れの時間を増やす.
	pI->m_ShakingTimeCnt += pI->m_DeltaTime;
	if ( pI->m_ShakingTimeCnt >= pI->m_ShakingTimeMax ) {
		// 揺れを停止させる.
		pI->m_ShakingNo			= EShakingType::None;
		pI->m_ShakingTimeCnt	= INIT_FLOAT;
		ShakingAddPos			= INIT_FLOAT3;
	};
	return ShakingAddPos;
}
