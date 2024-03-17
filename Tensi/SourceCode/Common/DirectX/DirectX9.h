#pragma once
#include "..\..\Global.h"

/************************************************
*	DirectX9 �Z�b�g�A�b�v.
**/
class DirectX9 final
{
public:
	DirectX9();
	~DirectX9();

	// DirectX9�\�z.
	static HRESULT Create( HWND hWnd );
	// DirectX9���.
	static void Release();

	// �f�o�C�X�I�u�W�F�N�g���擾.
	static LPDIRECT3DDEVICE9 GetDevice() { return GetInstance()->m_pDevice9; }

private:
	// �C���X�^���X�̎擾.
	static DirectX9* GetInstance();

private:
	LPDIRECT3DDEVICE9	m_pDevice9;	// �f�o�C�X�I�u�W�F�N�g.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	DirectX9( const DirectX9 & ) = delete;
	DirectX9& operator = ( const DirectX9 & ) = delete;
	DirectX9( DirectX9 && ) = delete;
	DirectX9& operator = ( DirectX9 && ) = delete;
};