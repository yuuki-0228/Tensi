#pragma once
#include "..\..\Global.h"
#include "..\Math\Math.h"
#include <algorithm>

using D3DXPOSITION2 = D3DXVECTOR2;
using D3DXPOSITION3 = D3DXVECTOR3;
using D3DXPOSITION4 = D3DXVECTOR4;
using D3DXROTATION2 = D3DXVECTOR2;
using D3DXROTATION3 = D3DXVECTOR3;
using D3DXROTATION4 = D3DXVECTOR4;
using D3DXSCALE2	= D3DXVECTOR2;
using D3DXSCALE3	= D3DXVECTOR3;
using D3DXSCALE4	= D3DXVECTOR4;

/************************************************
*	�g�����X�t�H�[���\����.
*		���c�F�P.
**/
struct stTransform
{
	D3DXPOSITION3	Position;			// ���[�J�����W( �e�����Ȃ��ꍇ���[���h���W ).
	D3DXROTATION3	Rotation;			// ���[�J����]( �e�����Ȃ��ꍇ���[���h��] ).
	D3DXSCALE3		Scale;				// ���[�J���g�k( �e�����Ȃ��ꍇ���[���h�g�k ).
	D3DXPOSITION3	OldPosition;		// �O�̃t���[���̃��[�J�����W( �e�����Ȃ��ꍇ���[���h���W ).
	D3DXROTATION3	OldRotation;		// �O�̃t���[���̃��[�J����]( �e�����Ȃ��ꍇ���[���h��] ).
	D3DXSCALE3		OldScale;			// �O�̃t���[���̃��[�J���g�k( �e�����Ȃ��ꍇ���[���h�g�k ).
	stTransform*	pParent;			// �e(�uAttachParent() / AddParent()�v���g�p���Đݒ肷�� ).
	bool			IsPostionRender;	// ���W��`�悷�邩.

	stTransform();
	stTransform(
		const D3DXPOSITION3&	NewPos,
		const D3DXROTATION3&	NewRot,
		const D3DXSCALE3&		NewScale,
		stTransform*			pNewParent = nullptr );

	// ���[���h���W��ݒ�.
	void SetWorldPosition( const D3DXPOSITION3& WorldPos );
	// ���[���h��]��ݒ�.
	void SetWorldRotation( const D3DXROTATION3& WorldRot );
	// ���[���h�g�k��ݒ�.
	void SetWorldScale( const D3DXSCALE3& WorldScale );

	// �x���ŉ�]���擾.
	D3DXROTATION3 GetDegRotation();
	// ���W�A���ŉ�]���擾����.
	D3DXROTATION3 GetRadRotation();
	// ��]�����̃x�N�g�����擾����.
	D3DXVECTOR3 GetRotationVector();

	// ����̃g�����X�t�H�[�����烍�[�J���g�����X�t�H�[�����擾.
	stTransform GetLocalTransform( const stTransform& Parent ) const;
	// ����̍��W���烍�[�J�����W���擾.
	D3DXPOSITION3 GetLocalPosition( const D3DXPOSITION3& ParPos ) const;
	// ����̉�]�l���烍�[�J����]���擾.
	D3DXROTATION3 GetLocalRotation( const D3DXROTATION3& ParRot ) const;
	// ����̊g�k���烍�[�J���g�k���擾.
	D3DXSCALE3 GetLocalScale( const D3DXSCALE3& ParScale ) const;

	// ���[���h��Ԃ̃g�����X�t�H�[�����擾.
	stTransform GetWorldTransform( const D3DXMATRIX* pRot = nullptr ) const;
	// ���[���h��Ԃ̍��W���擾.
	D3DXPOSITION3 GetWorldPosition( const D3DXMATRIX* pRot = nullptr ) const;
	// ���[���h��Ԃ̉�]���擾.
	D3DXROTATION3 GetWorldRotation() const;
	// ���[���h��Ԃ̊g�k���擾.
	D3DXSCALE3 GetWorldScale() const;
	
	// ���[���h�s����擾.
	D3DXMATRIX GetWorldMatrix( const D3DXMATRIX* pRot = nullptr );

	// �e��ݒ肷��.
	void AttachParent( stTransform* NewParent );
	// �e��ǉ�����.
	void AddParent( stTransform* NewParent );
	// �e�I�u�W�F�N�g����؂藣��.
	void DetachParent();

	// ���W��͈͓��Ɏ��߂�.
	void PostionClamp( const float Max );
	void PostionClamp( const float Max,		const float Min );
	void PostionClamp( const float Max_x,	const float Max_y, const float Max_z );
	void PostionClamp( const float Max_x,	const float Min_x, const float Max_y, const float Min_y, const float Max_z, const float Min_z );
	// ��]��͈͓��Ɏ��߂�.
	void RotationClamp();

	// �I�y���[�^�[.
	bool operator==( const stTransform& Trans );
	bool operator!=( const stTransform& Trans );
	void operator+=( const stTransform& Trans );
	void operator-=( const stTransform& Trans );
	void operator*=( const float& f );
	void operator/=( const float& f );
	stTransform operator+( const stTransform& Trans ) const;
	stTransform operator-( const stTransform& Trans ) const;
	stTransform operator*( const float& f ) const;
	stTransform operator/( const float& f ) const;

	// �萔.
	static constexpr float SCALE_MIN			= 0.0f;											// �g�k�̍ŏ��l.
	static constexpr float SCALE_MIN_VEC3[3]	= { SCALE_MIN, SCALE_MIN, SCALE_MIN };			// �g�k�̍ŏ��l.
	static constexpr float NORMAL_SCALE			= 1.0f;											// �ʏ�̊g�k�l.
	static constexpr float NORMAL_SCALE_VEC3[3]	= { NORMAL_SCALE, NORMAL_SCALE, NORMAL_SCALE };	// �ʏ�̊g�k�l.
} typedef STransform;