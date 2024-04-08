#pragma once
#include "..\Actor.h"
#include "..\..\..\..\Common\Font\Font.h"

/************************************************
*	�E�B���h�E�N���X.
**/
class CWindow final
	: public CActor
{
public:
	CWindow();
	~CWindow();

	// ������
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override {};
	virtual void SubRender() override;

	// �쐬
	void Create( const D3DXPOSITION3& pos, const D3DXSCALE2& size, const std::string& title = "", const D3DXCOLOR4& wndColor = Color4::Royalblue, const bool dispCancel = true);
	// �`��֐��̐ݒ�
	void SetRender( const std::function<void()>& f ) { m_RenderFunction = f; }

private:
	CSprite*			m_pSprite;			// �摜.
	CSprite*			m_pCancelSprite;	// ����摜.
	CFont*				m_pFont;			// �t�H���g
	SSpriteRenderState	m_SpriteState;		// �摜�̏��.
	SSpriteRenderState	m_CancelState;		// ����摜�̏��.
	SFontRenderState	m_FontState;		// �t�H���g�̏��
	stSpriteState		m_SpriteData;		// �摜�Ɋւ�����
	stSpriteState		m_CancelData;		// ����摜�Ɋւ�����
	D3DXPOSITION3		m_LeftUpPos;		// ������W
	D3DXSCALE2			m_Size;				// �E�B���h�E�̃T�C�Y
	D3DXVECTOR4			m_DispAreaPos;		// �E�B���h�E�̕`��G���A�̍��W(x,y:����, z,w:�E��)
	D3DXVECTOR2			m_DispAreaSize;		// �E�B���h�E�̕`��G���A�T�C�Y(x:��, y:����)
	bool				m_IsDispCancel;		// ����{�^����\������
	std::function<void()> m_RenderFunction;	// �`��
};
