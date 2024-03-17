#pragma once
#include "..\Actor.h"

/************************************************
*	�ƃN���X.
**/
class CHouse final
	: public CActor
{
public:
	CHouse();
	~CHouse();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �ݒu����.
	void Setting( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pTent;		// �e���g�̉摜.
	SSpriteRenderState	m_TentState;	// �e���g�̉摜�̏��.

};