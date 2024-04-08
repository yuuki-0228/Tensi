#pragma once
#include "..\GameObject.h"
#include "..\ActorCollisionManager\ActorCollisionManager.h"
#include "..\..\Collision\CollisionManager\CollisionManager.h"
#include "..\..\Collision\CollisionRenderer\CollisionRenderer.h"
#include "..\..\Collision\Collisions\CCollisions.h"
#include "..\..\..\Common\Sprite\SpriteStruct.h"

class CSkinMesh;
class CStaticMesh;
class CCollisionMesh;
class CSprite;

namespace {
	// �E�B���h�E����o�����Ƃɂ���T�C�Y.
	constexpr float WND_OUT_SIZE = 1000.0f;
}

/************************************************
*	�Q�[�����̃I�u�W�F�N�g�̃x�[�X�N���X.
**/
class CActor
	: public CGameObject
{
public:
	CActor();
	virtual ~CActor();

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) {};

	// �����蔻��̎擾.
	template<class T> T*	GetCollision() { return nullptr; }
	// �����蔻��̎擾(����).
	template<> CSphere*		GetCollision() { return m_pCollisions->GetCollision<CSphere>(); }
	// �����蔻��̎擾(�~��).
	template<> CCylinder*	GetCollision() { return m_pCollisions->GetCollision<CCylinder>(); }
	// �����蔻��̎擾(���C).
	template<> CRay*		GetCollision() { return m_pCollisions->GetCollision<CRay>(); }
	// �����蔻��̎擾(�\�����C).
	template<> CCrossRay*	GetCollision() { return m_pCollisions->GetCollision<CCrossRay>(); }
	// �����蔻��̎擾(���b�V��).
	template<> CMesh*		GetCollision() { return m_pCollisions->GetCollision<CMesh>(); }
	// �����蔻��̎擾(����2D).
	template<> CSphere2D*	GetCollision() { return m_pCollisions->GetCollision<CSphere2D>(); }
	// �����蔻��̎擾(�{�b�N�X2D).
	template<> CBox2D*		GetCollision() { return m_pCollisions->GetCollision<CBox2D>(); }

	// �X�N���[�����W���擾.
	D3DXPOSITION3 GetScreenPos();

	// ��ʊO�ɂ��邩.
	bool IsScreenOut( const float& Size = WND_OUT_SIZE );

	// �����ɂ��t�F�[�h�̍X�V.
	void DistanceFadeUpdate( const float& StartDistance, const float& AlphaDistance );

	// �\�����Ă��邩
	virtual bool GetIsDisp() { return m_IsDisp; }

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() {};
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() {};

protected:
	std::unique_ptr<CCollisions>	m_pCollisions;		// �����̓����蔻����������Ă���N���X.
	bool							m_IsDisp;			// �\�����邩.
	float							m_Distance;			// ����(1.0f ~ 0.0f).
	float							m_Wnd_w;			// �E�B���h�E�̕�.
	float							m_Wnd_h;			// �E�B���h�E�̍���.
	RECT							m_WndRect;			// �E�B���h�E�̏��.
};
