#pragma once
#include "..\Actor.h"

/************************************************
*	�X���C���t���[��.
**/
class CSlimeFrame final
	: public CActor
{
public:
	CSlimeFrame();
	~CSlimeFrame();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;
	virtual void SubRender() override;

private:
	// �E�B���h�E�̑I��.
	void SelectWindow();

	// �E�B���h�E�����ɖ߂�.
	void Reset();

private:
	CSprite*			m_pBack;		// �w�i.
	CSprite*			m_pIcon;		// �A�C�R��.
	SSpriteRenderState	m_BackState;	// �w�i�̏��.
	SSpriteRenderState	m_IconState;	// �A�C�R���̏��.
	HWND				m_hWnd;			// �t���[����t����E�B���h�E.
};