#pragma once
#include "..\Global.h"
#include "DirectX\DirectX11.h"

/************************************************
*	�f�o�C�X���g�p����n�̊�ՃN���X.
**/
class CCommon
{
public:
	CCommon();
	virtual ~CCommon();

	// �F�̐ݒ�.
	inline void SetColor( const D3DXCOLOR3& Color ) { m_Color = Color4::RGBA( Color, m_Color.w ); }
	inline void SetColor( const D3DXCOLOR4& Color ) { m_Color = Color; }
	// �A���t�@�l�̐ݒ�.
	void SetAlpha( const float Alpha );

	// �g�����X�t�H�[���̐ݒ�.
	inline void SetTransform(	const STransform& t ) { m_Transform = t; }
	// ���W�ݒ�.
	inline void SetPosition( const D3DXPOSITION3& p							) { m_Transform.Position = p; }
	inline void SetPosition( const float x, const float y, const float z	) { m_Transform.Position = { x, y, z }; }
	// ��]�ݒ�.
	inline void SetRotation( const D3DXROTATION3& r							) { m_Transform.Rotation = r; }
	inline void SetRotation( const float x, const float y, const float z	) { m_Transform.Rotation = { x, y, z }; }
	// �g�k�ݒ�.
	inline void SetScale(	 const D3DXSCALE3& s							) { m_Transform .Scale = s; }
	inline void SetScale(	 const float x, const float y, const float z	) { m_Transform .Scale = { x, y, z }; }


	// �g�����X�t�H�[���̎擾.
	inline STransform		GetTransform() {	return m_Transform; }
	// ���W�̎擾.
	inline D3DXPOSITION3	GetPosition() {		return m_Transform.Position; }
	// ��]�̎擾.
	inline D3DXROTATION3	GetRotation() {		return m_Transform.Rotation; }
	// �g�k�̎擾.
	inline D3DXSCALE3		GetScale() {		return m_Transform.Scale; }

protected:
	ID3D11Device*			m_pDevice;		// �f�o�C�X�I�u�W�F�N�g.
	ID3D11DeviceContext*	m_pContext;		// �f�o�C�X�R���e�L�X�g.

	STransform				m_Transform;	// �g�����X�t�H�[��.
	D3DXCOLOR4				m_Color;		// �F.
};
