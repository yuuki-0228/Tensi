#pragma once
#include "..\..\ColliderBase.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"

/**************************************
*	2D�̎l�p�N���X.
*		���c�F�P.
**/
class CBox2D final
	: public CColliderBase
{
public:
	CBox2D();
	virtual ~CBox2D();

	// ����̍��W���擾.
	D3DXPOSITION2 GetPosition();
	// ���������擾.
	SSize GetSize();

	// �X�v���C�g��ݒ�.
	void SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState );

	// ����̍��W��ݒ�.
	inline void SetPosition( const D3DXPOSITION2& Position ) {
		m_LUPosition = Position;
	}
	// ���A������ݒ�.
	inline void SetSize( const SSize& Size ) {
		m_Size = Size;
	}

private:
	// ����̍��W���擾����.
	D3DXPOSITION2 GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState );

private:
	CSprite*			m_pSprite;		// �X�v���C�g.
	SSpriteRenderState* m_pRenderState;	// �X�v���C�g�̏��.
	D3DXPOSITION2		m_LUPosition;	// ������W.
	SSize				m_Size;			// ���A����.
};