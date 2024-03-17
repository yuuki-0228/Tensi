#pragma once
#include "..\..\ColliderBase.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"

/**************************************
*	2D�̉~�N���X.
*		���c�F�P.
**/
class CSphere2D final
	: public CColliderBase
{
public:
	CSphere2D();
	virtual ~CSphere2D();

	// ����̍��W���擾.
	D3DXPOSITION2 GetPosition();
	// ���a���擾.
	float GetRadius();

	// �X�v���C�g��ݒ�.
	void SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState );

	// ���S�̍��W��ݒ�.
	inline void SetPosition( const D3DXPOSITION2& Position ) {
		m_CPosition = Position;
	}
	// ���a��ݒ�.
	inline void SetRadius( const float Radius ) {
		m_Radius = Radius;
	}

private:
	// ���S�̍��W���擾����.
	D3DXPOSITION2 GetSpriteCenterPos( SSpriteRenderState* pRenderState );

private:
	CSprite*			m_pSprite;		// �X�v���C�g.
	SSpriteRenderState* m_pRenderState;	// �X�v���C�g�̏��.
	D3DXPOSITION2		m_CPosition;	// ���S���W.
	float 				m_Radius;		// ���a.
};