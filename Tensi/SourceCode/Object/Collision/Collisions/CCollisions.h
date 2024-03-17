#pragma once
#include "..\..\..\Global.h"
#include "..\Sphere\Sphere.h"
#include "..\Cylinder\Cylinder.h"
#include "..\Ray\Ray.h"
#include "..\CrossRay\CrossRay.h"
#include "..\2D\Box2D\Box2D.h"
#include "..\2D\Sphere2D\Sphere2D.h"
#include "..\Mesh\Mesh.h"
#include <memory>

// �����蔻��̃^�C�v.
enum class enCollType : unsigned char
{
	None,

	Sphere,		// ����.
	Cylinder,	// �~��.
	Ray,		// ���C.
	CrossRay,	// �\�����C.
	Mesh,		// ���b�V��.

	Box2D,		// 2D�̃{�b�N�X.
	Sphere2D,	// 2D�̉~.

	Max
} typedef ECollType;

/************************************************
*	�����̓����蔻����������Ă���N���X.
*		���c�F�P.
**/
class CCollisions final
{
public:
	CCollisions();
	~CCollisions();

	// �w�肵�������蔻��̏�����.
	void InitCollision( const ECollType& Type );

	// �����蔻��̎擾.
	template<class T> T*	GetCollision() { return nullptr; }
	// �����蔻��̎擾(����).
	template<> CSphere*		GetCollision() { return m_pSphere.get(); }
	// �����蔻��̎擾(�~��).
	template<> CCylinder*	GetCollision() { return m_pCylinder.get(); }
	// �����蔻��̎擾(���C).
	template<> CRay*		GetCollision() { return m_pRay.get(); }
	// �����蔻��̎擾(�\�����C).
	template<> CCrossRay*	GetCollision() { return m_pCrossRay.get(); }
	// �����蔻��̎擾(���b�V��).
	template<> CMesh*		GetCollision() { return m_pMesh.get(); }
	// �����蔻��̎擾(2D�̃{�b�N�X).
	template<> CBox2D*		GetCollision() { return m_pBox2D.get(); }
	// �����蔻��̎擾(2D�̉~).
	template<> CSphere2D*	GetCollision() { return m_pSphere2D.get(); }

private:
	std::shared_ptr<CSphere>	m_pSphere;		// ����.
	std::shared_ptr<CCylinder>	m_pCylinder;	// �~��.
	std::shared_ptr<CRay>		m_pRay;			// ���C.
	std::shared_ptr<CCrossRay>	m_pCrossRay;	// �\�����C.
	std::shared_ptr<CMesh>		m_pMesh;		// ���b�V��.
	std::shared_ptr<CBox2D>		m_pBox2D;		// 2D�̃{�b�N�X.
	std::shared_ptr<CSphere2D>	m_pSphere2D;	// 2D�̉~.
};
