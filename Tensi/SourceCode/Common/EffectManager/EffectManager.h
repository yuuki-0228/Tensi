#pragma once
#include "..\..\Global.h"
#include "Effect\Effect.h"

/************************************************
*	�G�t�F�N�g�}�l�[�W���[.
**/
class CEffectManager
{
public:
	CEffectManager();
	~CEffectManager();

	// �G�t�F�N�g�̐ݒ�.
	bool SetEffect( const std::string& Name );

	// �`��.
	void Render();

	// �Đ�.
	bool Play( const STransform& Trans );
	bool Play( const D3DXPOSITION3& Pos, const D3DXROTATION3& Rot = { 0.0f, 0.0f, 0.0f }, const D3DXSCALE3& Scale = { 1.0f, 1.0f, 1.0f } );

	// ��~.
	void Stop();
	// ���X�ɒ�~������.
	void StopRoot();
	// �S�Ē�~.
	void StopAll();

	// �ꎞ��~�������͍ĊJ����.
	void Pause( const bool& Flag );
	// ���ׂĂ��ꎞ��~�������͍ĊJ����.
	void AllPause( const bool& Flag );

	// �G�t�F�N�g�Ɏg�p����Ă���C���X�^���X�����擾.
	int GetTotalInstanceCount();

	// �g�����X�t�H�[���̎擾.
	inline STransform  GetTransform() { return m_Transform; }
	// ���W�̎擾.
	inline D3DXPOSITION3 GetPosition() { return m_Transform.Position; }
	// ��]�̎擾.
	inline D3DXROTATION3 GetRotation() { return m_Transform.Rotation; }
	// �g�k�̎擾.
	inline D3DXSCALE3 GetScale() { return m_Transform.Scale; }

	// ���݂��Ă��邩.
	bool IsShow();

	// �ʒu���w�肷��.
	void SetPosition( const D3DXPOSITION3& Pos );
	// ��]���w�肷��.
	void SetRotation( const D3DXROTATION3& Rot );
	// �T�C�Y���w�肷��.
	void SetScale( const D3DXSCALE3& Scale );
	void SetScale( const float& Scale );
	// �Đ����x��ݒ肷��.
	void SetSpeed( const float& Speed );
	// �F�̐ݒ�.
	void SetColor( const D3DXCOLOR3& Color );
	void SetColor( const D3DXCOLOR4& Color );
	// �A���t�@�l�̐ݒ�.
	void SetAlpha( const float& Alpha );

private:
	CEffect*			m_pEffect;		// �G�t�F�N�g�N���X.
	Effekseer::Handle	m_Handle;		// �G�t�F�N�g�n���h��.
	STransform			m_Transform;	// �g�����X�t�H�[��.
};
