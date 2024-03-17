#pragma once
#include "..\..\Common.h"
#include "..\..\..\Global.h"

class CRay;

/************************************************
*	���C���b�V���N���X.
**/
class CRayMesh final
	: public CCommon
{
public:
	// �R���X�^���g�o�b�t�@�̃A�v�����̒�`.
	//	���V�F�[�_���̃R���X�^���g�o�b�t�@�ƈ�v���Ă���K�v����.
	struct SHADER_CONSTANT_BUFFER {
		D3DXMATRIX	mWVP;		// ���[���h,�r���[,�v���W�F�N�V�����̍����ϊ��s��.	
		D3DXCOLOR4	vColor;		// �J���[.
	};
	// ���_�̍\����.
	struct VERTEX {
		D3DXPOSITION3 Pos;		// ���_���W.
	};

public:
	CRayMesh();
	~CRayMesh();

	// ������.
	HRESULT Init();

	// �����_�����O�p.
	void Render( CRay* Ray );

private:
	// �V�F�[�_�쐬.
	HRESULT CreateShader();
	// ���f���쐬.
	HRESULT CreateModel( CRay* pRay );

private:
	ID3D11VertexShader*		m_pVertexShader;	// ���_�V�F�[�_.
	ID3D11InputLayout*		m_pVertexLayout;	// ���_���C�A�E�g.
	ID3D11PixelShader*		m_pPixelShader;		// �s�N�Z���V�F�[�_.
	ID3D11Buffer*			m_pConstantBuffer;	// �R���X�^���g�o�b�t�@.

	ID3D11Buffer*			m_pVertexBuffer;	// ���_�o�b�t�@.
};