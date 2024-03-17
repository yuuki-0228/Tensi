#pragma once
#include "FontStruct.h"

/********************************************
*	�t�H���g�N���X.
*		���c�F�P.
**/
class CFont final
{
public:
	using Sampler = unsigned long long;

public:
	CFont();
	virtual ~CFont();

	// ������.
	HRESULT Init( const std::string& FilePath, const std::string& FileName );

	// UI�ŕ`��.
	void RenderUI( SFontRenderState* pRenderState = nullptr );
	void RenderUI( const std::string& Text, SFontRenderState* pRenderState = nullptr );
	// 3D�ŕ`��.
	void Render3D( SFontRenderState* pRenderState = nullptr, const bool& IsBillBoard = false );
	void Render3D( const std::string& Text, SFontRenderState* pRenderState = nullptr, const bool& IsBillBoard = false );

	// �`����W�̎擾.
	inline D3DXVECTOR3 GetRenderPos() const { return m_FontRenderState.Transform.Position; }
	// �t�H���g���̎擾.
	inline SFontState GetFontState() const { return m_FontState; }
	// �t�H���g�̕`������擾.
	inline SFontRenderState GetRenderState() const { return m_FontRenderState; }

	// �`�悷��͈͂̐ݒ�.
	inline void SetRenderArea( const D3DXVECTOR4& Area ) { m_FontRenderState.RenderArea = Area; }
	inline void SetRenderArea( const float x, const float y, const float w, const float h ) { m_FontRenderState.RenderArea = { x, y, w, h }; }

	// �f�B�U�������g�p���邩.
	inline void SetDither( const bool Flag ) { m_DitherFlag = Flag; }
	// �A���t�@�u���b�N���g�p���邩.
	inline void SetAlphaBlock( const bool Flag ) { m_AlphaBlockFlag = Flag; }

private:
	// �t�H���g���̓ǂݍ���.
	HRESULT FontStateDataLoad( const std::string& FilePath );

	// �V�F�[�_�쐬.
	HRESULT CreateShader();
	// ���f���쐬(UI).
	HRESULT CreateModelUI();
	// ���f���쐬(3D).
	HRESULT CreateModel3D();
	// �T���v���쐬.
	HRESULT CreateSampler();
	// ���_���̍쐬.
	HRESULT CreateVertex( const float w, const float h, const float u, const float v );

	// �e�N�X�`���̔䗦���擾.
	int myGcd( int t, int t2 ) { if ( t2 == 0 ) return t; return myGcd( t2, t % t2 ); }

	// �e�N�X�`���̕`��.
	void RenderFontUI( const char* c, SFontRenderState* pRenderState );
	// �e�N�X�`���̕`��.
	void RenderFont3D( const char* c, SFontRenderState* pRenderState, const bool& IsBillBoard );

private:
	ID3D11Device*					m_pDevice;				// �f�o�C�X�I�u�W�F�N�g.
	ID3D11DeviceContext*			m_pContext;				// �f�o�C�X�R���e�L�X�g.

	ID3D11VertexShader*				m_pVertexShader;		// ���_�V�F�[�_.
	ID3D11InputLayout*				m_pVertexLayout;		// ���_���C�A�E�g.
	ID3D11PixelShader*				m_pPixelShader;			// �s�N�Z���V�F�[�_.

	ID3D11Buffer*					m_pVertexBufferUI;		// ���_�o�b�t�@(UI�p).
	ID3D11Buffer*					m_pVertexBuffer3D;		// ���_�o�b�t�@(3D�p).
	ID3D11Buffer*					m_pConstantBuffer;		// �R���X�^���g�o�b�t�@.

	SVertex							m_Vertices[4];			// ���_�쐬�p.
	SFontState						m_FontState;			// �t�H���g���.
	SFontRenderState				m_FontRenderState;		// �t�H���g�A�j���[�V�������.

	ID3D11SamplerState* m_pSampleLinears[static_cast<Sampler>(ESamplerState::Max)]; // �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	std::string						m_FileName;				// �t�H���g�̃t�@�C����.

	bool							m_DitherFlag;			// �f�B�U�������g�p���邩.
	bool							m_AlphaBlockFlag;		// �A���t�@�u���b�N���g�p���邩.
};