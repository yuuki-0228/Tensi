#pragma once
#include "..\Global.h"

/************************************************
*	�I�u�W�F�N�g�̊�ՃN���X.
**/
class CObject
{
public:
	CObject();
	virtual ~CObject();

	// �g�����X�t�H�[���̐ݒ�.
	inline void SetTransform( const STransform& t ) { m_Transform = t; }
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
	inline STransform	 GetTransform() {	return m_Transform; }
	// ���W�̎擾.
	inline D3DXPOSITION3 GetPosition() {	return m_Transform.Position; }
	// ��]�̎擾.
	inline D3DXROTATION3 GetRotation() {	return m_Transform.Rotation; }
	// �g�k�̎擾.
	inline D3DXSCALE3	 GetScale() {		return m_Transform.Scale; }

protected:
	// �g�����X�t�H�[���̍X�V.
	//	�摜�̃g�����X�t�H�[���ȂǂŊǗ����Ă���ꍇ.
	//	���̊֐���Update()�̍Ō�ɌĂ�.
	virtual void TransformUpdate( const STransform& t ) final { m_Transform = t; }

protected:
	STransform m_Transform;	// �g�����X�t�H�[��.
};