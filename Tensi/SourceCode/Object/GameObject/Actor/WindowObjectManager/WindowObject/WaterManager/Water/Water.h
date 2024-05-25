#pragma once
#include "..\..\WindowObject.h"

/************************************************
*	���N���X.
**/
class CWater final
	: public CWindowObject
{
public:
	CWater();
	~CWater();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;
	// �����蔻��I����Ăяo�����X�V.
	virtual void LateUpdate( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �����蔻��֐�.
	virtual void Collision( CActor* pActor ) override;

	// ���̐ݒ�.
	void SetWater( const D3DXPOSITION3& Pos, const D3DXVECTOR3& Vec3, const HWND hWnd );

protected:
	// �����蔻��̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitCollision() override;
	// �����蔻��̍X�V.
	//	Update�֐��̍Ō�ɌĂ�.
	virtual void UpdateCollision() override;

	// �͂߂Ȃ����邽��false���m��œn��.
	virtual bool GrabCheck() override { return false; }

	// ��ɓ����������̍X�V.
	virtual void HitUpUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitDownUpdate() override;
	// ���ɓ����������̍X�V.
	virtual void HitLeftUpdate() override;
	// �E�ɓ����������̍X�V.
	virtual void HitRightUpdate() override;

private:

};
