#pragma once
#include "..\..\..\Utility\Transform\Transform.h"

class CStaticMesh;

/************************************************
*	���W�̕\���N���X.
**/
class PositionRenderer final
{
public:
	PositionRenderer();
	~PositionRenderer();

	// �X�V.
	static void Update( const float& DeltaTime );

	// �`��.
	static void Render( const STransform& Transform );

private:
	// �C���X�^���X�̎擾.
	static PositionRenderer* GetInstance();

private:
	CStaticMesh*	m_pMesh;		// ���f��.
	STransform		m_Transform;	// �g�����X�t�H�[��.
#ifdef ENABLE_CLASS_BOOL
	CBool			m_IsRender;		// �\�����邩.
#else
	bool			m_IsRender;		// �\�����邩
#endif
};