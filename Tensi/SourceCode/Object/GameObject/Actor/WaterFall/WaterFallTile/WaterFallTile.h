#pragma once
#include "..\..\Actor.h"

/************************************************
*	��̃^�C���N���X.
**/
class CWaterFallTile final
	: public CActor
{
public:
	CWaterFallTile();
	~CWaterFallTile();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) override;

	// �^�C���̐ݒ�.
	void Setting( const D3DXPOSITION3& Pos, const bool IsWater );

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() override;
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() override;

private:
	CSprite*			m_pWaterFall;			// ��̉摜.
	SSpriteRenderState	m_WaterFallState;		// ��̉摜�̏��.
	SSpriteRenderState	m_SplashState;			// �����Ԃ��̉摜�̏��.
	float				m_ImageSize;			// �摜�T�C�Y.

};