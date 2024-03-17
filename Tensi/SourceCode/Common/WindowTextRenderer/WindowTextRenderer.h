#pragma once
#include "..\Common.h"
#include "..\..\Global.h"

/************************************************
*	�E�B���h�E�ɕ\������e�L�X�g�N���X.
*		���c�F�P.
**/
class WindowTextRenderer final
	: public CCommon
{
private:
	static constexpr float RECT_W			= 30.0f;
	static constexpr float RECT_H			= 36.0f;
	static constexpr float TEX_DIMENSION	= 384.0f;

public:
	// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
	//	���V�F�[�_�[���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16 D3DXMATRIX	mWVP;		// ���[���h,�r���[,�v���W�F�N�V�����̍����ϊ��s��.	
		ALIGN16 D3DXCOLOR4	vColor;		// �J���[(RGBA�̌^�ɍ��킹��).
		ALIGN16 float fViewPortWidth;	// �r���[�|�[�g��.
		ALIGN16 float fViewPortHeight;	// �r���[�|�[�g����.
	};
	// ���_�̍\����.
	struct VERTEX
	{
		D3DXVECTOR3 Pos;	// ���_���W.
		D3DXVECTOR2	Tex;	// �e�N�X�`�����W.
	};

public:
	WindowTextRenderer();
	~WindowTextRenderer();

	// ������.
	static HRESULT Init();

	// �`��.
	static void Render(		 const std::string& text, const D3DXPOSITION3& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void Render(		 const std::string& text, const D3DXPOSITION2& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void Render(		 const std::string& text, const int x, const int y,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const D3DXPOSITION3& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const D3DXPOSITION2& Pos,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	static void DebugRender( const std::string& text, const int x, const int y,	const float FontSize = 1.0f, const D3DXCOLOR4& Color = Color4::White );
	
	// �����̉摜�T�C�Y�̎擾.
	static D3DXVECTOR2 GetTextSize() { return { RECT_W * GetInstance()->m_FontSize, RECT_H * GetInstance()->m_FontSize }; }

private:
	// �C���X�^���X�̎擾.
	static WindowTextRenderer* GetInstance();

	// �V�F�[�_�[�쐬.
	static HRESULT CreateShader();
	// ���f���쐬.
	static HRESULT CreateModel();
	// �e�N�X�`���쐬.
	static HRESULT CreateTexture( LPCTSTR lpFileName );
	// �T���v���쐬.
	static HRESULT CreateSampler();
	// �t�H���g�����_�����O.
	static void RenderFont( int FontIndex, int x, int y );

private:
	ID3D11VertexShader*			m_pVertexShader;		// ���_�V�F�[�_�[.
	ID3D11InputLayout*			m_pVertexLayout;		// ���_���C�A�E�g.
	ID3D11PixelShader*			m_pPixelShader;			// �s�N�Z���V�F�[�_�[.
	ID3D11Buffer*				m_pConstantBuffer;		// �R���X�^���g�o�b�t�@.

	ID3D11Buffer*				m_pVertexBuffer[100];	// ���_�o�b�t�@(100��).

	ID3D11ShaderResourceView*	m_pTexture;				// �e�N�X�`��.
	ID3D11SamplerState*			m_pSampleLinear;		// �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	float						m_Kerning[100];			// �J�[�j���O(100��
	float						m_FontSize;				// �t�H���g�T�C�Y.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	WindowTextRenderer( const WindowTextRenderer & )				= delete;
	WindowTextRenderer& operator = ( const WindowTextRenderer & ) = delete;
	WindowTextRenderer( WindowTextRenderer && )					= delete;
	WindowTextRenderer& operator = ( WindowTextRenderer && )		= delete;
};