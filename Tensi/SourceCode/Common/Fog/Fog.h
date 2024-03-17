#pragma once
#include "..\Common.h"
#include "..\..\Global.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>

/************************************************
*	�t�H�O�N���X.
**/
class CFog final
	: public CCommon
{
private:
	struct ConstantBufferType
	{
		D3DXMATRIX	world;
		D3DXMATRIX	view;
		D3DXMATRIX	projection;
	};
	struct FogBufferType
	{
		float fogStart;
		float fogEnd;
		float e; 
		float density;
	};

public:
	CFog();
	~CFog();

	// ������.
	bool Init();
	// �`��.
	bool Render( int indexCount, D3DXMATRIX worldMatrix,
		ID3D11ShaderResourceView* texture, float fogStart, float fogEnd );

private:
	// �V�F�[�_�̏�����.
	bool InitializeShader();
	// ������.
	void Release();
	// �G���[���b�Z�[�W���o��.
	void OutputShaderErrorMessage( ID3D10Blob* errorMessage, WCHAR* shaderFilename );

	// �V�F�[�_�[�p�����[�^��ݒ�.
	bool SetShaderParameters( D3DXMATRIX worldMatrix, ID3D11ShaderResourceView* texture, float fogStart, float fogEnd );
	// �V�F�[�_�̕`��.
	void RenderShader( int indexCount );

private:
	ID3D11VertexShader*		m_vertexShader;
	ID3D11PixelShader*		m_pixelShader;

	ID3D11InputLayout*		m_layout;

	ID3D11Buffer*			m_constantBuffer;
	ID3D11Buffer*			m_fogBuffer;

	ID3D11SamplerState*		m_sampleState;

	float					m_Density;
};
