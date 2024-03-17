#pragma once
#include "..\Object.h"
#include "..\..\Global.h"

// カメラの状態構造体.
struct stCameraState {
	CObject*		pTarget;		// ターゲット.
	D3DXPOSITION3	Position;		// 視点.
	D3DXPOSITION3	LookPosition;	// 注視点.
	D3DXROTATION3	Rotation;		// 回転値.
	D3DXVECTOR3		ViewVector;		// ビューベクトル.
	float			Zoom;			// ズーム.

	stCameraState()
		: stCameraState(
			nullptr,
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			0.0f )
	{}
	stCameraState( CObject* pObj, D3DXPOSITION3 Pos, D3DXPOSITION3 Look, D3DXROTATION3 Rot, D3DXVECTOR3 Vec, float zoom )
		: pTarget		( pObj )
		, Position		( Pos )
		, LookPosition	( Look )
		, Rotation		( Rot )
		, ViewVector	( Vec )
		, Zoom			( zoom )
	{}

} typedef SCameraState;

/************************************************
*	カメラベースクラス.
*		﨑田友輝.
**/
class CCameraBase
	: public CObject
{
public:
	CCameraBase();
	virtual ~CCameraBase();

	// 更新.
	virtual void Update( const float& DeltaTime ) = 0;

	// ビュー・プロジェクションの更新.
	virtual void ViewProjUpdate() final;

	// 注視座標の取得.
	inline D3DXVECTOR3 GetLookPosition() const { return m_LookPosition; }
	// 注視ベクトルの取得.
	inline D3DXVECTOR3 GetLookVector() const { return m_Transform.Position - m_LookPosition; }

	// ビュー行列取得.
	inline D3DXMATRIX GetViewMatrix() const { return m_ViewMatrix; }
	// プロジェクション行列取得.
	inline D3DXMATRIX GetProjMatrix() const { return m_ProjMatrix; }
	// ビュー・プロジェクションの合成行列取得.
	inline D3DXMATRIX GetViewProjMatrix() const { return m_ViewMatrix * m_ProjMatrix; }
	// カメラの状態を取得.
	inline SCameraState GetCameraState() const {
		return SCameraState( m_pTarget,
			m_Transform.Position,
			m_LookPosition,
			m_Transform.Rotation,
			m_ViewVector,
			m_Zoom ); 
	}

	// ターゲットの取得.
	inline CObject* GetTarget() { return m_pTarget; }

	// 移動を停止させるか.
	inline void SetIsMoveStop( const bool Flag ) { m_IsMoveStop = Flag; }

	// 注視座標の設定.
	inline void SetLookPosition( const D3DXPOSITION3& p ) { m_LookPosition = p; }
	// ターゲットの設定.
	inline void SetTarget( CObject* pTarget ) { m_pTarget = pTarget; }
	inline void ResetTarget() { m_pTarget = nullptr; }
	// カメラの状態を設定.
	inline void SetCameraState( const SCameraState& State ) {
		m_pTarget				= State.pTarget;
		m_Transform.Position	= State.Position;
		m_LookPosition			= State.LookPosition;
		m_Transform.Rotation	= State.Rotation;
		m_ViewVector			= State.ViewVector;
		m_Zoom					= State.Zoom;
	}
	// 揺れの設定.
	inline void SetShaking( const D3DXPOSITION3& Shaking ) { m_ShakingAddPosition = Shaking; }

protected:
	CObject*		m_pTarget;				// ターゲット.
	D3DXPOSITION3	m_LookPosition;			// 注視点.
	D3DXVECTOR3		m_ViewVector;			// ビューベクトル.
	float			m_Zoom;					// ズーム.
	bool			m_IsMoveStop;			// 移動の停止.

private:
	D3DXMATRIX		m_ViewMatrix;			// ビュー行列.
	D3DXMATRIX		m_ProjMatrix;			// プロジェクション行列.
	D3DXPOSITION3	m_ShakingAddPosition;	// 揺れによる追加の座標.
	float			m_ViewAngle;			// 視野角.
	float			m_Aspect;				// アスペクト比(幅÷高さ).
	float			m_MaxZView;				// 遠いビュー平面のz値.
	float			m_MinZView;				// 近いビュー平面のz値.
};
