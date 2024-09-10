#pragma once
#include "..\..\..\Utility\Transform\Transform.h"

class CStaticMesh;

/************************************************
*	座標の表示クラス.
**/
class PositionRenderer final
{
public:
	PositionRenderer();
	~PositionRenderer();

	// 更新.
	static void Update( const float& DeltaTime );

	// 描画.
	static void Render( const STransform& Transform );

private:
	// インスタンスの取得.
	static PositionRenderer* GetInstance();

private:
	CStaticMesh*	m_pMesh;		// モデル.
	STransform		m_Transform;	// トランスフォーム.
#ifdef ENABLE_CLASS_BOOL
	CBool			m_IsRender;		// 表示するか.
#else
	bool			m_IsRender;		// 表示するか
#endif
};