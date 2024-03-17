#pragma once
#include "..\..\..\Global.h"
#include "..\..\Camera\CameraBase.h"
#include "..\..\Collision\Ray\Ray.h"
#include <functional>

class CFreeCamera;

/************************************************
*	カメラマネージャークラス.
*		﨑田友輝.
**/
class CameraManager final
{
public:
	CameraManager();
	~CameraManager();

	// 更新.
	static void Update( float DeltaTime );

	// カメラの設定.
	static void SetCamera( CCameraBase* pCamera );
	// カメラの変更を禁止させるか.
	static void SetIsCameraChangeLock( const bool Flag ) { GetInstance()->m_IsCameraChangeLock = Flag; }

	// 向きベクトルの取得.
	static D3DXVECTOR3 GetDireVector() { return GetInstance()->m_DireVec; }
	// 視点の取得.
	static D3DXPOSITION3 GetPosition() { return GetInstance()->m_pNowCamera->GetPosition(); }
	// 視点座標の取得.
	static D3DXPOSITION3 GetLookPosition() { return GetInstance()->m_pNowCamera->GetLookPosition(); }
	// 回転の取得.
	static D3DXROTATION3 GetRotation() { return GetInstance()->m_pNowCamera->GetRotation(); }
	// ビュー行列取得.
	static D3DXMATRIX GetViewMatrix() { return GetInstance()->m_pNowCamera->GetViewMatrix(); }
	// プロジェクション行列取得.
	static D3DXMATRIX GetProjMatrix() { return GetInstance()->m_pNowCamera->GetProjMatrix(); }
	// ビュー・プロジェクションの合成行列取得.
	static D3DXMATRIX GetViewProjMatrix() { return GetInstance()->m_pNowCamera->GetViewProjMatrix(); }

	// レイの取得.
	static CRay* GetRay() { return GetInstance()->m_pRay.get(); }

	// カメラの振動(縦揺れ)の設定.
	static void SetPitch( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );
	// カメラの振動(横揺れ)の設定.
	static void SetRolling( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );
	// カメラの振動(ミックス)の設定.
	static void SetMixShaking( const float Time, const int Num, const float MaxValue, const float MinValue = -1.0f );

	// ヒットストップの設定.
	static void HitStop( const float Time, const bool IsSlow = false, const std::function<void()>& EndProc = {}, const std::function<void()>& SlowProc = {} );

private:
	// インスタンスの取得.
	static CameraManager* GetInstance();

	// フリーカメラに変更.
	static void ChangeFreeCamera();
	// 設定されているカメラに変更.
	static void ChangeSetCamera();

	// 揺れの更新.
	static void ShakingUpdate();
	// 縦揺れの更新.
	static D3DXPOSITION3 PitchUpdate();
	// 横揺れの更新.
	static D3DXPOSITION3 RollingUpdate();

private:
	CCameraBase*					m_pNowCamera;			// 現在のカメラ.
	CCameraBase*					m_pTmpCamera;			// カメラを変更時に一時カメラを保管する.
	std::unique_ptr<CFreeCamera>	m_pFreeCamera;			// 自由に動かれるカメラ.
	std::unique_ptr<CRay>			m_pRay;					// カメラの向いている方向のレイ.
	SCameraState					m_FreeCameraState;		// フリーカメラの状態.
	SCameraState					m_TmpCameraState;		// カメラの変更時に一時的にカメラの状態を保管する.
	D3DXVECTOR3						m_DireVec;				// カメラの向きベクトル.
	std::function<void()>			m_HitStopEndProc;		// ヒットストップが終了した時に行う関数.
	std::function<void()>			m_HitStopSlowProc;		// ヒットストップがスローに切り替える時に行う関数.
	int								m_ShakingNo;			// カメラの振動の種類.
	int								m_ShakingNum;			// カメラの振動回数.
	float							m_DeltaTime;			// デルタタイム.
	float							m_ShakingTimeCnt;		// カメラの振動の時間用のカウント.
	float							m_ShakingTimeMax;		// カメラの振動時間.
	float							m_ShakingMax;			// カメラの揺れの最大値.
	float							m_ShakingMin;			// カメラの揺れの最小値.
	bool							m_IsHitStop;			// ヒットストップ中か.
	bool							m_IsHitStopSlow;		// ヒットストップ時スローをかけるか.
	bool							m_IsFreeCamera;			// フリーカメラかどうか.
	bool							m_IsCameraChangeLock;	// カメラの変更を禁止するか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	CameraManager( const CameraManager & ) = delete;
	CameraManager& operator = ( const CameraManager & ) = delete;
	CameraManager( CameraManager && ) = delete;
	CameraManager& operator = ( CameraManager && ) = delete;
};
