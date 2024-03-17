#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Common\Sprite\Sprite.h"
#include <queue>

class CSphere;
class CCylinder;
class CRay;
class CCrossRay;
class CRayMesh;
class CStaticMesh;
class CCollisionMesh;
class CBox2D;
class CSphere2D;

/************************************************
*	�����蔻��`��N���X.
*		���c�F�P.
**/
class CollisionRenderer final
{
public:
	using Mesh_queue = std::queue<std::pair<std::pair<CStaticMesh*, CCollisionMesh*>, STransform>>;

public:
	CollisionRenderer();
	~CollisionRenderer();

	// �`��.
	static void Render();

	// �_�̒ǉ�.
	static void PushPoint( const D3DXPOSITION3& pPos );
	// ���̂̒ǉ�.
	static void PushSphere( CSphere* pSphere );
	// �~���̒ǉ�.
	static void PushCylinder( CCylinder* pCylinder );
	// ���C�̒ǉ�.
	static void PushRay( CRay* pRay );
	// �\�����C�̒ǉ�.
	static void PushRay( CCrossRay* pRay );
	// ���b�V���̒ǉ�.
	static void PushMesh( CStaticMesh*		pMesh, STransform* pTrans = nullptr );
	static void PushMesh( CCollisionMesh*	pMesh, STransform* pTrans = nullptr );
	// �_2D�̒ǉ�.
	static void PushPoint2D( const D3DXPOSITION3& pPos );
	static void PushPoint2D( const D3DXPOSITION2& pPos );
	// �{�b�N�X2D�̒ǉ�.
	static void PushBox2D( CBox2D* pBox2D );
	// �~2D�̒ǉ�.
	static void PushSphere2D( CSphere2D* pSphere2D );

private:
	// �C���X�^���X�̎擾.
	static CollisionRenderer* GetInstance();

	// �_�̕`��.
	static void PointRender();
	// ���̂̕`��.
	static void SphereRender();
	// �~���̕`��.
	static void CylinderRender();
	// ���C�̕`��.
	static void RayRender();
	// ���b�V���̕`��.
	static void MeshRender();
	// �_2D�̕`��.
	static void Point2DRender();
	// �{�b�N�X2D�̕`��.
	static void Box2DRender();
	// �~2D�̕`��.
	static void Sphere2DRender();

private:
	std::queue<D3DXPOSITION3>	m_pPointQueue;		// �_�̃L���[.
	std::queue<CSphere*>		m_pSphereQueue;		// ���̂̃L���[.
	std::queue<CCylinder*>		m_pCylinderQueue;	// �~���̃L���[.
	std::queue<CRay*>			m_pRayQueue;		// ���C�̃L���[.
	Mesh_queue					m_pMeshQueue;		// ���b�V���̃L���[.
	std::queue<D3DXPOSITION2>	m_pPoint2DQueue;	// �_2D�̃L���[.
	std::queue<CBox2D*>			m_pBox2DQueue;		// �{�b�N�X2D�̃L���[.
	std::queue<CSphere2D*>		m_pSphere2DQueue;	// �~2D�̃L���[.
	CCollisionMesh*				m_pSphereMesh;		// ���̃��b�V��.
	CCollisionMesh*				m_pCylinderMesh;	// �~�����b�V��.
	std::unique_ptr<CRayMesh>	m_pRayMesh;			// ���C���b�V��.
	CSprite*					m_pBox2DSprite;		// �{�b�N�X2D�X�v���C�g.
	CSprite*					m_pSphere2DSprite;	// �~2D�X�v���C�g.
	CSprite*					m_pPoint2DSprite;	// �_2D�X�v���C�g.
	SSpriteRenderState			m_Box2DState;		// �{�b�N�X2D�̏��.
	SSpriteRenderState			m_Sphere2DState;	// �~2D�̏��.
	SSpriteRenderState			m_Point2DState;		// �_2D�̏��.

	CBool						m_IsPointRender;	// �_��`�悷�邩.
	CBool						m_IsSphereRender;	// ���̂�`�悷�邩.
	CBool						m_IsCylinderRender;	// �~����`�悷�邩.
	CBool						m_IsRayRender;		// ���C��`�悷�邩.
	CBool						m_IsMeshRender;		// ���b�V����`�悷�邩.
	CBool						m_IsPoint2DRender;	// �_2D��`�悷�邩.
	CBool						m_IsBox2DRender;	// �{�b�N�X2D��`�悷�邩.
	CBool						m_IsSphere2DRender;	// �~2D��`�悷�邩.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	CollisionRenderer( const CollisionRenderer & )				= delete;
	CollisionRenderer& operator = ( const CollisionRenderer & )	= delete;
	CollisionRenderer( CollisionRenderer && )						= delete;
	CollisionRenderer& operator = ( CollisionRenderer && )		= delete;
};
